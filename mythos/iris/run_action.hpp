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

#if !defined( MYTHOS_IRIS_RUN_ACTION_HPP )
#define MYTHOS_IRIS_RUN_ACTION_HPP

#include <mythos/iris/grammar.hpp>

#include <boost/xpressive/proto/proto.hpp>

#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/equal_to.hpp>
#include <boost/mpl/or.hpp>

#include <boost/utility/enable_if.hpp>

namespace mythos { namespace iris
{
    // metafunctions
    template <typename N>
    struct is_event_term
        : boost::proto::if_<boost::is_same<N, boost::proto::_arg0>()>
    {};

    template <typename N>
    struct is_correct_evt_g
        : boost::proto::subscript<
            boost::proto::or_<
                boost::proto::bitwise_or<
                    boost::proto::_,
                    is_event_term<N>
                >,
                boost::proto::bitwise_or<
                    is_correct_evt_g<N>,
                    boost::proto::_
                >,
                is_event_term<N>
            >,
            boost::proto::_
        >
    {};

    template <typename N, typename E>
    struct is_correct_evt
        : boost::proto::matches<E, is_correct_evt_g<N> >
    {};

    template <typename E>
    struct is_evt_list
        : boost::proto::matches<E, grammar::event_list>
    {};

    template <typename E>
    struct is_ctx
        : boost::proto::matches<E, grammar::context>
    {};

    // event_handler
    template <typename N>
    inline bool run_action(event_handler const& handler, void * ei)
    {
        return handler(N::value, ei);
    }

    // subscript(A | B..., ...)
    template <typename N, typename E>
    inline bool run_action(E const& expr, void * ei,
        typename boost::enable_if<is_correct_evt<N, E> >::type * d = 0)
    {
        return N::on_event(boost::proto::arg(boost::proto::right(expr)), ei);
    }

    template <typename N, typename E>
    inline bool run_action(E const&, void *,
        typename boost::disable_if<
            boost::mpl::or_<is_correct_evt<N, E>, is_evt_list<E>, is_ctx<E> >
        >::type * d = 0
    )
    {
        return false;
    }

    // comma(..., ...)
    template <typename N, typename E>
    inline bool run_action(E const& expr, void * ei, typename boost::enable_if<is_evt_list<E> >::type * d = 0)
    {
        return run_action<N>(boost::proto::left(expr), ei) | run_action<N>(boost::proto::right(expr), ei);
    }

    // context versions
    // event_handler
    template <typename N, typename S>
    inline bool run_action(event_handler const& handler, void * ei, S &)
    {
        return handler(N::value, ei);
    }

    // subscript(A | B..., ...)
    template <typename N, typename E, typename S>
    inline bool run_action(E const& expr, void * ei, S & s,
        typename boost::enable_if<is_correct_evt<N, E> >::type * d = 0)
    {
        // FIXME: shouldn't impose the state requirement on khaos
        return N::on_event(boost::proto::arg(boost::proto::right(expr)), ei, s);
    }

    template <typename N, typename E, typename S>
    inline bool run_action(E const&, void *, S &,
        typename boost::disable_if<
            boost::mpl::or_<is_correct_evt<N, E>, is_evt_list<E>, is_ctx<E> >
        >::type * d = 0
    )
    {
        return false;
    }

    // comma(..., ...)
    template <typename N, typename E, typename S>
    inline bool run_action(E const& expr, void * ei, S & s, typename boost::enable_if<is_evt_list<E> >::type * d = 0)
    {
        return run_action<N>(boost::proto::left(expr), ei, s) | run_action<N>(boost::proto::right(expr), ei, s);
    }

    // context(ctx[...])
    template <typename N, typename E>
    inline bool run_action(E const& expr, void * ei, typename boost::enable_if<is_ctx<E> >::type * d = 0)
    {
        return run_action<N>(boost::proto::right(expr), ei, boost::proto::arg(boost::proto::left(expr)).state);
    }
}}

#endif // #if !defined( MYTHOS_IRIS_RUN_ACTION_HPP )

