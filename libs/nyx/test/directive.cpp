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

#include <mythos/nyx/directive.hpp>
#include <mythos/nyx/cell.hpp>

#define BOOST_TEST_MODULE nyx directive test
#include <mythos/support/test.hpp>

struct cell_tree_fixture : boost::noncopyable
{
    cell_tree_fixture()
    {
        root = make_cell();

        // horizontal cells
        a1 = make_cell(root);
        a2 = make_cell(root);
        a3 = make_cell(root);

        // vertical cells
        b11 = make_cell(a1);
        b12 = make_cell(a1);
        b21 = make_cell(a2);
        b22 = make_cell(a2);
        b31 = make_cell(a3);
        b32 = make_cell(a3);

        set_cell(root, 0, 0, 1000, 1000);
        set_cell(a1, 300, 300, 400, 700);
        set_cell(a2, 400, 300, 500, 700);
        set_cell(a3, 500, 300, 700, 700);

        set_cell(b11, 300, 300, 400, 500);
        set_cell(b12, 300, 500, 400, 700);
        set_cell(b21, 400, 300, 500, 500);
        set_cell(b22, 400, 500, 500, 700);
        set_cell(b31, 500, 300, 700, 500);
        set_cell(b32, 500, 500, 700, 700);
    }
    ~cell_tree_fixture()
    {
        BOOST_FOREACH(cell * x, cells)
            delete x;
    }

    cell * make_cell(cell * p = 0)
    {
        cells.push_back(new cell(p));
        return cells.back();
    }

    void set_cell(cell * x, int x1, int y1, int x2, int y2)
    {
        x->x1 = x1;
        x->y1 = y1;
        x->x2 = x2;
        x->y2 = y2;
    }

    std::vector<cell *> cells;
    cell * root;

    // horizontal cells
    cell * a1, * a2, * a3;

    // vertical cells
    cell * b11, * b12;
    cell * b21, * b22;
    cell * b31, * b32;
};

BOOST_FIXTURE_TEST_CASE(bottom_directive_test, cell_tree_fixture)
{
}

BOOST_FIXTURE_TEST_CASE(center_directive_test, cell_tree_fixture)
{
}

BOOST_FIXTURE_TEST_CASE(hcenter_directive_test, cell_tree_fixture)
{
}

BOOST_FIXTURE_TEST_CASE(vcenter_directive_test, cell_tree_fixture)
{
}

BOOST_FIXTURE_TEST_CASE(left_directive_test, cell_tree_fixture)
{
}

BOOST_FIXTURE_TEST_CASE(right_directive_test, cell_tree_fixture)
{
}

BOOST_FIXTURE_TEST_CASE(top_directive_test, cell_tree_fixture)
{
}

MYTHOS_KHAOS_TEST_IMPLEMENT_MAIN();

