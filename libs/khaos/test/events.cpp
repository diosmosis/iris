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

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_same.hpp>

#define BOOST_TEST_MODULE khaos events test
#include <mythos/support/test.hpp>

#define MYTHOS_KHAOS_TEST_WINW 200
#define MYTHOS_KHAOS_TEST_WINH 200

#define MYTHOS_KHAOS_POINT_EVENT_TEST(name)                         \
    BOOST_FIXTURE_TEST_CASE(name ## _raise_test, window_fixture)    \
    {                                                               \
        point pt(50, 25);                                           \
                                                                    \
        win->handler = generic_event_handler<name, point>(pt);      \
                                                                    \
        name::raise(win, pt);                                       \
    }

using namespace mythos::khaos;

namespace mythos { namespace khaos
{
    inline std::ostream & operator << (std::ostream & os, point const& pt)
    {
        os << '(' << pt.x << ", " << pt.y << ')';
        return os;
    }
}}

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
        BOOST_CHECK_EQUAL(x, expected);
    }

    template <typename T>
    void operator()(T const& x, typename boost::enable_if<boost::is_same<T, image_view> >::type * d = 0) const
    {
        BOOST_CHECK(boost::gil::equal_pixels(x, expected));
    }

    bool operator()(int et, void * ei) const
    {
        BOOST_CHECK_EQUAL(et, E::value);

        if (et == E::value)
        {
            return E::on_event(*this, ei);
        }

        return false;
    }

    Data expected;
};

////////////////////////////////////////////////////////////////////////////////////////
BOOST_FIXTURE_TEST_CASE(paint_raise_test, window_fixture)
{
    image img(MYTHOS_KHAOS_TEST_WINW, MYTHOS_KHAOS_TEST_WINH);

    win->handler = generic_event_handler<paint, image_view>(boost::gil::view(img));

    paint::raise(win);
}

////////////////////////////////////////////////////////////////////////////////////////
MYTHOS_KHAOS_POINT_EVENT_TEST(mouse_move)
MYTHOS_KHAOS_POINT_EVENT_TEST(l_button_down)
MYTHOS_KHAOS_POINT_EVENT_TEST(m_button_down)
MYTHOS_KHAOS_POINT_EVENT_TEST(r_button_down)
MYTHOS_KHAOS_POINT_EVENT_TEST(l_button_up)
MYTHOS_KHAOS_POINT_EVENT_TEST(m_button_up)
MYTHOS_KHAOS_POINT_EVENT_TEST(r_button_up)
MYTHOS_KHAOS_POINT_EVENT_TEST(resize)
MYTHOS_KHAOS_POINT_EVENT_TEST(reconfigure)

////////////////////////////////////////////////////////////////////////////////////////
MYTHOS_KHAOS_TEST_IMPLEMENT_MAIN();

