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

#if !defined( MYTHOS_NYX_KEYWORD_HPP )
#define MYTHOS_NYX_KEYWORD_HPP

#include <mythos/nyx/config.hpp>
#include <mythos/nyx/terminal.hpp>

#include <boost/xpressive/proto/proto.hpp>

#include <boost/preprocessor/repetition/repeat.hpp>

#define MYTHOS_NYX_COL(z, n, data)                  \
    terminal<keyword::col<n> > const                \
        BOOST_PP_CAT(col, n);

namespace mythos { namespace nyx
{
    namespace keyword
    {
        struct rows {};

        template <int N>
        struct col
            : boost::mpl::int_<N>
        {};
    }

    terminal<keyword::rows> const rows;

    BOOST_PP_REPEAT(MYTHOS_NYX_MAX_COLS, MYTHOS_NYX_COL, _)
}}

#undef MYTHOS_NYX_COL

#endif // #if !defined( MYTHOS_NYX_KEYWORD_HPP )

