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

#include <mythos/nyx/fill_window.hpp>
#include <mythos/nyx/layout.hpp>

#include <boost/foreach.hpp>

namespace mythos { namespace nyx
{
    // FIXME: should get rid of any children w has
    void fill_window(layout_data & lay, window & w)
    {
        nyx::resize_window(
            w, lay.root.width() + lay.xpad, lay.root.height() + lay.ypad + MYTHOS_KHAOS_WINDOW_SIZE_ADD
        );

        BOOST_FOREACH(mapped_win const& vt, lay.items)
        {
            if (vt.parent)
            {
                khaos::reparent(vt.win.khaos_window(), vt.parent.khaos_window());
                nyx::move_window(vt.win, vt.c->x1 - vt.c->parent->x1, vt.c->y1 - vt.c->parent->y1);
            }
            else
            {
                khaos::reparent(vt.win.khaos_window(), w.khaos_window());
                nyx::move_window(vt.win, vt.c->x1 + lay.xpad, vt.c->y1 + lay.ypad);
            }
        }
    }
}}

