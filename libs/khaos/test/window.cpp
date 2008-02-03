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

#include <mythos/khaos/window.hpp>
#include <mythos/khaos/event/paint.hpp>
#include <mythos/khaos/event/resize.hpp>

#define BOOST_TEST_MODULE khaos window test
#include <mythos/support/test.hpp>
#include <iostream>
// TODO: implement these tests
// modalize_test
// destroy_window_test
// foreign_create_window_test

using namespace mythos::khaos;

struct raii_window
{
    raii_window(window * w) : win(w) {}
    ~raii_window()
    {
        destroy_window(win);
    }

    operator window *() const
    {
        return win;
    }

    window * operator -> () const
    {
        return win;
    }

    window * win;
};

BOOST_AUTO_TEST_CASE(create_toplevel_window_test)
{
    raii_window win = create_toplevel_window("a window", 10, 10, 300, 300);
    BOOST_REQUIRE(win);

    BOOST_CHECK(win->parent == NULL);
    BOOST_CHECK(win->is_toplevel);
    BOOST_CHECK(get_position(win) == point(10, 10));
    BOOST_CHECK(get_size(win) == point(300, 300));
    BOOST_CHECK(is_mythos_window(win));
    BOOST_CHECK(handle_of(win));
    BOOST_CHECK(!is_shown(win));

    show_window(win);

    window * win2 = create_toplevel_window("a window", 20, 20, 50, 50, win);
    BOOST_REQUIRE(win2);

    BOOST_CHECK(win2->parent == win);
    BOOST_CHECK(win2->is_toplevel);
    BOOST_CHECK(get_position(win2) == point(20, 20));
    BOOST_CHECK(get_size(win2) == point(50, 50));
    BOOST_CHECK(is_mythos_window(win2));
    BOOST_CHECK(handle_of(win2));
    BOOST_CHECK(!is_shown(win2));

    BOOST_CHECK(win->children.size() == 1);
}

BOOST_AUTO_TEST_CASE(create_child_window_test)
{
    raii_window win = create_child_window(0, 0, 50, 50, NULL);
    BOOST_REQUIRE(win);

    BOOST_CHECK(win->parent == NULL);
    BOOST_CHECK(!win->is_toplevel);
    BOOST_CHECK(get_position(win) == point(0, 0));
    BOOST_CHECK(get_size(win) == point(50, 50));
    BOOST_CHECK(is_mythos_window(win));
    BOOST_CHECK(handle_of(win));
    BOOST_CHECK(!is_shown(win));

    window * win2 = create_child_window(0, 0, 20, 20, win);
    BOOST_REQUIRE(win2);

    BOOST_CHECK(win2->parent == win);
    BOOST_CHECK(!win2->is_toplevel);
    BOOST_CHECK(get_position(win2) == point(0, 0));
    BOOST_CHECK(get_size(win2) == point(20, 20));
    BOOST_CHECK(is_mythos_window(win2));
    BOOST_CHECK(handle_of(win2));
    BOOST_CHECK(!is_shown(win2));

    BOOST_CHECK(win->children.size() == 1);

    raii_window top = create_toplevel_window("title", 0, 0, 100, 100, NULL);
    BOOST_REQUIRE(top);

    show_window(top);

    window * win3 = create_toplevel_window("title", 0, 0, 50, 50, top);
    BOOST_REQUIRE(win3);

    BOOST_CHECK(win3->parent == top);
    BOOST_CHECK(!win3->is_toplevel);
    BOOST_CHECK(get_position(win3) == point(0, 0));
    BOOST_CHECK(get_size(win3) == point(20, 20));
    BOOST_CHECK(is_mythos_window(win3));
    BOOST_CHECK(handle_of(win3));
    BOOST_CHECK(is_shown(win3));
}

struct clipping_test_handler
{
    clipping_test_handler(point const& ed) : expected_dims(ed) {}

    void operator()(image_view const& vw) const
    {
        BOOST_CHECK(vw.width() == expected_dims.x);
        BOOST_CHECK(vw.height() == expected_dims.y);
    }

    bool operator()(int et, void * ei) const
    {
        return paint::on_event(*this, ei);
    }

    point expected_dims;
};

static void test_clipping(window * win)
{
    window * parent = win->parent;

    point pos = get_position(win);
    point sz = get_size(win);

    point psz = get_size(parent);

    point expected = sz;

    if (pos.x < 0)
        expected.x += pos.x;

    if (pos.y < 0)
        expected.y += pos.y;

    if (pos.x + sz.x > psz.x)
        expected.x -= pos.x + sz.x - psz.x;

    if (pos.y + sz.y > psz.y)
        expected.y -= pos.y + sz.y - psz.y;

    win->handler = clipping_test_handler(expected);

    paint::raise(win);
std::cout << "there" << std::endl;
}

BOOST_AUTO_TEST_CASE(move_window_test)
{
    // toplevel tests
    raii_window top = create_toplevel_window("title", 0, 0, 50, 50);
    BOOST_REQUIRE(top);

    BOOST_CHECK(get_position(top) == point(0, 0));

    move_window(top, 25, 25);
    BOOST_CHECK(get_position(top) == point(25, 25));
    BOOST_CHECK(get_size(top) == point(50, 50));

    // toplevel w/ parent tests
    window * top2 = create_toplevel_window("title", 30, 30, 500, 500, top);
    BOOST_REQUIRE(top2);

    BOOST_CHECK(get_position(top2) == point(30, 30));

    move_window(top2, 100, 100);
    BOOST_CHECK(get_position(top2) == point(100, 100));
    BOOST_CHECK(get_size(top2) == point(500, 500));

    // child window tests
    window * child = create_child_window(0, 0, 10, 10, top);
    BOOST_REQUIRE(child);

    BOOST_CHECK(get_position(child) == point(0, 0));

    move_window(child, 25, 25);
    BOOST_CHECK(get_position(child) == point(25, 25));
    BOOST_CHECK(get_size(child) == point(10, 10));

    // move out of bounds of the top window
    move_window(child, 60, 60);
    BOOST_CHECK(get_position(child) == point(60, 60));
    BOOST_CHECK(get_size(child) == point(10, 10));

    test_clipping(child);

    move_window(child, -10, -10);
    BOOST_CHECK(get_position(child) == point(-10, -10));
    BOOST_CHECK(get_size(child) == point(10, 10));

    test_clipping(child);
}

struct check_resize
{
    check_resize(bool & r) : resized(r) {}

    bool operator()(int et, void * ei) const
    {
        if (et == resize::value)
            resized = true;
        return true;
    }

    bool & resized;
};

BOOST_AUTO_TEST_CASE(resize_window_test)
{
    // toplevel tests
    raii_window top = create_toplevel_window("title", 0, 0, 50, 50);
    BOOST_REQUIRE(top);

    BOOST_CHECK(get_size(top) == point(50, 50));

    // to test that resizing generates a resize event
    bool resize_event_sent = false;
    top->handler = check_resize(resize_event_sent);

    resize_window(top, 100, 100);
    BOOST_CHECK(get_size(top) == point(100, 100));
    BOOST_CHECK(get_position(top) == point(0, 0));
    BOOST_CHECK(resize_event_sent);

    // toplevel w/ parent tests
    raii_window top2 = create_toplevel_window("title", 0, 0, 50, 50, top);
    BOOST_REQUIRE(top2);

    BOOST_CHECK(get_size(top2) == point(50, 50));

    resize_window(top2, 100, 100);
    BOOST_CHECK(get_size(top2) == point(100, 100));
    BOOST_CHECK(get_position(top2) == point(0, 0));

    // child window tests
    window * child = create_child_window(0, 0, 10, 10, top);
    BOOST_REQUIRE(child);

    BOOST_CHECK(get_size(child) == point(10, 10));

    resize_window(child, 25, 25);
    BOOST_CHECK(get_position(child) == point(0, 0));
    BOOST_CHECK(get_size(child) == point(25, 25));

    // resize out of bounds of the top window
    resize_window(child, 60, 60);
    BOOST_CHECK(get_position(child) == point(0, 0));
    BOOST_CHECK(get_size(child) == point(60, 60));

    test_clipping(child);
}

BOOST_AUTO_TEST_CASE(show_hide_window_test)
{
    raii_window win = create_toplevel_window("abc", 0, 0, 100, 100);
    BOOST_REQUIRE(win);

    BOOST_CHECK(!is_shown(win));

    window * child = create_child_window(0, 0, 50, 50, win);
    BOOST_REQUIRE(child);

    BOOST_CHECK(!is_shown(child));

    window * child2 = create_child_window(0, 0, 50, 50, win);
    BOOST_REQUIRE(child2);

    BOOST_CHECK(!is_shown(child2));

    // showing a child w/ a hidden parent results in is_shown() == false
    show_window(child);
    BOOST_CHECK(!is_shown(child));

    // showing a toplevel window w/ child results in is_shown() == true for all
    show_window(win);
    BOOST_CHECK(is_shown(win));
    BOOST_CHECK(is_shown(child));
    BOOST_CHECK(is_shown(child2));

    // hiding a child window
    hide_window(child2);
    BOOST_CHECK(!is_shown(child2));

    // hiding a toplevel window
    hide_window(win);
    BOOST_CHECK(!is_shown(win));
    BOOST_CHECK(!is_shown(child));
    BOOST_CHECK(!is_shown(child2));
}

BOOST_AUTO_TEST_CASE(set_parent_test)
{
    raii_window extra_top = create_toplevel_window("title", 0, 0, 300, 300);

    show_window(extra_top);

    // toplevel, child
    {
        window * top = create_toplevel_window("abc", 50, 50, 100, 100, extra_top);
        window * child = create_child_window(0, 0, 200, 200, extra_top);

        BOOST_REQUIRE(top);
        BOOST_REQUIRE(child);

        show_window(top);

        set_parent(top, child);

        BOOST_CHECK(!top->is_toplevel);
        BOOST_CHECK(top->parent == child);
        BOOST_CHECK(get_position(top) == point(0, 0));
        BOOST_CHECK(get_size(top) == point(100, 100));
        BOOST_CHECK(!is_shown(top));

        destroy_window(child);
    }

    // child, toplevel
    {
        // extra_top is shown, so child will be too
        window * child = create_child_window(50, 50, 100, 100, extra_top);

        // top will not be shown
        window * top = create_toplevel_window("abc", 0, 0, 200, 200, extra_top);

        BOOST_REQUIRE(child);
        BOOST_REQUIRE(top);

        set_parent(child, top);

        BOOST_CHECK(!child->is_toplevel);
        BOOST_CHECK(child->parent == top);
        BOOST_CHECK(get_position(child) == point(50, 50));
        BOOST_CHECK(get_size(child) == point(100, 100));
        BOOST_CHECK(!is_shown(child));
    }

    // toplevel, toplevel
    {
        window * top = create_toplevel_window("abc", 0, 0, 100, 100, extra_top);
        window * top2 = create_toplevel_window("def", 100, 100, 50, 50, extra_top);

        BOOST_REQUIRE(top);
        BOOST_REQUIRE(top2);

        show_window(top);

        set_parent(top2, top);

        BOOST_CHECK(!top2->is_toplevel);
        BOOST_CHECK(top2->parent == top);
        BOOST_CHECK(get_position(top2) == point(0, 0));
        BOOST_CHECK(get_size(top2) == point(50, 50));
        BOOST_CHECK(is_shown(top2));
    }

    // child, child
    {
        window * child = create_child_window(0, 0, 100, 100, extra_top);
        window * child2 = create_child_window(25, 25, 50, 50, extra_top);

        BOOST_REQUIRE(child);
        BOOST_REQUIRE(child2);

        show_window(child);
        show_window(child2);

        set_parent(child2, child);

        BOOST_CHECK(!child2->is_toplevel);
        BOOST_CHECK(child2->parent == child);
        BOOST_CHECK(get_position(child2) == point(25, 25));
        BOOST_CHECK(get_size(child2) == point(50, 50));
        BOOST_CHECK(is_shown(child2));
    }
}

BOOST_AUTO_TEST_CASE(window_count_test)
{
    BOOST_CHECK(window_count() == 0);
    BOOST_CHECK(toplevel_window_count() == 0);

    window * top = create_toplevel_window("abc", 0, 0, 100, 100);
    BOOST_REQUIRE(top);

    BOOST_CHECK(window_count() == 1);
    BOOST_CHECK(toplevel_window_count() == 1);

    window * child = create_child_window(0, 0, 100, 100, top);
    BOOST_REQUIRE(child);

    BOOST_CHECK(window_count() == 2);
    BOOST_CHECK(toplevel_window_count() == 1);

    window * top2 = create_toplevel_window("abc", 0, 0, 300, 300);
    BOOST_REQUIRE(top2);

    BOOST_CHECK(window_count() == 3);
    BOOST_CHECK(toplevel_window_count() == 2);

    destroy_window(top);

    BOOST_CHECK(window_count() == 1);
    BOOST_CHECK(toplevel_window_count() == 1);

    destroy_window(top2);

    BOOST_CHECK(window_count() == 0);
    BOOST_CHECK(toplevel_window_count() == 0);
}

MYTHOS_KHAOS_TEST_IMPLEMENT_MAIN();

