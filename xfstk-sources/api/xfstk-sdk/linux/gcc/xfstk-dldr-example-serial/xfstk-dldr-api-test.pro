# -------------------------------------------------
# Project created by QtCreator 2011-05-24T10:08:08
# -------------------------------------------------
QT -= gui
TARGET = xfstk-dldr-api-test
CONFIG += console
CONFIG -= app_bundle
win32:INCLUDEPATH += $$quote(..)
win32:LIBS += $$quote(..\release\libxfstk-dldr-api.a)
unix:INCLUDEPATH += $$quote(..)
unix:LIBS += $$quote(../libxfstk-dldr-api.so)
TEMPLATE = app
SOURCES += xfstk-dldr-example-serial.cpp
