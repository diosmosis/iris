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

#if !defined( MYTHOS_GAIA_TEXT_HPP )
#define MYTHOS_GAIA_TEXT_HPP

#include <mythos/gaia/config.hpp>
#include <mythos/gaia/label.hpp>

#include <mythos/khaos/font.hpp>

#include <string>

namespace mythos { namespace gaia
{
    namespace detail
    {
        MYTHOS_GAIA_DECL nyx::window text(
            std::string & x,
            void const* fnt,
            void (*renderer)(void const*, khaos::image_view const&, std::string const&),
            nyx::point const& extents,
            nyx::window parent
        );

        template <typename F>
        void text_renderer(void const* fnt, khaos::image_view const& vw, std::string const& x)
        {
            khaos::render_text(*static_cast<F const*>(fnt), vw, x);
        }
    }

    template <typename F>
    inline nyx::window text(
        std::string & x,
        F const& fnt,
        size_t maxx = 0,
        size_t maxy = 0,
        nyx::window parent = nyx::window()
    )
    {
        void (*renderer)(void const*, khaos::image_view const&, std::string const&);

        if (maxx == 0 && maxy == 0)
            renderer = detail::label_renderer<F>;
        else
            renderer = detail::text_renderer<F>;

        return detail::text(x, &fnt, renderer, detail::text_extents(fnt, x, maxx, maxy), parent);
    }

    MYTHOS_GAIA_DECL nyx::window text(std::string & x);
}}

#endif // #if !defined( MYTHOS_GAIA_TEXT_HPP )

