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

#if !defined( MYTHOS_NYX_LAYOUT_HPP )
#define MYTHOS_NYX_LAYOUT_HPP

#include <mythos/nyx/config.hpp>
#include <mythos/nyx/cell.hpp>
#include <mythos/nyx/window.hpp>

#include <boost/noncopyable.hpp>

#include <vector>
#include <deque>

namespace mythos { namespace nyx
{
    struct directive_action
    {
        cell * x;
        void * data;

        void (*func)(void *, cell &);
    };

    struct mapped_win
    {
        mapped_win(cell * c_, window const& w, window const& p = window())
            : c(c_), win(w), parent(p)
        {}

        cell * c;
        window win, parent;
    };

    struct layout_data : boost::noncopyable
    {
        layout_data(size_t item_count, size_t dir_count)
        {
            items.reserve(item_count);
            dactions.reserve(dir_count);
        }

        MYTHOS_NYX_DECL cell & make_cell(int x, int y, cell & p);
        MYTHOS_NYX_DECL cell & get_specific_column(cell & cols, int N);

        // TODO: should ALL be arrays (layout_data should be templated)
        std::vector<mapped_win> items;
        std::vector<directive_action> dactions;
        std::deque<cell> cells;

        cell root;

        int xpad, ypad;
    };

    struct layout_impl
    {
        layout_impl(
            nyx::window const& x,
            int xpad_,
            int ypad_
        ) : win(x), xpad(xpad_), ypad(ypad_)
        {}

        // defined in dsel.hpp
        template <typename Expr>
        void operator = (Expr const& expr);

        nyx::window win;
        int xpad, ypad;
    };

    inline layout_impl layout(
        nyx::window const& x,
        int xpad = MYTHOS_NYX_CELL_X_PAD,
        int ypad = MYTHOS_NYX_CELL_Y_PAD
    )
    {
        return layout_impl(x, xpad, ypad);
    }
}}

#endif // #if !defined( MYTHOS_NYX_LAYOUT_HPP )

