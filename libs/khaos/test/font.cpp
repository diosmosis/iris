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

#include <mythos/khaos/main.hpp>
#include <mythos/khaos/font.hpp>

#include <boost/detail/lightweight_test.hpp>

using namespace mythos::khaos;

// FIXME: need a better test
static void find_font_test()
{
    font f;
    BOOST_TEST(handle_of(f));

    find_font(f, "made up font name", 12);
    BOOST_TEST(!f);

    find_font(f, "Times New Roman", 12);
    BOOST_TEST(f);
}

static void default_font_test()
{
    font const& f = default_font();

    BOOST_TEST(f);
    BOOST_TEST(handle_of(f));
}

static int run_tests(int argc, char ** argv)
{
    find_font_test();
    default_font_test();

    return boost::report_errors();
}

MYTHOS_KHAOS_IMPLEMENT_MAIN(run_tests);

