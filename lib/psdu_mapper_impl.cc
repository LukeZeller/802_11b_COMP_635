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
#include "psdu_mapper_impl.h"

void plcp_header::calc_crc() {
    uint32_t prot_fields = signal;
    prot_fields |= ((uint32_t) service) << 8;
    prot_fields |= ((uint32_t) length) << 16;

    uint16_t state = 0xFFFF;
    for(int i = 0; i < 32; ++i) {
        uint32_t feedback = (!!(state & 0x8000)) ^ (prot_fields & 0x01);
        state <<= 1;
        state |= feedback | (feedback << 5) | (feedback << 12);
    }
    crc = ~state;
};

ppdu_info::ppdu_info(int ppdu_len)
    : ppdu_len(ppdu_len)
{
    ppdu = static_cast<unsigned char *>(malloc(ppdu_len * sizeof(unsigned char)));
}

ppdu_info::~ppdu_info()
{
    free(ppdu);
}

namespace gr {
    namespace ieee802_11_b {

        psdu_mapper::sptr
        psdu_mapper::make(Modulation m, bool short_sync)
        {
            return gnuradio::get_initial_sptr
                (new psdu_mapper_impl(m, short_sync));
        }


        /*
         * The private constructor
         */
        psdu_mapper_impl::psdu_mapper_impl(Modulation m, bool short_sync)
            : gr::block("psdu_mapper",
                        gr::io_signature::make(0, 0, 0),
                        gr::io_signature::make(1, 1, sizeof(char))),
            d_modulation(m),
            d_short_sync(short_sync),
            d_ppdu_offset(0)
        {
            if (d_short_sync && m == DBPSK_1)
                throw std::runtime_error("Short Sync cannot be used with 1Mbps BPSK");

            message_port_register_in(pmt::intern("psdu in"));        
            set_tag_propagation_policy(block::TPP_DONT);
        }

        psdu_mapper_impl::~psdu_mapper_impl()
        {
        }

        void psdu_mapper_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required) {
            int prefix_len = (d_short_sync ? 9 : 18) + PPDU_HEADER_LEN;
            ninput_items_required[0] = std::max(0, noutput_items - prefix_len);
        }
        
        void psdu_mapper_impl::psdu_in(pmt::pmt_t msg) {
            gr::thread::scoped_lock lock(d_mutex);

            int psdu_len = pmt::blob_length(msg);
            const char *psdu = static_cast<const char*>(pmt::blob_data(msg));

            
            int preamble_len = d_short_sync ? 9 : 18;
            int prefix_len = preamble_len + PPDU_HEADER_LEN;
            ppdu_info ppdu_i(prefix_len + psdu_len);

            ppdu_i.mod_tags.push_back({0, DBPSK_1});
            if (d_short_sync) {
                insert_short_preamble(ppdu_i.ppdu);
                ppdu_i.mod_tags.push_back({preamble_len, DQPSK_2});
            } else {
                insert_long_preamble(ppdu_i.ppdu);
            }
            insert_header(ppdu_i.ppdu + preamble_len, psdu_len);
            ppdu_i.mod_tags.push_back({prefix_len, d_modulation});
            std::memcpy(ppdu_i.ppdu + prefix_len, psdu, psdu_len);

            d_ppdu_queue.push(ppdu_i);
        }

        int
        psdu_mapper_impl::general_work (int noutput_items,
                                        gr_vector_int &ninput_items,
                                        gr_vector_const_void_star &input_items,
                                        gr_vector_void_star &output_items)
        {
            gr::thread::scoped_lock lock(d_mutex);
            
            unsigned char *out = (unsigned char *) output_items[0];
            if (!d_ppdu_queue.size()) return 0;
            ppdu_info ppdu_i = d_ppdu_queue.front();
            
            if (d_ppdu_offset == 0) {
                const pmt::pmt_t len_key = pmt::mp("ppdu_len");
                const pmt::pmt_t val = pmt::from_long(ppdu_i.ppdu_len);
                const pmt::pmt_t srcid = pmt::mp(alias());
                add_item_tag(0, nitems_written(0), len_key, val, srcid);

                const pmt::pmt_t mod_key = pmt::mp("mod_change");
                for (auto& [rel_offset, m] : ppdu_i.mod_tags) {
                    const pmt::pmt_t val = pmt::from_long(m);
                    add_item_tag(0, nitems_written(0) + rel_offset, mod_key, val, srcid);
                }
            }

            int n_bytes_send = std::min(noutput_items, ppdu_i.ppdu_len - d_ppdu_offset);

            std::memcpy(out, ppdu_i.ppdu + d_ppdu_offset, n_bytes_send);
            d_ppdu_offset += n_bytes_send;

            if (d_ppdu_offset == ppdu_i.ppdu_len) {
                d_ppdu_offset = 0;
                d_ppdu_queue.pop();
            }
            
            return n_bytes_send;
        }

        void psdu_mapper_impl::insert_long_preamble(unsigned char* buffer) {
            std::memset(buffer, 0xFF, 16);
            buffer[16] = 0xA0;
            buffer[17] = 0xF3;
        }

        void psdu_mapper_impl::insert_short_preamble(unsigned char* buffer) {
            std::memset(buffer, 0x00, 7);
            buffer[7] = 0xCF;
            buffer[8] = 0x05;
        }

        void psdu_mapper_impl::insert_header(unsigned char* buffer, unsigned int psdu_len) {
            plcp_header header;
            header.service = 0x00;
            int doub_rate;
            switch(d_modulation) {
            case DBPSK_1:
                header.signal = 0x0A;
                doub_rate = 2;
                break;
            case DQPSK_2:
                header.signal = 0x14;
                doub_rate = 4;
                break;
            case CCK_5_5:
                header.signal = 0x37;
                doub_rate = 11;
                break;
            case CCK_11:
                header.signal = 0x6E;
                doub_rate = 22;
                break;
            default:
                throw std::runtime_error("How did you get here?");
                break;
            }
            int cmp = (16 * psdu_len) % doub_rate;
            int deficit = cmp > 0 ? doub_rate - cmp : 0;
            header.length = (16 * psdu_len + deficit) / doub_rate;
            if (d_modulation == CCK_11 && deficit >= 16)
                header.service |= 0x80;

            header.calc_crc();
            std::memcpy(buffer, &header, 4);
        }
        
    } /* namespace ieee802_11_b */
} /* namespace gr */
