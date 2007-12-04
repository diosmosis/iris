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

#include <mythos/nyx/cell.hpp>

#include <boost/foreach.hpp>

#include <algorithm>
#include <iostream>
namespace mythos { namespace nyx
{
    namespace detail
    {
        struct cell_x2_comp
        {
            bool operator()(cell const& l, cell const& r) const
            {
                return l.x2 < r.x2;
            }
        };

        struct cell_y2_comp
        {
            bool operator()(cell const& l, cell const& r) const
            {
                return l.y2 < r.y2;
            }
        };
    }

    cell::cell(cell * p)
        : x1(0), y1(0), x2(0), y2(0), parent(p)
    {
        if (parent)
            parent->children.push_back(*this);
    }

    cell::cell(cell const& x)
        : x1(x.x1), y1(x.y1), x2(x.x2), y2(x.y2), parent(x.parent), children()
    {
        BOOST_ASSERT(x.children.empty());

        if (parent)
            parent->children.push_back(*this);
    }

    // FIXME: too recursive
    void cell::move_by(int cx, int cy)
    {
        if (parent)
        {
            child_iterator i = parent->children.iterator_to(*this); ++i;
            if (i != parent->children.end())
            {
                i->move_by(cx, cy);
            }
        }

        if (!children.empty())
            children.front().move_by(cx, cy);

        x1 += cx;
        x2 += cx;

        y1 += cy;
        y2 += cy;
    }

    void cell::move_to(int tox, int toy)
    {
        BOOST_ASSERT(tox >= x1 && toy >= y1);

        move_by(tox - x1, toy - y1);
    }

    void cell::grow_by(int cx, int cy)
    {
        grow_to(x2 + cx, y2 + cy);
    }

    void cell::grow_to(int tox, int toy)
    {
        BOOST_ASSERT(tox >= x2 && toy >= y2);

        int dw = tox - x2;
        int dh = toy - y2;

        if (parent)
        {
            child_iterator i = parent->children.iterator_to(*this); ++i;
            for (; i != parent->children.end(); ++i)
            {
                i->x1 += dw;
                i->x2 += dw;

                i->y1 += dh;
                i->y2 += dh;
            }

            parent->grow_to(std::max(tox, parent->x2), std::max(toy, parent->y2));
        }

        x2 = tox;
        y2 = toy;
    }

    int cell::max_child_x2()
    {
        detail::cell_x2_comp comp;
        return children.empty() ? 0 : std::max_element(children.begin(), children.end(), comp)->x2;
    }

    int cell::max_child_y2()
    {
        detail::cell_y2_comp comp;
        return children.empty() ? 0 : std::max_element(children.begin(), children.end(), comp)->y2;
    }
}}

