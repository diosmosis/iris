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

#include <boost/intrusive/list.hpp>

#include <boost/system/system_error.hpp>

#include <boost/shared_ptr.hpp>

#include <boost/foreach.hpp>

#include <boost/detail/endian.hpp>
#include <boost/detail/atomic_count.hpp>
#include <boost/gil/extension/io/jpeg_io.hpp>

#include <stdexcept>
#include <iostream>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/Xatom.h>

#define MYTHOS_KHAOS_BUFFER_GROW_BY 128

static unsigned long const x11_all_events_mask =
    PointerMotionMask | ButtonPressMask | ButtonReleaseMask | ExposureMask |
    StructureNotifyMask
    ;

static unsigned long const x11_child_events_mask =
    PointerMotionMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask
    ;

static XWindowAttributes root_attr;

static Window parentless_root;

static Atom WM_DELETE_WINDOW, WM_PROTOCOLS;

#if defined( BOOST_BIG_ENDIAN )
static int const x11_endian = MSBFirst;
#elif defined( BOOST_LITTLE_ENDIAN )
static int const x11_endian = LSBFirst;
#else
#   error no endian definition
#endif

static boost::detail::atomic_count toplevel_windows(0), windows(0);

static Display * x11_display = NULL;
static XContext x11_user_data_context;

namespace mythos { namespace khaos
{
    struct x11_window;

    namespace detail
    {
        static void set_buffer(x11_window * xwin);
    }

    struct x11_window : window
    {
        x11_window(Window hnd, window * p, bool top, int x_, int y_, int w, int h)
            : window(p, top), handle(hnd), x(x_), y(y_), width(w), height(h), tlx(0), tly(0), mapped(false)
        {
            if (is_toplevel)
                ++::toplevel_windows;

            ++::windows;

            detail::set_buffer(this);
        }

        ~x11_window()
        {
            if (is_toplevel)
            {
                BOOST_FOREACH(window & child, children)
                    if (child.is_toplevel)
                        destroy_window(&child);

                --::toplevel_windows;
            }

            --::windows;
        }

        Window handle;

        image_view buffer;

        // x & y are window positions
        // width & height are both window size & the size of the buffer
        int x, y, width, height;

        // tlx & tly are indices into buff_impl
        int tlx, tly;

        bool mapped;

        boost::shared_ptr<image> buff_impl;
    };

    struct x11_event_info
    {
        x11_window * win;
        void * data;
        bool keep_looping;
    };

    namespace detail
    {
        // buffer related functions ONLY manipulate the buffer (and the indices into the buffer)
        static void set_buffer_view(x11_window * xwin)
        {
            xwin->buffer = image_view(
                xwin->width, xwin->height, boost::gil::view(*xwin->buff_impl).xy_at(xwin->tlx, xwin->tly)
            );
        }

        static void set_buffer_child(x11_window * xwin, int bx, int by)
        {
            BOOST_ASSERT(!xwin->is_toplevel && xwin->parent);

            xwin->buff_impl = static_cast<x11_window *>(xwin->parent)->buff_impl;

            xwin->tlx = bx + xwin->x;
            xwin->tly = by + xwin->y;

            set_buffer_view(xwin);

            BOOST_FOREACH(window & child, xwin->children)
                set_buffer_child((x11_window *) &child, xwin->tlx, xwin->tly);
        }

        // x_ & y_ are coords that tell where upper left corner of the image is, both should be <= 0
        // they are ignored if otherwise.
        // w & h are the width/height of the buffer when not taking into account x_ & y_
        static bool set_buffer_toplevel(x11_window * xwin, int x, int y, int w, int h)
        {
            BOOST_ASSERT(xwin->is_toplevel);

            // if the buffer does not exist create it
            if (!xwin->buff_impl)
                xwin->buff_impl.reset(new image());

            xwin->tlx = (x < 0) ? -x : 0;
            xwin->tly = (y < 0) ? -y : 0;

            int w_ = xwin->tlx + w, h_ = xwin->tly + h;

            // if the buffer needs to be resized, resize it, reset the buffer, and set the buffer
            // of all of this windows children
            if (w_ > boost::gil::view(*xwin->buff_impl).width() || h_ > boost::gil::view(*xwin->buff_impl).height())
            {
                int nw = std::max(w_, int(boost::gil::view(*xwin->buff_impl).width())),
                    nh = std::max(h_, int(boost::gil::view(*xwin->buff_impl).height()));

                // round nw/nh up to nearest multiple of MYTHOS_KHAOS_BUFFER_GROW_BY
                nw = nw + MYTHOS_KHAOS_BUFFER_GROW_BY - (nw % MYTHOS_KHAOS_BUFFER_GROW_BY);
                nh = nh + MYTHOS_KHAOS_BUFFER_GROW_BY - (nh % MYTHOS_KHAOS_BUFFER_GROW_BY);

                xwin->buff_impl->recreate(nw, nh, 4);

                // create view w/ windows width/height
                set_buffer_view(xwin);

                BOOST_FOREACH(window & child, xwin->children)
                    set_buffer_child((x11_window *) &child, xwin->tlx, xwin->tly);

                return true;
            }
            else
            {
                // create view w/ windows width/height
                set_buffer_view(xwin);

                return false;
            }
        }

        static void set_buffer(x11_window * xwin)
        {
            x11_window * p = static_cast<x11_window *>(xwin->parent);

            if (xwin->is_toplevel)
            {
                set_buffer_toplevel(xwin, xwin->tlx, xwin->tly, xwin->width, xwin->height);
            }
            else if (p && p->buff_impl)
            {
                // calculate topleft corner of this windows section of the buffer
                int x = xwin->x + p->tlx, y = xwin->y + p->tly;

                // get the toplevel root of this child window
                x11_window * root = p;
                for (; !root->is_toplevel; root = static_cast<x11_window *>(root->parent));

                // if the toplevel window did not resize the window and this current window
                // was a parentless child window, set the child windows of this window
                if (!set_buffer_toplevel(root, x, y, x + xwin->width, y + xwin->height))
                {
                    set_buffer_child(xwin, p->tlx, p->tly);
                }
            }
        }

        // get the mythos window associated w/ an X11 Window
        static x11_window * mythos_window_from_native(Window w)
        {
            XPointer result;

            if (XFindContext(::x11_display, w, ::x11_user_data_context, &result) != 0)
                return NULL;

            return (x11_window *) result;
        }

        static bool is_mythos_window(Window w)
        {
            XPointer result;

            return XFindContext(::x11_display, w, ::x11_user_data_context, &result) != XCNOENT;
        }

        static void draw_view(image_view const& vw, x11_window * xwin)
        {
            // create gc
            GC gc = XCreateGC(::x11_display, xwin->handle, 0, NULL);

            int w = vw.width(), h = vw.height();

            image_view sub_vw;

            // clip view
            if (xwin->is_toplevel)
            {
                sub_vw = vw;
            }
            else
            {
                if (x11_window * parent = static_cast<x11_window *>(xwin->parent))
                {
                    w = std::min(w, parent->width - xwin->x);
                    h = std::min(h, parent->height - xwin->y);
                }

                int tlx = 0, tly = 0;

                if (xwin->x < 0)
                {
                    tlx += -xwin->x;
                    w -= -xwin->x;
                }

                if (xwin->y < 0)
                {
                    tly += -xwin->y;
                    w -= -xwin->y;
                }

                sub_vw = image_view(w, h, vw.xy_at(tlx, tly));
            }

            XImage * img = XCreateImage(
                ::x11_display,
                root_attr.visual,
                root_attr.depth,
                ZPixmap,
                0,
                (char *) boost::gil::interleaved_view_get_raw_data(sub_vw),
                w,
                h,
                32,
                sub_vw.pixels().row_size()
            );

            // FIXME: img will be null if this gets called before event_loop().  is it a bug or a quirk?
            if (img)
            {
                // put image
                XPutImage(::x11_display, xwin->handle, gc, img, 0, 0, 0, 0, w, h);

                img->data = 0;
                XDestroyImage(img);
            }

            // free gc
            XFreeGC(::x11_display, gc);
        }

        static bool raise_event(int id, x11_event_info & ei)
        {
            if (ei.win->handler)
                return ei.win->handler(id, &ei);

            return false;
        }

        static void handle_event(XEvent & event, x11_event_info & ei, x11_window * focus)
        {
            ei.data = &event;

            switch (event.type)
            {
            case ButtonPress:
                switch (event.xbutton.button)
                {
                case Button1: l_button_down::raise(ei.win, point(event.xbutton.x, event.xbutton.y)); break;
                case Button2: m_button_down::raise(ei.win, point(event.xbutton.x, event.xbutton.y)); break;
                case Button3: r_button_down::raise(ei.win, point(event.xbutton.x, event.xbutton.y)); break;
                }

                break;
            case ButtonRelease:
                switch (event.xbutton.button)
                {
                case Button1: l_button_up::raise(ei.win, point(event.xbutton.x, event.xbutton.y)); break;
                case Button2: m_button_up::raise(ei.win, point(event.xbutton.x, event.xbutton.y)); break;
                case Button3: r_button_up::raise(ei.win, point(event.xbutton.x, event.xbutton.y)); break;
                }

                break;
            case MotionNotify: mouse_move::raise(ei.win, point(event.xbutton.x, event.xbutton.y)); break;
            case Expose:
                paint::raise(ei.win);

                break;
            // FIXME: resize & move will only take effect when this event is processed, which can cause
            // problems since many resize/move ops are done before the event loop is started (did I fix this?)
            case ConfigureNotify:
            {
                bool resized = false, moved = false;
                point old_dims;

                // if moved
                if (ei.win->x != event.xconfigure.x || ei.win->y != event.xconfigure.y)
                {
                    ei.win->x = event.xconfigure.x;
                    ei.win->y = event.xconfigure.y;

                    moved = true;
                }

                // if resized
                if (ei.win->width != event.xconfigure.width || ei.win->height != event.xconfigure.height)
                {
                    old_dims.x = ei.win->width;
                    old_dims.y = ei.win->height;

                    ei.win->width = event.xconfigure.width;
                    ei.win->height = event.xconfigure.height;

                    resized = true;
                }

                if (resized || moved)
                    set_buffer(ei.win);

                if (resized)
                {
                    x11_event_info temp_ei = {ei.win, &old_dims, ei.keep_looping};

                    raise_event(evt::resize, temp_ei);
                }

                break;
            }
            case DestroyNotify:
                delete ei.win;

                if (ei.win == focus || toplevel_window_count() == 0)
                    ei.keep_looping = false;

                break;
            case ClientMessage:
                if (event.xclient.message_type == WM_PROTOCOLS)
                {
                    if (event.xclient.format == 32 && event.xclient.data.l[0] == WM_DELETE_WINDOW)
                    {
                        // TODO: should send a delete event
                        destroy_window(ei.win);
                    }
                }

                break;
            default:
                break;
            };
        }
    }

    // main.hpp
    bool entry(int * argc, char *** argv)
    {
        ::x11_display = XOpenDisplay(NULL);

        if (!::x11_display)
            throw std::runtime_error("khaos(x11): failed to open display");

        ::x11_user_data_context = XUniqueContext();

        WM_DELETE_WINDOW = XInternAtom(::x11_display, "WM_DELETE_WINDOW", False);
        WM_PROTOCOLS = XInternAtom(::x11_display, "WM_PROTOCOLS", False);

        Window root = DefaultRootWindow(::x11_display);
        XGetWindowAttributes(::x11_display, root, &::root_attr);

        int screen = DefaultScreen(::x11_display);

        ::parentless_root = XCreateSimpleWindow(
            ::x11_display,
            DefaultRootWindow(::x11_display),
            0, 0, 10, 10,
            5,
            BlackPixel(::x11_display, screen),
            WhitePixel(::x11_display, screen)
        );

        return true;
    }

    void cleanup()
    {
        XCloseDisplay(::x11_display);
    }

    // event_loop.hpp
    int event_loop()
    {
        XEvent event;
        x11_event_info ei = {NULL, &event, true};

        while (ei.keep_looping)
        {
            XNextEvent(::x11_display, &event);

            x11_window * xwin = detail::mythos_window_from_native(event.xany.window);

            ei.win = xwin;

            detail::handle_event(event, ei, NULL);
        }

        return EXIT_SUCCESS;
    }

    // window.hpp
    window * create_toplevel_window(std::string const& title, int x, int y, int cx, int cy, window * p)
    {
        XSetWindowAttributes attributes;
        attributes.event_mask = ::x11_all_events_mask;

        unsigned long valuemask = CWEventMask;

        Window win = XCreateWindow(
            ::x11_display,
            DefaultRootWindow(::x11_display),
            x, y, cx, cy,
            MYTHOS_KHAOS_X11_BORDER_WIDTH,
            CopyFromParent,
            InputOutput,
            CopyFromParent,
            valuemask,
            &attributes
        );

        // set title
        XChangeProperty(
            ::x11_display,
            win,
            XA_WM_NAME,
            XA_STRING,
            8,
            PropModeReplace,
            (unsigned char *) title.data(),
            title.size()
        );

        // make sure we get delete events
        XChangeProperty(
            ::x11_display,
            win,
            WM_PROTOCOLS,
            XA_ATOM,
            32,
            PropModeReplace,
            (unsigned char *) &WM_DELETE_WINDOW,
            1
        );

        x11_window * result = new x11_window(win, p, true, x, y, cx, cy);

        // set window user data
        XSaveContext(::x11_display, win, ::x11_user_data_context, (char *) result);

        return result;
    }

    window * create_child_window(int x, int y, int cx, int cy, window * p)
    {
        x11_window * rp = static_cast<x11_window *>(p);

        XSetWindowAttributes attributes;
        attributes.event_mask = ::x11_child_events_mask;

        unsigned long valuemask = CWEventMask;

        Window win = XCreateWindow(
            ::x11_display,
            rp ? rp->handle : ::parentless_root,
            x, y, cx, cy,
            0,
            CopyFromParent,
            InputOutput,
            CopyFromParent,
            valuemask,
            &attributes
        );

        x11_window * result = new x11_window(win, p, false, x, y, cx, cy);

        // set window user data
        XSaveContext(::x11_display, win, ::x11_user_data_context, (char *) result);

        // map window, if a parent was supplied and is mapped
        if (rp && rp->mapped)
            show_window(result);

        return result;
    }

    window * foreign_create_window(void * win)
    {
        Window handle = *static_cast<Window *>(win);

        BOOST_ASSERT(!detail::is_mythos_window(handle));

        XWindowAttributes attributes;
        XGetWindowAttributes(::x11_display, handle, &attributes);

        Window parent_handle;
        Window * children;
        unsigned int num_children;
        {
            Window root;

            XQueryTree(::x11_display, handle, &root, &parent_handle, &children, &num_children);
        }

        x11_window * parent = NULL;
        bool is_toplevel = parent_handle == attributes.root;

        if (detail::is_mythos_window(parent_handle))
            parent = detail::mythos_window_from_native(parent_handle);
        else if (parent_handle != attributes.root)
            parent = static_cast<x11_window *>(foreign_create_window(&parent_handle));

        // create window
        x11_window * result = new x11_window(
            handle, parent, is_toplevel, attributes.x, attributes.y, attributes.width, attributes.height
        );

        // set window user data
        XSaveContext(::x11_display, handle, ::x11_user_data_context, (char *) result);

        if (children)
        {
            for (unsigned int i = 0; i != num_children; ++i)
            {
                if (detail::is_mythos_window(children[i])) // shouldn't ever come up
                    result->children.push_back(*detail::mythos_window_from_native(children[i]));
                else
                {
                    // the child window is added when the x11_window is created
                    foreign_create_window(&children[i]);
                }
            }

            XFree(children);
        }

        return result;
    }

    // destroys a window
    void destroy_window(window * win)
    {
        x11_window * xwin = static_cast<x11_window *>(win);

        XDestroyWindow(::x11_display, xwin->handle);
    }

    // get window handle, result points to handle
    void * handle_of(window * win)
    {
        return &static_cast<x11_window *>(win)->handle;
    }

    // move/resize
    void move_window(window * win, int x, int y)
    {
        x11_window * xwin = static_cast<x11_window *>(win);

        xwin->x = x;
        xwin->y = y;

        if (!xwin->is_toplevel)
            detail::set_buffer(xwin);

        XMoveWindow(::x11_display, xwin->handle, x, y);
    }

    void resize_window(window * win, int cx, int cy)
    {
        x11_window * xwin = static_cast<x11_window *>(win);

        point old(xwin->width, xwin->height);

        xwin->width = cx;
        xwin->height = cy;

        detail::set_buffer(xwin);

        XResizeWindow(::x11_display, xwin->handle, cx, cy);

        resize::raise(win, old);
    }

    point get_position(window * win)
    {
        return point(static_cast<x11_window *>(win)->x, static_cast<x11_window *>(win)->y);
    }

    point get_size(window * win)
    {
        return point(static_cast<x11_window *>(win)->width, static_cast<x11_window *>(win)->height);
    }

    // show/hide/is_visible
    void show_window(window * win)
    {
        if (static_cast<x11_window *>(win)->mapped)
            return;

        BOOST_FOREACH(window & child, win->children)
            show_window(&child);

        XMapWindow(::x11_display, static_cast<x11_window *>(win)->handle);

        static_cast<x11_window *>(win)->mapped = true;
    }

    void hide_window(window * win)
    {
        if (!static_cast<x11_window *>(win)->mapped)
            return;

        BOOST_FOREACH(window & child, win->children)
            hide_window(&child);

        XUnmapWindow(::x11_display, static_cast<x11_window *>(win)->handle);

        static_cast<x11_window *>(win)->mapped = false;
    }

    bool is_visible(window * win)
    {
        x11_window * xwin = static_cast<x11_window *>(win);

        if (xwin->is_toplevel)
        {
            return xwin->mapped;
        }
        else
        {
            return xwin->parent ? (is_visible(xwin->parent) && xwin->mapped) : xwin->mapped;
        }
    }

    // FIXME: not sure just what XReparentWindow will do to unmapped windows
    // FIXME: rewrite this
    // sets the parent of a child window
    void reparent(window * win, window * parent)
    {
        BOOST_ASSERT(win);
        BOOST_ASSERT(win->is_toplevel ? (parent ? parent->is_toplevel : true) : true);

        x11_window * xwin = static_cast<x11_window *>(win);
        x11_window * xpar = static_cast<x11_window *>(parent);

        xwin->unlink();

        Window phandle;
        Window rootwin = XDefaultRootWindow(::x11_display); // FIXME: should get root of win's screen

        if (parent)
        {
            xwin->parent = xpar;
            xpar->children.push_back(*xwin);

            phandle = xwin->is_toplevel ? rootwin : xpar->handle;
        }
        else
        {
            xwin->parent = NULL;

            phandle = rootwin;
        }

        detail::set_buffer(xwin);

        XReparentWindow(::x11_display, xwin->handle, phandle, xwin->x, xwin->y);
    }

    static bool modal_should_handle(XEvent & event)
    {
        switch (event.type)
        {
        case Expose:
        case DestroyNotify:
        case MotionNotify: // FIXME: should this be here?
            return true;
        default:
            return false;
        }
    }

    void modalize(window * win)
    {
        BOOST_ASSERT(win->is_toplevel);

        show_window(win);

        XEvent event;
        x11_event_info ei = {0, &event, true};

        while (ei.keep_looping)
        {
            XNextEvent(::x11_display, &event);

            ei.win = detail::mythos_window_from_native(event.xany.window);

            window * root = ei.win;
            while (!root->is_toplevel)
                root = root->parent;

            if (win == root || modal_should_handle(event))
                detail::handle_event(event, ei, static_cast<x11_window *>(win));
        }
    }

    bool is_mythos_window(window * win)
    {
        return true;
    }

    // window counting
    // FIXME: BROKEN IN MANY, MANY WAYS.  not sure if there's any way to tell if a Window
    // is toplevel, and not sure if there's any way to tell if a Window was created by this
    // application, so it may not be possible to fix.
    size_t toplevel_window_count()
    {
        return ::toplevel_windows;
    }

    size_t window_count()
    {
        return ::windows;
    }

    // event/button.hpp
    namespace detail
    {
        point pt_from_ei(void * info)
        {
            return *static_cast<point *>(static_cast<x11_event_info *>(info)->data);
        }

        bool button_raise(window * win, point const& pt, int N)
        {
            point data(pt);

            x11_event_info ei = {static_cast<x11_window *>(win), &data, true};

            for (; !ei.win->is_toplevel;)
            {
                if (detail::raise_event(N, ei))
                    return true;

                data.x += ei.win->x;
                data.y += ei.win->y;

                ei.win = static_cast<x11_window *>(ei.win->parent);
            }

            return detail::raise_event(N, ei);
        }
    }

    // event/paint.hpp
    namespace detail
    {
        image_view view_from_event_info(void * info)
        {
            x11_window * win = static_cast<x11_event_info *>(info)->win;

            return win->buffer;
        }

        bool draw_view(image_view const& vw, void * info)
        {
            x11_window * win = static_cast<x11_event_info *>(info)->win;

            detail::draw_view(vw, win);

            return true;
        }
    }

    bool paint::raise(window * win)
    {
        x11_event_info info = {static_cast<x11_window *>(win), NULL, true};

        bool result = detail::raise_event(paint::value, info);

        BOOST_FOREACH(window & child, win->children)
            result |= raise(&child);

        return result;
    }

    // event/resize.hpp
    namespace detail
    {
        point resize_old_width(void * ei)
        {
            return *static_cast<point *>(static_cast<x11_event_info *>(ei)->data);
        }
    }

    bool resize::raise(window * win, point pt)
    {
        x11_event_info ei = {static_cast<x11_window *>(win), &pt, true};

        return detail::raise_event(resize::value, ei);
    }

    // event/reconfigure.hpp
    namespace detail
    {
        point reconfigure_space(void * info)
        {
            return *static_cast<point *>(static_cast<x11_event_info *>(info)->data);
        }
    }

    bool reconfigure::raise(window * win, point pt)
    {
        x11_event_info ei = {static_cast<x11_window *>(win), &pt, true};

        return detail::raise_event(reconfigure::value, ei);
    }
}}

