# -------------------------------------------------
# Project created by QtCreator 2011-05-24T10:08:08
# -------------------------------------------------
QT -= gui
TARGET = xfstk-dldr-api-test
CONFIG += console
CONFIG -= app_bundle
win32:INCLUDEPATH += $$quote(..\xfstk-dldr-api)
win32:LIBS += $$quote(..\xfstk-dldr-api\release\libxfstk-dldr-api.a)
unix:INCLUDEPATH += $$quote(../xfstk-dldr-api)
unix:LIBS += $$quote(../xfstk-dldr-api/libxfstk-dldr-api.so)
TEMPLATE = app
SOURCES += xfstk-dldr-example-parallel.cpp
