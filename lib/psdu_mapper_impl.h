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

#ifndef INCLUDED_IEEE802_11_B_PSDU_MAPPER_IMPL_H
#define INCLUDED_IEEE802_11_B_PSDU_MAPPER_IMPL_H

#include <queue>
#include <utility>
#include <vector>

#include <ieee802_11_b/psdu_mapper.h>

#define PPDU_HEADER_LEN 6

struct plcp_header {
    uint8_t signal;
    uint8_t service;
    uint16_t length;
    uint16_t crc;

    void calc_crc();
}__attribute__((packed));

struct ppdu_info {
    ppdu_info(int ppdu_len);
    ~ppdu_info();
    
    int ppdu_len;
    unsigned char *ppdu;
    std::vector< std::pair<int, Modulation> > mod_tags;
};

namespace gr {
    namespace ieee802_11_b {

        class psdu_mapper_impl : public psdu_mapper
        {
        public:
            psdu_mapper_impl(Modulation m, bool short_sync);
            ~psdu_mapper_impl();

            // Where all the action really happens
            void forecast (int noutput_items, gr_vector_int &ninput_items_required);

            int general_work(int noutput_items,
                             gr_vector_int &ninput_items,
                             gr_vector_const_void_star &input_items,
                             gr_vector_void_star &output_items);

            void psdu_in(pmt::pmt_t msg);
	  
        private:
            Modulation d_modulation;
            bool d_short_sync;
            int d_ppdu_offset;
            std::queue<ppdu_info> d_ppdu_queue;
            gr::thread::mutex d_mutex;

            void insert_long_preamble(unsigned char* buffer);

            void insert_short_preamble(unsigned char* buffer);

            void insert_header(unsigned char* buffer, unsigned int psdu_len);

        };

    } // namespace ieee802_11_b
} // namespace gr

#endif /* INCLUDED_IEEE802_11_B_PSDU_MAPPER_IMPL_H */

