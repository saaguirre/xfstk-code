#######################################################################
#
# Project file for Downloader Baytrail Plugin
#
# Copyright (C) 2011 Intel Corporation
#
#######################################################################

XFSTK_ROOT = ../../../../
include( $${XFSTK_ROOT}/xfstk-config.pri )

QT += gui

DESTDIR = $${DEST_DOWNLOADER_PLUGIN_BAYTRAIL}
OBJECTS_DIR = $${OBJ_DOWNLOADER_PLUGIN_BAYTRAIL}
MOC_DIR = $${MOC_DOWNLOADER_PLUGIN_BAYTRAIL}
UI_DIR = $${UI_DOWNLOADER_PLUGIN_BAYTRAIL}

TARGET = XfstkDldrPluginBaytrail
TEMPLATE = lib

win32:DEFINES += XFSTK_OS_WIN
win32:INCLUDEPATH += $${SRC_DOWNLOADER_API}
win32:INCLUDEPATH += $${SRC_DOWNLOADER_PLUGIN_INTERFACE}
win32:LIBS += $${LIB_DOWNLOADER_API_MINGW}

unix:INCLUDEPATH += $${SRC_DOWNLOADER_PLUGIN_INTERFACE}
unix:INCLUDEPATH += $${SRC_DOWNLOADER_API}
unix:LIBS += $${LIB_DOWNLOADER_API_LINUX}

DEFINES += XFSTKDLDRPLUGINTEMPLATE_LIBRARY

SOURCES += $${SRC_DOWNLOADER_PLUGIN_BAYTRAIL}/xfstkdldrpluginuserinterface.cpp \
    $${SRC_DOWNLOADER_PLUGIN_BAYTRAIL}/xfstkdldrpluginoptionsinterface.cpp \
    $${SRC_DOWNLOADER_PLUGIN_BAYTRAIL}/xfstkdldrpluginbaytrail.cpp \
    $${SRC_DOWNLOADER_PLUGIN_BAYTRAIL}/xfstkdldrthread.cpp

HEADERS += $${SRC_DOWNLOADER_PLUGIN_BAYTRAIL}/xfstkdldrpluginuserinterface.h \
    $${SRC_DOWNLOADER_PLUGIN_BAYTRAIL}/xfstkdldrpluginoptionsinterface.h \
    $${SRC_DOWNLOADER_PLUGIN_INTERFACE}/xfstkdldrplugininterface.h \
    $${SRC_DOWNLOADER_PLUGIN_BAYTRAIL}/xfstkdldrpluginbaytrail.h \
    $${SRC_DOWNLOADER_PLUGIN_BAYTRAIL}/xfstkdldrthread.h

FORMS += $${SRC_DOWNLOADER_PLUGIN_BAYTRAIL}/xfstkdldrpluginuserinterface.ui \
    $${SRC_DOWNLOADER_PLUGIN_BAYTRAIL}/xfstkdldrpluginoptionsinterface.ui
