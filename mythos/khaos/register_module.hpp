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

// should only be included by source files
#if !defined( MYTHOS_KHAOS_REGISTER_MODULE_HPP )
#define MYTHOS_KHAOS_REGISTER_MODULE_HPP

#include <mythos/khaos/module.hpp>

#define MYTHOS_KHAOS_REGISTER_MODULE(mod)                   \
    template struct mythos::khaos::module_registrar<mod> 

namespace mythos { namespace khaos
{
    template <typename M>
    struct module_registrar
    {
        struct impl_type
        {
            impl_type()
            {
                mod.load = module_registrar<M>::load;
                mod.unload = module_registrar<M>::unload;

                register_module(mod);
            }
        };

        static void load(int * argc, char *** argv)
        {
            mod_impl.load(argc, argv);
        }

        static void unload()
        {
            mod_impl.unload();
        }

        static M mod_impl;
        static module mod;
        static impl_type registrar;
    };

    template <typename M>
    M module_registrar<M>::mod_impl = M();

    template <typename M>
    module module_registrar<M>::mod = module();

    template <typename M>
    typename module_registrar<M>::impl_type module_registrar<M>::registrar =
        typename module_registrar<M>::impl_type();
}}

#endif // #if !defined( MYTHOS_KHAOS_REGISTER_MODULE_HPP )

