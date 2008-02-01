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

#if !defined( MYTHOS_GAIA_PLUGIN_HPP )
#define MYTHOS_GAIA_PLUGIN_HPP

#include <mythos/gaia/gaia.hpp>

#include <mythos/khaos/annex/plugin.hpp>

namespace mythos { namespace gaia
{
    struct plugin
    {
        struct impl
        {
            virtual ~impl() {}

            // label.hpp
            virtual nyx::window label(
                std::string const& x,
                void const* fnt,
                void (*renderer)(void const*, khaos::image_view const&, std::string const&),
                nyx::point const& extents,
                nyx::window parent
            ) = 0;

            virtual nyx::window label(std::string const& x) = 0;

            // text.hpp
            virtual nyx::window text(
                std::string & x,
                void const* fnt,
                void (*renderer)(void const*, khaos::image_view const&, std::string const&),
                nyx::point const& extents,
                nyx::window parent
            ) = 0;

            virtual nyx::window text(std::string & x) = 0;

            // image.hpp
            virtual nyx::window image(
                boost::any const& view,
                void (*render)(boost::any const& img, khaos::image_view const& vw),
                int w, int h,
                nyx::window const& parent
            ) = 0;

            // button.hpp
            virtual nyx::window button(boost::function<void ()> const& action, nyx::window const& p) = 0;

            // checkbox.hpp
            virtual nyx::window checkbox(bool & value, nyx::window const& p) = 0;
        };

        plugin() {}

        // TODO: need some way of passing generic amount of arguments to plugin
        plugin(std::string const& fname) : pimpl(fname) {}
        plugin(std::string const& fname, std::string const& extra) : pimpl(fname, extra) {}

        void load(std::string const& fname)
        {
            pimpl.load(fname);
        }

        void load(std::string const& fname, std::string const& extra)
        {
            pimpl.load(fname, extra);
        }

        // label.hpp
        template <typename F>
        nyx::window label(
            std::string const& x,
            F const& fnt,
            size_t maxx = 0,
            size_t maxy = 0,
            nyx::window parent = nyx::window()
        )
        {
            return pimpl->label(
                x, &fnt, detail::label_renderer<F>, detail::text_extents(fnt, x, maxx, maxy), parent
            );
        }

        nyx::window label(std::string const& x)
        {
            return pimpl->label(x);
        }

        // text.hpp
        template <typename F>
        nyx::window label(
            std::string & x,
            F const& fnt,
            size_t maxx = 0,
            size_t maxy = 0,
            nyx::window parent = nyx::window()
        )
        {
            return pimpl->text(
                x, &fnt, detail::label_renderer<F>, detail::text_extents(fnt, x, maxx, maxy), parent
            );
        }

        nyx::window label(std::string & x)
        {
            return pimpl->text(x);
        }

        // image.hpp
        template <typename View>
        nyx::window image(View const& vw, nyx::window parent = nyx::window())
        {
            return pimpl->image(vw, detail::draw_image<View>, vw.width(), vw.height(), parent);
        }

        // button.hpp
        nyx::window button(boost::function<void ()> const& action, nyx::window const& p = nyx::window())
        {
            return pimpl->button(action, p);
        }

        // checkbox.hpp
        nyx::window checkbox(bool & value, nyx::window const& p = nyx::window())
        {
            return pimpl->checkbox(value, p);
        }

        khaos::annex::plugin<impl> pimpl;
    };
}}

#endif // #if !defined( MYTHOS_GAIA_PLUGIN_HPP )

