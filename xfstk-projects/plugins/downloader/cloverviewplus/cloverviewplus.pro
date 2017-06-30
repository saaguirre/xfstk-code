# ######################################################################
# Project file for Downloader Template Plugin
# Copyright (C) 2011 Intel Corporation
# ######################################################################
XFSTK_ROOT = ../../../../
include( $${XFSTK_ROOT}/xfstk-config.pri )
QT += gui
DESTDIR = $${DEST_DOWNLOADER_PLUGIN_CLOVERVIEWPLUS}
OBJECTS_DIR = $${OBJ_DOWNLOADER_PLUGIN_CLOVERVIEWPLUS}
MOC_DIR = $${MOC_DOWNLOADER_PLUGIN_CLOVERVIEWPLUS}
UI_DIR = $${UI_DOWNLOADER_PLUGIN_CLOVERVIEWPLUS}
TARGET = XfstkDldrPluginCloverviewPlus
TEMPLATE = lib
win32:DEFINES += XFSTK_OS_WIN
win32:INCLUDEPATH += $${SRC_DOWNLOADER_API}
win32:INCLUDEPATH += $${SRC_DOWNLOADER_PLUGIN_INTERFACE}
win32:INCLUDEPATH += $${SRC_DOWNLOADER_PLUGIN_CLOVERVIEWPLUS}
win32:LIBS += $${LIB_DOWNLOADER_API_MINGW}
unix:INCLUDEPATH += $${SRC_DOWNLOADER_PLUGIN_INTERFACE}
unix:INCLUDEPATH += $${SRC_DOWNLOADER_API}
unix:LIBS += $${LIB_DOWNLOADER_API_LINUX}
DEFINES += XFSTKDLDRPLUGINTEMPLATE_LIBRARY
SOURCES += $${SRC_DOWNLOADER_PLUGIN_CLOVERVIEWPLUS}/xfstkdldrpluginuserinterface.cpp \
    $${SRC_DOWNLOADER_PLUGIN_CLOVERVIEWPLUS}/xfstkdldrpluginoptionsinterface.cpp \
    $${SRC_DOWNLOADER_PLUGIN_CLOVERVIEWPLUS}/xfstkdldrplugincloverviewplus.cpp \
    $${SRC_DOWNLOADER_PLUGIN_CLOVERVIEWPLUS}/xfstkdldrthread.cpp
HEADERS += $${SRC_DOWNLOADER_PLUGIN_CLOVERVIEWPLUS}/xfstkdldrpluginuserinterface.h \
    $${SRC_DOWNLOADER_PLUGIN_CLOVERVIEWPLUS}/xfstkdldrpluginoptionsinterface.h \
    $${SRC_DOWNLOADER_PLUGIN_INTERFACE}/xfstkdldrplugininterface.h \
    $${SRC_DOWNLOADER_PLUGIN_CLOVERVIEWPLUS}/xfstkdldrplugincloverviewplus.h \
    $${SRC_DOWNLOADER_PLUGIN_CLOVERVIEWPLUS}/xfstkdldrthread.h
FORMS += $${SRC_DOWNLOADER_PLUGIN_CLOVERVIEWPLUS}/xfstkdldrpluginuserinterface.ui \
    $${SRC_DOWNLOADER_PLUGIN_CLOVERVIEWPLUS}/xfstkdldrpluginoptionsinterface.ui
