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

#if !defined( MYTHOS_NYX_FILL_LAYOUT_HPP )
#define MYTHOS_NYX_FILL_LAYOUT_HPP

#include <mythos/nyx/config.hpp>
#include <mythos/nyx/layout.hpp>
#include <mythos/nyx/grammar.hpp>

#include <boost/mpl/or.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>

#include <boost/utility/enable_if.hpp>

#include <boost/foreach.hpp>

// TODO: speed things up (don't need to use cell::max_child_-2() as much)

namespace mythos { namespace nyx
{
    namespace detail
    {
        template <typename D>
        void directive_impl(void * data, cell & x)
        {
            layout(*static_cast<D *>(data), x);
        }

        MYTHOS_NYX_DECL void place_x_cols(layout_data & lay, cell & cols);
    }

    // metafunctions
    template <typename E>
    struct is_rows_start
        : boost::proto::matches<E, boost::proto::subscript<grammar::rows_keyword, grammar::layout_g> >
    {};

    template <typename E>
    struct is_col_spec
        : boost::proto::matches<E, grammar::column_spec>
    {};

    template <typename E>
    struct is_directive_expr
        : boost::proto::matches<E, grammar::directive>
    {};

    template <typename E>
    struct is_spec_columns
        : boost::proto::matches<
            E, boost::proto::bitwise_or<boost::proto::not_<grammar::column_spec>, grammar::column_spec>
        >
    {};

    template <typename E>
    struct is_spec_columns_end
        : boost::proto::matches<
            E, boost::proto::bitwise_or<grammar::column_spec, boost::proto::_>
        >
    {};

    template <typename E>
    struct is_container_win
        : boost::proto::matches<
            E, boost::proto::subscript<window, boost::proto::_>
        >
    {};

    template <typename E>
    struct isnt_or
        : boost::mpl::not_<
            boost::is_same<typename E::proto_tag, boost::proto::tag::bitwise_or>
        >
    {};

    template <typename E>
    struct isnt_subscript
        : boost::mpl::not_<
            boost::is_same<typename E::proto_tag, boost::proto::tag::subscript>
        >
    {};

    template <typename E>
    struct is_term
        : boost::is_same<typename E::proto_tag, boost::proto::tag::terminal>
    {};

    // window
    MYTHOS_NYX_DECL void fill_layout(layout_data & lay, cell & c, window const& win);

    // window generator
    template <typename E>
    inline void fill_layout(layout_data & lay, cell & c, E const& expr,
        typename boost::enable_if<is_term<E> >::type * d = 0)
    {
        window win = generate_window(boost::proto::arg(expr));

        fill_layout(lay, c, win);
    }

    // subscript(layout, layout)       **rows[...][...]
    template <typename E>
    inline void fill_layout(layout_data & lay, cell & c, E const& expr,
        typename boost::disable_if<
            boost::mpl::or_<
                is_rows_start<E>,
                is_col_spec<E>,
                is_directive_expr<E>,
                is_container_win<E>,
                isnt_subscript<E>
            >
        >::type * d = 0
    )
    {
        fill_layout(lay, c, boost::proto::left(expr));
        fill_layout(lay, lay.make_cell(c.x1, c.max_child_y2(), c), boost::proto::right(expr));
    }

    // subscript(rows, layout)         **rows[...]
    template <typename E>
    inline void fill_layout(layout_data & lay, cell & c, E const& expr,
        typename boost::enable_if<is_rows_start<E> >::type * d = 0)
    {
        fill_layout(lay, lay.make_cell(c.x1, c.y1, c), boost::proto::right(expr));
    }

    // subscript(col_<N>, layout)       **colN[...]
    template <typename E>
    inline void fill_layout(layout_data & lay, cell & c, E const& expr,
        typename boost::enable_if<is_col_spec<E> >::type * d = 0)
    {
        typedef typename boost::proto::result_of::left<E>::type left_type;
        typedef typename boost::proto::result_of::arg<left_type>::type col_type;

        cell & col = lay.get_specific_column(c, col_type::value);

        fill_layout(lay, lay.make_cell(col.x1, col.max_child_y2(), col), boost::proto::right(expr));
    }

    // subscript(directive, layout)     **directive[...]
    template <typename E>
    inline void fill_layout(layout_data & lay, cell & c, E const& expr,
        typename boost::enable_if<is_directive_expr<E> >::type * d = 0)
    {
        typedef typename boost::proto::result_of::left<E>::type left_type;
        typedef typename boost::proto::result_of::arg<left_type>::type d_type;

        fill_layout(lay, c, boost::proto::right(expr));

        directive_action dact;
        dact.x = &c;
        dact.func = detail::directive_impl<d_type>;

        void const* data_impl = &boost::proto::arg(boost::proto::left(expr));
        dact.data = const_cast<void *>(data_impl); // left_type is const, so this is not a problem

        lay.dactions.push_back(dact);
    }

    // subscript(window, layout)        **window[...]
    template <typename E>
    inline void fill_layout(layout_data & lay, cell & c, E const& expr,
        typename boost::enable_if<is_container_win<E> >::type * d = 0)
    {
        typedef std::vector<mapped_win>::iterator item_iterator;

        window const& left = boost::proto::left(expr);

        fill_layout(lay, c, left);
        size_t i = lay.items.size();

        cell & child = lay.make_cell(c.x1 + lay.xpad, c.y1 + lay.ypad, c);
        fill_layout(lay, child, boost::proto::right(expr));

        // find all items to right of current one that have no parent, and
        // set the parent to left(expr)
        for (item_iterator item = lay.items.begin() + i; item != lay.items.end() && !item->parent; ++item)
            item->parent = left;
    }

    // bitwise_or(layout, layout)       **... | ...
    template <typename E>
    inline void fill_layout(layout_data & lay, cell & c, E const& expr,
        typename boost::disable_if<
            boost::mpl::or_<is_spec_columns<E>, is_spec_columns_end<E>, isnt_or<E> >
        >::type * d = 0
    )
    {
        fill_layout(lay, lay.make_cell(c.max_child_x2(), c.y1, c), boost::proto::left(expr));
        fill_layout(lay, lay.make_cell(c.max_child_x2(), c.y1, c), boost::proto::right(expr));
    }

    // bitwise_or(col_spec, col_spec)   **colN[...] | colN[...]
    template <typename E>
    inline void fill_layout(layout_data & lay, cell & c, E const& expr, cell & cols,
        typename boost::enable_if<is_spec_columns_end<E> >::type * d = 0)
    {
        fill_layout(lay, cols, boost::proto::left(expr));
        fill_layout(lay, cols, boost::proto::right(expr));
    }

    // bitwise_or(spec_cols, col_spec)  **... | colN[...]
    template <typename E>
    inline void fill_layout(layout_data & lay, cell & c, E const& expr,
        typename boost::enable_if<is_spec_columns<E> >::type * d = 0)
    {
        cell & cols = lay.make_cell(c.x1, c.y1, c);

        fill_layout(lay, c, boost::proto::left(expr), cols);
        fill_layout(lay, cols, boost::proto::right(expr));

        detail::place_x_cols(lay, cols);
    }

    template <typename E> // **... | colN[...]
    inline void fill_layout(layout_data & lay, cell & c, E const& expr, cell & cols,
        typename boost::disable_if<is_spec_columns_end<E> >::type * d = 0)
    {
        fill_layout(lay, c, boost::proto::left(expr), cols);
        fill_layout(lay, cols, boost::proto::right(expr));
    }

    // main
    template <typename E>
    inline void fill_layout(layout_data & lay, E const& expr)
    {
        fill_layout(lay, lay.root, expr);

        BOOST_FOREACH(directive_action & da, lay.dactions)
            da.func(da.data, *da.x);
    }
}}

#endif // #if !defined( MYTHOS_NYX_FILL_LAYOUT_HPP )

