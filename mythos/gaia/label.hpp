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

#if !defined( MYTHOS_GAIA_LABEL_HPP )
#define MYTHOS_GAIA_LABEL_HPP

#include <mythos/gaia/config.hpp>

#include <mythos/nyx/window.hpp>

#include <mythos/khaos/font.hpp>
#include <mythos/khaos/render_text.hpp>
#include <mythos/khaos/text_extents.hpp>
#include <mythos/khaos/image.hpp>

#include <boost/type_traits/is_convertible.hpp>

#include <string>

namespace mythos { namespace gaia
{
    namespace detail
    {
        MYTHOS_GAIA_DECL nyx::window label(
            std::string const& x,
            void const* fnt,
            void (*renderer)(void const*, khaos::image_view const&, std::string const&),
            nyx::point const& extents,
            nyx::window parent
        );

        template <typename F>
        void label_renderer(void const* fnt, khaos::image_view const& vw, std::string const& x)
        {
            khaos::render_text_line(*static_cast<F const*>(fnt), vw, x);
        }

        template <typename F>
        nyx::point text_extents(F const& fnt, std::string const& x, size_t maxx, size_t maxy)
        {
            if (!maxx || !maxy)
            {
                return text_extents(fnt, x);
            }

            return nyx::point(maxx * average_char_width(fnt), maxy * line_length(fnt));
        }
    }

    template <typename F>
    inline nyx::window label(
        std::string const& x,
        F const& fnt,
        size_t maxx = 0,
        size_t maxy = 0,
        nyx::window parent = nyx::window()
    )
    {
        return detail::label(
            x, &fnt, detail::label_renderer<F>, detail::text_extents(fnt, x, maxx, maxy), parent
        );
    }

    MYTHOS_GAIA_DECL nyx::window label(std::string const& x);
}}

namespace mythos { namespace nyx
{
    inline window generate_window(std::string const& x)
    {
        return gaia::label(x);
    }

    inline window generate_window(char const* x)
    {
        return gaia::label(x);
    }
}}

#endif // #if !defined( MYTHOS_GAIA_LABEL_HPP )

