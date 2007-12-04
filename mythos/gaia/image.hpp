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

#if !defined( MYTHOS_GAIA_IMAGE_HPP )
#define MYTHOS_GAIA_IMAGE_HPP

#include <mythos/gaia/config.hpp>

#include <mythos/nyx/window.hpp>

#include <mythos/khaos/image.hpp>

#include <boost/gil/algorithm.hpp>

#include <boost/any.hpp>

namespace mythos { namespace gaia
{
    namespace detail
    {
        MYTHOS_GAIA_DECL nyx::window image(
            boost::any const& view,
            void (*render)(boost::any const& img, khaos::image_view const& vw),
            int w, int h,
            nyx::window const& parent
        );

        template <typename View>
        inline void draw_image(boost::any const& img, khaos::image_view const& vw)
        {
            // TODO: should stretch
            boost::gil::copy_pixels(*boost::any_cast<View>(&img), vw);
        }
    }

    template <typename View>
    inline nyx::window image(View const& vw, nyx::window parent = nyx::window())
    {
        return detail::image(vw, detail::draw_image<View>, vw.width(), vw.height(), parent);
    }
}}

#endif // #if !defined( MYTHOS_GAIA_IMAGE_HPP )

