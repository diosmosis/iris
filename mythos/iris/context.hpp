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

#if !defined( MYTHOS_IRIS_CONTEXT_HPP )
#define MYTHOS_IRIS_CONTEXT_HPP

#include <boost/xpressive/proto/proto.hpp>
#include <boost/xpressive/proto/extends.hpp>

namespace mythos { namespace iris
{
    template <typename S>
    struct context_impl
    {
        context_impl(S const& s) : state(s) {}

        // HACK!!!
        mutable S state;
    };

    template <typename S>
    struct context_terminal
        : boost::proto::extends<typename boost::proto::terminal<context_impl<S> >::type, context_terminal<S> >
    {
        typedef typename boost::proto::terminal<context_impl<S> >::type expr_type;
        typedef boost::proto::extends<expr_type, context_terminal> base_type;

        context_terminal(S const& s) : base_type(expr_type::make(s)) {}
    };

    template <typename S>
    inline context_terminal<S> context(S const& s)
    {
        return context_terminal<S>(s);
    }
}}

#endif // #if !defined( MYTHOS_IRIS_CONTEXT_HPP )

