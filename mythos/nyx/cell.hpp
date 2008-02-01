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

#if !defined( MYTHOS_NYX_CELL_HPP )
#define MYTHOS_NYX_CELL_HPP

#include <mythos/nyx/config.hpp>

#include <boost/intrusive/list.hpp>

namespace mythos { namespace nyx
{
    struct cell
        : boost::intrusive::list_base_hook<
            boost::intrusive::link_mode<boost::intrusive::auto_unlink>
        >
    {
        typedef boost::intrusive::list<
            cell,
            boost::intrusive::constant_time_size<false>
        >                                                       list_type;
        typedef list_type::iterator                             child_iterator;

        MYTHOS_NYX_DECL cell(cell * p = 0);
        MYTHOS_NYX_DECL cell(cell const& x);

        MYTHOS_NYX_DECL void move_by(int cx, int cy);
        MYTHOS_NYX_DECL void move_to(int tox, int toy);

        MYTHOS_NYX_DECL void grow_by(int cx, int cy);
        MYTHOS_NYX_DECL void grow_to(int tox, int toy);

        MYTHOS_NYX_DECL int max_child_x2();
        MYTHOS_NYX_DECL int max_child_y2();

        int width() const { return x2 - x1; }
        int height() const { return y2 - y1; }

        int x1, y1, x2, y2;

        cell * parent;
        list_type children;
    };
}}

#endif // #if !defined( MYTHOS_NYX_CELL_HPP )

