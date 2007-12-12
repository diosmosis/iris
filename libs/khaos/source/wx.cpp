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

#include <mythos/khaos/event/paint.hpp>
#include <mythos/khaos/event/button.hpp>
#include <mythos/khaos/event/resize.hpp>
#include <mythos/khaos/event/reconfigure.hpp>

#include <boost/shared_ptr.hpp>

#include <boost/foreach.hpp>
#include <boost/assert.hpp>

#include <wx/wx.h>
#include <wx/popupwin.h>

#define MYTHOS_KHAOS_BUFFER_GROW_BY 128

#include <boost/gil/extension/io/bmp_io.hpp>

// wxGTK won't create certain windows w/o parents, which makes it impossible to
// get size/position info for parentless child windows.  to emulate parentless child
// windows, we add each child window to this wxPopupWindow (can't use a toplevel window,
// as otherwise the program would never finish)
static wxPopupWindow * parentless_child_win_parent = 0;

namespace mythos { namespace khaos
{
    struct wx_window;

    struct wx_event_info
    {
        wx_window * win;
        void * data;
    };

    namespace detail
    {
        wxWindow * get_parentless_child_win_parent()
        {
            return ::parentless_child_win_parent;
        }

        static void set_buffer(wx_window * win);
        static bool raise_event(int id, wx_event_info & ei);

        class mythos_event_handler : public wxEvtHandler
        {
        public:
            typedef mythos_event_handler self_type;

            mythos_event_handler(wx_window * w) : win(w)
            {
                // connect events

                // FIXME: wxEVT_PAINT doesn't seem to propagate on x11, is this normal?
                void (self_type::*paint_impl)(wxPaintEvent &) = &self_type::generic_event<paint, wxPaintEvent>;
                this->Connect(wxEVT_PAINT, (wxObjectEventFunction) paint_impl);

                void (self_type::*mouse_impl)(wxMouseEvent &);

                mouse_impl = &self_type::mouse_event<l_button_down>;
                this->Connect(wxEVT_LEFT_DOWN, (wxObjectEventFunction) mouse_impl);

                mouse_impl = &self_type::mouse_event<l_button_up>;
                this->Connect(wxEVT_LEFT_UP, (wxObjectEventFunction) mouse_impl);

                mouse_impl = &self_type::mouse_event<m_button_down>;
                this->Connect(wxEVT_MIDDLE_DOWN, (wxObjectEventFunction) mouse_impl);

                mouse_impl = &self_type::mouse_event<m_button_up>;
                this->Connect(wxEVT_MIDDLE_UP, (wxObjectEventFunction) mouse_impl);

                mouse_impl = &self_type::mouse_event<r_button_down>;
                this->Connect(wxEVT_RIGHT_DOWN, (wxObjectEventFunction) mouse_impl);

                mouse_impl = &self_type::mouse_event<r_button_up>;
                this->Connect(wxEVT_RIGHT_UP, (wxObjectEventFunction) mouse_impl);

                mouse_impl = &self_type::mouse_event<mouse_move>;
                this->Connect(wxEVT_MOTION, (wxObjectEventFunction) mouse_impl);

                void (self_type::*size_impl)(wxSizeEvent &) = &self_type::on_resize;
                this->Connect(wxEVT_SIZE, (wxObjectEventFunction) size_impl);

                void (self_type::*destroy_impl)(wxEvent &) = &self_type::on_destroy;
                this->Connect(wxEVT_DESTROY, (wxObjectEventFunction) destroy_impl);
            }

            template <typename E, typename wxE>
            void generic_event(wxE & event)
            {
                this->GetNextHandler()->ProcessEvent(event);

                wx_event_info ei = {win, NULL};

                raise_event(E::value, ei);
            }

            template <typename E>
            void mouse_event(wxMouseEvent & event)
            {
                this->GetNextHandler()->ProcessEvent(event);

                point pt(event.GetX(), event.GetY());

                wx_event_info ei = {win, &pt};

                raise_event(E::value, ei);
            }

            void on_resize(wxSizeEvent & event)
            {
                this->GetNextHandler()->ProcessEvent(event);

                point pt(event.GetSize().GetWidth(), event.GetSize().GetHeight());

                wx_event_info ei = {win, &pt};

                raise_event(resize::value, ei);
            }

            void on_destroy(wxEvent & event);

            wx_window * win;
        };
    }

    struct wx_window : window
    {
        wx_window(wxWindow * wxwin, window * p, bool top, int x_, int y_, int w, int h)
            : window(p, top), win(wxwin), x(x_), y(y_), width(w), height(h), main_handler(this)
        {
            detail::set_buffer(this);

            // FIXME: will this mask all other event handlers in the chain? if so this is a problem
            wxwin->PushEventHandler(&main_handler);
        }

        wxWindow * win;

        image_view buffer;

        // x & y are window positions
        // width & height are both window size & the size of the buffer
        int x, y, width, height;

        // tlx & tly are indices into buff_impl
        int tlx, tly;

        boost::shared_ptr<image> buff_impl;

        detail::mythos_event_handler main_handler;
    };

    namespace detail
    {
        inline void mythos_event_handler::on_destroy(wxEvent & event)
        {
            this->GetNextHandler()->ProcessEvent(event);

            delete win;
        }

        static bool raise_event(int id, wx_event_info & ei)
        {
            if (ei.win->handler)
                return ei.win->handler(id, &ei);

            return false;
        }

        // buffer related functions ONLY manipulate the buffer
        static void set_buffer_view(wx_window * win)
        {
            win->buffer = image_view(
                win->width, win->height, boost::gil::view(*win->buff_impl).xy_at(win->tlx, win->tly)
            );
        }

        static void set_buffer_child(wx_window * win, int bx, int by)
        {
            BOOST_ASSERT(!win->is_toplevel && win->parent);

            win->buff_impl = static_cast<wx_window *>(win->parent)->buff_impl;

            win->tlx = bx + win->x;
            win->tly = by + win->y;

            set_buffer_view(win);

            BOOST_FOREACH(window & child, win->children)
                set_buffer_child((wx_window *) &child, win->tlx, win->tly);
        }

        // x_ & y_ are coords that tell where upper left corner of the image is, both should be <= 0
        // they are ignored if otherwise.
        // w & h are the width/height of the buffer when not taking into account x_ & y_
        static bool set_buffer_toplevel(wx_window * win, int x, int y, int w, int h)
        {
            BOOST_ASSERT(win->is_toplevel);

            // if the buffer does not exist create it
            if (!win->buff_impl)
                win->buff_impl.reset(new image());

            win->tlx = (x < 0) ? -x : 0;
            win->tly = (y < 0) ? -y : 0;

            int w_ = win->tlx + w, h_ = win->tly + h;

            // if the buffer needs to be resized, resize it, reset the buffer, and set the buffer
            // of all of this windows children
            if (w_ > boost::gil::view(*win->buff_impl).width() || h_ > boost::gil::view(*win->buff_impl).height())
            {
                int nw = std::max(w_, int(boost::gil::view(*win->buff_impl).width())),
                    nh = std::max(h_, int(boost::gil::view(*win->buff_impl).height()));

                // round nw/nh up to nearest multiple of MYTHOS_KHAOS_BUFFER_GROW_BY
                nw = nw + MYTHOS_KHAOS_BUFFER_GROW_BY - (nw % MYTHOS_KHAOS_BUFFER_GROW_BY);
                nh = nh + MYTHOS_KHAOS_BUFFER_GROW_BY - (nh % MYTHOS_KHAOS_BUFFER_GROW_BY);

                win->buff_impl->recreate(nw, nh, 4);

                // create view w/ windows width/height
                set_buffer_view(win);

                BOOST_FOREACH(window & child, win->children)
                    set_buffer_child((wx_window *) &child, win->tlx, win->tly);

                return true;
            }
            else
            {
                // create view w/ windows width/height
                set_buffer_view(win);

                return false;
            }
        }

        static void set_buffer(wx_window * win)
        {
            BOOST_ASSERT(win->tlx >= 0 && win->tly >= 0);
            BOOST_ASSERT(win->width >= 0 && win->height >= 0);

            wx_window * p = static_cast<wx_window *>(win->parent);

            if (win->is_toplevel)
            {
                set_buffer_toplevel(win, win->tlx, win->tly, win->width, win->height);
            }
            else if (p && p->buff_impl)
            {
                // calculate topleft corner of this windows section of the buffer
                int x = win->x + p->tlx, y = win->y + p->tly;

                // get the toplevel root of this child window
                wx_window * root = p;
                for (; !root->is_toplevel; root = static_cast<wx_window *>(root->parent));

                // if the toplevel window did not resize the window and this current window
                // was a parentless child window, set the child windows of this window
                if (!set_buffer_toplevel(root, x, y, x + win->width, y + win->height))
                {
                    set_buffer_child(win, p->tlx, p->tly);
                }
            }

            BOOST_ASSERT(win->buffer.width() >= 0 && win->buffer.height() >= 0);
        }

        // draw_view
        static void draw_view(image_view const& vw, wx_window * wxwin)
        {
            // create paint DC
            wxPaintDC dc(wxwin->win);

            // clip view if neccessary
            int w = vw.width(), h = vw.height();

            image_view sub_vw;

            // clip view
            if (wxwin->is_toplevel)
            {
                sub_vw = vw;
            }
            else
            {
                if (wx_window * parent = static_cast<wx_window *>(wxwin->parent))
                {
                    w = std::min(w, parent->width - wxwin->x);
                    h = std::min(h, parent->height - wxwin->y);
                }

                int tlx = 0, tly = 0;

                if (wxwin->x < 0)
                {
                    tlx += -wxwin->x;
                    w -= -wxwin->x;
                }

                if (wxwin->y < 0)
                {
                    tly += -wxwin->y;
                    w -= -wxwin->y;
                }

                sub_vw = image_view(w, h, vw.xy_at(tlx, tly));
            }

            // FIXME: too slow
            // copy into wxBitmap
            wxImage wximg(sub_vw.width(), sub_vw.height());
            boost::gil::copy_pixels(
                sub_vw, boost::gil::interleaved_view(
                    wximg.GetWidth(), wximg.GetHeight(),
                    (boost::gil::rgb8_pixel_t *) wximg.GetData(), wximg.GetWidth() * 3
                )
            );

            wxBitmap bmp(wximg);

            // draw onto the screen
            dc.DrawBitmap(bmp, 0, 0, false);
        }

        static wx_window * mythos_window_from_native(wxWindow * wxwin)
        {
            if (!wxwin)
                return NULL;

            // if the top of the window's event handler chain is a mythos_event_handler,
            // it has been wrapped into a mythos window
            wxEvtHandler * handler = wxwin->GetEventHandler();

            if (mythos_event_handler * real = dynamic_cast<mythos_event_handler *>(handler))
            {
                return real->win;
            }

            return NULL;
        }
    }

    // main.hpp
    bool entry(int * argc, char *** argv)
    {
        ::parentless_child_win_parent = new wxPopupWindow(NULL, wxID_ANY);

        return true;
    }

    void cleanup()
    {
        delete parentless_child_win_parent;
    }

    // event_loop.hpp
    int event_loop()
    {
        throw std::runtime_error("khaos(wx): event_loop should not be used");
    }

    // window.hpp
    window * create_toplevel_window(std::string const& title, int x, int y, int cx, int cy, window * p)
    {
        wx_window * real_parent = static_cast<wx_window *>(p);

        wxFrame * frame = new wxFrame(
            real_parent ? real_parent->win : NULL,
            wxID_ANY,
            title,
            wxPoint(x, y),
            wxSize(cx, cy)
        );

        wx_window * result = new wx_window(frame, p, true, x, y, cx, cy);

        return result;
    }

    window * create_child_window(int x, int y, int cx, int cy, window * p)
    {
        wx_window * real_parent = static_cast<wx_window *>(p);

        wxWindow * win = new wxWindow(
            real_parent ? real_parent->win : ::parentless_child_win_parent,
            wxID_ANY,
            wxPoint(x, y),
            wxSize(cx, cy)
        );

        wx_window * result = new wx_window(win, p, false, x, y, cx, cy);

        if (real_parent && real_parent->win->IsShown())
            win->Show(true);

        return result;
    }

    window * foreign_create_window(void * win)
    {
        wxWindow * wxwin = *static_cast<wxWindow **>(win);

        BOOST_ASSERT(win && wxwin);

        wxWindow * wxpar = wxwin->GetParent();

        // wrap parent if necessary
        window * parent = detail::mythos_window_from_native(wxpar);

        if (wxpar && !parent)
            parent = foreign_create_window(&wxpar);

        int x, y, w, h;

        wxwin->GetPosition(&x, &y);
        wxwin->GetSize(&w, &h);

        // FIXME: will this cover all cases?
        bool is_toplevel = dynamic_cast<wxTopLevelWindow *>(wxwin);

        wx_window * result = new wx_window(wxwin, parent, is_toplevel, x, y, w, h);

        // wrap children if necessary
        wxWindowList & children = wxwin->GetChildren();
        for (wxWindowList::iterator i = children.begin(); i != children.end(); ++i)
        {
            // the child window is added to wxwin when the wx_window is created
            foreign_create_window(&(*i));
        }

        return result;
    }

    void destroy_window(window * win)
    {
        wx_window * wxwin = static_cast<wx_window *>(win);

        wxwin->win->Destroy();

        // the mythos window will be destroyed when a wxEVT_DESTROY event is generated
        // delete wxwin;
    }

    void * handle_of(window * win)
    {
        return &static_cast<wx_window *>(win)->win;
    }

    // move/resize
    void move_window(window * win, int x, int y)
    {
        wx_window * wxwin = static_cast<wx_window *>(win);

        wxwin->x = x;
        wxwin->y = y;

        if (!wxwin->is_toplevel)
            detail::set_buffer(wxwin);

        wxwin->win->Move(x, y);
    }

    void resize_window(window * win, int cx, int cy)
    {
        wx_window * wxwin = static_cast<wx_window *>(win);

        wxwin->width = cx;
        wxwin->height = cy;

        detail::set_buffer(wxwin);

        wxwin->win->SetSize(cx, cy);
    }

    point get_position(window * win)
    {
        wx_window * wxwin = static_cast<wx_window *>(win);

        wxPoint pos = wxwin->win->GetPosition();

        return point(pos.x, pos.y);
    }

    point get_size(window * win)
    {
        wx_window * wxwin = static_cast<wx_window *>(win);

        wxSize sz = wxwin->win->GetSize();

        return point(sz.GetWidth(), sz.GetHeight());
    }

    // show/hide/is_shown
    void show_window(window * win)
    {
        static_cast<wx_window *>(win)->win->Show(true);
    }

    void hide_window(window * win)
    {
        static_cast<wx_window *>(win)->win->Show(false);
    }

    bool is_shown(window * win)
    {
        return static_cast<wx_window *>(win)->win->IsShown();
    }

    void set_parent(window * win, window * parent)
    {
        BOOST_ASSERT(win && parent);

        wx_window * wxwin = static_cast<wx_window *>(win),
                  * wxpar = static_cast<wx_window *>(parent);

        wxwin->unlink();

        if (wxpar)
        {
            wxwin->parent = wxpar;
            wxpar->children.push_back(*wxwin);

            wxwin->is_toplevel = false;

            detail::set_buffer(wxwin);
        }
        else
        {
            wxwin->parent = wxpar;
            wxwin->is_toplevel = true;

            detail::set_buffer(wxwin);
        }

        // TODO: Error checking
        wxwin->win->Reparent(wxpar->win);

        if ((!parent || is_shown(wxpar)) && !is_shown(wxwin))
            show_window(wxwin);
    }

    void modalize(window * win)
    {
        static_cast<wx_window *>(win)->win->MakeModal(true);
    }

    // all windows are wrapped, so any window is a mythos window
    bool is_mythos_window(window * win)
    {
        return true;
    }

    // event/button.hpp
    namespace detail
    {
        point pt_from_ei(void * info)
        {
            return *static_cast<point *>(static_cast<wx_event_info *>(info)->data);
        }

        bool button_raise(window * win, point const& pt, int N)
        {
            WXTYPE event_type;

            switch (N)
            {
            case l_button_down::value: event_type = wxEVT_LEFT_DOWN; break;
            case l_button_up::value: event_type = wxEVT_LEFT_UP; break;
            case m_button_down::value: event_type = wxEVT_MIDDLE_DOWN; break;
            case m_button_up::value: event_type = wxEVT_MIDDLE_UP; break;
            case r_button_down::value: event_type = wxEVT_RIGHT_DOWN; break;
            case r_button_up::value: event_type = wxEVT_RIGHT_UP; break;
            case mouse_move::value: event_type = wxEVT_MOTION; break;
            };

            wxMouseEvent event(event_type);

            event.m_x = pt.x;
            event.m_y = pt.y;

            return static_cast<wx_window *>(win)->win->ProcessEvent(event);
        }
    }

    // event/paint.hpp
    namespace detail
    {
        image_view view_from_event_info(void * info)
        {
            wx_window * win = static_cast<wx_event_info *>(info)->win;

            return win->buffer;
        }

        bool draw_view(image_view const& vw, void * info)
        {
            wx_window * win = static_cast<wx_event_info *>(info)->win;

            detail::draw_view(vw, win);

            return true;
        }
    }

    bool paint::raise(window * win)
    {
        wxPaintEvent event;

        return static_cast<wx_window *>(win)->win->ProcessEvent(event);
    }

    // event/resize.hpp
    namespace detail
    {
        point resize_old_width(void * ei)
        {
            return *static_cast<point *>(static_cast<wx_event_info *>(ei)->data);
        }
    }

    bool resize::raise(window * win, point pt)
    {
        wxSizeEvent event(wxSize(pt.x, pt.y));

        return static_cast<wx_window *>(win)->win->ProcessEvent(event);
    }

    // event/reconfigure.hpp
    namespace detail
    {
        point reconfigure_space(void * info)
        {
            return *static_cast<point *>(static_cast<wx_event_info *>(info)->data);
        }
    }

    bool reconfigure::raise(window * win, point pt)
    {
        wx_event_info ei = {static_cast<wx_window *>(win), &pt};

        return detail::raise_event(reconfigure::value, ei);
    }

    // font.hpp
    font::font() : extra_data(0) {}

    font::~font()
    {
        if (!extra_data)
            return;

        delete static_cast<wxFont *>(extra_data);
    }

    void * handle_of(font const& f)
    {
        return static_cast<void *>(const_cast<void **>(&f.extra_data));
    }

    int kerning_distance(font const& f, char a, char b)
    {
        throw std::runtime_error("khaos(wx): kerning_distance not implemented");
    }

    glyph const& find_glyph(font const& f, char c)
    {
        throw std::runtime_error("khaos(wx): find_glyph not implemented");
    }

    int ascender(font const& f)
    {
        throw std::runtime_error("khaos(wx): ascender not implemented");
    }

    int descender(font const& f)
    {
        throw std::runtime_error("khaos(wx): descender not implemented");
    }

    int line_length(font const& f)
    {
        throw std::runtime_error("khaos(wx): line_length not implemented");
    }

    int average_char_width(font const& f)
    {
        wxMemoryDC dc;

        dc.SetFont(*static_cast<wxFont *>(f.extra_data));

        return dc.GetCharWidth();
    }

    int average_char_height(font const& f)
    {
        wxMemoryDC dc;

        dc.SetFont(*static_cast<wxFont *>(f.extra_data));

        return dc.GetCharHeight();
    }

    void find_font(font & f, std::string const& name, int height, int weight, int style)
    {
        BOOST_ASSERT(!f);

        // get wxweight
        int wxweight;

        if (weight < font_weight::normal)
            wxweight = wxFONTWEIGHT_LIGHT;
        else if (weight == font_weight::normal)
            wxweight = wxFONTWEIGHT_NORMAL;
        else if (weight <= font_weight::bold)
            wxweight = wxFONTWEIGHT_BOLD;
        else
            wxweight = wxFONTWEIGHT_MAX;

        // get wxstyle
        int wxstyle = 0;

        if (style & font_style::italic)
            wxstyle |= wxFONTFLAG_ITALIC;

// FIXME: is this necessary?
//        if (style & font_style::underline)
//            wxstyle |= wxFONTFLAG_UNDERLINE;

        if (style & font_style::strikethrough)
            wxstyle |= wxFONTFLAG_STRIKETHROUGH;

        // get wxunder
        bool wxunder = style & font_style::underline;

        // FIXME: height should be converted to point size!!!
        wxFont * fnt = new wxFont(height, wxFONTFAMILY_DEFAULT, wxstyle, wxweight, wxunder, name.c_str());

        if (fnt->IsOk())
            f.extra_data = fnt;
        else
            delete fnt;
    }

    // FIXME: do this right
    font const& default_font()
    {
        static font df;

        if (!df)
            find_font(df, "Monospace", 12);

        BOOST_ASSERT(df);

        return df;
    }

    // wxWidgets versions of render_text/render_text_line/text_extents
    void render_text(font const& fnt, wxBitmap & bmp, std::string const& str, wxColour const& color)
    {
        wxMemoryDC dc;

        dc.SelectObject(bmp);

        dc.SetFont(*static_cast<wxFont *>(fnt.extra_data));
        dc.SetTextForeground(color);
        dc.DrawText(str, 0, 0);
    }

    point text_extents(font const& fnt, std::string const& x, size_t maxx)
    {
        wxArrayInt arr;

        // for some reason using a memory dc here fails miserably (on wxX11 at least)
        wxPaintDC dc(::parentless_child_win_parent);
        dc.SetFont(*static_cast<wxFont *>(fnt.extra_data));

        // insert new lines
        std::string copy;
        copy.reserve(x.size() * 2);

        for (size_t i = 0, j = 0; i != x.size(); ++i, ++j)
        {
            copy.push_back(x[i]);

            if (x[i] == '\n')
                j = 0;
            else if (j == maxx)
            {
                j = 0;

                copy.push_back('\n');
            }
        }

        wxSize sz = dc.GetMultiLineTextExtent(x.c_str());

        return point(sz.GetWidth(), sz.GetHeight());
    }

    point text_extents(font const& fnt, std::string const& x)
    {
        wxCoord w = 0, h = 0;

        // for some reason using a memory dc here fails miserably (on wxX11 at least)
        wxPaintDC dc(::parentless_child_win_parent);

        dc.SetFont(*static_cast<wxFont *>(fnt.extra_data));
        dc.GetMultiLineTextExtent(x, &w, &h);

        return point(w, h);
    }
}}

