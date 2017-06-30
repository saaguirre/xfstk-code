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
cmake_minimum_required(VERSION 2.8.3)

if (ENTITYX_USE_CPP11_STDLIB)
    add_definitions(-DGTEST_USE_OWN_TR1_TUPLE=1)
    set(OLD_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    check_cxx_source_compiles(
    "
    #include <memory>

    int main() {
        std::shared_ptr<int>();
    }
    "
    ENTITYX_HAVE_CXX11_STDLIB
    )

    if (NOT ENTITYX_HAVE_CXX11_STDLIB)
        message("-- Not using -stdlib=libc++ (test failed to build)")
        set(CMAKE_CXX_FLAGS "${OLD_CMAKE_CXX_FLAGS}")
    else ()
        message("-- Using -stdlib=libc++")
    endif ()
else ()
    message("-- Using default stdlib (try -DENTITYX_USE_CPP11_STDLIB=1 to use -stdlib=libc++)")
endif ()

check_cxx_source_compiles(
"
#include <memory>

int main() { std::shared_ptr<int>(); }
"
ENTITYX_HAVE_STD_SHARED_PTR
)

check_cxx_source_compiles(
"
#include <boost/shared_ptr.hpp>

int main() { boost::shared_ptr<int>(); }
"
ENTITYX_HAVE_BOOST_SHARED_PTR
)

if (ENTITYX_HAVE_STD_SHARED_PTR AND ENTITYX_USE_STD_SHARED_PTR)
    message("-- Using std::shared_ptr<T>")
else()
    if (ENTITYX_USE_STD_SHARED_PTR)
        message("-- Using boost::shared_ptr<T> (std::shared_ptr<T> could not be used)")
    else()
        message("-- Using boost::shared_ptr<T> (try -DENTITYX_USE_STD_SHARED_PTR=1 to use std::shared_ptr<T>)")
    endif()
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/entityx/config.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/entityx/config.h
)

