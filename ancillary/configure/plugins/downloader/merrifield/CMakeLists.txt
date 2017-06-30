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
MESSAGE("\n**** Configuring Merrifield Plugin ****")

FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})

# Find ui files.
file(GLOB GUI_UI ${CMAKE_SOURCE_DIR}/xfstk-sources/plugins/downloader/merrifield/*.ui)
# Run UIC on them
QT4_WRAP_UI(UIC_SOURCES ${GUI_UI})

SET(MER     ${CMAKE_SOURCE_DIR}/xfstk-sources/plugins/downloader/merrifield)
SET(PLUG    ${CMAKE_SOURCE_DIR}/xfstk-sources/plugins/downloader)
SET(API     ${CMAKE_SOURCE_DIR}/xfstk-sources/api/downloader-api)

# Show Cmake where the header files are.
INCLUDE_DIRECTORIES(
    ${MER}
    ${PLUG}
    ${API}
    ${CMAKE_CURRENT_BINARY_DIR})

ADD_LIBRARY(XfstkDldrPluginMerrifield
    SHARED
        ${MER}/xfstkdldrpluginuserinterface.cpp
        ${MER}/xfstkdldrpluginoptionsinterface.cpp
        ${MER}/xfstkdldrpluginmerrifield.cpp
        ${MER}/xfstkdldrthread.cpp
        ${UIC_SOURCES})
    
target_link_libraries(XfstkDldrPluginMerrifield
    xfstk-command-line
    XfstkFactory
    ${QT_LIBRARIES}
    xfstk-dldr-gui-api)

SET_TARGET_PROPERTIES(XfstkDldrPluginMerrifield
    PROPERTIES
        AUTOMOC true)

INSTALL(TARGETS XfstkDldrPluginMerrifield
    LIBRARY DESTINATION /usr/lib/xfstk/xfstkdldrplugins
        COMPONENT gui)
