/* -*- c++ -*- */
/*
 * Copyright 2019 gr-ieee802_11_b author.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_IEEE802_11_B_SCRAMBLE_IMPL_H
#define INCLUDED_IEEE802_11_B_SCRAMBLE_IMPL_H

#include <ieee802_11_b/scramble.h>

namespace gr {
    namespace ieee802_11_b {

        class scramble_impl : public scramble
        {
        public:
            scramble_impl(bool d_reverse);
            ~scramble_impl();

            // Where all the action really happens
            int work(
                int noutput_items,
                gr_vector_const_void_star &input_items,
                gr_vector_void_star &output_items
                );
        private:
            bool d_reverse;
            int d_state;
            std::vector<gr::tag_t> d_tags;
        };

      
    } // namespace ieee802_11_b
} // namespace gr

#endif /* INCLUDED_IEEE802_11_B_SCRAMBLE_IMPL_H */

