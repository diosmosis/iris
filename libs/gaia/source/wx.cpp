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

#include <wx/wx.h>

namespace mythos { namespace gaia
{
    namespace detail
    {
        static wxWindow * wxwin_of(nyx::window const& win)
        {
            khaos::window * kw = win.khaos_window();

            return kw ? *static_cast<wxWindow **>(khaos::handle_of(kw)) : NULL;
        }

        static wxWindow * child_wxwin_of(nyx::window const& win)
        {
            wxWindow * p = wxwin_of(win);

            return p ? p : khaos::detail::get_parentless_child_win_parent();
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
    namespace detail
    {
        struct mythos_button : public wxButton
        {
            mythos_button(boost::function<void ()> const& a, nyx::window const& p)
                : wxButton(child_wxwin_of(p), wxID_ANY), action(a)
            {
                this->Connect(wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &mythos_button::on_click);
            }

            void on_click(wxEvent &)
            {
                action();
            }

            boost::function<void ()> action;

            DECLARE_EVENT_TABLE()
        };
    }

    nyx::window button(boost::function<void ()> const& action, nyx::window const& p)
    {
        detail::mythos_button * btn = new detail::mythos_button(action, p);

        // create window
        nyx::window win = nyx::foreign_create_window(&btn);

        return win;
    }

    // checkbox.hpp
    namespace detail
    {
        struct mythos_checkbox : public wxCheckBox
        {
            mythos_checkbox(bool & v, nyx::window const& p)
                : wxCheckBox(child_wxwin_of(p), wxID_ANY, wxString()), value(v)
            {
                this->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction) &mythos_checkbox::on_click);
                this->Connect(wxEVT_PAINT, (wxObjectEventFunction) &mythos_checkbox::on_paint);
            }

            void on_click(wxEvent &)
            {
                value = !value;
            }

            void on_paint(wxEvent &)
            {
                this->SetValue(value);
            }

            bool & value;

            DECLARE_EVENT_TABLE()
        };
    }

    nyx::window checkbox(bool & value, nyx::window const& p)
    {
        detail::mythos_checkbox * cb = new detail::mythos_checkbox(value, p);

        // create window
        nyx::window win = nyx::foreign_create_window(&cb);

        return win;
    }

    // init.hpp
    void init() {}
    void init(std::string const& extra) {}
    void init(std::string const& extra, std::string const& dummy) {}
}}

// for some reason, needs to be before the event tables...
MYTHOS_KHAOS_ANNEX_IMPLEMENT_PLUGIN(mythos::gaia::interface);

BEGIN_EVENT_TABLE(mythos::gaia::detail::mythos_button, wxButton)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(mythos::gaia::detail::mythos_checkbox, wxCheckBox)
END_EVENT_TABLE()

