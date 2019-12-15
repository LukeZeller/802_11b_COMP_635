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
#include "code_mapper_impl.h"


gr_complex q_phase::to_complex() const {
    return std::exp(gr_complex(0, PI / 2.0 * ph));
}

q_phase q_phase::neg () const {
    return {(ph + 2) % 4};
}

q_phase q_phase::operator + (const q_phase& o) const {
    return {(ph + o.ph) % 4};
}

q_phase q_phase::operator - () const {
    return {(4 - ph) % 4};
}


namespace gr {
    namespace ieee802_11_b {

        code_mapper::sptr
        code_mapper::make()
        {
            return gnuradio::get_initial_sptr
                (new code_mapper_impl());
        }

        code_mapper_impl::code_mapper_impl( )
            : gr::block("code_mapper",
                        gr::io_signature::make(1, 1, sizeof(unsigned char)),
                        gr::io_signature::make(1, 1, sizeof(gr_complex))),
            d_curr_mod(DBPSK_1),
            d_symbol(0),
	    d_curr_phase(q_phase{0})
        {
            set_tag_propagation_policy(block::TPP_DONT);
        }

 
        code_mapper_impl::~code_mapper_impl()
        {
        }

        void
        code_mapper_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
        {
            double div_factor;
            switch(d_curr_mod) {
            case DBPSK_1:
                div_factor = 11.0;
                break;
            case DQPSK_2:
                div_factor = 5.5;
                break;
            case CCK_5_5:
                div_factor = 2;
                break;
            case CCK_11:
                div_factor = 1;
                break;
            default:
                throw std::runtime_error("How did you get here?");
                break;
            }
            ninput_items_required[0] = (int) ((1.0 + noutput_items) / div_factor);
            /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
        }

        q_phase code_mapper_impl::dbpsk_symbol_to_phase (uint8_t symbol) {
            return PHASES[2 * symbol];
        }

        q_phase code_mapper_impl::dqpsk_symbol_to_phase (uint8_t symbol,
                                                         bool grey_coded) {
            if (!grey_coded || symbol <= 1) return PHASES[symbol];
            return PHASES[5 - symbol];
        }


        void code_mapper_impl::barker_spread () {
            for (int s : BARKER) {
	        if (s == -1) d_phase_buffer.push(d_curr_phase.neg());
                else d_phase_buffer.push(d_curr_phase);
            }
        }
        
        void code_mapper_impl::cck_spread(q_phase p2, q_phase p3, q_phase p4) {
            d_phase_buffer.push(d_curr_phase + p2 + p3 + p4);
            d_phase_buffer.push(d_curr_phase + p3 + p4);
            d_phase_buffer.push(d_curr_phase + p2 + p4);
            d_phase_buffer.push((d_curr_phase + p4).neg());
            d_phase_buffer.push(d_curr_phase + p2 + p3);
            d_phase_buffer.push(d_curr_phase + p3);
            d_phase_buffer.push((d_curr_phase + p2).neg());
            d_phase_buffer.push(d_curr_phase);
        }
        
        void code_mapper_impl::dbpsk_1_process_byte (uint8_t byte) {
            for (int i = 0; i < 8; ++i) {
                q_phase p = dbpsk_symbol_to_phase((byte >> i) & 0x01);
                d_curr_phase = d_curr_phase + p;
                barker_spread();
                d_symbol++;
            }
        }
        
        void code_mapper_impl::dqpsk_2_process_byte (uint8_t byte) {
            for (int i = 0; i < 8; i += 2) {
                q_phase p = dqpsk_symbol_to_phase((byte >> i) & 0x03, true);
                d_curr_phase = d_curr_phase + p;
                barker_spread();
                d_symbol++;
            }
        }

        void code_mapper_impl::cck_5_5_process_byte (uint8_t byte) {
            for (int i = 0; i < 8; i += 4) {
                uint8_t cck_symbol = ((byte >> i) & 0x0F);
                q_phase p = dqpsk_symbol_to_phase(cck_symbol & 0x03, true);
                if (d_symbol % 2) p = p.neg();
                d_curr_phase = d_curr_phase + p;
                uint8_t d2 = (cck_symbol >> 2) & 0x01;
                uint8_t d3 = (cck_symbol >> 3) & 0x01;

                q_phase p2 = d2 ? PHASES[3] : PHASES[1];
                q_phase p3 = PHASES[0];
                q_phase p4 = d3 ? PHASES[2] : PHASES[0];

                cck_spread(p2, p3, p4);
                d_symbol++;
            }
        }

        void code_mapper_impl::cck_11_process_byte (uint8_t byte) {
            q_phase p = dqpsk_symbol_to_phase(byte & 0x03, true);
            d_curr_phase = d_curr_phase + p;
            q_phase p2 = dqpsk_symbol_to_phase((byte >> 2) & 0x03, false);
            q_phase p3 = dqpsk_symbol_to_phase((byte >> 4) & 0x03, false);
            q_phase p4 = dqpsk_symbol_to_phase(byte >> 6, false);

            cck_spread(p2, p3, p4);
            d_symbol++;
        }

        
        void code_mapper_impl::process_byte (uint8_t in) {
            switch(d_curr_mod) {
            case DBPSK_1:
                dbpsk_1_process_byte(in);
                break;
            case DQPSK_2:
                dqpsk_2_process_byte(in);
                break;
            case CCK_5_5:
                cck_5_5_process_byte(in);
                break;
            case CCK_11:
                cck_11_process_byte(in);
                break;
            }
        }

        int
        code_mapper_impl::general_work (int noutput_items,
                                        gr_vector_int &ninput_items,
                                        gr_vector_const_void_star &input_items,
                                        gr_vector_void_star &output_items)
        {
            const unsigned char *in = (const unsigned char *) input_items[0];
            gr_complex *out = (gr_complex *) output_items[1];

            std::vector<gr::tag_t> tags;
            int s_offset = nitems_read(0);
            get_tags_in_range(tags, 0, s_offset, s_offset + ninput_items[0],
                              pmt::mp("mod_change"));
            
            int mod_offset = -1, tags_idx = -1;
            if (tags.size()) {
                std::sort(tags.begin(), tags.end(), gr::tag_t::offset_compare);
                mod_offset = tags[0].offset - s_offset;
                ++tags_idx;
            }
            
            int i = 0, o = 0;
            while (true) {
                while (o < noutput_items && d_phase_buffer.size()) {
                    if (d_curr_mod == DBPSK_1 || d_curr_mod == DQPSK_2)
		        out[o++] = d_phase_buffer.front().to_complex();
                    d_phase_buffer.pop();
                }
                if (o == noutput_items) break;

                if (i == ninput_items[0]) break;
                else if (i == mod_offset) {
                    d_symbol = 0;
                    d_curr_mod = (Modulation) pmt::to_long(tags[tags_idx++].value);
                }
                process_byte(in[i++]);
            }
            consume_each(i);
            return o;
        }

        
        const std::vector<q_phase> code_mapper_impl::PHASES {
            q_phase{0}, q_phase{1}, q_phase{2}, q_phase{3}
        };

        const std::vector<int> code_mapper_impl::BARKER {
            1, -1, 1, 1, -1, 1, 1, 1, -1, -1, -1
        };
        
    } /* namespace ieee802_11_b */
} /* namespace gr */
