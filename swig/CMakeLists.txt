# Copyright 2011 Free Software Foundation, Inc.
#
# This file was generated by gr_modtool, a tool from the GNU Radio framework
# This file is a part of gr-ieee802_11_b
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.

########################################################################
# Check if there is C++ code at all
########################################################################
if(NOT ieee802_11_b_sources)
    MESSAGE(STATUS "No C++ sources... skipping swig/")
    return()
endif(NOT ieee802_11_b_sources)

########################################################################
# Include swig generation macros
########################################################################
find_package(SWIG)
find_package(PythonLibs)
if(NOT SWIG_FOUND OR NOT PYTHONLIBS_FOUND)
    return()
endif()
include(GrSwig)
include(GrPython)

########################################################################
# Setup swig generation
########################################################################
set(GR_SWIG_INCLUDE_DIRS $<TARGET_PROPERTY:gnuradio::runtime_swig,INTERFACE_INCLUDE_DIRECTORIES>)
set(GR_SWIG_TARGET_DEPS gnuradio::runtime_swig)

set(GR_SWIG_LIBRARIES gnuradio-ieee802_11_b)

set(GR_SWIG_DOC_FILE ${CMAKE_CURRENT_BINARY_DIR}/ieee802_11_b_swig_doc.i)
set(GR_SWIG_DOC_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../include)

GR_SWIG_MAKE(ieee802_11_b_swig ieee802_11_b_swig.i)

########################################################################
# Install the build swig module
########################################################################
GR_SWIG_INSTALL(TARGETS ieee802_11_b_swig DESTINATION ${GR_PYTHON_DIR}/ieee802_11_b)

########################################################################
# Install swig .i files for development
########################################################################
install(
    FILES
    ieee802_11_b_swig.i
    ${CMAKE_CURRENT_BINARY_DIR}/ieee802_11_b_swig_doc.i
    DESTINATION ${GR_INCLUDE_DIR}/ieee802_11_b/swig
)
