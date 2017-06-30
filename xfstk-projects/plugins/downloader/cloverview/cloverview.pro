# ######################################################################
# Project file for Downloader Template Plugin
# Copyright (C) 2011 Intel Corporation
# ######################################################################
XFSTK_ROOT = ../../../../
include( $${XFSTK_ROOT}/xfstk-config.pri )
QT += gui
DESTDIR = $${DEST_DOWNLOADER_PLUGIN_CLOVERVIEW}
OBJECTS_DIR = $${OBJ_DOWNLOADER_PLUGIN_CLOVERVIEW}
MOC_DIR = $${MOC_DOWNLOADER_PLUGIN_CLOVERVIEW}
UI_DIR = $${UI_DOWNLOADER_PLUGIN_CLOVERVIEW}
TARGET = XfstkDldrPluginCloverview
TEMPLATE = lib
win32:DEFINES += XFSTK_OS_WIN
win32:INCLUDEPATH += $${SRC_DOWNLOADER_API}
win32:INCLUDEPATH += $${SRC_DOWNLOADER_PLUGIN_INTERFACE}
win32:INCLUDEPATH += $${SRC_DOWNLOADER_PLUGIN_CLOVERVIEW}
win32:LIBS += $${LIB_DOWNLOADER_API_MINGW}
unix:INCLUDEPATH += $${SRC_DOWNLOADER_PLUGIN_INTERFACE}
unix:INCLUDEPATH += $${SRC_DOWNLOADER_API}
unix:LIBS += $${LIB_DOWNLOADER_API_LINUX}
DEFINES += XFSTKDLDRPLUGINTEMPLATE_LIBRARY
SOURCES += $${SRC_DOWNLOADER_PLUGIN_CLOVERVIEW}/xfstkdldrpluginuserinterface.cpp \
    $${SRC_DOWNLOADER_PLUGIN_CLOVERVIEW}/xfstkdldrpluginoptionsinterface.cpp \
    $${SRC_DOWNLOADER_PLUGIN_CLOVERVIEW}/xfstkdldrplugincloverview.cpp \
    $${SRC_DOWNLOADER_PLUGIN_CLOVERVIEW}/xfstkdldrthread.cpp
HEADERS += $${SRC_DOWNLOADER_PLUGIN_CLOVERVIEW}/xfstkdldrpluginuserinterface.h \
    $${SRC_DOWNLOADER_PLUGIN_CLOVERVIEW}/xfstkdldrpluginoptionsinterface.h \
    $${SRC_DOWNLOADER_PLUGIN_INTERFACE}/xfstkdldrplugininterface.h \
    $${SRC_DOWNLOADER_PLUGIN_CLOVERVIEW}/xfstkdldrplugincloverview.h \
    $${SRC_DOWNLOADER_PLUGIN_CLOVERVIEW}/xfstkdldrthread.h
FORMS += $${SRC_DOWNLOADER_PLUGIN_CLOVERVIEW}/xfstkdldrpluginuserinterface.ui \
    $${SRC_DOWNLOADER_PLUGIN_CLOVERVIEW}/xfstkdldrpluginoptionsinterface.ui
