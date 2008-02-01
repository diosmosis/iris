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

#if !defined( MYTHOS_NYX_WINDOW_HPP )
#define MYTHOS_NYX_WINDOW_HPP

#include <mythos/nyx/config.hpp>

#include <mythos/nyx/domain.hpp>
#include <mythos/nyx/point.hpp>

#include <mythos/iris/event_handler.hpp>

#include <mythos/khaos/point.hpp>
#include <mythos/khaos/window.hpp>

// nuts.
#include <boost/xpressive/proto/proto.hpp>

#include <boost/intrusive/list.hpp>

#include <boost/range/iterator_range.hpp>

#include <boost/iterator/iterator_adaptor.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace mythos { namespace nyx
{
    // FIXME: the extends adds another (unused) pointer to window.  get rid of it.
    struct window
        : boost::proto::extends<boost::proto::terminal<window *>::type, window, domain>
    {
        struct impl_type
        {
            khaos::window * win;
            iris::event_handler main;

            boost::intrusive::list<
                iris::event_handler,
                boost::intrusive::member_hook<
                    iris::event_handler,
                    iris::event_handler::hook_type,
                    &iris::event_handler::hook
                >,
                boost::intrusive::constant_time_size<false>
            > handlers;
        };

        typedef khaos::window * impl_type::* unspecified_bool_t;

        window() : pimpl() {}
        explicit window(boost::shared_ptr<impl_type> const& p) : pimpl(p) {};
        window(window const& x) : pimpl(x.pimpl) {}

        // defined in dsel.hpp
        template <typename Expr>
        window(Expr const& x, typename boost::enable_if<boost::proto::is_expr<Expr> >::type * d = 0);

        window & operator = (window const& x)
        {
            pimpl = x.pimpl;
            return *this;
        }

        template <typename Expr>
        window & operator = (Expr const& x)
        {
            *this = window(x);
            return *this;
        }

        khaos::window * khaos_window() const
        {
            boost::shared_ptr<impl_type> p = pimpl.lock();

            return p ? p->win : 0;
        }

        window parent() const;

        operator unspecified_bool_t() const
        {
            return pimpl.expired() ? 0 : &impl_type::win;
        }

        void connect(iris::event_handler & evts) const
        {
            boost::shared_ptr<impl_type> p = pimpl.lock();

            BOOST_ASSERT(p);

            p->handlers.push_back(evts);
        }

        void disconnect(iris::event_handler & evts) const
        {
            evts.disconnect();
        }

        iris::event_handler & events() const
        {
            boost::shared_ptr<impl_type> p = pimpl.lock();

            BOOST_ASSERT(p);

            return p->main;
        }

        boost::weak_ptr<impl_type> pimpl;
    };

    struct child_iterator
        : boost::iterator_adaptor<
            child_iterator,
            khaos::window::child_iterator,
            window,
            boost::use_default,
            window
        >
    {
        typedef child_iterator::iterator_adaptor_ base_type;
        typedef khaos::window::child_iterator adapted;

        child_iterator() : base_type() {}
        child_iterator(child_iterator const& x) : base_type(x) {}
        child_iterator(adapted const& x) : base_type(x) {}

    protected:
        friend class boost::iterator_core_access;

        // in window.cpp
        MYTHOS_NYX_DECL window dereference() const;
    };

    // create_toplevel_window
    MYTHOS_NYX_DECL window create_toplevel_window(
        std::string const& title, int x, int y, int cx, int cy, window const& parent = window()
    );

    // create_child_window
    MYTHOS_NYX_DECL window create_child_window(
        int x, int y, int cx, int cy, window const& parent = window()
    );

    // foreign_create_window
    MYTHOS_NYX_DECL window foreign_create_window(void * win);

    // destroy_window
    inline void destroy_window(window const& x)
    {
        khaos::destroy_window(x.khaos_window());
    }

    // move_window
    inline void move_window(window const& win, int x, int y)
    {
        khaos::move_window(win.khaos_window(), x, y);
    }

    // get_position
    inline point get_position(window const& win)
    {
        return khaos::get_position(win.khaos_window());
    }

    // resize_window
    MYTHOS_NYX_DECL void resize_window(window const& win, int cx, int cy);

    // get_size
    inline point get_size(window const& win)
    {
        return khaos::get_size(win.khaos_window());
    }

    // show/hide_window
    inline void show_window(window const& win)
    {
        return khaos::show_window(win.khaos_window());
    }

    inline void hide_window(window const& win)
    {
        return khaos::hide_window(win.khaos_window());
    }

    // is_shown
    inline bool is_shown(window const& win)
    {
        return khaos::is_shown(win.khaos_window());
    }

    // modalize
    inline void modalize(window const& win)
    {
        khaos::modalize(win.khaos_window());
    }

    // set_parent
    inline void set_parent(window const& win, window const& par)
    {
        khaos::set_parent(win.khaos_window(), par.khaos_window());
    }

    // handle_event
    MYTHOS_NYX_DECL bool handle_event(window const& win, int et, void * ei);

    // children_of
    MYTHOS_NYX_DECL boost::iterator_range<child_iterator> children_of(window const& win);
}}

#endif // #if !defined( MYTHOS_NYX_WINDOW_HPP )

