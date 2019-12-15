#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2019 gr-ieee802_11_b author.
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#

from gnuradio import gr, gr_unittest
from gnuradio import blocks
import ieee802_11_b_swig as ieee802_11_b
import random
import time

class qa_scramble(gr_unittest.TestCase):

    def setUp(self):
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_scramble(self):
        # 0 1 0 1 1 1 1 0 0 1 0 1 1 1 0 1
        src_data = (0x7A, 0xBA)
        # 1 1 0 1 0 1 0 0 1 0 1 1 1 1 1 1
        expected_res = (0x2B, 0xFD)

        src_blk = blocks.vector_source_b(src_data)
        scramble_blk = ieee802_11_b.scramble(False)
        dst_blk = blocks.vector_sink_b()
        
        self.tb.connect(src_blk, scramble_blk)
        self.tb.connect(scramble_blk, dst_blk)
        
        self.tb.run()

        actual_res = dst_blk.data()
        self.assertEqual(expected_res, actual_res)

    def test_002_descramble(self):
        src_data = (0x2B, 0xFD)
        # 1 1 0 1 0 1 0 0 1 0 1 1 1 1 1 1
        expected_res = (0x00, 0xBA)
        # X X X X X X X 0 0 1 0 1 1 1 0 1

        src_blk = blocks.vector_source_b(src_data)
        scramble_blk = ieee802_11_b.scramble(True)
        dst_blk = blocks.vector_sink_b()
        
        self.tb.connect(src_blk, scramble_blk)
        self.tb.connect(scramble_blk, dst_blk)
        
        self.tb.run()

        actual_res = dst_blk.data()
        self.assertEqual(expected_res[0] >> 7, actual_res[0] >> 7)
        self.assertEqual(expected_res[1], actual_res[1])


    def test_003_no_data(self):
        print("Time elapsed = " + str(self._test_timing(0)))

    def test_004_1MB(self):
        print("Time elapsed = " + str(self._test_timing(1 << 20)))

    def test_avg_latency(self):
        for i in range(1, 10):
            # 10*i Mbits
            n_items = i * 10 * (1 << 17)
        
            fixed_overhead = self._test_timing(0)
            total_time = self._test_timing(n_items)

            avg_lat_us = total_time * 10**6 / n_items
            print("Number of bytes: " + str(n_items))
            print("Average byte latency (microseconds): " + str(avg_lat_us))
        
        
    def _test_timing(self, n_items):
        src_data = [random.randint(0, 255) for _ in range(n_items)]

        src_blk = blocks.vector_source_b(src_data)
        scramble_blk = ieee802_11_b.scramble(True)
        dst_blk = blocks.vector_sink_b(reserve_items = n_items)

        self.tb.connect(src_blk, scramble_blk)
        self.tb.connect(scramble_blk, dst_blk)

        s = time.time()
        self.tb.run()
        e = time.time()

        return e - s
        
if __name__ == '__main__':
    gr_unittest.run(qa_scramble)
