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

#if !defined( MYTHOS_IRIS_R_BUTTON_UP_HPP )
#define MYTHOS_IRIS_R_BUTTON_UP_HPP

#include <mythos/iris/detail/declare_event.hpp>

#include <mythos/nyx/point.hpp>
#include <mythos/nyx/window.hpp>

#include <mythos/khaos/event/r_button_up.hpp>

MYTHOS_IRIS_DECLARE_EVENT(mythos::khaos::r_button_up, r_button_up)

namespace mythos { namespace iris
{
    inline bool r_button_up(nyx::window const& win, nyx::point const& pt)
    {
        return khaos::r_button_up::raise(win.khaos_window(), pt);
    }
}}

#endif // #if !defined( MYTHOS_IRIS_R_BUTTON_UP_HPP )

