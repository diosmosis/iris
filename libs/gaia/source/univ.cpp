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

#include <mythos/gaia/detail/plugin_impl.hpp>

#include <mythos/iris/event_handler.hpp>
#include <mythos/iris/dsel.hpp>

#include <exception>

namespace mythos { namespace gaia
{
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
            // create window
            nyx::window win = nyx::create_child_window(0, 0, w, h, parent);

            // create event_handler
            win.events() = on::paint[boost::lambda::bind(render, view, _1)];

            return win;
        }
    }

    // button.hpp
    namespace detail
    {
        struct button_state
        {
            nyx::window win;
            bool is_up;
            boost::function<void ()> action;
        };

        static void button_ldown(button_state & state, point const&)
        {
            state.is_up = true;

            iris::paint(state.win);
        }

        static void button_lup(button_state & state, point const&)
        {
            state.is_up = false;

            iris::paint(state.win);

            state.action();
        }

        static void button_paint(button_state & state, khaos::image_view const& vw)
        {
#error TODO
        }
    }

    nyx::window button(boost::function<void ()> const& action, nyx::window const& p)
    {
        namespace on = iris::on;

        // create window
        nyx::window win = nyx::create_child_window(0, 0, 12, 4, p);

        // create state
        button_state state = { win, true, action };

        // create event handler
        win.events() = iris::context(state)
        [
            on::l_button_down   [button_ldown],
            on::l_button_up     [button_lup],
            on::paint           [button_paint]
        ]
        ;

        return win;
    }

    // checkbox.hpp
    namespace detail
    {
        struct checkbox_state
        {
            nyx::window win;
            bool & value;
        };

        static void change_checkbox_state(checkbox_state & value, point const& x)
        {
            state.value = !state.value;

            iris::paint(state.win);
        }

        static void checkbox_paint(checkbox_state & value, khaos::image_view const& vw)
        {
#error TODO
        }
    }

    nyx::window checkbox(bool & value, nyx::window const& p)
    {
        // create window
        nyx::window win = nyx::create_child_window(0, 0, , , p);

        detail::checkbox_state state = {win, value};

        // create event handler
        win.events() = iris::context(state)
        [
            (on::l_button_up | on::l_button_down)   [detail::change_checkbox_state],
            on::paint                               [detail::checkbox_paint]
        ]

        return win;
    }

    // init.hpp
    void init() {}
    void init(std::string const& extra, std::string const& dummy) {}
}}

MYTHOS_KHAOS_ANNEX_IMPLEMENT_PLUGIN(mythos::gaia::detail::plugin_impl);

