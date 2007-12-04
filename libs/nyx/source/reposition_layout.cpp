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

#include <mythos/nyx/reposition_layout.hpp>

#include <mythos/khaos/event/reconfigure.hpp>

#include <boost/foreach.hpp>

namespace mythos { namespace nyx
{
    void reposition_layout(window const& win, int oldw, int oldh)
    {
        point nw = get_size(win);

        BOOST_FOREACH(window child, children_of(win))
        {
            point op = get_position(child);
            point size = get_size(child);

            // new position of child window
            int newx = (op.x * nw.x) / oldw;
            int newy = (op.y * nw.y) / oldh;

            // the space the window has to grow
            point space(
                (size.x * nw.x) / oldw,
                (size.y * nw.y) / oldh
            );

            // move the window
            move_window(child, newx, newy);

            // send a reconfigure event
            khaos::reconfigure::raise(child.khaos_window(), space);

            // if the size has not changed, move the window over more
            point nsize = get_size(child);

            if (nsize.x == size.x && nsize.y == size.y)
            {
                move_window(child, space.x + newx - size.x, space.y + newy - size.y);
            }
        }
    }
}}

