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
message(STATUS "Processing emmc-dump")

include_directories(${CMAKE_SOURCE_DIR}/xfstk-sources/core/emmc-dump)

include_directories(${CMAKE_SOURCE_DIR}/xfstk-sources/core/factory)
include_directories(${CMAKE_CURRENT_BINARY_DIR})


add_library(emmc-dump SHARED
    ${CMAKE_SOURCE_DIR}/xfstk-sources/core/emmc-dump/emmcdump.cpp
    ${CMAKE_SOURCE_DIR}/xfstk-sources/core/emmc-dump/emmcdumpmain.cpp)

set_target_properties(emmc-dump
    PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output/obj/emmc-dump
        AUTOMOC true)

