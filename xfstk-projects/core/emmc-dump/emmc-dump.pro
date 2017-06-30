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
# Project file for Downloader Solo Application
#
# Copyright (C) 2011 Intel Corporation
#
#######################################################################

XFSTK_ROOT = ../../..
include( $${XFSTK_ROOT}/xfstk-config.pri )

TEMPLATE = app
CONFIG += qt \
    console
TARGET = emmc-dump

DESTDIR = $${DEST_EMMC_DUMP}
OBJECTS_DIR = $${OBJ_EMMC_DUMP}
MOC_DIR = $${MOC_EMMC_DUMP}

win32:DEFINES += XFSTK_OS_WIN
win32:INCLUDEPATH += $${SRC_FACTORY}
win32:INCLUDEPATH += $${INC_BOOST}
# win32:INCLUDEPATH += $${INC_LIBUSB_WIN32}
win32:INCLUDEPATH += $${INC_WD_DEVICE_FACTORY}

#Added for Jungo Driver
#win32:INCLUDEPATH += $${INC_WINDRIVER_WIN32}

# Libraries NOTE: To the overzealous organizers out there, order matters
# when declaring libraries.

win32:LIBS += $${LIB_FACTORY}
win32:LIBS += $${LIB_WD_DEVICE_FACTORY}
win32:LIBS += $${LIB_WINDRIVER_MINGW}
win32:LIBS += $${LIB_COMMAND_LINE_MINGW}
win32:LIBS += $${LIB_BOOST_MINGW}


#Added for Jungo Driver
#win32:LIBS += $${LIB_WINDRIVER_MINGW}

unix:INCLUDEPATH += $${SRC_FACTORY}
unix:INCLUDEPATH += $${SRC_COMMAND_LINE}
unix:INCLUDEPATH += $${INC_BOOST}
unix:INCLUDEPATH += $${INC_LIBUSB_LINUX}

unix:LIBS += $${LIB_FACTORY}
unix:LIBS += $${LIB_COMMAND_LINE_GCC}
unix:LIBS += $${LIB_BOOST_GCC}
unix:LIBS += $${LIB_LIBUSB_GCC}
#unix:LIBS += $${LIB_LIBUSB_GCC_1_0}
QMAKE_LIBS_THREAD += -lpthread \
    -lrt
HEADERS += $${SRC_EMMC_DUMP}/emmcdump.h
SOURCES += $${SRC_EMMC_DUMP}/emmcdump.cpp \
    $${SRC_EMMC_DUMP}/emmcdumpmain.cpp
