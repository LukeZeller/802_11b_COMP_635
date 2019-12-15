/* -*- c++ -*- */

#define IEEE802_11_B_API

%include "gnuradio.i"           // the common stuff

//load generated python docstrings
%include "ieee802_11_b_swig_doc.i"

%{
#include "ieee802_11_b/psdu_mapper.h"
#include "ieee802_11_b/code_mapper.h"
#include "ieee802_11_b/scramble.h"
%}

%include "ieee802_11_b/psdu_mapper.h"
GR_SWIG_BLOCK_MAGIC2(ieee802_11_b, psdu_mapper);

%include "ieee802_11_b/code_mapper.h"
GR_SWIG_BLOCK_MAGIC2(ieee802_11_b, code_mapper);
%include "ieee802_11_b/scramble.h"
GR_SWIG_BLOCK_MAGIC2(ieee802_11_b, scramble);

