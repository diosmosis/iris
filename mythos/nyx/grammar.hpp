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

#if !defined( MYTHOS_NYX_GRAMMAR_HPP )
#define MYTHOS_NYX_GRAMMAR_HPP

#include <mythos/nyx/is_directive.hpp>
#include <mythos/nyx/keyword.hpp>
#include <mythos/nyx/window.hpp>
#include <mythos/nyx/generate_window.hpp>

#include <boost/xpressive/proto/proto.hpp>

#include <boost/mpl/placeholders.hpp>

namespace mythos { namespace nyx
{
    struct window;

    template <typename T>
    struct is_column_specifier
        : boost::mpl::false_
    {};

    template <int N>
    struct is_column_specifier<keyword::col<N> >
        : boost::mpl::true_
    {};

    namespace grammar
    {
        struct layout_g;

        struct window_
            : boost::proto::or_<
                window,
                boost::proto::if_<generates_window<boost::proto::_arg0>()>
            >
        {};

        struct container_window
            : boost::proto::subscript<
                window_,
                layout_g
            >
        {};

        struct directive_terminal
            : boost::proto::if_<is_directive<boost::proto::_arg0>()>
        {};

        struct directive
            : boost::proto::subscript<
                directive_terminal,
                layout_g
            >
        {};

        struct column_spec_keyword
            : boost::proto::if_<is_column_specifier<boost::proto::_arg0>()>
        {};

        struct column_spec
            : boost::proto::subscript<
                column_spec_keyword,
                layout_g
            >
        {};

        struct normal_columns
            : boost::proto::bitwise_or<
                boost::proto::or_<
                    normal_columns,
                    boost::proto::and_< // not so clean...
                        layout_g,
                        boost::proto::not_<column_spec>
                    >
                >,
                boost::proto::and_<
                    layout_g,
                    boost::proto::not_<column_spec>
                >
            >
        {};

        struct spec_columns
            : boost::proto::bitwise_or<
                boost::proto::or_<
                    spec_columns,
                    column_spec
                >,
                column_spec
            >
        {};

        struct rows_keyword
            : boost::proto::terminal<keyword::rows>
        {};

        struct rows
            : boost::proto::subscript<
                boost::proto::or_<
                    rows,
                    rows_keyword
                >,
                layout_g
            >
        {};

        struct layout_g
            : boost::proto::or_<
                window_,
                container_window,
                directive,
                normal_columns,
                spec_columns,
                rows
            >
        {};
    }
}}

#endif // #if !defined( MYTHOS_NYX_GRAMMAR_HPP )

