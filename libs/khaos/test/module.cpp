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

#include <mythos/khaos/module.hpp>
#include <mythos/khaos/register_module.hpp>

#define BOOST_TEST_MODULE khaos module test
#include <mythos/support/test.hpp>

using namespace mythos::khaos;

static bool module_loaded = false;
static bool module_unloaded = false; // TODO: need a way to test that this is true

struct my_module
{
    void load(int * argc, char *** argv) const
    {
        module_loaded = true;
    }

    void unload() const
    {
        module_unloaded = true;
    }
};

BOOST_AUTO_TEST_CASE(module_loaded_test)
{
    BOOST_CHECK(module_loaded);
}

MYTHOS_KHAOS_REGISTER_MODULE(my_module);

MYTHOS_KHAOS_TEST_IMPLEMENT_MAIN()

