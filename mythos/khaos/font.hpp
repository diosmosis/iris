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

#if !defined( MYTHOS_KHAOS_FONT_HPP )
#define MYTHOS_KHAOS_FONT_HPP

#include <mythos/khaos/glyph_image.hpp>
#include <mythos/khaos/point.hpp>

#include <boost/optional.hpp>
#include <boost/noncopyable.hpp>

namespace mythos { namespace khaos
{
    namespace font_style
    {
        enum
        {
            italic = (1L << 0),
            underline = (1L << 1),
            strikethrough = (1L << 2)
        };
    }

    namespace font_weight
    {
        enum
        {
            thin        = 100,

            extra_light = 200,
            ultra_light = 200,

            light       = 300,

            normal      = 400,
            regular     = 400,

            medium      = 500,

            semi_bold   = 600,
            demi_bold   = 600,

            bold        = 700,

            extra_bold  = 800,
            ultra_bold  = 800,

            heavy       = 900,
            black       = 900
        };
    }

    struct glyph
    {
        typedef glyph_view      glyph_view_type;

        glyph_view view;
        point bearing;
        int advance;
    };

    struct font : boost::noncopyable
    {
        typedef char            char_type;
        typedef int             kerning_dist_type;
        typedef glyph           glyph_type;

        typedef void * font::*  unspecified_bool_t;

        MYTHOS_KHAOS_DECL font();
        MYTHOS_KHAOS_DECL ~font();

        operator unspecified_bool_t() const
        {
            return extra_data ? &font::extra_data : 0;
        }

        // FIXME: should be named pimpl
        void * extra_data;
    };

    MYTHOS_KHAOS_DECL void * handle_of(font const& f);

    MYTHOS_KHAOS_DECL glyph const& find_glyph(font const& f, char c);
    MYTHOS_KHAOS_DECL int kerning_distance(font const& f, char a, char b);

    MYTHOS_KHAOS_DECL int average_char_width(font const& f);
    MYTHOS_KHAOS_DECL int average_char_height(font const& f);

    MYTHOS_KHAOS_DECL int ascender(font const& f);
    MYTHOS_KHAOS_DECL int descender(font const& f);

    // height if vertical font, width if horizontal
    MYTHOS_KHAOS_DECL int line_length(font const& f);

    // this should be its own lib...
    // will always find a font
    MYTHOS_KHAOS_DECL void find_font(
        font & f,
        std::string const& name,
        int height,                         // in pixels
        int weight = font_weight::normal,
        int style = 0
    );

    MYTHOS_KHAOS_DECL font const& default_font();
}}

#endif // #if !defined( MYTHOS_KHAOS_FONT_HPP )

