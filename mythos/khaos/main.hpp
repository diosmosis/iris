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

#if !defined( MYTHOS_KHAOS_MAIN_HPP )
#define MYTHOS_KHAOS_MAIN_HPP

#include <mythos/khaos/config.hpp>
#include <mythos/khaos/event_loop.hpp>
#include <mythos/khaos/module.hpp>
#include <mythos/khaos/window.hpp>

// TODO: if there's a way to get nCmdShow w/o a WinMain, this can be changed to
// #if defined(BOOST_MSVC)
#if defined(BOOST_WINDOWS) || defined(__WINE__)
#   include <vector>
#   include <windows.h>

#   define MYTHOS_KHAOS_IMPLEMENT_MAIN(init)                        \
    int WINAPI WinMain(                                             \
        HINSTANCE hInstance,                                        \
        HINSTANCE hPrevInstance,                                    \
        LPSTR lpCmdLine,                                            \
        int nCmdShow                                                \
    )                                                               \
    {                                                               \
        if (!mythos::khaos::entry(hInstance, nCmdShow))             \
            return 0;                                               \
                                                                    \
        std::vector<char *> cmdargs;                                \
        char * args = mythos::khaos::parse_cmd_line(cmdargs);       \
                                                                    \
        int argc = cmdargs.size();                                  \
        char ** argv = &cmdargs.front();                            \
                                                                    \
        mythos::khaos::load_modules(&argc, &argv);                  \
                                                                    \
        if (init(argc, argv) == EXIT_FAILURE)                       \
            return 0;                                               \
                                                                    \
        delete [] args;                                             \
                                                                    \
        int r = 0;                                                  \
        if (mythos::khaos::toplevel_window_count())                 \
            r = mythos::khaos::event_loop();                        \
                                                                    \
        mythos::khaos::cleanup();                                   \
        mythos::khaos::unload_modules();                            \
                                                                    \
        return r;                                                   \
    }

namespace mythos { namespace khaos
{
    MYTHOS_KHAOS_DECL bool entry(HINSTANCE hInstance, int nCmdShow);
    MYTHOS_KHAOS_DECL char * parse_cmd_line(std::vector<char *> & argv);
    MYTHOS_KHAOS_DECL void cleanup();
}}

// HACK!!!  wxWidgets uses its own macros, so we must use them
#elif defined(MYTHOS_WX)
#   include <wx/app.h>

#   define MYTHOS_KHAOS_IMPLEMENT_MAIN(init)                        \
    IMPLEMENT_APP(mythos::khaos::detail::app<init>)

namespace mythos { namespace khaos
{
    MYTHOS_KHAOS_DECL bool entry(int * argc, char *** argv);
    MYTHOS_KHAOS_DECL void cleanup();

    namespace detail
    {
        template <int (*init)(int argc, char ** argv)>
        struct app : wxApp
        {
            bool OnInit()
            {
#if wxUSE_UNICODE
                // FIXME: i'm sure this'll cause some problems...
                char ** argv_ = this->argv;
                if (!entry(&this->argc, &argv_))
#else
                if (!entry(&this->argc, &this->argv))
                    return false;
#endif

                return init(this->argc, this->argv) == EXIT_SUCCESS;
            }

            int OnExit()
            {
                cleanup();

                return wxApp::OnExit();
            }
        };
    }
}}

#else
#   define MYTHOS_KHAOS_IMPLEMENT_MAIN(init)                        \
    int main(int argc, char ** argv)                                \
    {                                                               \
        if (!mythos::khaos::entry(&argc, &argv))                    \
            return EXIT_FAILURE;                                    \
                                                                    \
        mythos::khaos::load_modules(&argc, &argv);                  \
                                                                    \
        if (init(argc, argv) == EXIT_FAILURE)                       \
            return EXIT_FAILURE;                                    \
                                                                    \
        int r = EXIT_SUCCESS;                                       \
        if (mythos::khaos::toplevel_window_count())                 \
            r = mythos::khaos::event_loop();                        \
                                                                    \
        mythos::khaos::cleanup();                                   \
        mythos::khaos::unload_modules();                            \
                                                                    \
        return r;                                                   \
    }

namespace mythos { namespace khaos
{
    MYTHOS_KHAOS_DECL bool entry(int * argc, char *** argv);
    MYTHOS_KHAOS_DECL void cleanup();
}}

#endif

#endif // #if !defined( MYTHOS_KHAOS_MAIN_HPP )

