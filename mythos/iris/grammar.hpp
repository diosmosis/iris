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

#if !defined( MYTHOS_IRIS_GRAMMAR_HPP )
#define MYTHOS_IRIS_GRAMMAR_HPP

#include <mythos/iris/is_event.hpp>
#include <mythos/iris/context.hpp>

#include <boost/xpressive/proto/proto.hpp>

#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/bool.hpp>

namespace mythos { namespace iris
{
    struct event_handler;

    template <typename T>
    struct is_context_terminal
        : boost::mpl::false_
    {};

    template <typename T>
    struct is_context_terminal<T const>
        : is_context_terminal<T>
    {};

    template <typename T>
    struct is_context_terminal<T *>
        : is_context_terminal<T>
    {};

    template <typename S>
    struct is_context_terminal<context_impl<S> >
        : boost::mpl::true_
    {};

    namespace grammar
    {
        struct event_handler;

        struct event_terminal
            : boost::proto::if_<is_event<boost::proto::_arg0>()>
        {};

        struct event_composition
            : boost::proto::bitwise_or<
                boost::proto::or_<
                    event_composition,
                    event_terminal
                >,
                event_terminal
            >
        {};

        struct event
            : boost::proto::or_<
                boost::proto::subscript<
                    boost::proto::or_<
                        event_terminal,
                        event_composition
                    >,
                    boost::proto::_
                >,
                boost::proto::terminal<iris::event_handler *> // event_handler uses extends w/ event_handler *
            >
        {};

        struct event_list
            : boost::proto::comma<
                boost::proto::or_<
                    event_list,
                    event
                >,
                event
            >
        {};

        struct context_terminal
            : boost::proto::if_<is_context_terminal<boost::proto::_arg0>()>
        {};

        struct context
            : boost::proto::subscript<
                context_terminal,
                boost::proto::or_<
                    event,
                    event_list
                >
            >
        {};

        struct event_handler
            : boost::proto::or_<
                event,
                event_list,
                context,
                // FIXME: the following should not be allowed, but w/o it proto won't let
                // events be |'d, since the expr that results from (A | B) is not by itself
                // a valid event_handler expr
                event_composition
            >
        {};
    }
}}

#endif // #if !defined( MYTHOS_IRIS_GRAMMAR_HPP )

