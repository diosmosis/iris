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

#include <mythos/nyx/directive.hpp>

// FIXME: does not take padding into account
namespace mythos { namespace nyx { namespace directive
{
    namespace detail
    {
        static int move_right_pos(cell * p, cell * gp, cell & c)
        {
            if (gp)
            {
                int tox = gp->max_child_x2();
                for (cell::child_iterator i = p->children.iterator_to(c); i != p->children.end(); ++i)
                    tox -= i->width();

                return tox;
            }
            else
            {
                return p->x2 - c.width();
            }
        }

        static int move_bottom_pos(cell * p, cell * gp, cell & c)
        {
            if (gp)
            {
                int toy = gp->max_child_y2();
                for (cell::child_iterator i = p->children.iterator_to(c); i != p->children.end(); ++i)
                    toy -= i->height();

                return toy;
            }
            else
            {
                return p->y2 - c.height();
            }
        }
    }

    void layout(left, cell & x)
    {
        // TODO: anything to do here?
    }

    void layout(hcenter, cell & x)
    {
        if (!x.parent)
            return;

        cell * p = x.parent;
        cell * gp = p->parent;

        int tox = detail::move_right_pos(p, gp, x);

        tox = (tox + x.x1) / 2;

        x.move_to(tox, x.y1);
    }

    void layout(right, cell & x)
    {
        if (!x.parent)
            return;

        cell * p = x.parent;
        cell * gp = p->parent;

        int tox = detail::move_right_pos(p, gp, x);

        x.move_to(tox, x.y1);
    }

    void layout(top, cell & x)
    {
        // TODO: anything to do here?
    }

    void layout(vcenter, cell & x)
    {
        if (!x.parent)
            return;

        cell * p = x.parent;
        cell * gp = p->parent;

        int toy = detail::move_bottom_pos(p, gp, x);

        toy = (toy + x.y1) / 2;

        x.move_to(x.x1, toy);
    }

    void layout(bottom, cell & x)
    {
        if (!x.parent)
            return;

        cell * p = x.parent;
        cell * gp = p->parent;

        int toy = detail::move_bottom_pos(p, gp, x);

        x.move_to(x.x1, toy);
    }

    void layout(center, cell & x)
    {
        layout(hcenter(), x);
        layout(vcenter(), x);
    }
}}}

