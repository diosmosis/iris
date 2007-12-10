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

#include <iostream>

#include <mythos/gaia/gaia.hpp>
#include <mythos/nyx/nyx.hpp>
#include <mythos/iris/iris.hpp>

#include <mythos/nyx/dsel.hpp>
#include <mythos/iris/dsel.hpp>

#include <mythos/gaia/init.hpp>

#include <mythos/khaos/image.hpp>

#include <boost/gil/extension/io/bmp_io.hpp>

#include <boost/lambda/bind.hpp>

#include <mythos/khaos/main.hpp>

using namespace mythos;
// FIXME: if a window is parentless, it needs to be destroyed explicitly
// if weak count == 0 && window has no parent && is child window, it must be destroyed

namespace myapp
{
    nyx::window mainframe;
    boost::gil::rgb8_image_t img;
    boost::gil::rgb8_view_t vw;
    std::string mloc_str("(MAX,MAX)");
    iris::event_handler extra;
    bool v = true;

    // Tips taken from the game Blades of Exile
    std::string tips[5] =
    {
        "Someone important not there?  The friendly people in the town "
        "suddenly mad at you and you're not sure why?  You may have clipped "
        "them with Mass Charm or Shockwave.  Visit a few other towns and "
        "return, and you'll have been forgiven.",
        "Low on gold?  Look around for side adventures.  For example, "
        "Valley of Dying Things has several other dungeons.",
        "Trapped in a scenario?  You can often leave it by returning to the "
        "town you started in.  If you're trapped, run the Blades of Exile "
        "character editor.  It can pull you out of a scenario (even when "
        "unregistered).",
        "Be sure to talk to everyone in town.  This is often your only route "
        "to important tips.  For example, in Valley of Dying Things, Avizo, "
        "in the first town, knows something very important.  You just have "
        "to find out what to ask him.",
        "Make sure everyone has a strength of at least 3.  Make sure mages "
        "have plenty of intelligence.  Make sure everyone who does a lot of "
        "melee combat has a high dexterity."
    };

    struct increment_tip
    {
        increment_tip(std::string & t, size_t & tn, nyx::window const& w) : tip(t), tip_n(tn), dlog(w) {}

        void operator()() const
        {
            if (++tip_n == 5) tip_n = 0;

            tip = tips[tip_n];

            iris::paint(dlog);
        }

        std::string & tip;
        size_t & tip_n;
        nyx::window dlog;
    };

    void load_img()
    {
        boost::gil::bmp_read_image("abc.bmp", myapp::img);

        myapp::vw = boost::gil::view(myapp::img);
    }

    struct paint_bgnd
    {
        void operator()(khaos::image_view const& vw) const
        {
            std::fill(vw.begin(), vw.end(), khaos::pixel(255, 255, 255));
        }
    };

    size_t tip_n = 0;
    std::string tip = tips[0];

    void show_dialog()
    {
        using namespace nyx;
        using namespace gaia;

        window dlog = nyx::create_toplevel_window("Tip of the Day", 300, 300, 300, 300, mainframe);

        window next_tip = button(increment_tip(tip, tip_n, dlog))[ hcenter["Next Tip"] ];
        window done = button(boost::lambda::bind(destroy_window, dlog))[ hcenter["Done"] ];

        layout(dlog) =
            col1[ gaia::image(vw) ] | col2[ "Tip of the Day:"                       ]
                                    | col2[ text(tip, khaos::default_font(), 40, 7) ]
                                    | col2[ "See tips on startup:" | checkbox(v)    ]
                                    | col2[ next_tip               | right[done]    ]
            ;

        dlog.events() = iris::on::paint[paint_bgnd()];

        nyx::modalize(dlog);
    }

    void quit_prog()
    {
        nyx::destroy_window(mainframe);
    }

    struct set_mloc_str
    {
        void operator()(nyx::point const& pt) const
        {
            std::stringstream ss;
            ss << '(' << pt.x << ',' << pt.y << ')';

            mloc_str = ss.str();

            iris::paint(mainframe);
        }
    };

    void gui_main()
    {
        using namespace nyx;
        using namespace gaia;

        load_img();

        mainframe = nyx::create_toplevel_window("My window", 200, 200, 1, 1);

        // FIXME: labels should be centered
        window
            do_dlog = button(show_dialog)[ hcenter["Show Dialog"] ],
            quit = button(quit_prog)[ hcenter["Quit"] ]
            ;

        layout(mainframe) =
            rows[ "Hello World"                      ]
                //[ "Mouse coords:" | text(mloc_str)   ]
                //[ do_dlog         | right[quit]      ]
            ;

        extra = (iris::on::mouse_move[set_mloc_str()], iris::on::paint[paint_bgnd()]);
        mainframe.connect(extra);

        show_window(mainframe);
    }

    int main(int argc, char ** argv)
    {
        std::string toolkit;

        // too lazy to use Boost.Program-Options...
        for (int i = 1; i < argc; ++i)
        {
            if (std::string("--gaia") == argv[i])
            {
                ++i;

                if (toolkit.empty())
                {
                    if (i >= argc)
                    {
                        std::cout << "--gaia option must specify toolkit" << std::endl;

                        return EXIT_SUCCESS;
                    }

                    toolkit = argv[i];
                }
            }
        }

        // init gaia
        if (toolkit.empty())
            gaia::init();
        else
            gaia::init(toolkit);

        gui_main();

        return EXIT_SUCCESS;
    }
}

MYTHOS_KHAOS_IMPLEMENT_MAIN(myapp::main);

