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

#include <mythos/nyx/cell.hpp>

#include <boost/noncopyable.hpp>

#include <boost/foreach.hpp>

#include <vector>

#define BOOST_TEST_MODULE nyx cell test
#include <mythos/support/test.hpp>

#define CHECK_CELL_EQUAL(old, new)                      \
    BOOST_CHECK_EQUAL((old).x1, (new).x1);              \
    BOOST_CHECK_EQUAL((old).x2, (new).x2);              \
    BOOST_CHECK_EQUAL((old).y1, (new).y1);              \
    BOOST_CHECK_EQUAL((old).y2, (new).y2);

#define CHECK_CELL_MOVED(old, new, cx, cy)              \
    BOOST_CHECK_EQUAL((old).x1 + cx, (new).x1);         \
    BOOST_CHECK_EQUAL((old).x2 + cx, (new).x2);         \
    BOOST_CHECK_EQUAL((old).y1 + cy, (new).y1);         \
    BOOST_CHECK_EQUAL((old).y2 + cy, (new).y2)

#define CHECK_CELL_GROWN(old, new, cx, cy)              \
    BOOST_CHECK_EQUAL((old).x1, (new).x1);              \
    BOOST_CHECK_EQUAL((old).x2 + cx, (new).x2);         \
    BOOST_CHECK_EQUAL((old).y1, (new).y1);              \
    BOOST_CHECK_EQUAL((old).y2 + cy, (new).y2)

#define CHECK_CELL_GROWN_BACKWARD(old, new, cx, cy)     \
    BOOST_CHECK_EQUAL((old).x1 + cx, (new).x1);         \
    BOOST_CHECK_EQUAL((old).x2, (new).x2);              \
    BOOST_CHECK_EQUAL((old).y1 + cy, (new).y1);         \
    BOOST_CHECK_EQUAL((old).y2, (new).y2)

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

        set_cell(root, 0, 0, 400, 400);
        set_cell(a1, 0, 0, 100, 400);
        set_cell(a2, 100, 0, 200, 400);
        set_cell(a3, 200, 0, 400, 400);

        set_cell(b11, 0, 0, 100, 200);
        set_cell(b12, 0, 200, 100, 400);
        set_cell(b21, 100, 0, 200, 200);
        set_cell(b22, 100, 200, 200, 400);
        set_cell(b31, 200, 0, 400, 200);
        set_cell(b32, 200, 200, 400, 400);
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

// for making quick copies of cells
struct rect
{
    rect(cell const& c)
        : x1(c.x1), x2(c.x2), y1(c.y1), y2(c.y2)
    {}

    int x1, x2, y1, y2;
};

BOOST_AUTO_TEST_CASE(cell_construction_test)
{
    cell top;

    BOOST_CHECK_EQUAL(top.parent, 0);
    BOOST_CHECK_EQUAL(top.x1, 0);
    BOOST_CHECK_EQUAL(top.x2, 0);
    BOOST_CHECK_EQUAL(top.y1, 0);
    BOOST_CHECK_EQUAL(top.y2, 0);
    BOOST_CHECK(top.children.empty());

    top.x1 = top.y1 = 5;
    top.x2 = top.y2 = 10;

    cell child(&top);

    BOOST_CHECK_EQUAL(child.parent, &top);
    BOOST_CHECK_EQUAL(top.x1, 5);
    BOOST_CHECK_EQUAL(top.x2, 5);
    BOOST_CHECK_EQUAL(top.y1, 5);
    BOOST_CHECK_EQUAL(top.y2, 5);
    BOOST_CHECK(child.children.empty());
    BOOST_CHECK_EQUAL(top.children.size(), 1);

    cell grandchild(&child);
    cell copy(child);

    BOOST_CHECK_EQUAL(copy.parent, &top);
    BOOST_CHECK_EQUAL(copy.x1, 5);
    BOOST_CHECK_EQUAL(copy.x2, 5);
    BOOST_CHECK_EQUAL(copy.y1, 5);
    BOOST_CHECK_EQUAL(copy.y2, 5);
    BOOST_CHECK(copy.children.empty());
    BOOST_CHECK_EQUAL(top.children.size(), 2);
}

BOOST_FIXTURE_TEST_CASE(move_by_forward_test, cell_tree_fixture)
{
    rect old_root(*root), old_a1(*a1), old_a2(*a2), old_a3(*a3);
    rect old_b11(*b11), old_b12(*b12), old_b21(*b21), old_b22(*b22), old_b31(*b31), old_b32(*b32);

    a2->move_by(50, 50);

    // test that it moves the cell
    CHECK_CELL_MOVED(old_a2, *a2, 50, 50);
    CHECK_CELL_MOVED(old_b21, *b21, 50, 50);
    CHECK_CELL_MOVED(old_b22, *b22, 50, 50);

    // test that it doesn't affect any siblings before it
    CHECK_CELL_MOVED(old_a1, *a1, 0, 0);
    CHECK_CELL_MOVED(old_b11, *b11, 0, 0);
    CHECK_CELL_MOVED(old_b12, *b12, 0, 0);

    // test that it moves all siblings after it
    CHECK_CELL_MOVED(old_a3, *a3, 50, 50);
    CHECK_CELL_MOVED(old_b31, *b31, 50, 50);
    CHECK_CELL_MOVED(old_b32, *b32, 50, 50);

    // test that it stretches the parent cell
    CHECK_CELL_GROWN(old_root, *root, 50, 50);
}

BOOST_FIXTURE_TEST_CASE(move_by_backwards_test, cell_tree_fixture)
{
    rect old_root(*root), old_a1(*a1), old_a2(*a2), old_a3(*a3);
    rect old_b11(*b11), old_b12(*b12), old_b21(*b21), old_b22(*b22), old_b31(*b31), old_b32(*b32);

    a2->move_by(-50, -50);

    // test that it moves the cell
    CHECK_CELL_MOVED(old_a2, -50, -50);
    CHECK_CELL_MOVED(old_b21, *b21, -50, -50);
    CHECK_CELL_MOVED(old_b22, *b22, -50, -50);

    // test that it doesn't affect any siblings after it
    CHECK_CELL_MOVED(old_a3, 0, 0);
    CHECK_CELL_MOVED(old_b31, *b31, 0, 0);
    CHECK_CELL_MOVED(old_b32, *b32, 0, 0);

    // test that it moves all siblings before it
    CHECK_CELL_MOVED(old_a1, -50, -50);
    CHECK_CELL_MOVED(old_b11, *b11, -50, -50);
    CHECK_CELL_MOVED(old_b12, *b12, -50, -50);

    // test that it stretches the parent cell
    CHECK_CELL_GROWN_BACKWARD(old_root, -50, -50);
}

BOOST_FIXTURE_TEST_CASE(move_to_forward_test, cell_tree_fixture)
{
    rect old_root(*root), old_a1(*a1), old_a2(*a2), old_a3(*a3);
    rect old_b11(*b11), old_b12(*b12), old_b21(*b21), old_b22(*b22), old_b31(*b31), old_b32(*b32);

    a2->move_to(old_a2.x1 + 50, old_a2.y1 + 50)

    // test that it moves the cell
    CHECK_CELL_MOVED(old_a2, *a2, 50, 50);
    CHECK_CELL_MOVED(old_b21, *b21, 50, 50);
    CHECK_CELL_MOVED(old_b22, *b22, 50, 50);

    // test that it doesn't affect any siblings before it
    CHECK_CELL_EQUAL(old_a1, *a1);
    CHECK_CELL_EQUAL(old_b11, *b11);
    CHECK_CELL_EQUAL(old_b12, *b12);

    // test that it moves all siblings after it
    CHECK_CELL_MOVED(old_a3, *a3, 50, 50);
    CHECK_CELL_MOVED(old_b31, *b31, 50, 50);
    CHECK_CELL_MOVED(old_b32, *b32, 50, 50);

    // test that it stretches the parent cell
    CHECK_CELL_GROWN(old_root, *root, 50, 50);
}

BOOST_FIXTURE_TEST_CASE(move_to_backwards_test, cell_tree_fixture)
{
    rect old_root(*root), old_a1(*a1), old_a2(*a2), old_a3(*a3);
    rect old_b11(*b11), old_b12(*b12), old_b21(*b21), old_b22(*b22), old_b31(*b31), old_b32(*b32);

    a2->move_to(old_a2.x1 - 50, old_a2.y1 - 50);

    // test that it moves the cell
    CHECK_CELL_MOVED(old_a2, -50, -50);
    CHECK_CELL_MOVED(old_b21, *b21, -50, -50);
    CHECK_CELL_MOVED(old_b22, *b22, -50, -50);

    // test that it doesn't affect any siblings after it
    CHECK_CELL_EQUAL(old_a3, *a3);
    CHECK_CELL_EQUAL(old_b31, *b31);
    CHECK_CELL_EQUAL(old_b32, *b32);

    // test that it moves all siblings before it
    CHECK_CELL_MOVED(old_a1, -50, -50);
    CHECK_CELL_MOVED(old_b11, *b11, -50, -50);
    CHECK_CELL_MOVED(old_b12, *b12, -50, -50);

    // test that it stretches the parent cell
    CHECK_CELL_GROWN_BACKWARD(old_root, -50, -50);
}

BOOST_FIXTURE_TEST_CASE(grow_by_forward_test, cell_tree_fixture)
{
    rect old_root(*root), old_a1(*a1), old_a2(*a2), old_a3(*a3);
    rect old_b11(*b11), old_b12(*b12), old_b21(*b21), old_b22(*b22), old_b31(*b31), old_b32(*b32);

    a2->grow_by(50, 50);

    // test that it grows the cell
    CHECK_CELL_GROWN_FORWARD(old_a2, *a2, 50, 50);

    // test that it does not affect child cells
    CHECK_CELL_EQUAL(old_b21, *b21);
    CHECK_CELL_EQUAL(old_b22, *b22);

    // test that it moves sibling cells
    CHECK_CELL_MOVED(old_a3, *a3, 50, 50);
    CHECK_CELL_MOVED(old_b31, *b31, 50, 50);
    CHECK_CELL_MOVED(old_b32, *b32, 50, 50);

    // test that it doesn't affect siblings before it
    CHECK_CELL_EQUAL(old_a1, *a1);
    CHECK_CELL_EQUAL(old_b11, *b11);
    CHECK_CELL_EQUAL(old_b12, *b12);

    // test that it stretches the parent
    CHECK_CELL_GROWN_FORWARD(old_root, *root, 50, 50);
}

BOOST_FIXTURE_TEST_CASE(grow_by_backward_test, cell_tree_fixture)
{
    rect old_root(*root), old_a1(*a1), old_a2(*a2), old_a3(*a3);
    rect old_b11(*b11), old_b12(*b12), old_b21(*b21), old_b22(*b22), old_b31(*b31), old_b32(*b32);

    a2->grow_by(-50, -50);

    // test that it grows the cell
    CHECK_CELL_GROWN_BACKWARD(old_a2, *a2, -50, -50);

    // test that it does not affect child cells
    CHECK_CELL_EQUAL(old_b21, *b21);
    CHECK_CELL_EQUAL(old_b22, *b22);

    // test that it moves sibling cells
    CHECK_CELL_MOVED(old_a1, *a1, -50, -50);
    CHECK_CELL_MOVED(old_b11, *b11, -50, -50);
    CHECK_CELL_MOVED(old_b12, *b12, -50, -50);

    // test that it doesn't affect siblings before it
    CHECK_CELL_EQUAL(old_a3, *a3);
    CHECK_CELL_EQUAL(old_b31, *b31);
    CHECK_CELL_EQUAL(old_b32, *b32);

    // test that it stretches the parent
    CHECK_CELL_GROWN_BACKWARD(old_root, *root, -50, -50);
}

BOOST_FIXTURE_TEST_CASE(grow_to_forward_test, cell_tree_fixture)
{
    rect old_root(*root), old_a1(*a1), old_a2(*a2), old_a3(*a3);
    rect old_b11(*b11), old_b12(*b12), old_b21(*b21), old_b22(*b22), old_b31(*b31), old_b32(*b32);

    a2->grow_by(old_a2.x2 + 50, old_a2.y2 + 50);

    // test that it grows the cell
    CHECK_CELL_GROWN_FORWARD(old_a2, *a2, 50, 50);

    // test that it does not affect child cells
    CHECK_CELL_EQUAL(old_b21, *b21);
    CHECK_CELL_EQUAL(old_b22, *b22);

    // test that it moves sibling cells
    CHECK_CELL_MOVED(old_a3, *a3, 50, 50);
    CHECK_CELL_MOVED(old_b31, *b31, 50, 50);
    CHECK_CELL_MOVED(old_b32, *b32, 50, 50);

    // test that it doesn't affect siblings before it
    CHECK_CELL_EQUAL(old_a1, *a1);
    CHECK_CELL_EQUAL(old_b11, *b11);
    CHECK_CELL_EQUAL(old_b12, *b12);

    // test that it stretches the parent
    CHECK_CELL_GROWN_FORWARD(old_root, *root, 50, 50);
}

BOOST_FIXTURE_TEST_CASE(grow_to_backwards_test, cell_tree_fixture)
{
    rect old_root(*root), old_a1(*a1), old_a2(*a2), old_a3(*a3);
    rect old_b11(*b11), old_b12(*b12), old_b21(*b21), old_b22(*b22), old_b31(*b31), old_b32(*b32);

    a2->grow_by(old_a2.x1 - 50, old_a2.y1 - 50);

    // test that it grows the cell
    CHECK_CELL_GROWN_BACKWARD(old_a2, *a2, -50, -50);

    // test that it does not affect child cells
    CHECK_CELL_EQUAL(old_b21, *b21);
    CHECK_CELL_EQUAL(old_b22, *b22);

    // test that it moves sibling cells
    CHECK_CELL_MOVED(old_a1, *a1, -50, -50);
    CHECK_CELL_MOVED(old_b11, *b11, -50, -50);
    CHECK_CELL_MOVED(old_b12, *b12, -50, -50);

    // test that it doesn't affect siblings before it
    CHECK_CELL_EQUAL(old_a3, *a3);
    CHECK_CELL_EQUAL(old_b31, *b31);
    CHECK_CELL_EQUAL(old_b32, *b32);

    // test that it stretches the parent
    CHECK_CELL_GROWN_BACKWARD(old_root, *root, -50, -50);
}

BOOST_FIXTURE_TEST_CASE(max_child_x2_test, cell_tree_fixture)
{
    BOOST_CHECK(a2->max_child_x2() == 200);
    BOOST_CHECK(root->max_child_x2() == 400);
    BOOST_CHECK(b12->max_child_x2() == 0);
}

BOOST_FIXTURE_TEST_CASE(max_child_y2_test, cell_tree_fixture)
{
    BOOST_CHECK(a2->max_child_y2() == 400);
    BOOST_CHECK(root->max_child_y2() == 400);
    BOOST_CHECK(b12->max_child_y2() == 0);
}

MYTHOS_KHAOS_TEST_IMPLEMENT_MAIN();

