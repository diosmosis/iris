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

#if !defined( MYTHOS_IRIS_DSEL_HPP )
#define MYTHOS_IRIS_DSEL_HPP

#include <mythos/iris/event_handler.hpp>
#include <mythos/iris/grammar.hpp>
#include <mythos/iris/event_list.hpp>
#include <mythos/iris/run_action.hpp>

#include <mythos/khaos/event_from_id.hpp>

#include <boost/xpressive/proto/deep_copy.hpp>

#include <boost/switch.hpp>

namespace mythos { namespace iris
{
    template <typename Expr>
    struct run_action_impl
    {
        typedef bool result_type;

        run_action_impl(void * ei_, Expr const& expr_)
            : ei(ei_), expr(expr_)
        {}

        template <typename C>
        bool operator()(C) const
        {
            typedef typename khaos::event_from_id<C::value>::type event_type;

            return run_action<event_type>(expr, ei);
        }

        void * ei;
        Expr const& expr;
    };

    struct evt_def
    {
        typedef bool result_type;

        template <typename C> bool operator()(C c) const { return false; }
    };

    template <typename Expr>
    struct gen_event_handler
    {
        typedef typename boost::proto::result_of::deep_copy<Expr>::type expr_type;
        typedef typename event_list::apply<Expr, int, int>::type cases;

        gen_event_handler(Expr const& x) : expr(boost::proto::deep_copy(x)) {}

        bool operator()(int et, void * ei) const
        {
            return boost::switch_<cases>(et, run_action_impl<expr_type>(ei, expr), evt_def());
        }

        expr_type expr;
    };

    template <typename Expr>
    inline event_handler::event_handler(
        Expr const& expr, typename boost::enable_if<boost::proto::is_expr<Expr> >::type * d
    )
    {
        BOOST_MPL_ASSERT((boost::proto::matches<Expr, grammar::event_handler>));

        impl = gen_event_handler<Expr>(expr);
    }
}}

#endif // #if !defined( MYTHOS_IRIS_DSEL_HPP )

