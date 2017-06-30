# Top-level CMakeLists.txt file for Downloader.
#	 Copyright (C) 2015  Intel Corporation
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
#######################################################################
#
# Project file for library used to process command line options
#
# Copyright (C) 2011 Intel Corporation
#
#######################################################################

XFSTK_ROOT = ../../..
include( $${XFSTK_ROOT}/xfstk-config.pri )

QT -= core \
    gui
TARGET = xfstk-command-line
TEMPLATE = lib
CONFIG += staticlib

DESTDIR = $${DEST_COMMAND_LINE}
OBJECTS_DIR = $${OBJ_COMMAND_LINE}
MOC_DIR = $${MOC_COMMAND_LINE}

win32:INCLUDEPATH += $${INC_BOOST}
win32:LIBS += $${LIB_BOOST_MINGW}
win32:DEFINES += XFSTK_MULTIDEVICE

unix:INCLUDEPATH += $${INC_BOOST}
unix:LIBS += $${LIB_BOOST_GCC}
unix:DEFINES += XFSTK_MULTIDEVICE

SOURCES += $${SRC_COMMAND_LINE}/downloaderoptions.cpp
HEADERS += $${SRC_COMMAND_LINE}/downloaderoptions.h \
    $${SRC_COMMAND_LINE}/.ioption.h
