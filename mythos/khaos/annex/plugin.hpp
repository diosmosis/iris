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

#if !defined( MYTHOS_KHAOS_ANNEX_PLUGIN_HPP )
#define MYTHOS_KHAOS_ANNEX_PLUGIN_HPP

#include <boost/function_types/result_type.hpp>

#include <boost/config.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/assert.hpp>

#include <string>

#if defined(BOOST_WINDOWS) // windows
#   include <windows.h> // FIXME: GET RID OF THIS!!!

#   define MYTHOS_KHAOS_ANNEX_DECL __declspec(dllexport)

namespace mythos { namespace khaos { namespace annex { namespace detail
{
    typedef void * shared_library_handle;

    // TODO: put all WINAPI functions in a separate library that must be built from source
    inline shared_library_handle load_shared_library(std::string const& file)
    {
        void * result = (void *) LoadLibrary(file.c_str());

        BOOST_ASSERT(result);

        return result;
    }

    inline void unload_shared_library(shared_library_handle handle)
    {
        BOOST_ASSERT(handle);

        int result = FreeLibrary((HMODULE) handle);

        BOOST_ASSERT(result);
    }

    inline void * get_function_ptr(shared_library_handle handle, char const* str)
    {
        BOOST_ASSERT(handle && str);

        void * result = static_cast<void *>(GetProcAddress((HMODULE) handle, str));

        BOOST_ASSERT(result);

        return result;
    }
}}}}
#else // linux
#   include <dlfcn.h>

#   define MYTHOS_KHAOS_ANNEX_DECL

namespace mythos { namespace khaos { namespace annex { namespace detail
{
    typedef void * shared_library_handle;

    inline shared_library_handle load_shared_library(std::string const& file)
    {
        void * result = dlopen(file.c_str(), RTLD_LAZY);

        // FIXME: if !result throw here

        return result;
    }

    inline void unload_shared_library(shared_library_handle handle)
    {
        BOOST_ASSERT(handle);

        int result = dlclose(handle);

        BOOST_ASSERT(result == 0);
    }

    inline void * get_function_ptr(shared_library_handle handle, char const* str)
    {
        BOOST_ASSERT(handle && str);

        void * result = dlsym(handle, str);

        BOOST_ASSERT(result);

        return result;
    }
}}}}
#endif

#define MYTHOS_KHAOS_ANNEX_IMPLEMENT_PLUGIN(impl)                                       \
    extern "C"                                                                          \
    {                                                                                   \
        MYTHOS_KHAOS_ANNEX_DECL impl * annex_plugin_init() { return new impl(); }       \
        MYTHOS_KHAOS_ANNEX_DECL impl * annex_plugin_init_extra(std::string const& extra)\
            { return new impl(extra); }                                                 \
        MYTHOS_KHAOS_ANNEX_DECL void annex_plugin_cleanup(impl * lib) { delete lib; }   \
    }

// TODO: put debug parts to ensure type safety, at least in debug mode
// TODO: put some way of passing arguments to plugin
namespace mythos { namespace khaos { namespace annex
{
    struct shared_library
    {
        shared_library(std::string const& file)
        {
            handle = detail::load_shared_library(file);
        }

        ~shared_library()
        {
            detail::unload_shared_library(handle);
        }

        // TODO: Use file iteration
        template <typename Signature>
        typename boost::function_types::result_type<Signature>::type
            execute(char const* str)
        {
            Signature * fptr = (Signature *) detail::get_function_ptr(handle, str);

            return fptr();
        }

        template <typename Signature, typename A0>
        typename boost::function_types::result_type<Signature>::type
            execute(char const* str, A0 & a0)
        {
            Signature * fptr = (Signature *) detail::get_function_ptr(handle, str);

            return fptr(a0);
        }

        detail::shared_library_handle handle;
    };

    template <typename Impl>
    struct plugin
    {
        struct impl_type
        {
            impl_type(std::string const& file) : lib(file)
            {
                impl = lib.execute<Impl * ()>("annex_plugin_init");
            }

            impl_type(std::string const& file, std::string const& extra) : lib(file)
            {
                impl = lib.execute<Impl * (std::string const&)>("annex_plugin_init_extra", extra);
            }

            ~impl_type()
            {
                lib.execute<void (Impl *)>("annex_plugin_cleanup", impl);
            }

            shared_library lib;
            Impl * impl;
        };

        plugin() {}
        plugin(std::string const& file)
        {
            load(file);
        }

        plugin(std::string const& file, std::string const& extra)
        {
            load(file, extra);
        }

        void load(std::string const& file)
        {
            pimpl.reset(new impl_type(file));
        }

        void load(std::string const& file, std::string const& extra)
        {
            pimpl.reset(new impl_type(file, extra));
        }

        Impl * operator -> () const { return pimpl->impl; }

        boost::shared_ptr<impl_type> pimpl;
    };
}}}

#endif // #if !defined( MYTHOS_KHAOS_ANNEX_PLUGIN_HPP )

