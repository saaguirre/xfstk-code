#!/bin/bash

################################################
#Setup
################################################
XFSTK_DOWNLOADER_LINUX_SRC_ROOT=${PWD}
TIMESTAMPCHANGELOG=`date --rfc-2822`

#check if there is a build system supplied version number
#if no build system version then versions are set here 
if [ -z $BUILD_VERSION ]; then
  echo "No build system supplied version number, setting version numbers here"
  ################################################
  #Set app version here for manual setup
  ################################################
  DOWNLOADER_VERSION=1.3.3
  ################################################

  echo "Performing a version check on debian/changelog file."
  echo "If this fails then  manual intervention will be needed."
  echo " "

  #search in file ./xfstk-sources/installer/linux/debian/internal/changelog for VERSION_NUMBER_INTERNAL
  grep $DOWNLOADER_VERSION ./xfstk-sources/installer/linux/xfstk-downloader-0.0.0/debian/changelog
  if [ $? -eq 0 ];then
     echo "Version number appears to be set correctly"
  else
     echo "ERROR: Version numbers do not match.  Buld script variable \$DOWNLOADER_VERSION = $DOWNLOADER_VERSION"
     echo "and xfstk-sources/installer/linux/xfstk-downloader-0.0.0/debian/changelog does not contain this version number."
     echo "Please manually update the changelog file with correct version and also document the changes."
     exit 1
  fi

else  #use the build system supplied version number  
  echo Build system supplied version number is $BUILD_VERSION
  DOWNLOADER_VERSION=$BUILD_VERSION

  cd $XFSTK_DOWNLOADER_LINUX_SRC_ROOT
  cd ./xfstk-sources/installer/linux/xfstk-downloader-0.0.0/debian/
  mv changelog tmp.txt
  echo "xfstk-downloader ($DOWNLOADER_VERSION) unstable; urgency=low" > header.txt
  echo " * " >> header.txt
  echo " -- Intel Corporation <xfstk@intel.com>  $TIMESTAMPCHANGELOG" >> header.txt
  echo "" >> header.txt
  cat header.txt tmp.txt > changelog
  cd $XFSTK_DOWNLOADER_LINUX_SRC_ROOT

fi




#use this line below for debug, it will pause the script
#so you can inspect progress
#read -p "pausing... press enter" nothing


################################################
#Pre-requisite packages checks for Ubuntu 10.04
################################################
echo "Checking pre-requisites"
echo " "

dpkg -s qtcreator > /dev/null 2> /dev/null
if [ $? -eq 0 ];then
   echo "Qt creator installed"
else
   echo "ERROR: Qt creator not installed"
   exit 1
fi

dpkg -s g++ > /dev/null 2> /dev/null
if [ $? -eq 0 ];then
   echo "g++ installed"
else
   echo "ERROR: g++ not installed"
   exit 1
fi

dpkg -s build-essential > /dev/null 2> /dev/null
if [ $? -eq 0 ];then
   echo "build-essential installed"
else
   echo "ERROR: build-essential not installed"
   exit 1
fi

dpkg -s devscripts > /dev/null 2> /dev/null
if [ $? -eq 0 ];then
   echo "devscripts installed"
else
   echo "WARNING: devscripts not installed"
   echo "Devscripts is not required but has great tools for pacakge maintanence"
fi

dpkg -s libxml2-dev > /dev/null 2> /dev/null
if [ $? -eq 0 ];then
   echo "libxml2-dev installed"
else
   echo "ERROR: libxml2-dev not installed"
   exit 1
fi

dpkg -s alien > /dev/null 2> /dev/null
if [ $? -eq 0 ];then
   echo "alien installed"
else
   echo "ERROR: alien not installed"
   exit 1
fi

which doxygen > /dev/null 2> /dev/null
if [ $? -eq 0 ];then
   echo "doxygen installed"
else
   echo "ERROR: doxygen not installed"
   exit 1
fi

dpkg -s graphviz > /dev/null 2> /dev/null
if [ $? -eq 0 ];then
   echo "graphviz installed"
else
   echo "ERROR: graphviz not installed"
   ### ADD THIS WHEN build system has proper graphviz tool installed ### exit 1
fi

dpkg -s libusb-dev > /dev/null 2> /dev/null
if [ $? -eq 0 ];then
   echo "libusb-dev installed"
else
   echo "ERROR: libusb-dev not installed"
   exit 1
fi

################################################
# Print out machine and tool chain version information
################################################
echo "Version information for build tools"
echo " "
echo "Make version"
make --version
echo " "
echo "Qmake-qt4 version"
echo " " 
qmake-qt4  -v
echo " "

echo "Dpkg-buildpackage version"
echo " " 
dpkg-buildpackage --version
echo " "
echo "Alien version"
echo " " 
alien -V
echo " " 
echo "Doxygen version"
echo " " 
doxygen --version
echo " " 

echo "Build Machine Operating System Information"
echo " "
echo "Linux kernel version "
uname -a
echo " "
echo "Linux distribution "
lsb_release -a
echo " "
echo "Host name"
hostname
echo "Domain name"
domainname
echo " "


################################################
#Cleanup
################################################
make clean -w in $XFSTK_DOWNLOADER_LINUX_SRC_ROOT
rm -rf xfstk-build
find . -name "Makefile*" -exec rm {} \;

################################################
#Build
################################################
echo " "
echo "It's build time"
echo " "

cd $XFSTK_DOWNLOADER_LINUX_SRC_ROOT

echo "Generating all makefiles"


#Call qmake, include version number
# the crazy syntax is because we go through the shell, qmake, shell, and make
# sh -> qmake -> sh -> make
#an alternate method is to remove version number usage from qmake call and define the versions in the .pri, or lfstk.cpp source
qmake-qt4 -spec linux-g++ -recursive $XFSTK_DOWNLOADER_LINUX_SRC_ROOT/xfstk.pro CONFIG+=debug DEFINES+=DOWNLOADER_VERSION=\\\\\"\\\"$DOWNLOADER_VERSION\\\\\"\\\"
if [ $? -eq 0 ];then
   echo "qmake completed successfully"
else
   echo "ERROR: qmake-qt4 step problem"
   exit 1
fi

make -w
if [ $? -eq 0 ];then
   echo "make -w completed successfully"
else
   echo "ERROR: make -w step problem"
   exit 1
fi

################################################
#Documentation generation
################################################
echo " "
echo "Generating documentation"
echo " "
cd $XFSTK_DOWNLOADER_LINUX_SRC_ROOT

mkdir -p ./xfstk-build/temp/docs/
mkdir -p ./xfstk-build/temp/docs/source
mkdir -p ./xfstk-build/temp/docs/images

cp ./xfstk-sources/api/downloader-api/xfstkdldrapi.h \
   ./xfstk-build/temp/docs/source/
cp ./xfstk-sources/api/SDK/xfstk-dldr-example-parallel/{*.cpp,*.h,*.hpp} \
   ./xfstk-build/temp/docs/source/
cp ./xfstk-sources/api/SDK/xfstk-dldr-example-parallel-dynamic-load/{*.cpp,*.h,*.hpp} \
   ./xfstk-build/temp/docs/source/
cp ./xfstk-sources/api/SDK/xfstk-dldr-example-serial/{*.cpp,*.h,*.hpp} \
   ./xfstk-build/temp/docs/source/
cp -R ./ancillary/doxygen/source/* ./xfstk-build/temp/docs/source/
cp -R ./ancillary/doxygen/images/* ./xfstk-build/temp/docs/images/
cp ./xfstk-docs/doxygen/xfstk-downloader-doxyfile.config ./xfstk-build/temp/docs/ -fv

cd ./xfstk-build/temp/docs

(cat xfstk-downloader-doxyfile.config ; \
echo "OUTPUT_DIRECTORY=."; \
echo "PROJECT_NUMBER=$DOWNLOADER_VERSION") \
| doxygen -

################################################
#Packaging 
################################################
echo "Packaging preparation"
echo " "
echo "Version is stored in the control file 'changelog'... set it there, not in directory name"

cd $XFSTK_DOWNLOADER_LINUX_SRC_ROOT

#make the first directories used for packaging
mkdir -p ./xfstk-build/installer/linux/
mkdir -p ./xfstk-build/installer/linux/fedora
mkdir -p ./xfstk-build/installer/linux/ubuntu

#copy the packager source files into place
cp -ax ./xfstk-sources/installer/linux/xfstk-downloader-0.0.0  \
   ./xfstk-build/installer/linux/

#copy the binaries and plugins into place so the packager can use them
cp ./xfstk-build/bin/release/downloader-solo/xfstk-dldr-solo \
   ./xfstk-build/installer/linux/xfstk-downloader-0.0.0/usr/bin/
cp ./xfstk-build/bin/release/downloader-ui/xfstk-dldr-gui \
   ./xfstk-build/installer/linux/xfstk-downloader-0.0.0/usr/bin/
cp ./xfstk-build/bin/release/plugins/downloader/libXfstkDldrPluginMedfield.so.1.0.0 \
   ./xfstk-build/installer/linux/xfstk-downloader-0.0.0/usr/lib/xfstk/xfstkdldrplugins
cp ./xfstk-build/bin/release/plugins/downloader/libXfstkDldrPluginCloverview.so.1.0.0 \
   ./xfstk-build/installer/linux/xfstk-downloader-0.0.0/usr/lib/xfstk/xfstkdldrplugins
cp ./xfstk-build/bin/release/plugins/downloader/libXfstkDldrPluginCloverviewPlus.so.1.0.0 \
   ./xfstk-build/installer/linux/xfstk-downloader-0.0.0/usr/lib/xfstk/xfstkdldrplugins
cp ./xfstk-build/bin/release/plugins/downloader/libXfstkDldrPluginMerrifield.so.1.0.0 \
   ./xfstk-build/installer/linux/xfstk-downloader-0.0.0/usr/lib/xfstk/xfstkdldrplugins
cp ./xfstk-build/bin/release/plugins/downloader/libXfstkDldrPluginBaytrail.so.1.0.0 \
   ./xfstk-build/installer/linux/xfstk-downloader-0.0.0/usr/lib/xfstk/xfstkdldrplugins
cp -ax ./xfstk-build/bin/release/downloader-api/* \
   ./xfstk-build/installer/linux/xfstk-downloader-0.0.0/usr/lib/

#copy the documentation files into place
cp -a ./xfstk-build/temp/docs/html/* \
   ./xfstk-build/installer/linux/xfstk-downloader-0.0.0/usr/share/doc/xfstk-downloader/html

#build the sdk
mkdir ./xfstk-build/temp/xfstk-sdk
mkdir ./xfstk-build/temp/xfstk-sdk/linux
mkdir ./xfstk-build/temp/xfstk-sdk/linux/gcc
mkdir ./xfstk-build/temp/xfstk-sdk/linux/gcc/xfstk-dldr-api
cp ./xfstk-sources/api/downloader-api/xfstkdldrapi.h \
   ./xfstk-build/temp/xfstk-sdk/linux/gcc/xfstk-dldr-api
cp ./xfstk-sources/api/downloader-api/xfstk-dldr-api_global.h \
   ./xfstk-build/temp/xfstk-sdk/linux/gcc/xfstk-dldr-api
cp ./xfstk-build/bin/release/downloader-api/*.* \
   ./xfstk-build/temp/xfstk-sdk/linux/gcc/xfstk-dldr-api
mkdir ./xfstk-build/temp/xfstk-sdk/linux/gcc/xfstk-dldr-example-parallel
mkdir ./xfstk-build/temp/xfstk-sdk/linux/gcc/xfstk-dldr-example-parallel-dynamic-load
mkdir ./xfstk-build/temp/xfstk-sdk/linux/gcc/xfstk-dldr-example-serial
cp ./xfstk-sources/api/xfstk-sdk/linux/gcc/xfstk-dldr-example-parallel/*.pro \
   ./xfstk-build/temp/xfstk-sdk/linux/gcc/xfstk-dldr-example-parallel
cp ./xfstk-sources/api/xfstk-sdk/linux/gcc/xfstk-dldr-example-parallel/*.cpp \
   ./xfstk-build/temp/xfstk-sdk/linux/gcc/xfstk-dldr-example-parallel
cp ./xfstk-sources/api/xfstk-sdk/linux/gcc/xfstk-dldr-example-parallel/*.h \
   ./xfstk-build/temp/xfstk-sdk/linux/gcc/xfstk-dldr-example-parallel
cp ./xfstk-build/bin/release/downloader-api/*.* \
   ./xfstk-build/temp/xfstk-sdk/linux/gcc/xfstk-dldr-example-parallel
cp ./xfstk-sources/api/xfstk-sdk/linux/gcc/xfstk-dldr-example-parallel-dynamic-load/*.pro \
   ./xfstk-build/temp/xfstk-sdk/linux/gcc/xfstk-dldr-example-parallel-dynamic-load
cp ./xfstk-sources/api/xfstk-sdk/linux/gcc/xfstk-dldr-example-parallel-dynamic-load/*.cpp \
   ./xfstk-build/temp/xfstk-sdk/linux/gcc/xfstk-dldr-example-parallel-dynamic-load
cp ./xfstk-sources/api/xfstk-sdk/linux/gcc/xfstk-dldr-example-parallel-dynamic-load/*.h \
   ./xfstk-build/temp/xfstk-sdk/linux/gcc/xfstk-dldr-example-parallel-dynamic-load
cp ./xfstk-build/bin/release/downloader-api/*.* \
   ./xfstk-build/temp/xfstk-sdk/linux/gcc/xfstk-dldr-example-parallel-dynamic-load
cp ./xfstk-sources/api/xfstk-sdk/linux/gcc/xfstk-dldr-example-serial/*.pro \
   ./xfstk-build/temp/xfstk-sdk/linux/gcc/xfstk-dldr-example-serial
cp ./xfstk-sources/api/xfstk-sdk/linux/gcc/xfstk-dldr-example-serial/*.cpp \
   ./xfstk-build/temp/xfstk-sdk/linux/gcc/xfstk-dldr-example-serial
cp ./xfstk-build/bin/release/downloader-api/*.* \
   ./xfstk-build/temp/xfstk-sdk/linux/gcc/xfstk-dldr-example-serial
cd ./xfstk-build/temp
tar -czvf xfstk-sdk.tar.gz ./xfstk-sdk/
cd $XFSTK_DOWNLOADER_LINUX_SRC_ROOT
cp ./xfstk-build/temp/xfstk-sdk.tar.gz \
   ./xfstk-build/installer/linux/xfstk-downloader-0.0.0/usr/lib/xfstk/


################################################
#Create a i386 debian package so we can make an rpm
################################################
echo "Creating i386 deb package to convert to rpm"
echo " "

# cp "i386" control file into place
cd $XFSTK_DOWNLOADER_LINUX_SRC_ROOT/xfstk-build/installer/linux/xfstk-downloader-0.0.0/debian
cp control.i386 control

#change to the downloader dir to run the packager
cd ..
dpkg-buildpackage -rfakeroot -b -uc -us
#the call below was used previously for cross compliing
##dpkg-buildpackage -ai386 -ti386-linux-gnu -rfakeroot -b
if [ $? -eq 0 ];then
   echo "dpkg-buildpackage completed successfully"
else
   echo "ERROR: dpkg-buildpackage problem"
   exit 1
fi

#move the i386 debian package into the fedora directory 
cd ..
mv xfstk-downloader*i386.deb ./fedora/

#run alien to create the fedora pakcage from the debian package
cd ./fedora/
alien --to-rpm --keep-version --scripts *.deb
if [ $? -eq 0 ];then
   echo "alien -r -k completed successfully"
else
   echo "ERROR: alien -r -k step problem"
   exit 1
fi

#remove debian pkg, set prompt back to starting dir
rm *.deb
cd $XFSTK_DOWNLOADER_LINUX_SRC_ROOT


################################################
#Create an "all" debian package
################################################
echo "Creating .deb for ubuntu"
echo " "

# cp "all" control file into place
cd $XFSTK_DOWNLOADER_LINUX_SRC_ROOT/xfstk-build/installer/linux/xfstk-downloader-0.0.0/debian
cp control.all control

#change to the downloader dir to run the packager
cd ..
dpkg-buildpackage -rfakeroot -b -uc -us
if [ $? -eq 0 ];then
   echo "dpkg-buildpackage completed successfully"
else
   echo "ERROR: dpkg-buildpackage problem"
   exit 1
fi

cd ..
mv *.deb ./ubuntu/

cd $XFSTK_DOWNLOADER_LINUX_SRC_ROOT

