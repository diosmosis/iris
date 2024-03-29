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

#include <boost/preprocessor/stringize.hpp>

#include <string>

#define BOOST_TEST_MODULE khaos implement main test
#include <mythos/support/test.hpp>

#if !defined(MYTHOS_KHAOS_MAIN_TEST_NAME)
#   error MYTHOS_KHAOS_MAIN_TEST_NAME should specify test exe's name
#endif

static int argc = 0;
static char ** argv = NULL;

// test should be run w/ the command line: arg1 "a r g 2" arg3
BOOST_AUTO_TEST_CASE(commnd_line_test)
{
    std::string exename = BOOST_PP_STRINGIZE(MYTHOS_KHAOS_MAIN_TEST_NAME);

    BOOST_MESSAGE("MYTHOS_KHAOS_MAIN_TEST_NAME = " << exename);

    // test argv & argc
    BOOST_REQUIRE(argv != NULL);
    BOOST_REQUIRE(argc == 4);

    // test arguments
    BOOST_CHECK_EQUAL(std::string(argv[0]), exename);

    BOOST_CHECK_EQUAL(std::string(argv[1]), "arg1");
    BOOST_CHECK_EQUAL(std::string(argv[2]), "a r g 2");
    BOOST_CHECK_EQUAL(std::string(argv[3]), "arg3");
}

boost::unit_test::test_suite * init_unit_test_suite(int argc_, char * argv_[])
{
    ::argc = argc_;
    ::argv = argv_;

    return 0;
}

MYTHOS_KHAOS_TEST_IMPLEMENT_MAIN();

