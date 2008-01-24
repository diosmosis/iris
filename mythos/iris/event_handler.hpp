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

#if !defined( MYTHOS_IRIS_EVENT_HANDLER_HPP )
#define MYTHOS_IRIS_EVENT_HANDLER_HPP

#include <mythos/iris/config.hpp>

#include <mythos/khaos/event_handler.hpp>

#include <mythos/support/safe_bool.hpp>

#include <boost/xpressive/proto/traits.hpp>

#include <boost/intrusive/list_hook.hpp>

#include <algorithm>

namespace mythos { namespace iris
{
    namespace detail
    {
        inline void swap_handlers(khaos::event_handler & x, khaos::event_handler & y)
        {
            using namespace std;
            swap(x, y);
        }
    }

    // TODO: event_handlers should be considered iris primitives
    // TODO: get rid of parentheses needed due to ',' operator
    struct event_handler
    {
        typedef boost::intrusive::list_member_hook<
            boost::intrusive::link_mode<boost::intrusive::auto_unlink>
        > hook_type;

        event_handler() : impl() {}

        // defined in dsel.hpp
        template <typename Expr>
        event_handler(
            Expr const& expr,
            typename boost::enable_if<
                boost::proto::is_expr<Expr>
            >::type * d = 0
        );

        template <typename F>
        event_handler(
            F const& f,
            typename boost::disable_if<
                boost::proto::is_expr<F>
            >::type * d = 0
        ) : impl(f)
        {}

        template <typename Expr>
        event_handler & operator = (Expr const& x)
        {
            event_handler(x).swap(*this);

            return *this;
        }

        void swap(event_handler & x)
        {
            detail::swap_handlers(x.impl, impl);
        }

        bool operator()(int et, void * ei) const
        {
            return impl(et, ei);
        }

        MYTHOS_SAFE_BOOL_OPERATOR(impl)

        void disconnect()
        {
            hook.unlink();
        }

        khaos::event_handler impl;
        hook_type            hook;
    };
}}

#endif // #if !defined( MYTHOS_IRIS_EVENT_HANDLER_HPP )

