/*
    This file is part of mythos.

    Copyright (c) 2007 Benaka Moorthi

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#define MYTHOS_KHAOS_SOURCE

#include <mythos/khaos/window.hpp>
#include <mythos/khaos/event_loop.hpp>
#include <mythos/khaos/image.hpp>
#include <mythos/khaos/font.hpp>
#include <mythos/khaos/main.hpp>
#include <mythos/khaos/font.hpp>

#include <mythos/khaos/event/paint.hpp>
#include <mythos/khaos/event/button.hpp>
#include <mythos/khaos/event/resize.hpp>
#include <mythos/khaos/event/reconfigure.hpp>

#include <mythos/khaos/register_module.hpp>

#include <boost/intrusive/list.hpp>

#include <boost/spirit.hpp>

#include <boost/system/system_error.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/none.hpp>

#include <map>

#include <windows.h>

static char const mythos_window_class[] = "MYTHOS_WINDOW_CLASS";
static ATOM mythos_window_class_atom;

static HINSTANCE app_instance;
static HWND message_window;
static int cmd_show;

static mythos::khaos::font default_font;

namespace mythos { namespace khaos
{
    struct msw_window;

    namespace detail
    {
        static void set_buffer(msw_window * win);
    }

    struct msw_window : window
    {
        msw_window(HWND hwnd, window * p, bool top, int x_, int y_, int w, int h)
            : window(p, top), handle(hwnd), x(x_), y(y_), width(w), height(h), tlx(0), tly(0), wrapped_proc(NULL)
        {
            detail::set_buffer(this);
        }

        HWND handle;

        image_view buffer;

        // x & y are window positions
        // width & height are both window size & the size of the buffer
        int x, y, width, height;

        // tlx & tly are indices into buff_impl
        int tlx, tly;

        boost::shared_ptr<image> buff_impl;

        // for wrapped windows
        WNDPROC wrapped_proc;
    };

    struct msw_event_info
    {
        msw_window * mswwin;
        UINT msg;
        WPARAM wparam;
        LPARAM lparam;
        void * extra;
    };

    namespace detail
    {
        // buffer related functions ONLY manipulate the buffer
        static void set_buffer_view(msw_window * mswwin)
        {
            mswwin->buffer = image_view(
                mswwin->width, mswwin->height, boost::gil::view(*mswwin->buff_impl).xy_at(mswwin->tlx, mswwin->tly)
            );
        }

        static void set_buffer_child(msw_window * mswwin, int bx, int by)
        {
            BOOST_ASSERT(!mswwin->is_toplevel && mswwin->parent);

            mswwin->buff_impl = static_cast<msw_window *>(mswwin->parent)->buff_impl;

            mswwin->tlx = bx + mswwin->x;
            mswwin->tly = by + mswwin->y;

            set_buffer_view(mswwin);

            BOOST_FOREACH(window & child, mswwin->children)
                set_buffer_child((msw_window *) &child, mswwin->tlx, mswwin->tly);
        }

        // x_ & y_ are coords that tell where upper left corner of the image is, both should be <= 0
        // they are ignored if otherwise.
        // w & h are the width/height of the buffer when not taking into account x_ & y_
        static bool set_buffer_toplevel(msw_window * mswwin, int x, int y, int w, int h)
        {
            BOOST_ASSERT(mswwin->is_toplevel);

            // if the buffer does not exist create it
            if (!mswwin->buff_impl)
                mswwin->buff_impl.reset(new image());

            mswwin->tlx = (x < 0) ? -x : 0;
            mswwin->tly = (y < 0) ? -y : 0;

            int w_ = mswwin->tlx + w, h_ = mswwin->tly + h;

            // if the buffer needs to be resized, resize it, reset the buffer, and set the buffer
            // of all of this windows children
            if (w_ > boost::gil::view(*mswwin->buff_impl).width() || h_ > boost::gil::view(*mswwin->buff_impl).height())
            {
                int nw = std::max(w_, int(boost::gil::view(*mswwin->buff_impl).width())),
                    nh = std::max(h_, int(boost::gil::view(*mswwin->buff_impl).height()));

                // round nw/nh up to nearest multiple of MYTHOS_KHAOS_BUFFER_GROW_BY
                nw = nw + MYTHOS_KHAOS_BUFFER_GROW_BY - (nw % MYTHOS_KHAOS_BUFFER_GROW_BY);
                nh = nh + MYTHOS_KHAOS_BUFFER_GROW_BY - (nh % MYTHOS_KHAOS_BUFFER_GROW_BY);

                mswwin->buff_impl->recreate(nw, nh, 4);

                // create view w/ windows width/height
                set_buffer_view(mswwin);

                BOOST_FOREACH(window & child, mswwin->children)
                    set_buffer_child((msw_window *) &child, mswwin->tlx, mswwin->tly);

                return true;
            }
            else
            {
                // create view w/ windows width/height
                set_buffer_view(mswwin);

                return false;
            }
        }

        static void set_buffer(msw_window * mswwin)
        {
            msw_window * p = static_cast<msw_window *>(mswwin->parent);

            if (mswwin->is_toplevel)
            {
                set_buffer_toplevel(mswwin, mswwin->tlx, mswwin->tly, mswwin->width, mswwin->height);
            }
            else if (p && p->buff_impl)
            {
                // calculate topleft corner of this windows section of the buffer
                int x = mswwin->x + p->tlx, y = mswwin->y + p->tly;

                // get the toplevel root of this child window
                msw_window * root = p;
                for (; !root->is_toplevel; root = static_cast<msw_window *>(root->parent));

                // if the toplevel window did not resize the window and this current window
                // was a parentless child window, set the child windows of this window
                if (!set_buffer_toplevel(root, x, y, x + mswwin->width, y + mswwin->height))
                {
                    set_buffer_child(mswwin, p->tlx, p->tly);
                }
            }
        }

        static bool is_mythos_window(HWND hwnd)
        {
            return ::GetClassLongPtr(hwnd, GCW_ATOM) == ::mythos_window_class_atom;
        }

        static msw_window * mythos_window_from_native(HWND hwnd)
        {
            // if win is a wrapped window, the id is set to the msw_window *
            if (is_mythos_window(hwnd))
            {
                return (msw_window *) ::GetWindowLongPtr(hwnd, GWLP_USERDATA);
            }
            else
            {
                return (msw_window *) ::GetWindowLongPtr(hwnd, GWLP_ID);
            }
        }

        static void draw_view(image_view const& vw, msw_window * mswwin)
        {
            // create dc
            HDC hdc = ::GetDC(mswwin->handle);

            // clip view
            int w = vw.width(), h = vw.height();

            image_view sub_vw;

            // clip view
            if (mswwin->is_toplevel)
            {
                sub_vw = vw;
            }
            else
            {
                if (msw_window * parent = static_cast<msw_window *>(mswwin->parent))
                {
                    w = std::min(w, parent->width - mswwin->x);
                    h = std::min(h, parent->height - mswwin->y);
                }

                int tlx = 0, tly = 0;

                if (mswwin->x < 0)
                {
                    tlx += -mswwin->x;
                    w -= -mswwin->x;
                }

                if (mswwin->y < 0)
                {
                    tly += -mswwin->y;
                    w -= -mswwin->y;
                }

                sub_vw = image_view(w, h, vw.xy_at(tlx, tly));
            }

            // create the BITMAPINFO structure
            BITMAPINFO bmi;
            memset(&bmi, 0, sizeof(BITMAPINFO));

            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = sub_vw.width();
            bmi.bmiHeader.biHeight = sub_vw.height();
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 24;
            bmi.bmiHeader.biCompression = BI_RGB;

            // draw view
            ::StretchDIBits(
                hdc,
                0, 0, sub_vw.width(), sub_vw.height(),
                0, 0, sub_vw.width(), sub_vw.height(),
                boost::gil::interleaved_view_get_raw_data(sub_vw),
                &bmi,
                DIB_RGB_COLORS,
                SRCCOPY
            );

            ::ReleaseDC(mswwin->handle, hdc);
        }

        static bool raise_event(int id, event_info & ei)
        {
            if (ei.mswwin->handler)
                return ei.mswwin->handler(id, &ei);

            return false;
        }

        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            msw_window * mswwin = mythos_window_from_native(hwnd);
            msw_event_info ei = {mswwin, uMsg, wParam, lParam, NULL};

            LRESULT wrapped_result = 1;

            // call wrapped window proc first (if any)
            if (mswwin->wrapped_proc)
            {
                wrapped_result = ::CallWindowProc(mswwwin->wrapped_proc, hwnd, uMsg, wParam, lParam);
            }

            switch (uMsg)
            {
            case WM_PAINT:
                return raise_event(evt::paint::value, ei) && wrapped_result;
            case WM_LBUTTONDOWN:
                return raise_event(evt::l_button_down::value, ei) && wrapped_result;
            case WM_MBUTTONDOWN:
                return raise_event(evt::m_button_down::value, ei) && wrapped_result;
            case WM_RBUTTONDOWN:
                return raise_event(evt::r_button_down::value, ei) && wrapped_result;
            case WM_LBUTTONUP:
                return raise_event(evt::l_button_up::value, ei) && wrapped_result;
            case WM_MBUTTONUP:
                return raise_event(evt::m_button_up::value, ei) && wrapped_result;
            case WM_RBUTTONUP:
                return raise_event(evt::r_button_up::value, ei) && wrapped_result;
            case WM_MOUSEMOVE:
                return raise_event(evt::mouse_move::value, ei) && wrapped_result;
            case WM_SIZE:
                return raise_event(evt::resize::value, ei) && wrapped_result;
            };

            return wrapped_result;
        }
    }

    // main.hpp
    bool entry(HINSTANCE hInstance, int nCmdShow)
    {
        ::app_instance = hInstance;
        ::cmd_show = nCmdShow;

        // register mythos window class
        WNDCLASSEX wcex;
        memset(&wcex, 0, sizeof(WNDCLASSEX));

        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
        wcex.lpfnWndProc = detail::WindowProc;
        wcex.cbWndExtra = sizeof(void *);
        wcex.hInstance = ::app_instance;
        wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcex.lpszClassName = ::mythos_window_class;
        wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

        ::mythos_window_class_atom = ::RegisterClassEx(&wcex);

        if (!::mythos_window_class_atom)
        {
            throw std::runtime_error("khaos(msw): RegisterClassEx failed in mythos::entry");
        }

        // create message window
        ::message_window = ::CreateWindow(
            ::mythos_window_class,
            "",
            0,
            0, 0, 0, 0,
            HWND_MESSAGE,
            NULL,
            ::app_instance,
            NULL
        );

        if (!::message_window)
        {
            throw std::runtime_error("khaos(msw): CreateWindow failed in mythos::entry");
        }

        // load default font
        find_font(::default_font, "Monospace", 12);
    }

    namespace detail
    {
        struct parse_cmd_line_action
        {
            parse_cmd_line_action(std::vector<char *> & argv_) : argv(argv_) {}

            void operator(char * f, char * l) const
            {
                argv.push_back(f);

                *l = 0;
            }

            std::vector<char *> & argv;
        };
    }

    char * parse_cmd_line(std::vector<char *> & argv)
    {
        using namespace boost::spirit;

        // reserve space
        argv.reserve(10);

        // copy the command line args
        size_t len = strlen(::GetCommandLine());

        char * args = new char[len + 1];
        std::copy(::GetCommandLine(), ::GetCommandLine() + len + 1, args);

        // parse the command line args
        boost::spirit::parse(
            args, args + len,
            +( confix_p('"', '"')           [detail::parse_cmd_line_action(argv)]
             | lexeme_d
               [
                 (+(anychar_p - space_p))   [detail::parse_cmd_line_action(argv)]
               ]
             ),
            space_p
        );

        return args;
    }

    void cleanup() {}

    // event_loop.hpp
    int event_loop()
    {
        MSG msg;
        BOOL ret; 

        while ((ret = ::GetMessage(&msg, NULL, 0, 0)) != 0)
        {
            if (ret == -1)
            {
                throw std::runtime_error("khaos(msw): GetMessage failed in event_loop()");
            }
            else
            {
                ::TranslateMessage(&msg); 
                ::DispatchMessage(&msg); 
            }
        }

        return msg.wParam;
    }

    // window.hpp
    window * create_toplevel_window(std::string const& title, int x, int y, int cx, int cy, window * p)
    {
        msw_window * real_parent = static_cast<msw_window *>(p);

        HWND hwnd = ::CreateWindow(
            ::mythos_window_class,
            title.c_str(),
            WS_OVERLAPPEDWINDOW,
            x, y, cx, cy,
            real_parent ? real_parent->handle : NULL,
            NULL,
            ::app_instance,
            NULL
        );

        msw_window * result = new msw_window(hwnd, p, true, x, y, cx, cy);

        ::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) result);

        return result;
    }

    window * create_child_window(int x, int y, int cx, int cy, window * p)
    {
        msw_window * real_parent = static_cast<msw_window *>(p);

        HWND hwnd = ::CreateWindow(
            ::mythos_window_class,
            title.c_str(),
            WS_CHILD | WS_CLIPCHILDREN,
            x, y, cx, cy,
            real_parent ? real_parent->handle ? NULL,
            NULL,
            ::app_instance,
            NULL
        );

        msw_window * result = new msw_window(hwnd, p, false, x, y, cx, cy);

        ::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) result);

        if (is_visible(p))
            show_window(result);

        return result;
    }

    namespace detail
    {
        static BOOL CALLBACK foreign_create_child(HWND hwnd, LPARAM lParam)
        {
            msw_window * result = (msw_window *) lParam;

            if (detail::is_mythos_window(hwnd)) // shouldn't ever come up
                result->children.push_back(*detail::mythos_window_from_native(hwnd));
            else
            {
                // the child window is added when the msw_window is created
                foreign_create_window(&hwnd);
            }
        }
    }

    window * foreign_create_window(void * win)
    {
        BOOST_ASSERT(!detail::is_mythos_window(handle));

        HWND hwnd = *static_cast<HWND *>(win);
        HWND parent_handle = ::GetParent(hwnd);

        RECT area;
        ::GetWindowRect(hwnd, &area);

        msw_window * parent = NULL;
        bool is_child = ::GetWindowLongPtr(hwnd, GWL_STYLE) & WS_CHILD;

        // get parent as a mythos window
        if (detail::is_mythos_window(parent_handle))
            parent = detail::mythos_window_from_native(parent_handle);
        else
            parent = static_cast<msw_window *>(foreign_create_window(&parent_handle));

        // create window
        msw_window * result = new msw_window(
            hwnd, parent, area.left, area.top, area.right - area.left, area.bottom - area.top
        );

        // set window id to result
        ::SetWindowLongPtr(hwnd, GWLP_ID, (LONG_PTR) result);

        // wrap children
        ::EnumChildWindows(hwnd, detail::foreign_create_child, (LPARAM) result);

        return result;
    }

    void destroy_window(window * win)
    {
        msw_window * mswwin = static_cast<msw_window *>(win);

        ::DestroyWindow(mswwin->handle);
    }

    void * handle_of(window * win)
    {
        return &static_cast<msw_window *>(win)->handle;
    }

    // move/resize
    void move_window(window * win, int x, int y)
    {
        msw_window * mswwin = static_cast<msw_window *>(win);

        mswwin->x = x;
        mswwin->y = y;

        if (!mswwin->is_toplevel)
            detail::set_buffer(mswwin);

        ::MoveWindow(mswwin->handle, x, y, mswwin->width, mswwin->height, false); // FIXME: need to redraw?
    }

    void resize_window(window * win, int cx, int cy)
    {
        msw_window * mswwin = static_cast<msw_window *>(win);

//        point old(mswwin->width, mswwin->height);

        mswwin->width = cx;
        mswwin->height = cy;

        detail::set_buffer(mswwin);

        ::MoveWindow(mswwin->handle, mswwin->x, mswwin->y, cx, cy, true);

//        resize::raise(win, old);
    }

    point get_position(window * win)
    {
        return point(static_cast<msw_window *>(win)->x, static_cast<msw_window *>(win)->y);
    }

    point get_size(window * win)
    {
        return point(static_cast<msw_window *>(win)->width, static_cast<msw_window *>(win)->height);
    }

    void show_window(window * win)
    {
        // make sure the first toplevel window created uses nCmdShow
        static int nCmdShow = ::cmd_show;

        if (win->is_toplevel)
        {
            ::ShowWindow(static_cast<msw_window *>(win)->handle, nCmdShow);

            nCmdShow = SW_SHOW;
        }
        else
        {
            ::ShowWindow(static_cast<msw_window *>(win)->handle, SW_SHOW);
        }
    }

    void hide_window(window * win)
    {
        ::ShowWindow(static_cast<msw_window *>(win)->handle, SW_HIDE);
    }

    bool is_visible(window * win)
    {
        return ::IsWindowVisible(static_cast<msw_window *>(win)->handle);
    }

    // TODO: rewrite this
    void reparent(window * win, window * parent)
    {
        BOOST_ASSERT(win);
        BOOST_ASSERT(parent ? win->is_toplevel == parent->is_toplevel : true);

        msw_window * mswwin = static_cast<x11_window *>(win);
        msw_window * mswpar = static_cast<x11_window *>(parent);

        mswwin->unlink();

        if (mswpar)
        {
            mswwin->parent = mswpar;
            mswpar->children.push_back(*mswwin);
        }

        detail::set_buffer(mswwin);

        ::SetParent(mswwin->handle, mswpar ? mswpar->handle : NULL);

        LONG_PTR style = ::GetWindowLongPtr(mswwin->handle, GWL_STYLE);

        // update UI state FIXME: no idea if this is done right
        ::SendMessage(mswwin->handle, WM_UPDATEUISTATE, UIS_INITIALIZE, 0);
    }

    void modalize(window * win)
    {
        BOOST_ASSERT(win->is_toplevel);

        msw_window * mswwin = static_cast<msw_window *>(win);

        MSG msg;
        BOOL ret; 

        while ((ret = ::GetMessage(&msg, mswwin->handle, 0, 0)) != 0)
        {
            if (ret == -1 && ::IsWindow(mswwin->handle))
            {
                throw std::runtime_error("khaos(msw): GetMessage failed in modalize(window *)");
            }
            else
            {
                ::TranslateMessage(&msg); 
                ::DispatchMessage(&msg); 
            }
        }

        return msg.wParam;
    }

    bool is_mythos_window(window * win)
    {
        return detail::is_mythos_window(win->handle);
    }

    // event/button.hpp
    namespace detail
    {
        point pt_from_ei(void * info)
        {
            msw_event_info * ei = static_cast<msw_event_info *>(info);

            return point(GET_X_LPARAM(ei->lparam), GET_Y_LPARAM(ei->lparam));
        }

        bool button_raise(window * win, point const& pt, int N)
        {
            LPARAM lparam = pt.x + (pt.y << (sizeof(WORD) * 8));

            msw_window * mswwin = static_cast<msw_window *>(win);

            UINT msg = 0;
            switch (N)
            {
            case l_button_down::value: msg = WM_LBUTTONDOWN; break;
            case m_button_down::value: msg = WM_MBUTTONDOWN; break;
            case r_button_down::value: msg = WM_RBUTTONDOWN; break;
            case l_button_up::value: msg = WM_LBUTTONUP; break;
            case m_button_up::value: msg = WM_MBUTTONUP; break;
            case r_button_up::value: msg = WM_RBUTTONUP; break;
            case mouse_move::value: msg = WM_MOUSEMOVE; break;
            default:
                throw std::runtime_error("khaos(msw): Illegal button event type in detail::button_raise");
            };

            ::SendMessage(mswwin->handle, msg, 0, lparam);
        }
    }

    // event/paint.hpp
    namespace detail
    {
        image_view view_from_event_info(void * info)
        {
            msw_window * win = static_cast<msw_event_info *>(info)->win;

            return win->buffer;
        }

        bool draw_view(image_view const& vw, void * info)
        {
            msw_window * win = static_cast<msw_event_info *>(info)->win;

            detail::draw_view(vw, win);

            return true;
        }
    }

    bool paint::raise(window * win)
    {
        return ::RedrawWindow(
            static_cast<msw_window *>(win)->handle,
            NULL,
            NULL,
            RDW_UPDATENOW|RDW_ALLCHILDREN|RDW_INVALIDATE
        );
    }

    // event/resize.hpp
    namespace detail
    {
        point resize_old_width(void * ei)
        {
            msw_event_info * info = static_cast<msw_event_info *>(ei);

            return point(LOWORD(info->lparam), HIWORD(info->lparam));
        }
    }

    bool resize::raise(window * win, point pt)
    {
        LPARAM lparam = pt.x + (pt.y << (sizeof(WORD) * 8));

        return ::SendMessage(static_cast<msw_window *>(win)->handle, WM_SIZE, 0, lparam) == 0;
    }

    // event/reconfigure.hpp
    namespace detail
    {
        point reconfigure_space(void * info)
        {
            return *static_cast<point *>(static_cast<msw_event_info *>(info)->data);
        }
    }

    bool reconfigure::raise(window * win, point pt)
    {
        msw_event_info ei = {static_cast<msw_window *>(win), 0, 0, &pt};

        return detail::raise_event(reconfigure::value, ei);
    }

    // font.hpp
    struct glyph_impl : glyph
    {
        glyph_image img;
    };

    struct msw_font_extra
    {
        msw_font_extra(HFONT f)
            : face(f), dc(NULL), height(0), width(0), ascender(0), descender(0)
        {
            dc = ::CreateCompatibleDC(::GetDC(NULL));
            old = ::SelectObject(dc, (HGDIOBJ) face);

            ::GetTextMetrics(dc, &tmetrics);
        }
        ~msw_font_extr()
        {
            ::SelectObject(dc, old);
            ::DeleteDC(dc);
            ::DeleteObject((HGDIOBJ) face);
        }

        HFONT face;
        HDC dc;
        TEXTMETRICS tmetrics;

        HGDIOBJ old;

        std::map<char, glyph_impl> glyphs;
    };

    font::font() : extra_data(0) {}

    font::~font()
    {
        if (!extra_data) return;

        msw_font_extra * extra = static_cast<msw_font_extra *>(extra_data);

        delete static_cast<msw_font_extra *>(extra_data);
    }

    void * handle_of(font const& f)
    {
        BOOST_ASSERT(f);

        return &static_cast<msw_font_extra *>(f.extra_data)->handle;
    }

    int kerning_distance(font const& f, char a, char b)
    {
        BOOST_ASSERT(f);

        KERNINGPAIR kern;
        kern.wFirst = a;
        kern.wSecond = b;

        ::GetKerningPairs(static_cast<msw_font_extra *>(f.extra_data)->dc, 1, &kern);

        return kern.iKernAmount;
    }

    glyph const& find_glyph(font const& f, char c)
    {
        typedef std::map<char, glyph_impl>::iterator   iterator;

        BOOST_ASSERT(f);

        msw_font_extra * extra = static_cast<msw_font_extra *>(f.extra_data);

        iterator i = extra->glyphs.find(c);

        // if c already has a cached image in glyphs, return it, otherwise create one
        if (i != extra->glyphs.end())
            return i->second;
        else
            i = extra->glyphs.insert(std::make_pair(c, glyph_impl())).first;

        // get glyph metrics
        GLYPHMETRICS metrics;
        ::GetGlyphOutline(extra->dc, c, GGO_METRICS, &metrics, 0, NULL, NULL);

        i->second.bearing = point(metrics.gmptGlyphOrigin.x, metrics.gmptGlyphOrigin.y);
        i->second.advance = gmCellIncX;

        // get glyph outline
        int w = metrics.gmBlackBoxX;
        int h = metrics.gmBlackBoxY;

        i->second.img.recreate(w, h);
        i->second.view = boost::gil::view(i->second.img);

        size_t bytes = i->second.view.height() * i->second.view.xy_at(0, 0).row_size();

        BOOST_ASSERT(bytes == ::GetGlyphOutline(extra->dc, c, GGO_GRAY8_BITMAP, &metrics, 0, NULL, NULL));

        DWORD result = ::GetGlyphOutline(
            extra->dc,
            c,
            GGO_GRAY8_BITMAP,
            &metrics,
            bytes,
            boost::gil::interleaved_view_get_raw_data(i->second.view),
            NULL
        );

        return i->second;
    }

    int average_char_width(font const& f)
    {
        return static_cast<msw_font_extra *>(f.extra_data)->tmetrics.tmAveCharWidth;
    }

    int average_char_height(font const& f)
    {
        return static_cast<msw_font_extra *>(f.extra_data)->tmetrics.tmHeight;
    }

    int line_length(font const& f)
    {
        return average_char_height(f);
    }

    int ascender(font const& f)
    {
        return static_cast<msw_font_extra *>(f.extra_data)->tmetrics.tmAscent;
    }

    int descender(font const& f)
    {
        return static_cast<msw_font_extra *>(f.extra_data)->tmetrics.tmDescent;
    }

    void find_font(font & f, std::string const& name, int height, int weight, int style)
    {
        // FIXME: shouldn't require fonts to be empty
        BOOST_ASSERT(!f);

        HFONT hfont = ::CreateFont(
            height,
            0,
            0,
            0,
            weight,
            style & font_style::italic,
            style & font_style::underline,
            style & font_style::strikethrough,
            0,
            0,
            0,
            0,
            name.c_str()
        );

        if (!hfont) return;

        f.extra_data = new msw_font_extra(hfont);
    }

    font const& default_font()
    {
        return ::default_font;
    }

    // msw.hpp
    namespace detail
    {
        void * get_messagewin()
        {
            return (void *) ::message_window;
        }

        void * get_hinstance()
        {
            return (void *) ::app_instance;
        }
    }
}}

