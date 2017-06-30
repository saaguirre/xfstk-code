#######################################################################
#
# Project file for Downloader Template Plugin
#
# Copyright (C) 2011 Intel Corporation
#
#######################################################################

XFSTK_ROOT = ../../../../
include( $${XFSTK_ROOT}/xfstk-config.pri )

QT += gui

DESTDIR = $${DEST_DOWNLOADER_PLUGIN_TEMPLATE}
OBJECTS_DIR = $${OBJ_DOWNLOADER_PLUGIN_TEMPLATE}
MOC_DIR = $${MOC_DOWNLOADER_PLUGIN_TEMPLATE}
UI_DIR = $${UI_DOWNLOADER_PLUGIN_TEMPLATE}

TARGET = XfstkDldrPluginTemplate
TEMPLATE = lib
DEFINES += XFSTKDLDRPLUGINTEMPLATE_LIBRARY

win32:DEFINES += XFSTK_OS_WIN
win32:INCLUDEPATH += $${SRC_DOWNLOADER_PLUGIN_INTERFACE}
unix:INCLUDEPATH += $${SRC_DOWNLOADER_PLUGIN_INTERFACE}

SOURCES += $${SRC_DOWNLOADER_PLUGIN_TEMPLATE}/xfstkdldrplugintemplate.cpp \
    $${SRC_DOWNLOADER_PLUGIN_TEMPLATE}/xfstkdldrpluginuserinterface.cpp \
    $${SRC_DOWNLOADER_PLUGIN_TEMPLATE}/xfstkdldrpluginoptionsinterface.cpp

HEADERS += $${SRC_DOWNLOADER_PLUGIN_TEMPLATE}/xfstkdldrplugintemplate.h \
    $${SRC_DOWNLOADER_PLUGIN_TEMPLATE}/xfstkdldrpluginuserinterface.h \
    $${SRC_DOWNLOADER_PLUGIN_TEMPLATE}/xfstkdldrpluginoptionsinterface.h

FORMS += $${SRC_DOWNLOADER_PLUGIN_TEMPLATE}/xfstkdldrpluginuserinterface.ui \
    $${SRC_DOWNLOADER_PLUGIN_TEMPLATE}/xfstkdldrpluginoptionsinterface.ui
