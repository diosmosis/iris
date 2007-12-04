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
#include <mythos/gaia/init.hpp>

#include <mythos/iris/event_handler.hpp>
#include <mythos/iris/dsel.hpp>

#include <exception>
#include <iostream>
static mythos::gaia::plugin gaiaimpl;

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
            return gaiaimpl.pimpl->label(x, fnt, renderer, extents, parent);
        }
    }

    nyx::window label(std::string const& x)
    {
        return gaiaimpl.pimpl->label(x);
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
            return gaiaimpl.pimpl->text(x, fnt, renderer, extents, parent);
        }
    }

    nyx::window text(std::string & x)
    {
        return gaiaimpl.pimpl->text(x);
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
            return gaiaimpl.pimpl->image(view, render, w, h, parent);
        }
    }

    // button.hpp
    nyx::window button(boost::function<void ()> const& action, nyx::window const& p)
    {
        return gaiaimpl.pimpl->button(action, p);
    }

    // checkbox.hpp
    nyx::window checkbox(bool & value, nyx::window const& p)
    {
        return gaiaimpl.pimpl->checkbox(value, p);
    }

    // init.hpp
    void init()
    {
        // TODO: should search for an appropriate default gaia lib
        throw std::runtime_error("gaia::init() not implemented");
    }

    // extra is file to load, dummy is extra data
    void init(std::string const& extra, std::string const& dummy)
    {
        gaiaimpl.load(extra, dummy);
    }
}}

