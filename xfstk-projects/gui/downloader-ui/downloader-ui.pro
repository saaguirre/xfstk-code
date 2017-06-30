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
# Project file for Downloader GUI Application
#
# Copyright (C) 2011 Intel Corporation
#
#######################################################################

XFSTK_ROOT = ../../..
include( $${XFSTK_ROOT}/xfstk-config.pri )

TEMPLATE = app
TARGET = xfstk-dldr-gui

DESTDIR = $${DEST_DOWNLOADER_GUI}
OBJECTS_DIR = $${OBJ_DOWNLOADER_GUI}
MOC_DIR = $${MOC_DOWNLOADER_GUI}
RCC_DIR = $${RCC_DOWNLOADER_GUI}
UI_DIR = $${UI_DOWNLOADER_GUI}

win32:DEFINES += XFSTK_OS_WIN
unix:DEFINES += _LINUX

win32:INCLUDEPATH += $${SRC_DOWNLOADER_PLUGIN_INTERFACE}
unix:INCLUDEPATH += $${SRC_DOWNLOADER_PLUGIN_INTERFACE}

SOURCES += $${SRC_DOWNLOADER_GUI}/main.cpp \
    $${SRC_DOWNLOADER_GUI}/xfstkdldrgui.cpp \
    $${SRC_DOWNLOADER_GUI}/xfstkdldrpluginabstractor.cpp \
    $${SRC_DOWNLOADER_GUI}/xfstksettingsdialog.cpp \
    $${SRC_DOWNLOADER_GUI}/xfstkaboutdialog.cpp

HEADERS += $${SRC_DOWNLOADER_GUI}/xfstkdldrgui.h \
    $${SRC_DOWNLOADER_GUI}/xfstkdldrpluginabstractor.h \
    $${SRC_DOWNLOADER_GUI}/xfstksettingsdialog.h \
    $${SRC_DOWNLOADER_GUI}/xfstkaboutdialog.h

FORMS += $${SRC_DOWNLOADER_GUI}/xfstkdldrgui.ui \
    $${SRC_DOWNLOADER_GUI}/xfstksettingsdialog.ui \
    $${SRC_DOWNLOADER_GUI}/xfstkaboutdialog.ui

RESOURCES += $${SRC_DOWNLOADER_GUI}/xfstkresources.qrc

RC_FILE = $${SRC_DOWNLOADER_GUI}/xfstk.rc
