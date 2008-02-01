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

// FIXME!!! THIS DOES NOT BELONG IN MYTHOS.

#if !defined( MYTHOS_KHAOS_TEXT_EXTENTS_HPP )
#define MYTHOS_KHAOS_TEXT_EXTENTS_HPP

#include <mythos/khaos/config.hpp>

#include <mythos/khaos/glyph_image.hpp>
#include <mythos/khaos/point.hpp>

#include <string>

namespace mythos { namespace khaos
{
    template <typename Font>
    inline point text_extents(Font const& fnt, std::string const& x, size_t maxx)
    {
        typedef typename Font::glyph_type glyph;

        BOOST_ASSERT(maxx != 0);

        point result(0, 0);
        size_t i = 0;

        while (i != x.size())
        {
            int w = 0;

            if (x[i] == ' ')
            {
                w = average_char_width(fnt);
            }
            else
            {
                glyph const& glyph_i = find_glyph(fnt, x[i]);

                w = glyph_i.view.width() + glyph_i.bearing.x;
            }

            ++i;

            for (int j = 0; j != maxx && i != x.size(); ++j, ++i)
            {
                if (x[i] == ' ')
                {
                    w += average_char_width(fnt);
                }
                else
                {
                    glyph const& glyph_j = find_glyph(fnt, x[i]);

                    w += kerning_distance(fnt, x[i - 1], x[i]);
                    w += glyph_j.view.width() + glyph_j.bearing.x;
                }
            }

            result.x = std::max(result.x, w);
            result.y += line_length(fnt);
        }

        return result;
    }

    template <typename Font>
    inline point text_extents(Font const& fnt, std::string const& x)
    {
        typedef typename Font::glyph_type glyph;

        point result(0, 0);

        if (x.empty())
            return result;

        if (x[0] == ' ')
        {
            result.x = average_char_width(fnt);
        }
        else
        {
            glyph const& first = find_glyph(fnt, x[0]);

            result.x = first.view.width() + first.bearing.x;
        }

        result.y = line_length(fnt);

        for (size_t i = 1; i != x.size(); ++i)
        {
            if (x[i] == ' ')
            {
                result.x += average_char_width(fnt);
            }
            else
            {
                glyph const& g = find_glyph(fnt, x[i]);

                result.x += kerning_distance(fnt, x[i - 1], x[i]);
                result.x += g.view.width() + g.bearing.x;
            }
        }

        return result;
    }
}}

#endif // #if !defined( MYTHOS_KHAOS_TEXT_EXTENTS_HPP )

