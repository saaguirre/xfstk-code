# Generate Documentation
#	 Copyright (C) 2014  Intel Corporation
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

MESSAGE("\n**** Begin Generate documenation ****")

FIND_PACKAGE(Doxygen)
IF (NOT DOXYGEN_FOUND)
	MESSAGE(FATAL_ERROR "Please install Doxygen.")
ENDIF()

# Here are the items in the doxy file that need to get customized.
SET(version			$ENV{BUILD_VERSION})
SET(images			${CMAKE_SOURCE_DIR}/ancillary/doxygen/images)
SET(logo			"${CMAKE_SOURCE_DIR}/ancillary/doxygen/xfstk-128x128.png")
SET(footer			${CMAKE_SOURCE_DIR}/ancillary/doxygen/source/_footer.html)
SET(inputs
	"${CMAKE_SOURCE_DIR}/ancillary/doxygen/source/_SDK.dox \\
	${CMAKE_SOURCE_DIR}/xfstk-sources/api/downloader-api/xfstkdldrapi.h \\
	${CMAKE_SOURCE_DIR}/xfstk-sources/api/SDK/xfstk-dldr-example-parallel/xfstk-dldr-example-parallel.cpp \\
	${CMAKE_SOURCE_DIR}/xfstk-sources/api/SDK/xfstk-dldr-example-parallel/xfstk-dldr-example-parallel.h \\
	${CMAKE_SOURCE_DIR}/xfstk-sources/api/SDK/xfstk-dldr-example-parallel-dynamic-load/xfstk-dldr-example-parallel.cpp \\
	${CMAKE_SOURCE_DIR}/xfstk-sources/api/SDK/xfstk-dldr-example-parallel-dynamic-load/xfstk-dldr-example-parallel.h \\
	${CMAKE_SOURCE_DIR}/xfstk-sources/api/SDK/xfstk-dldr-example-serial/xfstk-dldr-example-serial.cpp")

# We could output directly to the package but then you wouldn't be able to do `make docs`.
SET(outdir				docs) 

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/ancillary/doxygen/xfstk-downloader-doxyfile.config 
	${PROJECT_BINARY_DIR}/_doxyfile @ONLY IMMEDIATE)

ADD_CUSTOM_TARGET(docs ALL
	COMMAND ${DOXYGEN_EXECUTABLE} '${PROJECT_BINARY_DIR}/_doxyfile')
		
