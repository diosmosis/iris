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

#define MYTHOS_KHAOS_SOURCE

#include <mythos/khaos/font.hpp>
#include <mythos/khaos/point.hpp>

#include <mythos/khaos/register_module.hpp>

#include <boost/none.hpp>

#include <map>
#include <stdexcept>

#include <fontconfig/fontconfig.h>

#include <ft2build.h>
#include FT_FREETYPE_H

static FT_Library ftlib;
static mythos::khaos::font default_font;

namespace mythos { namespace khaos
{
    struct glyph_impl : glyph
    {
        glyph_image img;
    };

    struct ft_font_extra
    {
        ft_font_extra() : height(0), width(0), ascender(0), descender(0) {}

        FT_Face face;
        int height, width, ascender, descender;
        std::map<char, glyph_impl> glyphs;
    };

    // modules
    struct ft_module
    {
        void load(int * argc, char *** argv)
        {
            // init freetype library
            if (FT_Init_FreeType(&::ftlib))
                throw std::runtime_error("khaos(freetype): Failed to initialize FreeType.");

            // init font config library
            if (!FcInit())
                throw std::runtime_error("khaos(freetype): Failed to initialize fontconfig.");

            // FIXME: shouldn't use find_font for this
            // load default font
            find_font(::default_font, "Monospace", 12);
        }

        void unload()
        {
            FcFini();

            FT_Done_FreeType(::ftlib);
        }
    };

    // font.hpp
    namespace detail
    {
        static point font_units_to_pixels(FT_Face face, int fux, int fuy)
        {
            FT_Size_Metrics * metrics = &face->size->metrics; /* shortcut */

            double pixels_x, pixels_y;
            double em_size, x_scale, y_scale;

            /* compute floating point scale factors */
            em_size = 1.0 * face->units_per_EM;
            x_scale = metrics->x_ppem / em_size;
            y_scale = metrics->y_ppem / em_size;

            /* convert design distances to floating point pixels */
            pixels_x = fux * x_scale;
            pixels_y = fuy * y_scale;

            return point(lround(pixels_x), lround(pixels_y));
        }
    }

    font::font() : extra_data(0) {}

    font::~font()
    {
        if (!extra_data)
            return;

        FT_Done_Face(static_cast<ft_font_extra *>(extra_data)->face);
    }

    void * handle_of(font const& f)
    {
        return &static_cast<ft_font_extra *>(f.extra_data)->face;
    }

    int kerning_distance(font const& f, char a, char b)
    {
        BOOST_ASSERT(f);

        FT_Face face = static_cast<ft_font_extra *>(f.extra_data)->face;

        if (!FT_HAS_KERNING(face))
            return 0;

        int ag = FT_Get_Char_Index(face, a);
        int bg = FT_Get_Char_Index(face, b);

        // FIXME: dist isn't guaranteed to be in F26Dot6, though all the code i've seen assumes it is
        FT_Vector dist;
        FT_Get_Kerning(face, ag, bg, FT_KERNING_DEFAULT, &dist);

        return (dist.x >> 6);
    }

    glyph const& find_glyph(font const& f, char c)
    {
        typedef std::map<char, glyph_impl>::iterator   iterator;

        BOOST_ASSERT(f);

        ft_font_extra * extra = static_cast<ft_font_extra *>(f.extra_data);

        iterator i = extra->glyphs.find(c);

        // if c already has a cached image in glyphs, return it, otherwise create one
        if (i != extra->glyphs.end())
            return i->second;
        else
            i = extra->glyphs.insert(std::make_pair(c, glyph_impl())).first;

        // load glyph
        int error = FT_Load_Char(extra->face, c, FT_LOAD_RENDER);

        BOOST_ASSERT(!error);

        FT_GlyphSlot slot = extra->face->glyph;

        int w = slot->metrics.width >> 6;
        int h = slot->metrics.height >> 6;

        // create glyph image
        glyph_image & img = i->second.img;
        img.recreate(w, h); // TODO: check if there's an alignment to optimize copy_pixels below

        boost::gil::gray8_pixel_t * pixels = (boost::gil::gray8_pixel_t *) slot->bitmap.buffer;

        boost::gil::copy_pixels(
            boost::gil::interleaved_view(w, h, pixels, slot->bitmap.width),
            boost::gil::view(img)
        );

        i->second.view = boost::gil::view(i->second.img);

        // save advance/bearing
        i->second.bearing = point(slot->metrics.horiBearingX >> 6, slot->metrics.horiBearingY >> 6);
        i->second.advance = slot->metrics.horiAdvance >> 6;

        return i->second;
    }

    int average_char_width(font const& f)
    {
        return static_cast<ft_font_extra *>(f.extra_data)->width;
    }

    int average_char_height(font const& f)
    {
        return static_cast<ft_font_extra *>(f.extra_data)->height;
    }

    int line_length(font const& f)
    {
        return static_cast<ft_font_extra *>(f.extra_data)->height;
    }

    int ascender(font const& f)
    {
        return static_cast<ft_font_extra *>(f.extra_data)->ascender;
    }

    int descender(font const& f)
    {
        return static_cast<ft_font_extra *>(f.extra_data)->descender;
    }

    void find_font(font & f, std::string const& name, int height, int weight, int style)
    {
        FcValue fcval;

        // build pattern
        FcPattern * pattern = FcPatternCreate();

        // typeface name
        fcval.type = FcTypeString;
        fcval.u.s = (FcChar8 const*) name.c_str();
        FcPatternAdd(pattern, FC_FAMILY, fcval, FcTrue);

        // pixel size
        fcval.type = FcTypeInteger;
        fcval.u.i = height;
        FcPatternAdd(pattern, FC_PIXEL_SIZE, fcval, FcTrue);

        // italic
        if (style & font_style::italic)
        {
            fcval.type = FcTypeInteger;
            fcval.u.i = FC_SLANT_ITALIC;
            FcPatternAdd(pattern, FC_SLANT, fcval, FcTrue);
        }

        // weight
        fcval.type = FcTypeInteger;
        switch (weight)
        {
        case font_weight::thin:          fcval.u.i = FC_WEIGHT_THIN; break;
        case font_weight::extra_light:   fcval.u.i = FC_WEIGHT_EXTRALIGHT; break;
        case font_weight::light:         fcval.u.i = FC_WEIGHT_LIGHT; break;
        case font_weight::normal:        fcval.u.i = FC_WEIGHT_NORMAL; break;
        case font_weight::medium:        fcval.u.i = FC_WEIGHT_MEDIUM; break;
        case font_weight::semi_bold:     fcval.u.i = FC_WEIGHT_SEMIBOLD; break;
        case font_weight::bold:          fcval.u.i = FC_WEIGHT_BOLD; break;
        case font_weight::ultra_bold:    fcval.u.i = FC_WEIGHT_ULTRABOLD; break;
        case font_weight::heavy:         fcval.u.i = FC_WEIGHT_HEAVY; break;
        default:                         fcval.u.i = FC_WEIGHT_NORMAL; break;
        };
        FcPatternAdd(pattern, FC_WEIGHT, fcval, FcTrue);

        // transform pattern
        FcConfigSubstitute(NULL, pattern, FcMatchPattern);
        FcDefaultSubstitute(pattern);

        FcResult res;
        FcPattern * real_pattern = FcFontMatch(NULL, pattern, &res);

        FcPatternDestroy(pattern);

        // get filename
        if (real_pattern)
        {
            char * fname;
            int id;

            FcPatternGetString(real_pattern, FC_FILE, 0, (FcChar8 **) &fname);
            FcPatternGetInteger(real_pattern, FC_INDEX, 0, &id);

            // load font
            FT_Face face;
            if (FT_New_Face(::ftlib, fname, id, &face))
                return;

            FcPatternDestroy(real_pattern);

            // create font
            ft_font_extra * extra;
            f.extra_data = extra = new ft_font_extra();

            extra->face = face;

            FT_Set_Pixel_Sizes(face, 0, height);

            // get avg char width/height
            point pt = detail::font_units_to_pixels(face, face->max_advance_width, face->max_advance_height);
            extra->width = pt.x;
            extra->height = pt.y;

            pt = detail::font_units_to_pixels(face, 0, face->ascender);
            extra->ascender = pt.y;

            pt = detail::font_units_to_pixels(face, 0, face->descender);
            extra->descender = pt.y;
        }
    }

    font const& default_font()
    {
        return ::default_font;
    }
}}

MYTHOS_KHAOS_REGISTER_MODULE(mythos::khaos::ft_module);

