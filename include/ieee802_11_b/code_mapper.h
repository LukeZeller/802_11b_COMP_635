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

#ifndef INCLUDED_IEEE802_11_B_CODE_MAPPER_H
#define INCLUDED_IEEE802_11_B_CODE_MAPPER_H

#include <ieee802_11_b/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace ieee802_11_b {

    /*!
     * \brief <+description of block+>
     * \ingroup ieee802_11_b
     *
     */
    class IEEE802_11_B_API code_mapper : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<code_mapper> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of ieee802_11_b::code_mapper.
       *
       * To avoid accidental use of raw pointers, ieee802_11_b::code_mapper's
       * constructor is in a private implementation
       * class. ieee802_11_b::code_mapper::make is the public interface for
       * creating new instances.
       */
      static sptr make( );
    };

  } // namespace ieee802_11_b
} // namespace gr

#endif /* INCLUDED_IEEE802_11_B_CODE_MAPPER_H */

