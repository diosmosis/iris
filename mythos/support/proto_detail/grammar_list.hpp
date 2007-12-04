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

#if !defined( MYTHOS_SUPPORT_PROTO_DETAIL_GRAMMAR_LIST_HPP )
#define MYTHOS_SUPPORT_PROTO_DETAIL_GRAMMAR_LIST_HPP

#include <boost/xpressive/proto/proto.hpp>
#include <boost/xpressive/proto/transform/fold.hpp>
#include <boost/xpressive/proto/transform/apply.hpp>

#include <boost/mpl/list.hpp>
#include <boost/mpl/end.hpp>
#include <boost/mpl/insert_range.hpp>
#include <boost/mpl/placeholders.hpp>

namespace mythos { namespace proto_detail
{
    using namespace boost::proto;
    namespace mplpl = boost::mpl::placeholders;

    struct grammar_list_end
    {
        template <typename E>
        struct apply
            : boost::mpl::list<typename result_of::arg<E>::type>
        {};
    };

    template <typename G>
    struct grammar_list
        : or_<
            transform::apply1<G, grammar_list_end>,
            transform::always<
                terminal<_>,
                boost::mpl::list<>
            >,
            transform::fold<
                nary_expr<
                    _,
                    transform::apply2<
                        vararg<grammar_list<G> >,
                        boost::mpl::insert_range<
                            mplpl::_2,
                            boost::mpl::end<mplpl::_2>,
                            mplpl::_1
                        >
                    >
                >,
                boost::mpl::list<>
            >
        >
    {};
}}

#endif // #if !defined( MYTHOS_SUPPORT_PROTO_DETAIL_GRAMMAR_LIST_HPP )

