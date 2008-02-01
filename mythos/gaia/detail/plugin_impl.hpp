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

// should only be used by gaia implementations
// also, plugin_impl will only work if the particular implementation has no global
// state or it is not intended for the user to load/use two instances of the same plugin.
// if such a set up is required, one must subclass gaia::plugin::impl directly, and use
// that class in MYTHOS_KHAOS_ANNEX_IMPLEMENT_PLUGIN.

// NOTE: if there is a way to reload a shared library, but use newly allocated memory for
// its global data, this can be fixed

// GAIA MUST BE STATICALLY LINKED!!!
// NOTE: it seems that, for some reason, the calls plugin_impl makes all redirect to the original
// gaia, and not the specific plugin.  since the original gaia does not get linked to, whatever
// happens, happens at at too low a level, which means the only way plugin_impl can be used is if
// static linking is used

#if !defined( MYTHOS_GAIA_DETAIL_PLUGIN_IMPL_HPP )
#define MYTHOS_GAIA_DETAIL_PLUGIN_IMPL_HPP

#include <mythos/gaia/plugin.hpp>
#include <mythos/gaia/init.hpp>

namespace mythos { namespace gaia { namespace detail
{
    struct plugin_impl : plugin::impl
    {
        plugin_impl()
        {
            init();
        }

        plugin_impl(std::string const& extra)
        {
            init(extra);
        }

        nyx::window label(
            std::string const& x,
            void const* fnt,
            void (*renderer)(void const*, khaos::image_view const&, std::string const&),
            nyx::point const& extents,
            nyx::window parent
        )
        {
            return gaia::detail::label(x, fnt, renderer, extents, parent);
        }

        nyx::window label(std::string const& x)
        {
            return gaia::label(x);
        }

        // text.hpp
        nyx::window text(
            std::string & x,
            void const* fnt,
            void (*renderer)(void const*, khaos::image_view const&, std::string const&),
            nyx::point const& extents,
            nyx::window parent
        )
        {
            return gaia::detail::text(x, fnt, renderer, extents, parent);
        }

        nyx::window text(std::string & x)
        {
            return gaia::text(x);
        }

        // image.hpp
        nyx::window image(
            boost::any const& view,
            void (*render)(boost::any const& img, khaos::image_view const& vw),
            int w, int h,
            nyx::window const& parent
        )
        {
            return gaia::detail::image(view, render, w, h, parent);
        }

        // button.hpp
        nyx::window button(boost::function<void ()> const& action, nyx::window const& p)
        {
            return gaia::button(action, p);
        }

        // checkbox.hpp
        nyx::window checkbox(bool & value, nyx::window const& p)
        {
            return gaia::checkbox(value, p);
        }
    };
}}}

#endif // #if !defined( MYTHOS_GAIA_DETAIL_PLUGIN_IMPL_HPP )

