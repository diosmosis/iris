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

#if !defined( MYTHOS_KHAOS_EVENT_PAINT_HPP )
#define MYTHOS_KHAOS_EVENT_PAINT_HPP

#include <mythos/khaos/config.hpp>

#include <mythos/khaos/event/ids.hpp>
#include <mythos/khaos/event_from_id.hpp>

#include <mythos/khaos/image.hpp>

#include <boost/mpl/int.hpp>

namespace mythos { namespace khaos
{
    struct window;

    namespace detail
    {
        MYTHOS_KHAOS_DECL image_view view_from_event_info(void * info);
        MYTHOS_KHAOS_DECL bool draw_view(image_view const& vw, void * info);
    }

    struct paint : boost::mpl::int_<evt::paint>
    {
        MYTHOS_KHAOS_DECL static bool raise(window * win);

        template <typename F>
        static bool on_event(F & f, void * info)
        {
            image_view vw = detail::view_from_event_info(info);

            f(vw);

            return detail::draw_view(vw, info);
        }

        template <typename F, typename S>
        static bool on_event(F & f, void * info, S & s)
        {
            image_view vw = detail::view_from_event_info(info);

            f(s, vw);

            return detail::draw_view(vw, info);
        }
    };
}}

MYTHOS_KHAOS_EVENT_FROM_ID(mythos::khaos::paint)

#endif // #if !defined( MYTHOS_KHAOS_EVENT_PAINT_HPP )

