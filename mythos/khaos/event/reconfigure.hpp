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

#if !defined( MYTHOS_KHAOS_EVENT_RECONFIGURE_HPP )
#define MYTHOS_KHAOS_EVENT_RECONFIGURE_HPP

#include <mythos/khaos/config.hpp>

#include <mythos/khaos/event/ids.hpp>
#include <mythos/khaos/event_from_id.hpp>

#include <mythos/khaos/point.hpp>

#include <boost/mpl/int.hpp>

namespace mythos { namespace khaos
{
    struct window;

    namespace detail
    {
        MYTHOS_KHAOS_DECL point reconfigure_space(void * info);
    }

    // sent when child window has room to expand
    // args: point space
    struct reconfigure : boost::mpl::int_<evt::reconfigure>
    {
        MYTHOS_KHAOS_DECL static bool raise(window * win, point pt);

        template <typename F>
        static bool on_event(F & f, void * info)
        {
            f(detail::reconfigure_space(info));

            return true;
        }

        template <typename F, typename S>
        static bool on_event(F & f, void * info, S & s)
        {
            f(s, detail::reconfigure_space(info));

            return true;
        }
    };
}}

MYTHOS_KHAOS_EVENT_FROM_ID(mythos::khaos::reconfigure);

#endif // #if !defined( MYTHOS_KHAOS_EVENT_RECONFIGURE_HPP )

