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

#if !defined( MYTHOS_KHAOS_PLATFORM_WX_HPP )
#define MYTHOS_KHAOS_PLATFORM_WX_HPP

#include <mythos/khaos/point.hpp>

#include <boost/gil/typedefs.hpp>
#include <boost/gil/algorithm.hpp>

#include <wx/wx.h>

#include <string>

// FIXME: not sure what to set this to...
#define MYTHOS_KHAOS_WINDOW_SIZE_ADD 0

#define MYTHOS_KHAOS_IMAGE_TYPE                                     \
    boost::gil::image<boost::gil::rgb8_pixel_t, false>

// not actually used
#define MYTHOS_KHAOS_GLYPH_IMAGE_TYPE                               \
    boost::gil::gray8_image_t

namespace mythos { namespace khaos
{
    struct font;

    MYTHOS_KHAOS_DECL void render_text(
        font const& fnt, wxBitmap & bmp, std::string const& str, wxColour const& color
    );

    MYTHOS_KHAOS_DECL point text_extents(font const& fnt, std::string const& x, size_t maxx);
    MYTHOS_KHAOS_DECL point text_extents(font const& fnt, std::string const& x);

    namespace detail
    {
        MYTHOS_KHAOS_DECL wxWindow * get_parentless_child_win_parent();
    }

    // TODO: put optimization for rgb8_view_t
    template <typename View>
    inline void render_text(
        font const& fnt,
        View const& vw,
        std::string const& str,
        typename View::value_type const& color = typename View::value_type(0)
    )
    {
        boost::gil::rgb8_pixel_t copy = color;

        wxColour wxcolor(boost::gil::at_c<0>(copy), boost::gil::at_c<1>(copy), boost::gil::at_c<2>(copy));

        wxImage img(vw.width(), vw.height());

        boost::gil::copy_pixels(
            vw, boost::gil::interleaved_view(
                img.GetWidth(), img.GetHeight(), (boost::gil::rgb8_pixel_t *) img.GetData(), img.GetWidth() * 3
            )
        );

        wxBitmap bmp(img);// = img.ConvertToBitmap();

        render_text(fnt, bmp, str, wxcolor);

        img = bmp.ConvertToImage();

        boost::gil::copy_pixels(
            boost::gil::interleaved_view(
                img.GetWidth(), img.GetHeight(), (boost::gil::rgb8_pixel_t *) img.GetData(), img.GetWidth() * 3
            ), vw
        );
    }

    // FIXME: won't render in one line
    template <typename View>
    inline void render_text_line(
        font const& fnt,
        View const& vw,
        std::string const& str,
        typename View::value_type const& color = typename View::value_type(0)
    )
    {
        render_text(fnt, vw, str, color);
    }
}}

#endif // #if !defined( MYTHOS_KHAOS_PLATFORM_WX_HPP )

