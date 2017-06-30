# ######################################################################
# Project file for Downloader Template Plugin
# Copyright (C) 2011 Intel Corporation
# ######################################################################
XFSTK_ROOT = ../../../../
include( $${XFSTK_ROOT}/xfstk-config.pri )
QT += gui
DESTDIR = $${DEST_DOWNLOADER_PLUGIN_MEDFIELD}
OBJECTS_DIR = $${OBJ_DOWNLOADER_PLUGIN_MEDFIELD}
MOC_DIR = $${MOC_DOWNLOADER_PLUGIN_MEDFIELD}
UI_DIR = $${UI_DOWNLOADER_PLUGIN_MEDFIELD}
TARGET = XfstkDldrPluginMedfield
TEMPLATE = lib
win32:DEFINES += XFSTK_OS_WIN
win32:INCLUDEPATH += $${SRC_DOWNLOADER_API}
win32:INCLUDEPATH += $${SRC_DOWNLOADER_PLUGIN_INTERFACE}
win32:INCLUDEPATH += $${SRC_DOWNLOADER_PLUGIN_MEDFIELD}
win32:LIBS += $${LIB_DOWNLOADER_API_MINGW}
unix:INCLUDEPATH += $${SRC_DOWNLOADER_PLUGIN_INTERFACE}
unix:INCLUDEPATH += $${SRC_DOWNLOADER_API}
unix:LIBS += $${LIB_DOWNLOADER_API_LINUX}
DEFINES += XFSTKDLDRPLUGINTEMPLATE_LIBRARY
SOURCES += $${SRC_DOWNLOADER_PLUGIN_MEDFIELD}/xfstkdldrpluginuserinterface.cpp \
    $${SRC_DOWNLOADER_PLUGIN_MEDFIELD}/xfstkdldrpluginoptionsinterface.cpp \
    $${SRC_DOWNLOADER_PLUGIN_MEDFIELD}/xfstkdldrpluginmedfield.cpp \
    $${SRC_DOWNLOADER_PLUGIN_MEDFIELD}/xfstkdldrthread.cpp
HEADERS += $${SRC_DOWNLOADER_PLUGIN_MEDFIELD}/xfstkdldrpluginuserinterface.h \
    $${SRC_DOWNLOADER_PLUGIN_MEDFIELD}/xfstkdldrpluginoptionsinterface.h \
    $${SRC_DOWNLOADER_PLUGIN_INTERFACE}/xfstkdldrplugininterface.h \
    $${SRC_DOWNLOADER_PLUGIN_MEDFIELD}/xfstkdldrpluginmedfield.h \
    $${SRC_DOWNLOADER_PLUGIN_MEDFIELD}/xfstkdldrthread.h
FORMS += $${SRC_DOWNLOADER_PLUGIN_MEDFIELD}/xfstkdldrpluginuserinterface.ui \
    $${SRC_DOWNLOADER_PLUGIN_MEDFIELD}/xfstkdldrpluginoptionsinterface.ui
