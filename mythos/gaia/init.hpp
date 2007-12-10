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

#if !defined( MYTHOS_GAIA_INIT_HPP )
#define MYTHOS_GAIA_INIT_HPP

#include <mythos/gaia/config.hpp>

#include <string>

namespace mythos { namespace gaia
{
    MYTHOS_GAIA_DECL void init();

    // in pluggable build, extra is file to load
    MYTHOS_GAIA_DECL void init(std::string const& extra);

    // 2nd parameter only used in pluggable gaia build.  not used in plugins
    MYTHOS_GAIA_DECL void init(std::string const& extra, std::string const& dummy);
}}

#endif // #if !defined( MYTHOS_GAIA_INIT_HPP )

