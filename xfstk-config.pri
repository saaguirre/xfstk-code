# Build all projects by going to the XFSTK Root Directory and run QMake
# On Windows you may need to specify a value for MINGW_DIR if you are not
#    QT Creator 1.3.1
# On Linux you may have to specify INC_LIBUSB_LINUX and LIB_LIBUSB_GCC if
#    libusb is installed in a custom directory. You will also need to specify
#    the correct qmake spec when building on 32 bit systems.
#CONFIG Parameters These values impact how make files are generated
unix:INCLUDEPATH += $$quote(/usr/include/libusb-1.0)


CONFIG += debug_and_release
CONFIG += build_all
CONFIG += warn_on

isEmpty(XFSTK_ROOT) {
    error(xfstk-config project include requires XFSTK_ROOT to be defined. XFSTK should specify where the project exists in relation the the root)
}

DEBUG_DIR = debug
RELEASE_DIR = release
CONFIG(debug, debug|release) {
    # Debug Specific Assignments
    OUTPUT_DIR = $${DEBUG_DIR}

} else {
    # Release Specific Assignments
    OUTPUT_DIR = $${RELEASE_DIR}
}

XFSTK_VER_MAJ = 1
XFSTK_VER_MIN = 1
XFSTK_VER_REV = 3
XFSTK_VERSION = $${XFSTK_VER_MAJ}.$${XFSTK_VER_MIN}.$${XFSTK_VER_REV}a

# These values are the names of top level directory
SRC_DIR = xfstk-sources
EXTERNAL_DEPENDS_DIR = external-dependencies
BUILD_DIR = xfstk-build

# This value is based on the version of MinGW used to compile
isEmpty(MINGW_DIR) {
    message("MINGW_DIR not specified, setting to default value")
    MINGW_DIR = C:/Qt/2010.02.1/mingw
}

# Header only references
INC_BOOST = $$quote($${XFSTK_ROOT}/$${EXTERNAL_DEPENDS_DIR}/min_boost_1_46_1)
INC_LIBUSB_WIN32 = $$quote($${XFSTK_ROOT}/$${EXTERNAL_DEPENDS_DIR}/libusb-win32-bin/include)
isEmpty(INC_LIBUSB_LINUX) {
    message("INC_LIBUSB_LINUX not specified, setting to default value")
    INC_LIBUSB_LINUX = $$quote(/usr/include/libxml2)
}

INC_WINDRIVER_WIN32 = $$quote($${XFSTK_ROOT}/$${EXTERNAL_DEPENDS_DIR}/WinDriver/include)
isEmpty(INC_WINDRIVER_WIN32) {
    message("INC_WINDRIVER_WIN32 not specified, setting to default value")
    # INC_WINDRIVER_LINUX = $$quote(/usr/include/libxml2)
}

INC_WD_DEVICE_FACTORY = $$quote($${XFSTK_ROOT}/$${EXTERNAL_DEPENDS_DIR}/IDeviceWinDriver/include)

# Contains .cpp and other inputs that are used in the build process
SRC_CLSOCKETS = $$quote($${XFSTK_ROOT}/$${EXTERNAL_DEPENDS_DIR}/Release_clsockets_1_4_3)
SRC_COMMAND_LINE = $$quote($${XFSTK_ROOT}/$${SRC_DIR}/core/command-line-helper)
SRC_DOWNLOADER = $$quote($${XFSTK_ROOT}/$${SRC_DIR}/core/downloader)
SRC_DOWNLOADER_API = $$quote($${XFSTK_ROOT}/$${SRC_DIR}/api/downloader-api)
SRC_DOWNLOADER_GUI = $$quote($${XFSTK_ROOT}/$${SRC_DIR}/gui/downloader-ui)
SRC_DOWNLOADER_SOLO = $$quote($${XFSTK_ROOT}/$${SRC_DIR}/core/downloader-solo)
SRC_EMMC_DUMP = $$quote($${XFSTK_ROOT}/$${SRC_DIR}/core/emmc-dump)
SRC_STATUS_CLIENT = $$quote($${XFSTK_ROOT}/$${SRC_DIR}/core/status-client)
SRC_TEST_DOWNLOADER_API_MINGW = $$quote($${XFSTK_ROOT}/$${SRC_DIR}/tools/downloader-api-test-mingw)
SRC_FACTORY = $$quote($${XFSTK_ROOT}/$${SRC_DIR}/core/factory) ##JG NEW
SRC_DOWNLOADER_PLUGIN_INTERFACE = $$quote($${XFSTK_ROOT}/$${SRC_DIR}/plugins/downloader) ##JG NEW
SRC_DOWNLOADER_PLUGIN_TEMPLATE = $$quote($${XFSTK_ROOT}/$${SRC_DIR}/plugins/downloader/template) ##JG NEW
SRC_DOWNLOADER_PLUGIN_MEDFIELD = $$quote($${XFSTK_ROOT}/$${SRC_DIR}/plugins/downloader/medfield) ##JG NEW
SRC_DOWNLOADER_PLUGIN_CLOVERVIEW = $$quote($${XFSTK_ROOT}/$${SRC_DIR}/plugins/downloader/cloverview) ##JG NEW
SRC_DOWNLOADER_PLUGIN_CLOVERVIEWPLUS = $$quote($${XFSTK_ROOT}/$${SRC_DIR}/plugins/downloader/cloverviewplus) ##JG NEW
SRC_DOWNLOADER_PLUGIN_MERRIFIELD = $$quote($${XFSTK_ROOT}/$${SRC_DIR}/plugins/downloader/merrifield) ##JG NEW
SRC_DOWNLOADER_PLUGIN_BAYTRAIL = $$quote($${XFSTK_ROOT}/$${SRC_DIR}/plugins/downloader/baytrail) ##JG NEW
SRC_TEST_API_MINGW = $$quote($${XFSTK_ROOT}/$${SRC_DIR}/test/windows/mingw) #DP NEW
SRC_TEST_API_GCC = $$quote($${XFSTK_ROOT}/$${SRC_DIR}/test/linux) #DP NEW

# Locations where intermediate object files are placed
OBJ_COMMAND_LINE = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/obj/command-line-helper)
OBJ_DOWNLOADER = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/obj/downloader)
OBJ_DOWNLOADER_API = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/obj/downloader-api)
OBJ_DOWNLOADER_GUI = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/obj/downloader-ui)
OBJ_DOWNLOADER_SOLO = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/obj/downloader-solo)
OBJ_EMMC_DUMP = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/obj/emmc-dump)
OBJ_SOCKET = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/obj/socket-lib)
OBJ_STATUS_CLIENT = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/obj/status-client)
OBJ_TEST_DOWNLOADER_API_MINGW = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/obj/test-downloader-api-mingw)
OBJ_TEST_DOWNLOADER_API_GCC = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/obj/test-downloader-api-gcc)
OBJ_FACTORY = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/obj/factory) ##JG NEW
OBJ_DOWNLOADER_PLUGIN_TEMPLATE = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/obj/plugins/downloader/template) ##JG NEW
OBJ_DOWNLOADER_PLUGIN_MEDFIELD = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/obj/plugins/downloader/medfield) ##JG NEW
OBJ_DOWNLOADER_PLUGIN_CLOVERVIEW = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/obj/plugins/downloader/cloverview) ##JG NEW
OBJ_DOWNLOADER_PLUGIN_CLOVERVIEWPLUS = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/obj/plugins/downloader/cloverviewplus) ##JG NEW
OBJ_DOWNLOADER_PLUGIN_MERRIFIELD = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/obj/plugins/downloader/merrifield) ##JG NEW
OBJ_DOWNLOADER_PLUGIN_BAYTRAIL = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/obj/plugins/downloader/baytrail) ##JG NEW
OBJ_TEST_API_MINGW = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/obj/test/mingw) #DP NEW
OBJ_TEST_API_GCC = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/obj/test/gcc) #DP NEW

# Locations where outputs from the moc tool are placed
# This is only needed if the project builds QT componenents
MOC_COMMAND_LINE = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/moc/command-line-helper)
MOC_DOWNLOADER = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/moc/downloader)
MOC_DOWNLOADER_API = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/moc/downloader-api)
MOC_DOWNLOADER_GUI = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/moc/downloader-ui)
MOC_DOWNLOADER_SOLO = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/moc/downloader-solo)
MOC_EMMC_DUMP = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/moc/emmc-dump)
MOC_SOCKET = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/moc/socket-lib)
MOC_STATUS_CLIENT = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/moc/status-client)
MOC_TEST_DOWNLOADER_API_MINGW = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/moc/test-downloader-api-mingw)
MOC_TEST_DOWNLOADER_API_GCC = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/moc/test-downloader-api-gcc)
MOC_FACTORY = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/moc/factory) ##JG NEW
MOC_DOWNLOADER_PLUGIN_TEMPLATE = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/moc/plugins/downloader/template) ##JG NEW
MOC_DOWNLOADER_PLUGIN_MEDFIELD = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/moc/plugins/downloader/medfield) ##JG NEW
MOC_DOWNLOADER_PLUGIN_CLOVERVIEW = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/moc/plugins/downloader/cloverview) ##JG NEW
MOC_DOWNLOADER_PLUGIN_CLOVERVIEWPLUS = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/moc/plugins/downloader/cloverviewplus) ##JG NEW
MOC_DOWNLOADER_PLUGIN_MERRIFIELD = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/moc/plugins/downloader/merrifield) ##JG NEW
MOC_DOWNLOADER_PLUGIN_BAYTRAIL = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/moc/plugins/downloader/baytrail) ##JG NEW
MOC_TEST_API_MINGW = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/moc/test/mingw) ##DP NEW
MOC_TEST_API_GCC = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/moc/test/gcc) ##DP NEW

# Location of intermediate resource files
RCC_DOWNLOADER_GUI = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/rcc/downloader-ui)
##JG TODO FOR GUI 

# Location of intermediate files created by the UI Compiler
UI_DOWNLOADER_GUI = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/ui/downloader-ui)
UI_DOWNLOADER_PLUGIN_TEMPLATE = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/ui/plugins/downloader/template)
UI_DOWNLOADER_PLUGIN_MEDFIELD = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/ui/plugins/downloader/medfield)
UI_DOWNLOADER_PLUGIN_CLOVERVIEW = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/ui/plugins/downloader/cloverview)
UI_DOWNLOADER_PLUGIN_CLOVERVIEWPLUS = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/ui/plugins/downloader/cloverviewplus)
UI_DOWNLOADER_PLUGIN_MERRIFIELD = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/ui/plugins/downloader/merrifield)
UI_DOWNLOADER_PLUGIN_BAYTRAIL = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/temp/$${OUTPUT_DIR}/ui/plugins/downloader/baytrail)
##JG TODO FOR GUI

# Where the output of building is placed
DEST_COMMAND_LINE = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/bin/$${OUTPUT_DIR}/command-line-helper)
DEST_DOWNLOADER = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/bin/$${OUTPUT_DIR}/downloader)
DEST_DOWNLOADER_API = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/bin/$${OUTPUT_DIR}/downloader-api)
DEST_DOWNLOADER_GUI = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/bin/$${OUTPUT_DIR}/downloader-ui)
DEST_DOWNLOADER_SOLO = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/bin/$${OUTPUT_DIR}/downloader-solo)
DEST_EMMC_DUMP = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/bin/$${OUTPUT_DIR}/emmc-dump)
DEST_SOCKET = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/bin/$${OUTPUT_DIR}/socket-lib)
DEST_STATUS_CLIENT = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/bin/$${OUTPUT_DIR}/status-client)
DEST_TEST_DOWNLOADER_API_MINGW = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/bin/$${OUTPUT_DIR}/test-downloader-api-mingw)
DEST_TEST_DOWNLOADER_API_GCC = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/bin/$${OUTPUT_DIR}/test-downloader-api-gcc)
DEST_FACTORY = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/bin/$${OUTPUT_DIR}/factory) #JG NEW
DEST_DOWNLOADER_PLUGIN_TEMPLATE = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/bin/$${OUTPUT_DIR}/plugins/downloader) #JG NEW
DEST_DOWNLOADER_PLUGIN_MEDFIELD = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/bin/$${OUTPUT_DIR}/plugins/downloader) #JG NEW
DEST_DOWNLOADER_PLUGIN_CLOVERVIEW = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/bin/$${OUTPUT_DIR}/plugins/downloader) #JG NEW
DEST_DOWNLOADER_PLUGIN_CLOVERVIEWPLUS = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/bin/$${OUTPUT_DIR}/plugins/downloader) #JG NEW
DEST_DOWNLOADER_PLUGIN_MERRIFIELD = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/bin/$${OUTPUT_DIR}/plugins/downloader) #JG NEW
DEST_DOWNLOADER_PLUGIN_BAYTRAIL = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/bin/$${OUTPUT_DIR}/plugins/downloader) #JG NEW
DEST_TEST_API_MINGW = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/test/$${OUTPUT_DIR}) #DP NEW
DEST_TEST_API_GCC = $$quote($${XFSTK_ROOT}/$${BUILD_DIR}/test/$${OUTPUT_DIR}) #DP NEW

# Library locations that other code can link to
#MINGW Specific
LIB_BOOST_MINGW = $$quote($${XFSTK_ROOT}/$${EXTERNAL_DEPENDS_DIR}/min_boost_1_46_1/bin.v2/libs/program_options/libboost_program_options-mgw44-mt-d-1_46_1.a)
LIB_COMMAND_LINE_MINGW = $$quote($${DEST_COMMAND_LINE}/libxfstk-command-line.a)
LIB_DOWNLOADER_API_MINGW = $$quote($${DEST_DOWNLOADER_API}/libxfstk-dldr-api.a)
LIB_DOWNLOADER_API_LINUX = $$quote($${DEST_DOWNLOADER_API}/libxfstk-dldr-api.so)
LIB_PTHREAD_MINGW = $$quote($${MINGW_DIR}/mingw32/lib/libpthread.a)
LIB_FACTORY  = $$quote($${DEST_FACTORY}/libXfstkFactory.a) #JG NEW
LIB_WINDRIVER_MINGW = $$quote($${XFSTK_ROOT}/$${EXTERNAL_DEPENDS_DIR}/WinDriver/lib/x86/wdapi1140.lib)
isEmpty(LIB_WINDRIVER_MINGW) {
    message("LIB_WINDRIVER_MINGW not specified, setting to default value")
}
LIB_WD_DEVICE_FACTORY = $$quote($${XFSTK_ROOT}/$${EXTERNAL_DEPENDS_DIR}/IDeviceWinDriver/bin/libwdfactory.a)

#GCC Specific
LIB_BOOST_GCC = $$quote($${XFSTK_ROOT}/$${EXTERNAL_DEPENDS_DIR}/min_boost_1_46_1/bin.v2/libs/program_options/libboost_program_options.a)
LIB_COMMAND_LINE_GCC = $$quote($${DEST_COMMAND_LINE}/libxfstk-command-line.a)
LIB_DOWNLOADER_API_GCC = $$quote($${DEST_DOWNLOADER_API}/libxfstk-dldr-api.so)
isEmpty(LIB_LIBUSB_GCC) {
    message("LIB_LIBUSB_GCC not specified, setting to default value")
    LIB_LIBUSB_GCC = /usr/lib/libusb.a
}
