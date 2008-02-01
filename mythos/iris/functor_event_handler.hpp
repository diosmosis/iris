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

#if !defined( MYTHOS_IRIS_FUNCTOR_EVENT_HANDLER_HPP )
#define MYTHOS_IRIS_FUNCTOR_EVENT_HANDLER_HPP

#include <mythos/khaos/event/paint.hpp>
#include <mythos/khaos/event/button.hpp>

#include <boost/assert.hpp>

#define MYTHOS_IRIS_FUNCTOR_EVENT_HANDLER_CASE(name)                    \
    case khaos::evt::name:                                              \
        return khaos::name::on_event(                                   \
            detail::on_event<khaos::name, Derived>(derived()), info)

namespace mythos { namespace iris
{
    namespace detail
    {
        template <typename E, typename D>
        struct on_event
        {
            on_event(D & f_) : f(f_) {}

            // FIXME: need a better solution than this... possibly using phoenix
            template <typename A0>
            void operator()(A0 & a0) const
            {
                f.on(E(), a0);
            }

            template <typename A0, typename A1>
            void operator()(A0 & a0, A1 & a1) const
            {
                f.on(E(), a0, a1);
            }

            template <typename A0, typename A1, typename A2>
            void operator()(A0 & a0, A1 & a1, A2 & a2) const
            {
                f.on(E(), a0, a1, a2);
            }

            D & f;
        };
    }

    template <typename Derived>
    struct functor_event_handler
    {
        bool operator()(int et, void * info)
        {
            switch (et)
            {
            MYTHOS_IRIS_FUNCTOR_EVENT_HANDLER_CASE(paint);
            MYTHOS_IRIS_FUNCTOR_EVENT_HANDLER_CASE(l_button_up);
            MYTHOS_IRIS_FUNCTOR_EVENT_HANDLER_CASE(m_button_up);
            MYTHOS_IRIS_FUNCTOR_EVENT_HANDLER_CASE(r_button_up);
            MYTHOS_IRIS_FUNCTOR_EVENT_HANDLER_CASE(l_button_down);
            MYTHOS_IRIS_FUNCTOR_EVENT_HANDLER_CASE(m_button_down);
            MYTHOS_IRIS_FUNCTOR_EVENT_HANDLER_CASE(r_button_down);
            MYTHOS_IRIS_FUNCTOR_EVENT_HANDLER_CASE(mouse_move);
            default:
                BOOST_ASSERT(false);

                return false;
            };
        }

        Derived & derived() { return *static_cast<Derived *>(this); }
    };
}}

#undef MYTHOS_IRIS_FUNCTOR_EVENT_HANDLER_CASE

#endif // #if !defined( MYTHOS_IRIS_FUNCTOR_EVENT_HANDLER_HPP )

