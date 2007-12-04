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

#define MYTHOS_NYX_SOURCE

#include <mythos/nyx/window.hpp>
#include <mythos/nyx/reposition_layout.hpp>

#include <mythos/khaos/event/resize.hpp>

#include <boost/foreach.hpp>

namespace mythos { namespace nyx
{
    namespace detail
    {
        struct event_handler
        {
            event_handler(window const& w, boost::shared_ptr<window::impl_type> const& hr)
                : win(w), hard_ref(hr)
            {}

            bool operator()(int et, void * ei) const
            {
                return handle_event(win, et, ei);
            }

            window win;
            boost::shared_ptr<window::impl_type> hard_ref;
        };

        static window from_khaos(khaos::window * win)
        {
            event_handler * h = win->handler.target<event_handler>();

            BOOST_ASSERT(h);

            return window(h->hard_ref);
        }

        static window from_khaos(khaos::window & win) { return from_khaos(&win); }
    }

    window window::parent() const
    {
        boost::shared_ptr<impl_type> p = pimpl.lock();

        return p ? detail::from_khaos(p->win->parent) : window();
    }

    // create_toplevel_window
    window create_toplevel_window(std::string const& title, int x, int y, int cx, int cy, window const& parent)
    {
        // create impl_type
        boost::shared_ptr<window::impl_type> pimpl(new window::impl_type());

        // create window
        pimpl->win = khaos::create_toplevel_window(title, x, y, cx, cy, parent.khaos_window());

        // create/set event handler
        pimpl->win->handler = detail::event_handler(window(pimpl), pimpl);

        return window(pimpl);
    }

    // create_child_window
    window create_child_window(int x, int y, int cx, int cy, window const& parent)
    {
        // create impl_type
        boost::shared_ptr<window::impl_type> pimpl(new window::impl_type());

        // create window
        pimpl->win = khaos::create_child_window(x, y, cx, cy, parent.khaos_window());

        // create/set event handler
        pimpl->win->handler = detail::event_handler(window(pimpl), pimpl);

        return window(pimpl);
    }

    // foreign_create_window
    window foreign_create_window(void * win)
    {
        // create impl_type
        boost::shared_ptr<window::impl_type> pimpl(new window::impl_type());

        // create window
        pimpl->win = khaos::foreign_create_window(win);

        // create/set event handler
        pimpl->win->handler = detail::event_handler(window(pimpl), pimpl);

        return window(pimpl);
    }

    // handle_event
    bool handle_event(window const& win, int et, void * ei)
    {
        bool result = false;

        boost::shared_ptr<window::impl_type> pimpl = win.pimpl.lock();

        if (et == khaos::resize::value && win.khaos_window()->is_toplevel)
        {
            point oldw = khaos::detail::resize_old_width(ei);

            reposition_layout(win, oldw.x, oldw.y);
        }

        if (pimpl->main)
            result |= pimpl->main(et, ei);

        BOOST_FOREACH(iris::event_handler const& handler, pimpl->handlers)
        {
            result |= handler(et, ei);
        }

        return result;
    }

    // resize_window
    void resize_window(window const& win, int cx, int cy)
    {
        khaos::resize_window(win.khaos_window(), cx, cy);
    }

    boost::iterator_range<child_iterator> children_of(window const& win)
    {
        khaos::window * w = win.khaos_window();

        return boost::iterator_range<child_iterator>(w->children.begin(), w->children.end());
    }

    window child_iterator::dereference() const
    {
        return detail::from_khaos(*base());
    }
}}

