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

#define MYTHOS_GAIA_SOURCE

#include <mythos/gaia/label.hpp>
#include <mythos/gaia/text.hpp>
#include <mythos/gaia/image.hpp>
#include <mythos/gaia/button.hpp>

#include <mythos/gaia/plugin.hpp>

#include <mythos/iris/event_handler.hpp>
#include <mythos/iris/dsel.hpp>
#include <mythos/iris/paint.hpp>
#include <mythos/iris/l_button_up.hpp>
#include <mythos/iris/l_button_down.hpp>

#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include <windows.h>

namespace mythos { namespace gaia
{
    namespace detail
    {
        static HWND hwnd_of(nyx::window const& w)
        {
            khaos::window * kw = w.khaos_window();

            return kw ? *static_cast<HWND *>(khaos::handle_of(kw)) : NULL;
        }
    }

    // label.hpp
    namespace detail
    {
        nyx::window label(
            std::string const& x,
            void const* fnt,
            void (*renderer)(void const*, khaos::image_view const&, std::string const&),
            nyx::point const& extents,
            nyx::window parent
        )
        {
            namespace on = mythos::iris::on;
            using namespace boost::lambda;

            // create window
            nyx::window win = nyx::create_child_window(0, 0, extents.x, extents.y, parent);

            // create event_handler
            win.events() = on::paint[boost::lambda::bind(renderer, fnt, _1, x)];

            return win;
        }
    }

    nyx::window label(std::string const& x)
    {
        return label(x, khaos::default_font(), 0, 0, nyx::window());
    }

    // text.hpp
    namespace detail
    {
        nyx::window text(
            std::string & x,
            void const* fnt,
            void (*renderer)(void const*, khaos::image_view const&, std::string const&),
            nyx::point const& extents,
            nyx::window parent
        )
        {
            namespace on = mythos::iris::on;
            using namespace boost::lambda;

            // create window
            nyx::window win = nyx::create_child_window(0, 0, extents.x, extents.y, parent);

            // create event_handler
            win.events() = on::paint[boost::lambda::bind(renderer, fnt, _1, boost::ref(x))];

            return win;
        }
    }

    nyx::window text(std::string & x)
    {
        return text(x, khaos::default_font());
    }

    // image.hpp
    namespace detail
    {
        nyx::window image(
            boost::any const& view,
            void (*render)(boost::any const& img, khaos::image_view const& vw),
            int w,
            int h,
            nyx::window const& parent
        )
        {
            namespace on = mythos::iris::on;
            using namespace boost::lambda;

            // create window
            nyx::window win = nyx::create_child_window(0, 0, w, h, parent);

            // create event_handler
            win.events() = on::paint[boost::lambda::bind(render, view, _1)];

            return win;
        }
    }

    // button.hpp
    nyx::window button(boost::function<void ()> const& action, nyx::window const& p)
    {
        HWND phwnd = detail::hwnd_of(p);

        HWND hwnd = ::CreateWindow(
            "BUTTON",
            "",
            BS_PUSHBUTTON | WS_CHILD,
            0, 0, 12, 6,
            phwnd ? phwnd : (HWND) khaos::detail::get_messagewin(),
            NULL,
            (HINSTANCE) khaos::detail::get_hinstance(),
            NULL
        );

        // create window
        nyx::window win = nyx::foreign_create_window(&hwnd);

        // create event_handler
        win.events() = on::l_button_down[boost::lambda::bind(action)];

        return win;
    }

    namespace detail
    {
        struct set_checkbox_value
        {
            set_checkbox_value(bool & v_) : v(v_) {}

            void operator()(nyx::point const&) const
            {
                v = !v;
            }

            bool & v;
        };
    }

    nyx::window checkbox(bool & value, nyx::window const& p)
    {
        HWND phwnd = detail::hwnd_of(p);

        HWND hwnd = ::CreateWindow(
            "BUTTON",
            "",
            BS_CHECKBOX,
            0, 0, 10, 10, // FIXME: Need default size
            phwnd ? phwnd : (HWND) khaos::detail::get_messagewin(),
            NULL,
            (HINSTANCE) khaos::detail::get_hinstance(),
            NULL
        );

        // set checkbox value
        ::SendMessage(hwnd, BM_SETCHECK, value ? BST_CHECKED : BST_UNCHECKED, 0);

        // create window
        nyx::window win = nyx::foreign_create_window(&hwnd);

        // create event_handler
        win.events() = (on::l_button_down | on::l_button_up)[detail::set_checkbox_value(value)];

        return win;
    }
}}

MYTHOS_KHAOS_ANNEX_IMPLEMENT_PLUGIN(mythos::gaia::interface);

