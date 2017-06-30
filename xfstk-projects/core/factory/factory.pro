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
# ######################################################################
# Project file for Factory
# Copyright (C) 2011 Intel Corporation
# ######################################################################
XFSTK_ROOT = ../../..
include( $${XFSTK_ROOT}/xfstk-config.pri )
TEMPLATE = lib
CONFIG += staticlib
TARGET = XfstkFactory
QT -= gui
DESTDIR = $${DEST_FACTORY}
OBJECTS_DIR = $${OBJ_FACTORY}
MOC_DIR = $${MOC_FACTORY}
win32:DEFINES += XFSTK_FORCE_ONESHOT_HACK
win32:DEFINES += XFSTK_OS_WIN
win32:DEFINES += XFSTK_STATUS_SOCKET
win32:DEFINES += XFSTK_MULTIDEVICE
win32:DEFINES += XFSTK_CLOVERTRAIL

unix:DEFINES += XFSTK_STATUS_SOCKET
unix:DEFINES += XFSTK_MULTIDEVICE
unix:DEFINES += XFSTK_FORCE_ONESHOT_HACK
unix:DEFINES += XFSTK_CLOVERTRAIL
win32:INCLUDEPATH += $${INC_BOOST}

win32:INCLUDEPATH += $${SRC_COMMAND_LINE}
win32:INCLUDEPATH += $${INC_WD_DEVICE_FACTORY}
win32:INCLUDEPATH += $${SRC_FACTORY}

# Libraries NOTE: To the overzealous organizers out there, order matters
# when declaring libraries.
win32:LIBS += $${LIB_BOOST_MINGW}
win32:LIBS += $${LIB_COMMAND_LINE_MINGW}

win32:LIBS += $${LIB_PTHREAD_MINGW}
win32:LIBS += $${LIB_WINDRIVER_MINGW}
win32:LIBS += $${LIB_WD_DEVICE_FACTORY}

unix:INCLUDEPATH += $${SRC_COMMAND_LINE}
unix:INCLUDEPATH += $${INC_BOOST}
unix:INCLUDEPATH += $${INC_LIBUSB_LINUX}
unix:INCLUDEPATH += $${SRC_FACTORY}
unix:LIBS += $${LIB_COMMAND_LINE_GCC}

unix:LIBS += $${LIB_BOOST_GCC}
unix:LIBS += $${LIB_LIBUSB_GCC}
#unix:LIBS += $${LIB_LIBUSB_GCC_1_0}

SOURCES += $${SRC_FACTORY}/platforms/cloverviewplus/cloverviewplusutils.cpp \
    $${SRC_FACTORY}/platforms/cloverviewplus/cloverviewplusoptions.cpp \
    $${SRC_FACTORY}/platforms/cloverviewplus/cloverviewplusdownloader.cpp \
    $${SRC_FACTORY}/platforms/cloverviewplus/cloverviewplusdevice.cpp \
    $${SRC_FACTORY}/platforms/cloverview/cloverviewutils.cpp \
    $${SRC_FACTORY}/platforms/cloverview/cloverviewoptions.cpp \
    $${SRC_FACTORY}/platforms/cloverview/cloverviewdownloader.cpp \
    $${SRC_FACTORY}/platforms/cloverview/cloverviewdevice.cpp \
    $${SRC_FACTORY}/platforms/cloverview/cloverviewfw.cpp \
    $${SRC_FACTORY}/platforms/cloverview/cloverviewos.cpp \
    $${SRC_FACTORY}/platforms/cloverview/clvdldrstate.cpp \
    $${SRC_FACTORY}/platforms/medfield/medfieldutils.cpp \
    $${SRC_FACTORY}/platforms/medfield/medfieldoptions.cpp \
    $${SRC_FACTORY}/platforms/medfield/medfielddownloader.cpp \
    $${SRC_FACTORY}/platforms/medfield/medfielddevice.cpp \
    $${SRC_FACTORY}/platforms/medfield/medfieldfw.cpp \
    $${SRC_FACTORY}/platforms/medfield/dldrstate.cpp \
    $${SRC_FACTORY}/platforms/medfield/medfieldos.cpp \
    $${SRC_FACTORY}/platforms/merrifield/merrifieldutils.cpp \
    $${SRC_FACTORY}/platforms/merrifield/merrifieldoptions.cpp \
    $${SRC_FACTORY}/platforms/merrifield/merrifielddownloader.cpp \
    $${SRC_FACTORY}/platforms/merrifield/merrifielddevice.cpp \
    $${SRC_FACTORY}/platforms/merrifield/mrfdldrstate.cpp \
    $${SRC_FACTORY}/platforms/merrifield/merrifieldfw.cpp \
    $${SRC_FACTORY}/platforms/merrifield/merrifieldos.cpp \
    $${SRC_FACTORY}/xfstkfactory.cpp \
    $${SRC_FACTORY}/platforms/cloverviewplus/cloverviewplusfw.cpp \
    $${SRC_FACTORY}/platforms/cloverviewplus/cloverviewplusos.cpp \
    $${SRC_FACTORY}/platforms/cloverviewplus/clvpdldrstate.cpp \
#    $${SRC_FACTORY}/platforms/merrifield/xserialcomm.cpp \
    $${SRC_FACTORY}/platforms/hashverify/hashverifyoptions.cpp \
    $${SRC_FACTORY}/platforms/hashverify/hashverifydownloader.cpp \
    $${SRC_FACTORY}/platforms/hashverify/hashverifyutils.cpp \
    $${SRC_FACTORY}/platforms/softfuses/softfusesfw.cpp \
    $${SRC_FACTORY}/platforms/emmc/emmcoptions.cpp \
    $${SRC_FACTORY}/platforms/emmc/emmcstate.cpp \
    $${SRC_FACTORY}/platforms/emmc/emmcdownloader.cpp \
    $${SRC_FACTORY}/platforms/emmc/emmcfw.cpp \
    $${SRC_FACTORY}/platforms/emmc/emmcutils.cpp \
    $${SRC_FACTORY}/platforms/baytrail/baytrailutils.cpp \
    $${SRC_FACTORY}/platforms/baytrail/baytrailoptions.cpp \
    $${SRC_FACTORY}/platforms/baytrail/baytrailfw.cpp \
    $${SRC_FACTORY}/platforms/baytrail/baytraildownloader.cpp \
    $${SRC_FACTORY}/platforms/baytrail/baytraildevice.cpp
HEADERS += $${SRC_FACTORY}/interfaces/ioptions.h \
    $${SRC_FACTORY}/interfaces/ifactory.h \
    $${SRC_FACTORY}/interfaces/idownloader.h \
    $${SRC_FACTORY}/interfaces/idevice.h \
    $${SRC_FACTORY}/interfaces/idevicefactory.h \
    $${SRC_FACTORY}/interfaces/ivisitor.h \
    $${SRC_FACTORY}/common/xfstktypes.h \
    $${SRC_FACTORY}/common/xfstkcommon.h \
    $${SRC_FACTORY}/common/scoped_file.h \
    $${SRC_FACTORY}/platforms/cloverviewplus/cloverviewplusutils.h \
    $${SRC_FACTORY}/platforms/cloverviewplus/cloverviewplusoptions.h \
    $${SRC_FACTORY}/platforms/cloverviewplus/cloverviewplusmessages.h \
    $${SRC_FACTORY}/platforms/cloverviewplus/cloverviewplusdownloader.h \
    $${SRC_FACTORY}/platforms/cloverviewplus/cloverviewplusdevice.h \
    $${SRC_FACTORY}/platforms/cloverview/cloverviewutils.h \
    $${SRC_FACTORY}/platforms/cloverview/cloverviewoptions.h \
    $${SRC_FACTORY}/platforms/cloverview/cloverviewmessages.h \
    $${SRC_FACTORY}/platforms/cloverview/cloverviewdownloader.h \
    $${SRC_FACTORY}/platforms/cloverview/cloverviewdevice.h \
    $${SRC_FACTORY}/platforms/cloverview/cloverviewfw.h \
    $${SRC_FACTORY}/platforms/cloverview/cloverviewos.h \
    $${SRC_FACTORY}/platforms/cloverview/clvdldrhandler.h \
    $${SRC_FACTORY}/platforms/cloverview/clvdldrstate.h \
    $${SRC_FACTORY}/platforms/medfield/medfieldutils.h \
    $${SRC_FACTORY}/platforms/medfield/medfieldoptions.h \
    $${SRC_FACTORY}/platforms/medfield/medfieldmessages.h \
    $${SRC_FACTORY}/platforms/medfield/medfielddownloader.h \
    $${SRC_FACTORY}/platforms/medfield/medfielddevice.h \
    $${SRC_FACTORY}/platforms/medfield/dldrhandler.h \
    $${SRC_FACTORY}/platforms/medfield/medfieldfw.h \
    $${SRC_FACTORY}/platforms/medfield/dldrstate.h \
    $${SRC_FACTORY}/platforms/medfield/medfieldos.h \
    $${SRC_FACTORY}/platforms/merrifield/merrifieldoptions.h \
    $${SRC_FACTORY}/platforms/merrifield/merrifieldmessages.h \
    $${SRC_FACTORY}/platforms/merrifield/merrifielddownloader.h \
    $${SRC_FACTORY}/platforms/merrifield/merrifielddevice.h \
    $${SRC_FACTORY}/platforms/merrifield/merrifieldutils.h \
    $${SRC_FACTORY}/platforms/merrifield/mrfdldrstate.h \
    $${SRC_FACTORY}/platforms/merrifield/merrifieldfw.h \
    $${SRC_FACTORY}/platforms/merrifield/mrfdldrhandler.h \
    $${SRC_FACTORY}/platforms/merrifield/merrifieldos.h\
    $${SRC_FACTORY}/platforms/cloverviewplus/cloverviewplusfw.h \
    $${SRC_FACTORY}/platforms/cloverviewplus/cloverviewplusos.h \
    $${SRC_FACTORY}/platforms/cloverviewplus/clvpdldrhandler.h \
    $${SRC_FACTORY}/platforms/cloverviewplus/clvpdldrstate.h \
    $${SRC_FACTORY}/xfstkfactory.h \
    $${SRC_FACTORY}/platforms/hashverify/hashverifyoptions.h \
    $${SRC_FACTORY}/platforms/hashverify/hashverifydownloader.h \
    $${SRC_FACTORY}/platforms/hashverify/hashverifyutils.h \
    $${SRC_FACTORY}/platforms/hashverify/HashVerifyMessages.h \
    $${SRC_FACTORY}/platforms/softfuses/softfusesMessages.h \
    $${SRC_FACTORY}/platforms/softfuses/softfusesfw.h \
    $${SRC_FACTORY}/platforms/softfuses/softfusesdldrhandler.h \
    $${SRC_FACTORY}/platforms/emmc/emmcutils.h \
    $${SRC_FACTORY}/platforms/emmc/emmcoptions.h \
    $${SRC_FACTORY}/platforms/emmc/emmcmessages.h \
    $${SRC_FACTORY}/platforms/emmc/emmcstate.h \
    $${SRC_FACTORY}/platforms/emmc/emmcdownloader.h \
    $${SRC_FACTORY}/platforms/emmc/emmchandler.h \
    $${SRC_FACTORY}/platforms/emmc/emmcfw.h \
    $${SRC_FACTORY}/platforms/baytrail/baytrailutils.h \
    $${SRC_FACTORY}/platforms/baytrail/baytrailoptions.h \
    $${SRC_FACTORY}/platforms/baytrail/baytrailfw.h \
    $${SRC_FACTORY}/platforms/baytrail/baytraildownloader.h \
    $${SRC_FACTORY}/platforms/baytrail/baytraildevice.h \
    $${SRC_FACTORY}/platforms/baytrail/baytrailmessages.h
