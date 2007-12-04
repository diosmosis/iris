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

#define MYTHOS_NYX_SOURCE

#include <mythos/nyx/fill_layout.hpp>

namespace mythos { namespace nyx
{
    // layout.hpp
    cell & layout_data::make_cell(int x, int y, cell & p)
    {
        cells.push_back(cell(&p));

        cell & result = cells.back();
        result.x1 = result.x2 = x;
        result.y1 = result.y2 = y;

        return result;
    }

    cell & layout_data::get_specific_column(cell & cols, int N)
    {
        int i = 0;

        for (cell::child_iterator ci = cols.children.begin(); ci != cols.children.end(); ++ci)
            if (++i == N)
                return *ci;

        for (; i != N; ++i)
            make_cell(cols.x1, cols.y1, cols);

        return cols.children.back();
    }

    // fill_layout.hpp
    namespace detail
    {
        void place_x_cols(layout_data & lay, cell & cols)
        {
            cell::child_iterator i = cols.children.begin(), last = cols.children.begin();
            ++i;

            int x1 = cols.x1;
            for (; i != cols.children.end(); ++i, ++last)
            {
                x1 += last->max_child_x2();

                i->move_to(x1, i->y1);
            }

            cols.grow_to(last->max_child_x2(), cols.y2);
        }
    }

    void fill_layout(layout_data & lay, cell & c, window const& win)
    {
        point pt = nyx::get_size(win);

        c.grow_by(pt.x + lay.xpad, pt.y + lay.ypad);

        lay.items.push_back(mapped_win(&c, win, window()));
    }
}}

