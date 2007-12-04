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

// FIXME!!! THIS DOES NOT BELONG IN MYTHOS.

#if !defined( MYTHOS_KHAOS_RENDER_TEXT_HPP )
#define MYTHOS_KHAOS_RENDER_TEXT_HPP

#include <mythos/khaos/glyph_image.hpp>
#include <mythos/khaos/font.hpp>

#include <string>
#include <iostream>
namespace mythos { namespace khaos
{
    namespace detail
    {
        struct glyph_alpha_blend
        {
            typedef boost::gil::channel_type<glyph_pixel>::type channel;

            glyph_alpha_blend(glyph_pixel const& a)
                : alpha(a), max_alpha(boost::gil::channel_traits<channel>::max_value())
            {}

            template <typename T0, typename T1>
            void operator()(T0 & dst, T1 src) const
            {
        		double dbl = (dst * alpha - src * alpha + src * max_alpha) / max_alpha;
        		dst = (T0) dbl;
            }

            channel alpha;
            channel max_alpha;
        };

        template <typename SrcV, typename DstV>
        inline void draw_glyph(
            SrcV const& glyph,
            DstV const& vw,
            typename DstV::x_coord_t x,
            typename DstV::y_coord_t y,
            typename DstV::value_type const& color
        )
        {
            typedef typename DstV::x_coord_t    x_t;
            typedef typename DstV::y_coord_t    y_t;
            typedef typename DstV::value_type   dest_pixel;

            x_t w = (x + glyph.width() < vw.width()) ? glyph.width() : std::max(vw.width() - x, 0);
            y_t h = (y + glyph.height() < vw.height()) ? glyph.height() : std::max(vw.height() - y, 0);

            // TODO: should be using iterators
            for (x_t i = 0; i != w; ++i)
                for (y_t j = 0; j != h; ++j)
                {
                    glyph_alpha_blend ftor(glyph(i, j));
                    dest_pixel pix = color;

                    boost::gil::static_for_each(pix, vw(i + x, j + y), ftor);

                    vw(i + x, j + y) = pix;
                }
        }
    }

    template <typename Font, typename View>
    inline void render_text(
        Font const& fnt,
        View const& vw,
        std::string const& str,
        typename View::value_type const& color = typename View::value_type(0)
    )
    {
        // if there is nothing to draw return
        if (str.empty() || (vw.width() == 0 && vw.height() == 0))
            return;

        typename View::x_coord_t x = 0;
        typename View::y_coord_t y = 0;

        int as = ascender(fnt);

        for (size_t i = 0; i != str.size(); ++i)
        {
            if (str[i] == ' ')
            {
                x += average_char_width(fnt);
            }
            else
            {
                typename Font::glyph_type const& glyph = find_glyph(fnt, str[i]);

                // if the current x pos is past the end, increment to the next row
                if (x + glyph.view.width() > vw.width())
                {
                    y += line_length(fnt);

                    x = 0;
                }

                // if the current y pos is past the end, there is no more room to draw
                // so return
                if (y > vw.height())
                    return;

                detail::draw_glyph(glyph.view, vw, x, y + as - glyph.bearing.y, color);

                // if there is more to draw, get the kerning distance
                int kd = 0;
                if (i + 1 != str.size())
                    kd = kerning_distance(fnt, str[i], str[i + 1]);

                x += glyph.view.width() + kd + glyph.bearing.x;
            }
        }
    }

    template <typename Font, typename View>
    inline void render_text_line(
        Font const& fnt,
        View const& vw,
        std::string const& str,
        typename View::value_type color = typename View::value_type(0)
    )
    {
        // if there is nothing to draw return
        if (str.empty() || vw.width() == 0 || vw.height() == 0)
            return;

        typename View::x_coord_t x = 0;

        int as = ascender(fnt);

        for (size_t i = 0; i != str.size(); ++i)
        {
            if (str[i] == ' ')
            {
                x += average_char_width(fnt);
            }
            else
            {
                typename Font::glyph_type const& glyph = find_glyph(fnt, str[i]);

                // if x is past the end, return
                if (x > vw.width())
                    return;

                detail::draw_glyph(glyph.view, vw, x, as - glyph.bearing.y, color);

                // if there is more to draw, get the kerning distance
                int kd = 0;
                if (i + 1 != str.size())
                    kd = kerning_distance(fnt, str[i], str[i + 1]);

                x += glyph.view.width() + kd + glyph.bearing.x;
            }
        }
    }
}}

#endif // #if !defined( MYTHOS_KHAOS_RENDER_TEXT_HPP )

