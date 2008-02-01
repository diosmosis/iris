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

#if !defined( MYTHOS_NYX_DSEL_HPP )
#define MYTHOS_NYX_DSEL_HPP

#include <mythos/nyx/window.hpp>
#include <mythos/nyx/grammar.hpp>
#include <mythos/nyx/window_count.hpp>
#include <mythos/nyx/directive_count.hpp>
#include <mythos/nyx/fill_layout.hpp>
#include <mythos/nyx/fill_window.hpp>
#include <mythos/nyx/set_window.hpp>
#include <mythos/nyx/layout.hpp>

#include <mythos/nyx/directive.hpp>

#include <boost/xpressive/proto/proto.hpp>

#include <boost/mpl/assert.hpp>

namespace mythos { namespace nyx
{
    template <typename Expr>
    inline void layout_impl::operator = (Expr const& expr)
    {
        BOOST_MPL_ASSERT((boost::proto::matches<Expr, grammar::layout_g>));

        layout_data lay(
            boost::result_of<window_count(Expr, int, int)>::type::value,
            boost::result_of<directive_count(Expr, int, int)>::type::value
        );

        lay.xpad = xpad;
        lay.ypad = ypad;

        fill_layout(lay, expr);
        fill_window(lay, win);
    }

    template <typename Expr>
    inline window::window(Expr const& x, typename boost::enable_if<boost::proto::is_expr<Expr> >::type * d)
    {
        BOOST_MPL_ASSERT((boost::proto::matches<Expr, grammar::container_window>));

        layout_data lay(
            boost::result_of<window_count(Expr, int, int)>::type::value,
            boost::result_of<directive_count(Expr, int, int)>::type::value
        );

        // FIXME: should be customizable...
        lay.xpad = MYTHOS_NYX_CELL_X_PAD;
        lay.ypad = MYTHOS_NYX_CELL_Y_PAD;

        fill_layout(lay, x);
        set_window(lay, *this);
    }
}}

#endif // #if !defined( MYTHOS_NYX_DSEL_HPP )

