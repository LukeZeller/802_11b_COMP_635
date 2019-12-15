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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "scramble_impl.h"

#define INITIAL_STATE 0x1b

namespace gr {
    namespace ieee802_11_b {

        scramble::sptr
        scramble::make(bool reverse)
        {
            return gnuradio::get_initial_sptr
                (new scramble_impl(reverse));
        }

        scramble_impl::scramble_impl(bool reverse)
            : gr::sync_block("scramble",
                             gr::io_signature::make(1, 1, sizeof(char)),
                             gr::io_signature::make(1, 1, sizeof(char))),
            d_reverse(reverse),
            d_state(reverse ? 0 : INITIAL_STATE)
        {
        }

        scramble_impl::~scramble_impl()
        {
        }

        int
        scramble_impl::work(int noutput_items,
                            gr_vector_const_void_star &input_items,
                            gr_vector_void_star &output_items)
        {
            const unsigned char *bytes_in = (const unsigned char *) input_items[0];
            unsigned char *bytes_out = (unsigned char *) output_items[0];

            int s_offset = nitems_read(0);
            get_tags_in_range(d_tags, 0, s_offset, s_offset + noutput_items);

            int rel_frame_s = -1, tags_idx = -1;
            if (d_tags.size()) {
                std::sort(d_tags.begin(), d_tags.end(), gr::tag_t::offset_compare);
                rel_frame_s = d_tags[0].offset - s_offset;
                ++tags_idx;
            }
            
            for (int i = 0; i < noutput_items; ++i) {

                if (i == rel_frame_s) {
                    d_state = d_reverse ? 0 : INITIAL_STATE;
                    if (tags_idx < d_tags.size() - 1)
                        rel_frame_s = d_tags[++tags_idx].offset - s_offset;
                }
                bytes_out[i] = 0;
                for (int b = 0; b < 8; ++b) {
                    unsigned char bit_in, bit_out;
                    bit_in = (bytes_in[i] >> b) & 0x01;
                    unsigned char feedback = !!(d_state & (1 << 3)) ^ !!(d_state & (1 << 6));
                    bit_out = bit_in ^ feedback;
                    d_state = ((d_state << 1) & ((1 << 7) - 1));
                    if (d_reverse)
                        d_state |= bit_in;
                    else
                        d_state |= bit_out;
                    bytes_out[i] |= (bit_out << b);
                }
            }
            
            return noutput_items;
        }

    } /* namespace ieee802_11_b */
} /* namespace gr */
