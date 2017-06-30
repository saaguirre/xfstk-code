#	 Copyright (C) 2014  Intel Corporation
#
#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Lesser General Public
#    License as published by the Free Software Foundation; either
#    version 2.1 of the License, or (at your option) any later version.
#
#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public
#    License along with this library; if not, write to the Free Software
#    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
MESSAGE("\n**** Configuring command-line-helper ****")

SET(Boost_NO_BOOST_CMAKE ON)
FIND_PACKAGE(Boost REQUIRED COMPONENTS program_options)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/xfstk-sources/core/command-line-helper
    ${CMAKE_CURRENT_BINARY_DIR})

ADD_LIBRARY(xfstk-command-line
    STATIC
        ${CMAKE_SOURCE_DIR}/xfstk-sources/core/command-line-helper/downloaderoptions.cpp)

TARGET_LINK_LIBRARIES(xfstk-command-line
    ${Boost_LIBRARIES})

SET_TARGET_PROPERTIES(xfstk-command-line
    PROPERTIES
        AUTOMOC true)
