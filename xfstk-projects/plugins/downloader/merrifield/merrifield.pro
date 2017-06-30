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

DESTDIR = $${DEST_DOWNLOADER_PLUGIN_MERRIFIELD}
OBJECTS_DIR = $${OBJ_DOWNLOADER_PLUGIN_MERRIFIELD}
MOC_DIR = $${MOC_DOWNLOADER_PLUGIN_MERRIFIELD}
UI_DIR = $${UI_DOWNLOADER_PLUGIN_MERRIFIELD}

TARGET = XfstkDldrPluginMerrifield
TEMPLATE = lib

win32:DEFINES += XFSTK_OS_WIN
win32:INCLUDEPATH += $${SRC_DOWNLOADER_API}
win32:INCLUDEPATH += $${SRC_DOWNLOADER_PLUGIN_INTERFACE}
win32:LIBS += $${LIB_DOWNLOADER_API_MINGW}

unix:INCLUDEPATH += $${SRC_DOWNLOADER_PLUGIN_INTERFACE}
unix:INCLUDEPATH += $${SRC_DOWNLOADER_API}
unix:LIBS += $${LIB_DOWNLOADER_API_LINUX}

DEFINES += XFSTKDLDRPLUGINTEMPLATE_LIBRARY

SOURCES += $${SRC_DOWNLOADER_PLUGIN_MERRIFIELD}/xfstkdldrpluginuserinterface.cpp \
    $${SRC_DOWNLOADER_PLUGIN_MERRIFIELD}/xfstkdldrpluginoptionsinterface.cpp \
    $${SRC_DOWNLOADER_PLUGIN_MERRIFIELD}/xfstkdldrpluginmerrifield.cpp \
    $${SRC_DOWNLOADER_PLUGIN_MERRIFIELD}/xfstkdldrthread.cpp

HEADERS += $${SRC_DOWNLOADER_PLUGIN_MERRIFIELD}/xfstkdldrpluginuserinterface.h \
    $${SRC_DOWNLOADER_PLUGIN_MERRIFIELD}/xfstkdldrpluginoptionsinterface.h \
    $${SRC_DOWNLOADER_PLUGIN_INTERFACE}/xfstkdldrplugininterface.h \
    $${SRC_DOWNLOADER_PLUGIN_MERRIFIELD}/xfstkdldrpluginmerrifield.h \
    $${SRC_DOWNLOADER_PLUGIN_MERRIFIELD}/xfstkdldrthread.h

FORMS += $${SRC_DOWNLOADER_PLUGIN_MERRIFIELD}/xfstkdldrpluginuserinterface.ui \
    $${SRC_DOWNLOADER_PLUGIN_MERRIFIELD}/xfstkdldrpluginoptionsinterface.ui
