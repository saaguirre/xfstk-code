# XfstkDldrPluginTemplate
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

set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost REQUIRED COMPONENTS filesystem system program_options)

FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})

SET( QT_USE_QTSQL FALSE )
# Find ui files.
file(GLOB GUI_UI ${FSTKDOWNLOADER_SOURCE_DIR}/xfstk-sources/plugins/downloader/template/*.ui)
# Run UIC on them
QT4_WRAP_UI(UIC_SOURCES ${GUI_UI})


SET(T ${FSTKDOWNLOADER_SOURCE_DIR}/xfstk-sources/plugins/downloader/template)

# Show Cmake where the header files are.
INCLUDE_DIRECTORIES(${T}
                    ${C}
                    ${D}
                    ${DA}
                    ${CMAKE_CURRENT_BINARY_DIR})


ADD_LIBRARY(XfstkDldrPluginTemplate
                SHARED
                    ${T}/xfstkdldrplugintemplate.cpp
                    ${T}/xfstkdldrpluginuserinterface.cpp
                    ${T}/xfstkdldrpluginoptionsinterface.cpp
                    ${UIC_SOURCES})

SET_TARGET_PROPERTIES(XfstkDldrPluginTemplate
                        PROPERTIES
                            AUTOMOC true)


