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

#if !defined( MYTHOS_KHAOS_MODULE_HPP )
#define MYTHOS_KHAOS_MODULE_HPP

#include <mythos/khaos/config.hpp>

#include <boost/intrusive/list_hook.hpp>

namespace mythos { namespace khaos
{
    struct module
        : boost::intrusive::list_base_hook<
            boost::intrusive::link_mode<boost::intrusive::normal_link>
        >
    {
        void (*load)(int *, char ***);
        void (*unload)();
    };

    MYTHOS_KHAOS_DECL void load_modules(int * argc, char *** argv);
    MYTHOS_KHAOS_DECL void unload_modules();

    MYTHOS_KHAOS_DECL void register_module(module & x);
}}

#endif // #if !defined( MYTHOS_KHAOS_MODULE_HPP )

