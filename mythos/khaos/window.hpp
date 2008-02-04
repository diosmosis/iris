/*
    This file is part of mythos.

    Copyright (c) 2007-2008 Benaka Moorthi

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

#if !defined( MYTHOS_KHAOS_WINDOW_HPP )
#define MYTHOS_KHAOS_WINDOW_HPP

#include <mythos/khaos/config.hpp>

#include <mythos/khaos/point.hpp>
#include <mythos/khaos/event_handler.hpp>

#include <boost/intrusive/list.hpp>

#include <boost/noncopyable.hpp>

// NOTE: coordinates are related to parent window (unless the window is toplevel)
namespace mythos { namespace khaos
{
    // TODO: use constant-time size() in intrusive list of child windows

    // implementations derive from this.  since there will only be one derived class,
    // there is no virtual destructor.
    struct window
        : boost::noncopyable,
          boost::intrusive::list_base_hook<
              boost::intrusive::link_mode<boost::intrusive::auto_unlink>
          >
    {
        typedef boost::intrusive::list_base_hook<
            boost::intrusive::link_mode<boost::intrusive::auto_unlink>
        > hook_type;

        typedef boost::intrusive::list<window, boost::intrusive::constant_time_size<false> > list_type;

        typedef list_type::iterator child_iterator;

        window(window * p, bool is_toplevel_) : parent(p), is_toplevel(is_toplevel_)
        {
            if (parent)
                parent->children.push_back(*this);
        }

        window * parent;
        event_handler handler;

        list_type children;

        bool is_toplevel;
    };

    // creates a toplevel window
    MYTHOS_KHAOS_DECL window * create_toplevel_window(
        std::string const& title, int x, int y, int cx, int cy, window * p = NULL
    );

    // creates a child window that is shown, if parent is valid & shown.
    MYTHOS_KHAOS_DECL window * create_child_window(
        int x, int y, int cx, int cy, window * p = NULL
    );

    // wraps native window as mythos window.  win points to the handle
    MYTHOS_KHAOS_DECL window * foreign_create_window(void * win);

    // destroys a window
    MYTHOS_KHAOS_DECL void destroy_window(window * win);

    // get window handle, result points to handle
    MYTHOS_KHAOS_DECL void * handle_of(window * win);

    // move/resize
    MYTHOS_KHAOS_DECL void move_window(window * win, int x, int y);
    MYTHOS_KHAOS_DECL void resize_window(window * win, int cx, int cy);

    MYTHOS_KHAOS_DECL point get_position(window * win);
    MYTHOS_KHAOS_DECL point get_size(window * win);

    // show/hide/is_visible
    MYTHOS_KHAOS_DECL void show_window(window * win);
    MYTHOS_KHAOS_DECL void hide_window(window * win);

    MYTHOS_KHAOS_DECL bool is_visible(window * win);

    // reparent
    // win->is_toplevel must == parent->is_toplevel
    MYTHOS_KHAOS_DECL void reparent(window * win, window * parent);

    MYTHOS_KHAOS_DECL void modalize(window * win);

    // tells if the supplied window is a mythos window.  calling this with windows created by
    // create_..._window functions will return true.  calling this w/ windows created by
    // foreign_create_window may return true or false depending on the particular backend.
    MYTHOS_KHAOS_DECL bool is_mythos_window(window * win);

    // counting functions
    MYTHOS_KHAOS_DECL size_t toplevel_window_count();
    MYTHOS_KHAOS_DECL size_t window_count();
}}

#endif // #if !defined( MYTHOS_KHAOS_WINDOW_HPP )

