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

#if !defined( MYTHOS_SUPPORT_BYTE_ALIGNED_PIXEL_HPP )
#define MYTHOS_SUPPORT_BYTE_ALIGNED_PIXEL_HPP

#include <boost/gil/color_base.hpp>
#include <boost/gil/metafunctions.hpp>

#include <boost/type_traits/alignment_of.hpp>
#include <boost/type_traits/aligned_storage.hpp>

#include <boost/utility/enable_if.hpp>

// i do not like gcc.
namespace mythos
{
    template <typename P, int N>
    struct byte_aligned_pixel;
}

namespace boost { namespace gil
{
    template <int N_, typename P, int N>
    inline typename boost::gil::element_reference_type<P>::type at_c(mythos::byte_aligned_pixel<P, N> & p);

    template <int N_, typename P, int N>
    inline typename boost::gil::element_const_reference_type<P>::type at_c(mythos::byte_aligned_pixel<P, N> const& p);
}}

#include <boost/gil/pixel.hpp>

namespace mythos
{
    // pixel/number of bytes
    template <typename P, int N>
    struct byte_aligned_pixel
        : boost::gil::detail::homogeneous_color_base_impl<
            typename boost::gil::channel_type<P>::type,
            typename P::layout_t
        >
    {
        typedef typename boost::gil::channel_type<P>::type channel_t;

        typedef byte_aligned_pixel value_type;
        typedef byte_aligned_pixel & reference;
        typedef byte_aligned_pixel const& const_reference;

        enum
        {
            N_ = sizeof(P) + N - (sizeof(P) % N),

            is_mutable = P::is_mutable
        };

        // TODO: use file iteration
        byte_aligned_pixel()
        {
            new (impl.address()) P();
        }

        explicit byte_aligned_pixel(channel_t v)
        {
            new (impl.address()) P(v);
        }

        byte_aligned_pixel(channel_t v0, channel_t v1)
        {
            new (impl.address()) P(v0, v1);
        }

        byte_aligned_pixel(channel_t v0, channel_t v1, channel_t v2)
        {
            new (impl.address()) P(v0, v1, v2);
        }

        byte_aligned_pixel(byte_aligned_pixel const& p)
        {
            new (impl.address()) P(p.pixel_impl());
        }

        template <typename P0>
        byte_aligned_pixel(P0 const& p)
        {
            new (impl.address()) P(p);
        }

        byte_aligned_pixel & operator = (byte_aligned_pixel const& x)
        {
            pixel_impl() = x.pixel_impl();

            return *this;
        }

        template <typename P0>
        byte_aligned_pixel & operator = (P0 const& p)
        {
            pixel_impl() = p;

            return *this;
        }

        template <typename P0>
        bool operator == (P0 const& p) const
        {
            return pixel_impl() == p;
        }

        template <typename P0>
        bool operator != (P0 const& p) const
        {
            return pixel_impl() != p;
        }

        // homogeneous pixels have operator[]
        typename boost::gil::channel_traits<channel_t>::reference
            operator [] (std::size_t i)
        {
            return pixel_impl()[i];
        }

        typename boost::gil::channel_traits<channel_t>::const_reference
            operator [] (std::size_t i) const
        {
            return pixel_impl()[i];
        }

        P & pixel_impl() { return *static_cast<P *>(impl.address()); }
        P const& pixel_impl() const { return *static_cast<P const*>(impl.address()); }

        boost::aligned_storage<N_, boost::alignment_of<P>::value> impl;
    };
}

namespace boost { namespace gil
{
    template <typename P, int N>
    struct channel_type<mythos::byte_aligned_pixel<P, N> >
        : channel_type<P>
    {};

    template <typename P, int N>
    struct is_planar<mythos::byte_aligned_pixel<P, N> >
        : is_planar<P>
    {};

    template <typename P, int N>
    struct pixel_reference_is_basic<mythos::byte_aligned_pixel<P, N> &>
        : pixel_reference_is_basic<P &>
    {};

    template <typename P, int N>
    struct pixel_reference_is_basic<mythos::byte_aligned_pixel<P, N> const&>
        : pixel_reference_is_basic<P const&>
    {};

    template <typename P, int N>
    struct color_space_type<mythos::byte_aligned_pixel<P, N> >
        : color_space_type<P>
    {};

    // iterator versions
    template <typename P, int N>
    struct channel_type<mythos::byte_aligned_pixel<P, N> *>
        : channel_type<P>
    {};

    template <typename P, int N>
    struct is_planar<mythos::byte_aligned_pixel<P, N> *>
        : is_planar<P>
    {};

    template <typename P, int N>
    struct iterator_is_basic<mythos::byte_aligned_pixel<P, N> *>
        : iterator_is_basic<P *>
    {};

    template <typename P, int N>
    struct iterator_is_basic<mythos::byte_aligned_pixel<P, N> const*>
        : iterator_is_basic<P const*>
    {};

    template <typename P, int N>
    struct color_space_type<mythos::byte_aligned_pixel<P, N> *>
        : color_space_type<P *>
    {};

    template <int N_, typename P, int N>
    inline typename boost::gil::element_reference_type<P>::type at_c(mythos::byte_aligned_pixel<P, N> & p)
    {
        return p.pixel_impl().at(boost::mpl::int_<N_>());
    }

    template <int N_, typename P, int N>
    inline typename boost::gil::element_const_reference_type<P>::type at_c(mythos::byte_aligned_pixel<P, N> const& p)
    {
        return p.pixel_impl().at(boost::mpl::int_<N_>());
    }
}}

#endif // #if !defined( MYTHOS_SUPPORT_BYTE_ALIGNED_PIXEL_HPP )

