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

#if !defined( MYTHOS_NYX_DIRECTIVE_CENTER_HPP )
#define MYTHOS_NYX_DIRECTIVE_CENTER_HPP

#include <mythos/nyx/config.hpp>
#include <mythos/nyx/cell.hpp>
#include <mythos/nyx/is_directive.hpp>
#include <mythos/nyx/terminal.hpp>

#include <boost/xpressive/proto/proto.hpp>

namespace mythos { namespace nyx
{
    namespace directive
    {
        struct center {};

        MYTHOS_NYX_DECL void layout(center, cell & x);
    }

    terminal<directive::center> const center;
}}

MYTHOS_NYX_IS_DIRECTIVE(mythos::nyx::directive::center)

#endif // #if !defined( MYTHOS_NYX_DIRECTIVE_CENTER_HPP )

