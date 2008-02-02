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

#include <mythos/khaos/window.hpp>
#include <mythos/khaos/event_loop.hpp>
#include <mythos/khaos/main.hpp>

#include <boost/optional.hpp>

#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>

#include <boost/detail/lightweight_test.hpp>

using namespace mythos::khaos;

static window * win;
static boost::timed_mutex elt_mutex;
static boost::condition cond;
static boost::optional<int> event_loop_result = boost::none;

static void run_event_loop()
{
    boost::timed_mutex::scoped_lock lock(elt_mutex);

    cond.notify_all();

    event_loop_result = event_loop();
}

static bool event_loop_running()
{
    boost::timed_mutex::scoped_try_lock lock(elt_mutex, boost::defer_lock);

    return !lock.try_lock();
}

static bool event_loop_stopped()
{
    boost::timed_mutex::scoped_timed_lock lock(elt_mutex, boost::defer_lock);

    boost::xtime xt = {5, 0};
    return !lock.timed_lock(xt);
}

static int run_tests(int argc, char ** argv)
{
    win = create_toplevel_window("some window", 0, 0, 200, 200);
    window * child1 = create_child_window(0, 0, 50, 50, win);
    window * child2 = create_child_window(50, 0, 50, 50, win);
    window * child3 = create_child_window(0, 50, 50, 50);
    window * child4 = create_child_window(0, 0, 50, 50);

    window * win2 = create_toplevel_window("another window", 0, 0, 200, 200);

    BOOST_TEST(win && child1 && child2 && child3 && child4);

    if (!(win && child1 && child2 && child3 && child4))
        return boost::report_errors();

    boost::timed_mutex::scoped_lock elt_lock(elt_mutex);

    // create event loop thread
    boost::thread elt(run_event_loop);

    // wait till mutex is locked
    boost::xtime xt = {5, 0};
    if (!cond.timed_wait(elt_lock, xt))
    {
        std::cout << "event loop failed to start" << std::endl;

        return 1;
    }

    // make sure destroying child windows doesn't stop the event loop
    destroy_window(child2);
    BOOST_TEST(event_loop_running());

    destroy_window(child4);
    BOOST_TEST(event_loop_running());

    // make sure destroying a toplevel window doesn't stop the event loop, unless it is the only toplevel
    // window left
    destroy_window(win2);
    BOOST_TEST(event_loop_running());

    // make sure destroying the last toplevel window active does stop the event loop
    destroy_window(win);
    BOOST_TEST(event_loop_stopped());

    // make sure the result is correct
    BOOST_TEST(event_loop_result);

    if (!event_loop_result)
        return boost::report_errors();

#if defined(MYTHOS_WINDOWS)
    BOOST_TEST(event_loop_result != 0);
#else
    BOOST_TEST(event_loop_result == EXIT_SUCCESS);
#endif

    return boost::report_errors();
}

MYTHOS_KHAOS_IMPLEMENT_MAIN(run_tests);

