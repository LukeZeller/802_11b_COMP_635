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

#ifndef INCLUDED_IEEE802_11_B_CODE_MAPPER_IMPL_H
#define INCLUDED_IEEE802_11_B_CODE_MAPPER_IMPL_H

#include "common.h"
#include <ieee802_11_b/code_mapper.h>

#include <queue>

struct q_phase {
    int ph;

    gr_complex to_complex() const;

    q_phase neg() const;
    q_phase operator + (const q_phase& o) const;
    q_phase operator - () const;
};

namespace gr {
    namespace ieee802_11_b {

        class code_mapper_impl : public code_mapper
        {
        public:
            code_mapper_impl();
            ~code_mapper_impl();

            // Where all the action really happens
            void forecast (int noutput_items, gr_vector_int &ninput_items_required);

            int general_work(int noutput_items,
                             gr_vector_int &ninput_items,
                             gr_vector_const_void_star &input_items,
                             gr_vector_void_star &output_items);

        private:
            static const std::vector<q_phase> PHASES;
            static const std::vector<int> BARKER;

            Modulation d_curr_mod;
            int d_symbol;
            q_phase d_curr_phase;
            std::queue<q_phase> d_phase_buffer;

            q_phase dbpsk_symbol_to_phase (unsigned char symbol);

            q_phase dqpsk_symbol_to_phase (unsigned char symbol,
                                           bool grey_coded);

            void barker_spread();

            void cck_spread(q_phase p2, q_phase p3, q_phase p4);
            
            void dbpsk_1_process_byte (unsigned char byte);

            void dqpsk_2_process_byte (unsigned char byte);

            void cck_5_5_process_byte (unsigned char byte);

            void cck_11_process_byte (unsigned char byte);
        
            void process_byte (unsigned char in);};

    } // namespace ieee802_11_b
} // namespace gr

#endif /* INCLUDED_IEEE802_11_B_CODE_MAPPER_IMPL_H */

