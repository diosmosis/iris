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

#include <mythos/khaos/event/paint.hpp>
#include <mythos/khaos/event/mouse_move.hpp>
#include <mythos/khaos/event/l_button_down.hpp>
#include <mythos/khaos/event/m_button_down.hpp>
#include <mythos/khaos/event/r_button_down.hpp>
#include <mythos/khaos/event/l_button_up.hpp>
#include <mythos/khaos/event/m_button_up.hpp>
#include <mythos/khaos/event/r_button_up.hpp>
#include <mythos/khaos/event/resize.hpp>
#include <mythos/khaos/event/reconfigure.hpp>

#include <mythos/khaos/window.hpp>
#include <mythos/khaos/main.hpp>

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_same.hpp>

#include <boost/detail/lightweight_test.hpp>

#include <iostream>
//#include "mythos_test.hpp"

#define MYTHOS_KHAOS_TEST_WINW 200
#define MYTHOS_KHAOS_TEST_WINH 200

using namespace mythos::khaos;

struct window_fixture
{
    window_fixture()
    {
        win = create_toplevel_window("abcdefg", 0, 0, MYTHOS_KHAOS_TEST_WINW, MYTHOS_KHAOS_TEST_WINH);
    }
    ~window_fixture()
    {
        destroy_window(win);
    }

    window * win;
};

// generic_event_handler
template <typename E, typename Data>
struct generic_event_handler
{
    typedef bool result_type;

    generic_event_handler(Data const& expected_) : expected(expected_) {}

    template <typename T>
    void operator()(T const& x, typename boost::disable_if<boost::is_same<T, image_view> >::type * d = 0) const
    {
        BOOST_TEST(x == expected);
    }

    template <typename T>
    void operator()(T const& x, typename boost::enable_if<boost::is_same<T, image_view> >::type * d = 0) const
    {
        BOOST_TEST(boost::gil::equal_pixels(x, expected));
    }

    bool operator()(int et, void * ei) const
    {
        //BOOST_CHECK_MESSAGE(et == E::value, "Unexpected event: " << et);
        BOOST_TEST(et == E::value);

        if (et == E::value)
        {
            return E::on_event(*this, ei);
        }

        return false;
    }

    Data expected;
};

////////////////////////////////////////////////////////////////////////////////////////
void paint_raise_test()
{
    window_fixture fixture;

    image img(MYTHOS_KHAOS_TEST_WINW, MYTHOS_KHAOS_TEST_WINH);

    fixture.win->handler = generic_event_handler<paint, image_view>(boost::gil::view(img));

    paint::raise(fixture.win);
}

////////////////////////////////////////////////////////////////////////////////////////
template <typename E>
void point_event_raise_test(int x, int y)
{
    window_fixture fixture;

    point pt(x, y);

    fixture.win->handler = generic_event_handler<E, point>(pt);

    E::raise(fixture.win, pt);
}

int run_tests(int argc, char ** argv)
{
    paint_raise_test();
    point_event_raise_test<mouse_move>(50, 25);
    point_event_raise_test<l_button_down>(50, 25);
    point_event_raise_test<m_button_down>(50, 25);
    point_event_raise_test<r_button_down>(50, 25);
    point_event_raise_test<l_button_up>(50, 25);
    point_event_raise_test<m_button_up>(50, 25);
    point_event_raise_test<r_button_up>(50, 25);
    point_event_raise_test<resize>(50, 25);
    point_event_raise_test<reconfigure>(50, 25);

    return boost::report_errors();
}

MYTHOS_KHAOS_IMPLEMENT_MAIN(run_tests);

