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

#include <mythos/nyx/set_window.hpp>
#include <mythos/nyx/layout.hpp>

#include <boost/foreach.hpp>

namespace mythos { namespace nyx
{
    void set_window(layout_data & lay, window & w)
    {
        BOOST_ASSERT(lay.items.size() >= 1);

        std::vector<mapped_win>::iterator i = lay.items.begin() + 1;

        for (; i != lay.items.end(); ++i)
        {
            BOOST_ASSERT(i->parent);

            khaos::set_parent(i->win.khaos_window(), i->parent.khaos_window());
            nyx::move_window(i->win, i->c->x1 - i->c->parent->x1, i->c->y1 - i->c->parent->y1);
        }

        w = lay.items[0].win;
    }
}}

