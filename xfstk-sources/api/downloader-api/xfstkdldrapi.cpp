/*
    Copyright (C) 2014  Intel Corporation
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <stdexcept>
#include <string>
#include "xfstkdldrfactory.h"
#include "xfstkdldrapi.h"
#include <boost/smart_ptr/scoped_array.hpp>
QMutex apimultitargetstatusmutex;
QMutex apiinterfaceavailablemutex;
QMutex apiusbdevicemutex;
//The build script searches for the x.y.z string and replaces it with the build system version number
//Please do not change this string "0.0.0"
#ifndef DOWNLOADER_VERSION
#define DOWNLOADER_VERSION "0.0.0"
#endif
#ifdef XFSTK_OS_WIN
bool xfstkdldrapi::registerdevicecallback(DevicesCallBack* callBackStruct, unsigned long ProductId)
{
    bool retval = true;
    xfstkdldrfactory* xfstkfactoryinterface = (xfstkdldrfactory*)xfstkfactoryhandle;
    if(ProductId >= XFSTK_MAXDEVICE)
    {
        retval = false;
        return retval;
    }
    retval = ProductId ? xfstkfactoryinterface->SetDeviceCallBackInstances(reinterpret_cast<void*>(callBackStruct),ProductId):
                        xfstkfactoryinterface->SetDeviceCallBackInstances(reinterpret_cast<void*>(callBackStruct));
    return retval;
}
#endif
void xfstkdldrapi::xfstklogmessage(char *message, void *clientdata)
{
    if(physstatuspfn != NULL) {
        physstatuspfn(message,clientdata);
    }
    else {
        printf("%s\n",message);
    }
}
void xfstkdldrapi::xfstkmultitargetstatuscallback(char *message, void *clientdata)
{
    QMutexLocker locker(&apimultitargetstatusmutex);
    if(physstatuspfn) {
        physstatuspfn(message,clientdata);
    }
    else {
        printf("%s",message);
    }
}
xfstkdldrapi::xfstkdldrapi()
{  
    xfstkdldrfactory* xfstkfactoryinterface  = NULL;
    if(xfstkfactoryinterface == NULL) {
        xfstkfactoryinterface = new xfstkdldrfactory();
        if(xfstkfactoryinterface == NULL) {
            exit(-1);
        }
    }
    xfstkfactoryhandle = (void*)xfstkfactoryinterface;
    binterfaceavailable = true;
    binterfaceclaimed = false;
    miscdnxenable = false;
    softfuseinclude = false;
    physstatuspfn = NULL;
    physclientdata = NULL;
    retrycount = 40;
    this->miscdnx = new char[2048];
    this->softfuse = new char[2048];
}
xfstkdldrapi::~xfstkdldrapi()
{
    xfstkdldrfactory* xfstkfactoryinterface = (xfstkdldrfactory*)xfstkfactoryhandle;
    if(xfstkfactoryinterface != NULL) {
        xfstkfactoryinterface->ClearAllLists();
        if(xfstkfactoryinterface)
        {
            delete xfstkfactoryinterface;
            xfstkfactoryinterface = NULL;
        }
        xfstkfactoryinterface = NULL;
    }
    if(this->miscdnx)
    {
        delete[] this->miscdnx;
        this->miscdnx = NULL;
    }
    if(this->softfuse)
    {
        delete[] this->softfuse;
        this->softfuse = NULL;
    }
}
void xfstkdldrapi::settargetretrycount(int retries)
{
	retrycount = retries;
}
void xfstkdldrapi::setmiscdnxpath(bool enable, char * miscdnx)
{
    xfstkdldrfactory* xfstkfactoryinterface = (xfstkdldrfactory*)xfstkfactoryhandle;
    memcpy(this->miscdnx, miscdnx, strlen(miscdnx)+1);
    xfstkfactoryinterface->miscdnxenable = enable;
    memcpy(xfstkfactoryinterface->miscdnx.get(), miscdnx, strlen(miscdnx)+1);
    miscdnxenable = enable;
}
void xfstkdldrapi::setsoftfusepath(bool include, char * softfuse)
{
    xfstkdldrfactory* xfstkfactoryinterface = (xfstkdldrfactory*)xfstkfactoryhandle;
    memcpy(this->softfuse, softfuse, strlen(softfuse)+1);
    xfstkfactoryinterface->softfuseinclude = include;
    if( xfstkfactoryinterface->softfuse )
    {
        delete xfstkfactoryinterface->softfuse;
        xfstkfactoryinterface->softfuse = NULL;
    }
	xfstkfactoryinterface->softfuse = new char[ strlen(softfuse)+1 ];
    memcpy(xfstkfactoryinterface->softfuse, softfuse, strlen(softfuse)+1);
    softfuseinclude = include;
}
void xfstkdldrapi::setloglevel(unsigned long loglevel)
{
    xfstkdldrfactory* xfstkfactoryinterface = (xfstkdldrfactory*)xfstkfactoryhandle;
    xfstkfactoryinterface->debuglevel = loglevel;
}
void xfstkdldrapi::setusbreadwritedelay(unsigned long delayms)
{
    xfstkdldrfactory* xfstkfactoryinterface = (xfstkdldrfactory*)xfstkfactoryhandle;
    xfstkfactoryinterface->usbdelayms = delayms;
}
void xfstkdldrapi::setwipeifwi(bool enable)
{
    xfstkdldrfactory* xfstkfactoryinterface = (xfstkdldrfactory*)xfstkfactoryhandle;
    xfstkfactoryinterface->wipeifwi = enable;
}
bool xfstkdldrapi::idrqresponse(unsigned char *buffer, int maxsize)
{
    int tmpargc = 3;
    char* tmpargv[3];
    boost::scoped_array<char> message (new char[1024]);
    xfstksleep sleeper;
    xfstkdldrfactory* xfstkfactoryinterface = (xfstkdldrfactory*)xfstkfactoryhandle;
    this->showversion();
    boost::scoped_array<char> arg0(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg1(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg2(new char[XFSTK_PARAM_LENGTH]);
    sprintf(arg0.get(),"xfstk-dldr-api");
    sprintf(arg1.get(),"--idrq");
    sprintf(arg2.get(),"--verbose");
    tmpargv[0] = &arg0[0];
    tmpargv[1] = &arg1[0];
    tmpargv[2] = &arg2[0];
    xfstkfactoryinterface->SetIdrqResponse(buffer,maxsize);
    int devicedetected = 0;
    while(!xfstkfactoryinterface->EnumerateDevices()) {
        xfstklogmessage(const_cast<char*>("XFSTK-PROGRESS--0"),this->physclientdata);
        sprintf(message.get(),"XFSTK-STATUS--Detecting Intel Device - Attempt #%d",devicedetected);
        xfstklogmessage(message.get(),this->physclientdata);
        xfstklogmessage(const_cast<char*>("XFSTK-PROGRESS--5"),this->physclientdata);
        if(devicedetected > this->retrycount) {
            xfstklogmessage(const_cast<char*>("XFSTK-STATUS--Aborting hash verification process."),this->physclientdata);
            xfstklogmessage(const_cast<char*>("XFSTK-PROGRESS--100"),this->physclientdata);
            xfstkfactoryinterface->ClearAllLists();
            return false;
        }
        devicedetected++;
        sleeper.sleep(1);
    }
    //Parse the options from the user
    if(!xfstkfactoryinterface->SetOptions(tmpargc,tmpargv)) {
        printf("XFSTK: Download options could not be parsed correctly.\n");
        printf("XFSTK: Please connect only a single SoC device and cycle device power.\n");
        printf("XFSTK: Aborting hash verification process.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    //Bind all interfaces together
    if(!xfstkfactoryinterface->BindInterfaces()) {
        printf("XFSTK: Binding failed for Download, Device, and Options interfaces.\n");
        printf("XFSTK: Aborting hash verification process.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    //Execute the download
    xfstkfactoryinterface->retrycount = this->retrycount;
    if(!xfstkfactoryinterface->ExecuteDownloadSerial()) {
        printf("XFSTK: Download operation encountered errors.\n");
        printf("XFSTK: Please verify hash file integrity and reprovision target.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    printf( "\nXFSTK: IDRQ Reponse Completed Successfully.\n");
    xfstkfactoryinterface->ClearAllLists();
    this->releaseinterface();
    return true;
}
bool xfstkdldrapi::downloadfw(char *fwfile)
{
    int tmpargc = 4;
    char* tmpargv[4];
    char message[1024];
    xfstksleep sleeper;
    xfstkdldrfactory* xfstkfactoryinterface = (xfstkdldrfactory*)xfstkfactoryhandle;
    xfstkfactoryinterface->expandedOptions = BAYTRAIL;
    xfstkfactoryinterface->XfstkDldrFactory->CreateDownloaderOptions(BAYTRAIL);
    xfstkfactoryinterface->transporttype = XFSTK_DEDI_PROG;
    this->showversion();
    char arg0[XFSTK_PARAM_LENGTH];
    char arg1[XFSTK_PARAM_LENGTH];
    char arg2[XFSTK_PARAM_LENGTH];
    char arg3[XFSTK_PARAM_LENGTH];
    memset(&arg0,0,sizeof(arg0));
    memset(&arg1,0,sizeof(arg1));
    memset(&arg2,0,sizeof(arg2));
    memset(&arg3,0,sizeof(arg3));
    sprintf(arg0,"spi");
    sprintf(arg1,"--fwimage");
    sprintf(arg2,"%s",fwfile);
    sprintf(arg3,"--verbose");
    sprintf(&message[0],"\ndownload file -- %s\n", fwfile);
    xfstklogmessage(&message[0], const_cast<void* >(this->physclientdata));
    tmpargv[0] = &arg0[0];
    tmpargv[1] = &arg1[0];
    tmpargv[2] = &arg2[0];
    tmpargv[3] = &arg3[0];
    int devicedetected = 0;
    while(!xfstkfactoryinterface->EnumerateDevices())
    {
        xfstklogmessage(const_cast<char* >("XFSTK-PROGRESS--0"), const_cast<void* >(this->physclientdata));
        sprintf(&message[0],"XFSTK-STATUS--Detecting Intel Device - Attempt #%d",devicedetected);
        xfstklogmessage(&message[0], const_cast<void* >(this->physclientdata));
        xfstklogmessage(const_cast<char* >("XFSTK-PROGRESS--5"), const_cast<void* >(this->physclientdata));
        if(devicedetected > this->retrycount)
        {
            xfstklogmessage(const_cast<char* >("XFSTK-STATUS--Aborting downloadfw() spi process.\n"), this->physclientdata);
            xfstklogmessage(const_cast<char* >("XFSTK-PROGRESS--100"),this->physclientdata);
            xfstkfactoryinterface->ClearAllLists();
            return false;
        }
        devicedetected++;
        sleeper.sleep(1);
    }
    //Parse the options from the user
    if(!xfstkfactoryinterface->SetOptions(tmpargc,tmpargv)) {
        printf("XFSTK: Download options could not be parsed correctly.\n");
        printf("XFSTK-STATUS--Aborting downloadfw() spi process.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    //Bind all interfaces together
    if(!xfstkfactoryinterface->BindInterfaces()) {
        printf("XFSTK: Binding failed for Download, Device, and Options interfaces.\n");
        printf("XFSTK-STATUS--Aborting downloadfw() spi process.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    //Execute the download
   xfstkfactoryinterface->retrycount = this->retrycount;
    if(!xfstkfactoryinterface->ExecuteDownloadSerial()) {
        printf("XFSTK: Download operation encountered errors.\n");
        printf("XFSTK-STATUS--Aborting downloadfw() spi process.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    printf( "\nXFSTK: API downloadfw(char *fwfile) Completed Successfully.\n");
    xfstkfactoryinterface->ClearAllLists();
    this->releaseinterface();
    return true;
}
bool xfstkdldrapi::hashverify(char *hashfile, bool write)
{
    int tmpargc = 3;
    char* tmpargv[4];
    boost::scoped_array<char> message (new char[1024]);
    xfstksleep sleeper;
    xfstkdldrfactory* xfstkfactoryinterface = (xfstkdldrfactory*)xfstkfactoryhandle;
    xfstkfactoryinterface->expandedOptions =  HASHVERIFY;
    this->showversion();
    boost::scoped_array<char> arg0(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg1(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg2(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg3(new char[XFSTK_PARAM_LENGTH]);
    sprintf(arg0.get(),"hashverify");
    sprintf(arg1.get(),"--hashfile");
    sprintf(arg2.get(),"%s",hashfile);
    sprintf(message.get(),"\nhashfile -- %s\n", hashfile);
    xfstklogmessage(message.get(),this->physclientdata);
    tmpargv[0] = &arg0[0];
    tmpargv[1] = &arg1[0];
    tmpargv[2] = &arg2[0];
    if(write)
    {
        sprintf(arg3.get(),"--write");
        tmpargv[3] = &arg3[0];
        tmpargc = 4;
    }
    int devicedetected = 0;
    while(!xfstkfactoryinterface->EnumerateDevices()) {
        xfstklogmessage(const_cast<char*>("XFSTK-PROGRESS--0"),this->physclientdata);
        sprintf(message.get(),"XFSTK-STATUS--Detecting Intel Device - Attempt #%d",devicedetected);
        xfstklogmessage(message.get(),this->physclientdata);
        xfstklogmessage(const_cast<char*>("XFSTK-PROGRESS--5"),this->physclientdata);
        if(devicedetected > this->retrycount) {
            xfstklogmessage(const_cast<char* >("XFSTK-STATUS--Aborting hash verification process."), this->physclientdata);
            xfstklogmessage(const_cast<char* >("XFSTK-PROGRESS--100"),this->physclientdata);
            xfstkfactoryinterface->ClearAllLists();
            return false;
        }
        devicedetected++;
        sleeper.sleep(1);
    }
    //Parse the options from the user
    if(!xfstkfactoryinterface->SetOptions(tmpargc,tmpargv)) {
        printf("XFSTK: Download options could not be parsed correctly.\n");
        printf("XFSTK: Please connect only a single SoC device and cycle device power.\n");
        printf("XFSTK: Aborting hash verification process.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    //Bind all interfaces together
    if(!xfstkfactoryinterface->BindInterfaces()) {
        printf("XFSTK: Binding failed for Download, Device, and Options interfaces.\n");
        printf("XFSTK: Aborting hash verification process.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    //Execute the download
    xfstkfactoryinterface->retrycount = this->retrycount;
    if(!xfstkfactoryinterface->ExecuteDownloadSerial()) {
        printf("XFSTK: Download operation encountered errors.\n");
        printf("XFSTK: Please verify hash file integrity and reprovision target.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    printf( "\nXFSTK: Hash Verification Completed Successfully.\n");
    xfstkfactoryinterface->ClearAllLists();
    this->releaseinterface();
    return true;
}
void xfstkdldrapi::setcsdbResponsebuffer(unsigned char *responseBuffer, int maxsize)
{
    xfstkdldrfactory* xfstkfactoryinterface = (xfstkdldrfactory*)xfstkfactoryhandle;
    xfstkfactoryinterface->SetIdrqResponse(responseBuffer,maxsize);
}
bool xfstkdldrapi::downloadcsdb(char *fwdnx, char *miscbin, char *cmdcode,char *fwimage, bool )
{
    int tmpargc = 8;
    char* tmpargv[11];
    xfstksleep sleeper;
    boost::scoped_array<char> message (new char[1024]);
    xfstkdldrfactory* xfstkfactoryinterface = (xfstkdldrfactory*)xfstkfactoryhandle;
    this->showversion();
    //Print options
    sprintf(message.get(),"\nfwdnx -- %s\n", fwdnx);
    xfstklogmessage(message.get(),this->physclientdata);
    if(fwimage != NULL)
    {
        sprintf(message.get(),"\nfwimage -- %s\n", fwimage);
        xfstklogmessage(message.get(),this->physclientdata);
    }
    sprintf(message.get(),"\nmiscbin -- %s\n", miscbin);
    xfstklogmessage(message.get(),this->physclientdata);
    sprintf(message.get(),"\ncmdcode -- %s\n", cmdcode);
    xfstklogmessage(message.get(),this->physclientdata);
    boost::scoped_array<char> arg0(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg1(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg2(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg3(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg4(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg5(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg6(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg7(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg8(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg9(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg10(new char[XFSTK_PARAM_LENGTH]);
    if(!this->interfaceavailable()) {
        return false;
    }
    if(!this->claiminterface()) {
        return false;
    }
    sprintf(arg0.get(),"xfstk-dldr-api");
    sprintf(arg1.get(),"--fwdnx");
    sprintf(arg2.get(),"%s",fwdnx);
    sprintf(arg3.get(),"--miscbin");
    sprintf(arg4.get(),"%s",miscbin);
    sprintf(arg5.get(), "--csdb");
    sprintf(arg6.get(), "%s", cmdcode);
    sprintf(arg7.get(), "--verbose");
    tmpargv[0] = &arg0[0];
    tmpargv[1] = &arg1[0];
    tmpargv[2] = &arg2[0];
    tmpargv[3] = &arg3[0];
    tmpargv[4] = &arg4[0];
    tmpargv[5] = &arg5[0];
    tmpargv[6] = &arg6[0];
    tmpargv[7] = &arg7[0];
    if(fwimage != NULL)
    {
        sprintf(arg8.get(),"--fwimage");
        sprintf(arg9.get(),"%s",fwimage);
        tmpargv[8] = &arg8[0];
        tmpargv[9] = &arg9[0];
        tmpargc += 2;
    }
    // Detect the device type and create the downloader interfaces
    int devicedetected = 0;
    while(!xfstkfactoryinterface->EnumerateDevices()) {
        xfstklogmessage(const_cast<char*>("XFSTK-PROGRESS--0"),this->physclientdata);
        sprintf(message.get(),"XFSTK-STATUS--Detecting Intel Device - Attempt #%d",devicedetected);
        xfstklogmessage(message.get(),this->physclientdata);
        xfstklogmessage(const_cast<char*>("XFSTK-PROGRESS--5"),this->physclientdata);
        if(devicedetected > this->retrycount) {
            xfstklogmessage(const_cast<char* >("XFSTK-STATUS--Aborting download process."), const_cast<void* >(this->physclientdata));
            xfstklogmessage(const_cast<char* >("XFSTK-PROGRESS--100"), const_cast<void* >(this->physclientdata));
            xfstkfactoryinterface->ClearAllLists();
            return false;
        }
        devicedetected++;
        sleeper.sleep(1);
    }
    //Parse the options from the user
    if(!xfstkfactoryinterface->SetOptions(tmpargc,tmpargv)) {
        printf("XFSTK: Download options could not be parsed correctly.\n");
        printf("XFSTK: Please connect only a single SoC device and cycle device power.\n");
        printf("XFSTK: Aborting download process.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    //Bind all interfaces together
    if(!xfstkfactoryinterface->BindInterfaces()) {
        printf("XFSTK: Binding failed for Download, Device, and Options interfaces.\n");
        printf("XFSTK: Aborting download process.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    //Execute the download
    xfstkfactoryinterface->retrycount = this->retrycount;
    if(!xfstkfactoryinterface->ExecuteDownloadSerial()) {
        printf("XFSTK: Download operation encountered errors.\n");
        printf("XFSTK: Please verify fw/os image integrity and reprovision target.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    printf( "\nXFSTK: Transfer Completed Successfully.\n");
    if(physstatuspfn)
        physstatuspfn(const_cast<char* >("Success: Download of FW Completed."), const_cast<void* >(this->physclientdata));
    xfstkfactoryinterface->ClearAllLists();
    this->releaseinterface();
    return true;
}
bool xfstkdldrapi::downloadcli(const char *cli)
{
    int tmpargc = 10;
    char* tmpargv[MAX_ARGS];
    xfstksleep sleeper;
    boost::scoped_array<char> message (new char[1024]);
    xfstkdldrfactory* xfstkfactoryinterface = (xfstkdldrfactory*)xfstkfactoryhandle;
    this->showversion();
    if(!this->interfaceavailable())
    {
        return false;
    }
    if(!this->claiminterface())
    {
        return false;
    }
    // Detect the device type and create the downloader interfaces
    int devicedetected = 0;
    while(!xfstkfactoryinterface->EnumerateDevices()) {
        xfstklogmessage(const_cast<char*>("XFSTK-PROGRESS--0"),this->physclientdata);
        sprintf(message.get(),"XFSTK-STATUS--Detecting Intel Device - Attempt #%d",devicedetected);
        xfstklogmessage(message.get(),this->physclientdata);
        xfstklogmessage(const_cast<char*>("XFSTK-PROGRESS--5"),this->physclientdata);
        if(devicedetected > this->retrycount) {
            xfstklogmessage(const_cast<char* >("XFSTK-STATUS--Aborting download process."), const_cast<void* >(this->physclientdata));
            xfstklogmessage(const_cast<char* >("XFSTK-PROGRESS--100"), const_cast<void* >(this->physclientdata));
            xfstkfactoryinterface->ClearAllLists();
            return false;
        }
        devicedetected++;
        sleeper.sleep(1);
    }
    QString cliQstr(cli);
    QStringList args = cliQstr.split("--");
    tmpargc = args.size();
    int j = 0;
    for(int i = 0 ; i<tmpargc && j<MAX_ARGS; ++i)
    {
        QString localStr = args.at(i);
        //the first arg 'cli::' is not needed
        localStr.prepend("--");
        QStringList splitz = localStr.trimmed().split(" ");
        foreach(QString strz,splitz)
        {
            tmpargv[j] = new char[strz.size() + 1];
            memset(tmpargv[j],0,strz.size() + 1);
            strcpy(tmpargv[j],strz.trimmed().toStdString().c_str());
            j++;
        }
    }
    tmpargc = j;
    //Parse the options from the user
    bool retval = xfstkfactoryinterface->SetOptions(tmpargc,tmpargv);
    for(int i = 0; i < tmpargc; ++i)
    {
        if(tmpargv[i])
        {
            delete [] tmpargv[i];
            tmpargv[i] = NULL;
        }
    }
    if(!retval) {
        printf("XFSTK: Download options could not be parsed correctly.\n");
        printf("XFSTK: Please connect only a single SoC device and cycle device power.\n");
        printf("XFSTK: Aborting download process.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    //Bind all interfaces together
    if(!xfstkfactoryinterface->BindInterfaces()) {
        printf("XFSTK: Binding failed for Download, Device, and Options interfaces.\n");
        printf("XFSTK: Aborting download process.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    //Execute the download
    xfstkfactoryinterface->retrycount = this->retrycount;
    if(!xfstkfactoryinterface->ExecuteDownloadSerial()) {
        printf("XFSTK: Download operation encountered errors.\n");
        printf("XFSTK: Please verify fw/os image integrity and reprovision target.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    printf( "\nXFSTK: Transfer Completed Successfully.\n");
    if(physstatuspfn)
        physstatuspfn(const_cast<char* >("Success: Download of FW Completed."), const_cast<void* >(this->physclientdata));
    xfstkfactoryinterface->ClearAllLists();
    this->releaseinterface();
    return true;
}
bool xfstkdldrapi::downloadfw(char *fwdnx, char *fwimage, char *gpflags)
{
    int tmpargc = 10;
    char* tmpargv[13];
    xfstksleep sleeper;
    boost::scoped_array<char> message (new char[1024]);
    xfstkdldrfactory* xfstkfactoryinterface = (xfstkdldrfactory*)xfstkfactoryhandle;
    this->showversion();
    //Print options
    sprintf(message.get(),"\nfwdnx -- %s\n", fwdnx);
    xfstklogmessage(message.get(),this->physclientdata);
    sprintf(message.get(),"\nfwimage -- %s\n", fwimage);
    xfstklogmessage(message.get(),this->physclientdata);
    sprintf(message.get(),"\ngpflags -- %s\n", gpflags);
    xfstklogmessage(message.get(),this->physclientdata);
    boost::scoped_array<char> arg0(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg1(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg2(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg3(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg4(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg5(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg6(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg7(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg8(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg9(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg10(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg11(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg12(new char[XFSTK_PARAM_LENGTH]);
    if(!this->interfaceavailable()) {
        return false;
    }
    if(!this->claiminterface()) {
        return false;
    }
    //xfstk-dldr-solo --fwdnx /intel/fwdnx.bin --fwimage /intel/fwimage.bin
    sprintf(arg0.get(),"xfstk-dldr-api");
    sprintf(arg1.get(),"--fwdnx");
    sprintf(arg2.get(),"%s",fwdnx);
    sprintf(arg3.get(),"--fwimage");
    sprintf(arg4.get(),"%s",fwimage);
    sprintf(arg5.get(), "--gpflags");
    sprintf(arg6.get(), "%s", gpflags);
    sprintf(arg7.get(), "--debuglevel");
    QString str = QString::number(xfstkfactoryinterface->debuglevel, 16);
    sprintf(arg8.get(), "%s", str.toLatin1().data() );
    sprintf(arg9.get(), "--verbose");
    tmpargv[0] = &arg0[0];
    tmpargv[1] = &arg1[0];
    tmpargv[2] = &arg2[0];
    tmpargv[3] = &arg3[0];
    tmpargv[4] = &arg4[0];
    tmpargv[5] = &arg5[0];
    tmpargv[6] = &arg6[0];
    tmpargv[7] = &arg7[0];
    tmpargv[8] = &arg8[0];
    tmpargv[9] = &arg9[0];
    if(miscdnxenable) {
        sprintf(arg10.get(),"--miscdnx");
        sprintf(arg11.get(),"%s",this->miscdnx);
        tmpargv[10] = &arg10[0];
        tmpargv[11] = &arg11[0];
        tmpargc += 2;
    }
    if(xfstkfactoryinterface->wipeifwi)
    {
        sprintf(arg12.get(),"--wipeifwi");
        tmpargv[tmpargc] = &arg12[0];
        tmpargc++;
    }
    // Detect the device type and create the downloader interfaces
    int devicedetected = 0;
    while(!xfstkfactoryinterface->EnumerateDevices()) {
        xfstklogmessage(const_cast<char*>("XFSTK-PROGRESS--0"),this->physclientdata);
        sprintf(message.get(),"XFSTK-STATUS--Detecting Intel Device - Attempt #%d",devicedetected);
        xfstklogmessage(message.get(),this->physclientdata);
        xfstklogmessage(const_cast<char*>("XFSTK-PROGRESS--5"),this->physclientdata);
        if(devicedetected > this->retrycount) {
            xfstklogmessage(const_cast<char* >("XFSTK-STATUS--Aborting download process."), const_cast<void* >(this->physclientdata));
            xfstklogmessage(const_cast<char* >("XFSTK-PROGRESS--100"), const_cast<void* >(this->physclientdata));
            xfstkfactoryinterface->ClearAllLists();
            return false;
        }
        devicedetected++;
        sleeper.sleep(1);
    }
    //Parse the options from the user
    if(!xfstkfactoryinterface->SetOptions(tmpargc,tmpargv)) {
        printf("XFSTK: Download options could not be parsed correctly.\n");
        printf("XFSTK: Please connect only a single SoC device and cycle device power.\n");
        printf("XFSTK: Aborting download process.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    //Bind all interfaces together
    if(!xfstkfactoryinterface->BindInterfaces()) {
        printf("XFSTK: Binding failed for Download, Device, and Options interfaces.\n");
        printf("XFSTK: Aborting download process.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    //Execute the download
    xfstkfactoryinterface->retrycount = this->retrycount;
    if(!xfstkfactoryinterface->ExecuteDownloadSerial()) {
        printf("XFSTK: Download operation encountered errors.\n");
        printf("XFSTK: Please verify fw/os image integrity and reprovision target.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    printf( "\nXFSTK: Transfer Completed Successfully.\n");
    if(physstatuspfn)
        physstatuspfn(const_cast<char* >("Success: Download of FW Completed."), const_cast<void* >(this->physclientdata));
    xfstkfactoryinterface->ClearAllLists();
    this->releaseinterface();
    return true;
}
bool xfstkdldrapi::downloados(char *osdnx, char *osimage, char *gpflags)
{
    int tmpargc = 10;
    char* tmpargv[13];
    xfstksleep sleeper;
    boost::scoped_array<char> message (new char[1024]);
    xfstkdldrfactory* xfstkfactoryinterface = (xfstkdldrfactory*)xfstkfactoryhandle;
    this->showversion();
    //Print options
    sprintf(message.get(),"\nosdnx -- %s\n", osdnx ? osdnx : "");
    xfstklogmessage(message.get(),this->physclientdata);
    sprintf(message.get(),"osimage -- %s\n", osimage);
    xfstklogmessage(message.get(),this->physclientdata);
    sprintf(message.get(),"gpflags -- %s\n", gpflags);
    xfstklogmessage(message.get(),this->physclientdata);
    boost::scoped_array<char> arg0(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg1(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg2(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg3(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg4(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg5(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg6(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg7(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg8(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg9(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg10(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg11(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg12(new char[XFSTK_PARAM_LENGTH]);
    if(!this->interfaceavailable()) {
        return false;
    }
    if(!this->claiminterface()) {
        return false;
    }
    //xfstk-dldr-solo --fwdnx /intel/BLANK.bin --fwimage /intel/BLANK.bin --osdnx /intel/osdnx.bin --osimage /intel/osimage.bin --gpflags 80000001
    sprintf(arg0.get(),"xfstk-dldr-api");
    sprintf(arg1.get(), "--osdnx");
    sprintf(arg2.get(), "%s", osdnx ? osdnx : "");
    sprintf(arg3.get(), "--osimage");
    sprintf(arg4.get(), "%s", osimage);
    sprintf(arg5.get(), "--gpflags");
    sprintf(arg6.get(), "%s", gpflags);
    sprintf(arg7.get(), "--debuglevel");
    QString str = QString::number(xfstkfactoryinterface->debuglevel, 16);
    sprintf(arg8.get(), "%s", str.toLatin1().data() );
    sprintf(arg9.get(), "--verbose");
    tmpargv[0] = &arg0[0];
    tmpargv[1] = &arg1[0];
    tmpargv[2] = &arg2[0];
    tmpargv[3] = &arg3[0];
    tmpargv[4] = &arg4[0];
    tmpargv[5] = &arg5[0];
    tmpargv[6] = &arg6[0];
    tmpargv[7] = &arg7[0];
    tmpargv[8] = &arg8[0];
    tmpargv[9] = &arg9[0];
    if(miscdnxenable) {
        sprintf(arg10.get(),"--miscdnx");
        sprintf(arg11.get(),"%s",this->miscdnx);
        tmpargv[10] = &arg10[0];
        tmpargv[11] = &arg11[0];
        tmpargc+=2;
    }
    if(xfstkfactoryinterface->wipeifwi)
    {
        sprintf(arg12.get(),"--wipeifwi");
        tmpargv[tmpargc] = &arg12[0];
        tmpargc++;
    }
    unsigned int tmpGPFlag;
    sscanf(gpflags,"%x",&tmpGPFlag);
    // Detect the device type and create the downloader interfaces
    int devicedetected = 0;
    while(!xfstkfactoryinterface->EnumerateDevices()) {
        xfstklogmessage(const_cast<char*>("XFSTK-PROGRESS--0"),this->physclientdata);
        sprintf(message.get(),"XFSTK-STATUS--Detecting Intel Device - Attempt #%d",devicedetected);
        xfstklogmessage(message.get(),this->physclientdata);
        xfstklogmessage(const_cast<char*>("XFSTK-PROGRESS--5"),this->physclientdata);
        if(devicedetected > this->retrycount) {
            xfstklogmessage(const_cast<char* >("XFSTK-STATUS--Aborting download process."), const_cast<void* >(this->physclientdata));
            xfstklogmessage(const_cast<char* >("XFSTK-PROGRESS--100"), const_cast<void* >(this->physclientdata));
            xfstkfactoryinterface->ClearAllLists();
            return false;
        }
        devicedetected++;
        sleeper.sleep(1);
    }
    //Parse the options from the user
    if(!xfstkfactoryinterface->SetOptions(tmpargc,tmpargv)) {
        printf("XFSTK: Download options could not be parsed correctly.\n");
        printf("XFSTK: Please connect only a single SoC device and cycle device power.\n");
        printf("XFSTK: Aborting download process.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    //Bind all interfaces together
    if(!xfstkfactoryinterface->BindInterfaces()) {
        printf("XFSTK: Binding failed for Download, Device, and Options interfaces.\n");
        printf("XFSTK: Aborting download process.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    //Execute the download
    xfstkfactoryinterface->retrycount = this->retrycount;
    if(!xfstkfactoryinterface->ExecuteDownloadSerial()) {
        printf("XFSTK: Download operation encountered errors.\n");
        printf("XFSTK: Please verify fw/os image integrity and reprovision target.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    printf( "\nXFSTK: Transfer Completed Successfully.\n");
    if (tmpGPFlag & 0x00000001)
        xfstklogmessage(const_cast<char*>("Success: Download of OS Completed."), this->physclientdata);
    else
        xfstklogmessage(const_cast<char* >("Download of OS Skipped."),  const_cast<void* >(this->physclientdata));
    xfstkfactoryinterface->ClearAllLists();
    this->releaseinterface();
    return true;
}
bool xfstkdldrapi::downloadfwos(char *fwdnx, char *fwimage, char *osdnx, char* osimage, char* gpflags)
{
    int tmpargc = 14;
    char* tmpargv[17];
    boost::scoped_array<char> message (new char[1024]);
    xfstkdldrfactory* xfstkfactoryinterface = (xfstkdldrfactory*)xfstkfactoryhandle;
    this->showversion();
    //Print options
    sprintf(message.get(),"\nfwdnx -- %s\n", fwdnx);
    xfstklogmessage(message.get(),this->physclientdata);
    sprintf(message.get(),"\nfwimage -- %s\n", fwimage);
    xfstklogmessage(message.get(),this->physclientdata);
    sprintf(message.get(),"\nosdnx -- %s\n", osdnx ? osdnx : "");
    xfstklogmessage(message.get(),this->physclientdata);
    sprintf(message.get(),"osimage -- %s\n", osimage);
    xfstklogmessage(message.get(),this->physclientdata);
    sprintf(message.get(),"gpflags -- %s\n", gpflags);
    xfstklogmessage(message.get(),this->physclientdata);
    xfstksleep sleeper;
    boost::scoped_array<char> arg0(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg1(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg2(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg3(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg4(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg5(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg6(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg7(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg8(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg9(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg10(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg11(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg12(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg13(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg14(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg15(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg16(new char[XFSTK_PARAM_LENGTH]);
    if(!this->interfaceavailable()) {
        return false;
    }
    if(!this->claiminterface()) {
        return false;
    }
    //xfstk-dldr-solo --fwdnx /intel/fwdnx.bin --fwimage /intel/fwimage.bin --osdnx /intel/osdnx.bin --osimage /intel/osimage.bin --gpflags 80000001
    sprintf(arg0.get(),"xfstk-dldr-api");
    sprintf(arg1.get(),"--fwdnx");
    sprintf(arg2.get(),"%s",fwdnx);
    sprintf(arg3.get(),"--fwimage");
    sprintf(arg4.get(),"%s",fwimage);
    sprintf(arg5.get(), "--osdnx");
    sprintf(arg6.get(), "%s", osdnx ? osdnx : "");
    sprintf(arg7.get(), "--osimage");
    sprintf(arg8.get(), "%s", osimage);
    sprintf(arg9.get(), "--gpflags");
    sprintf(arg10.get(), "%s", gpflags);
    sprintf(arg11.get(), "--debuglevel");
    QString str = QString::number(xfstkfactoryinterface->debuglevel, 16);
    sprintf(arg12.get(), "%s", str.toLatin1().data() );
    sprintf(arg13.get(), "--verbose");
    tmpargv[0] = &arg0[0];
    tmpargv[1] = &arg1[0];
    tmpargv[2] = &arg2[0];
    tmpargv[3] = &arg3[0];
    tmpargv[4] = &arg4[0];
    tmpargv[5] = &arg5[0];
    tmpargv[6] = &arg6[0];
    tmpargv[7] = &arg7[0];
    tmpargv[8] = &arg8[0];
    tmpargv[9] = &arg9[0];
    tmpargv[10] = &arg10[0];
    tmpargv[11] = &arg11[0];
    tmpargv[12] = &arg12[0];
    tmpargv[13] = &arg13[0];
    if(miscdnxenable)
    {
        sprintf(arg14.get(),"--miscdnx");
        sprintf(arg15.get(),"%s",this->miscdnx);
        tmpargv[14] = &arg14[0];
        tmpargv[15] = &arg15[0];
        tmpargc+=2;
    }
    if(xfstkfactoryinterface->wipeifwi)
    {
        sprintf(arg16.get(),"--wipeifwi");
        tmpargv[tmpargc] = &arg16[0];
        tmpargc++;
    }
    // Detect the device type and create the downloader interfaces
    int devicedetected = 0;
    while(!xfstkfactoryinterface->EnumerateDevices()) {
        xfstklogmessage(const_cast<char*>("XFSTK-PROGRESS--0"),this->physclientdata);
        sprintf(message.get(),"XFSTK-STATUS--Detecting Intel Device - Attempt #%d",devicedetected);
        xfstklogmessage(message.get(),this->physclientdata);
        xfstklogmessage(const_cast<char*>("XFSTK-PROGRESS--5"),this->physclientdata);
        if(devicedetected > this->retrycount) {
            xfstklogmessage(const_cast<char* >("XFSTK-STATUS--Aborting download process."), const_cast<void* >(this->physclientdata));
            xfstklogmessage(const_cast<char* >("XFSTK-PROGRESS--100"), const_cast<void* >(this->physclientdata));
            xfstkfactoryinterface->ClearAllLists();
            return false;
        }
        devicedetected++;
        sleeper.sleep(1);
    }
    //Parse the options from the user
    if(!xfstkfactoryinterface->SetOptions(tmpargc,tmpargv)) {
        printf("XFSTK: Download options could not be parsed correctly.\n");
        printf("XFSTK: Please connect only a single SoC device and cycle device power.\n");
        printf("XFSTK: Aborting download process.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    //Bind all interfaces together
    if(!xfstkfactoryinterface->BindInterfaces()) {
        printf("XFSTK: Binding failed for Download, Device, and Options interfaces.\n");
        printf("XFSTK: Aborting download process.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    xfstklogmessage(const_cast<char* >("Initiating download..."),  const_cast<void* >(this->physclientdata));
    //Execute the download
    xfstkfactoryinterface->retrycount = this->retrycount;
    if(!xfstkfactoryinterface->ExecuteDownloadSerial()) {
        printf("XFSTK: Download operation encountered errors.\n");
        printf("XFSTK: Please verify fw/os image integrity and reprovision target.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    printf( "\nXFSTK: Transfer Completed Successfully.\n");
    if ((gpflags[9]==0x31)||(gpflags[9]==0x35))
        xfstklogmessage(const_cast<char* >("Success: Download of FW and OS Completed."),  const_cast<void* >(this->physclientdata));
    else
        xfstklogmessage(const_cast<char* >("Success: Download of FW Completed."),  const_cast<void* >(this->physclientdata));
    xfstkfactoryinterface->ClearAllLists();
    this->releaseinterface();
    return true;
}
bool xfstkdldrapi::downloadfwos(char *fwdnx, char *fwimage, char *osdnx, char* osimage, char* gpflags, char *softfuse)
{
    int tmpargc = 16;
    char* tmpargv[19];
    boost::scoped_array<char> message (new char[1024]);
    xfstkdldrfactory* xfstkfactoryinterface = (xfstkdldrfactory*)xfstkfactoryhandle;
    this->showversion();
    //Print options
    sprintf(message.get(),"\nfwdnx -- %s\n", fwdnx);
    xfstklogmessage(message.get(),this->physclientdata);
    sprintf(message.get(),"\nfwimage -- %s\n", fwimage);
    xfstklogmessage(message.get(),this->physclientdata);
    sprintf(message.get(),"\nosdnx -- %s\n", osdnx ? osdnx : "");
    xfstklogmessage(message.get(),this->physclientdata);
    sprintf(message.get(),"osimage -- %s\n", osimage);
    xfstklogmessage(message.get(),this->physclientdata);
    sprintf(message.get(),"gpflags -- %s\n", gpflags);
    xfstklogmessage(message.get(),this->physclientdata);
    sprintf(message.get(),"softfuse -- %s\n", softfuse);
    xfstklogmessage(message.get(),this->physclientdata);
    xfstksleep sleeper;
    boost::scoped_array<char> arg0(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg1(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg2(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg3(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg4(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg5(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg6(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg7(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg8(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg9(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg10(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg11(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg12(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg13(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg14(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg15(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg16(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg17(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg18(new char[XFSTK_PARAM_LENGTH]);
    if(!this->interfaceavailable()) {
        return false;
    }
    if(!this->claiminterface()) {
        return false;
    }
    //xfstk-dldr-solo --fwdnx /intel/fwdnx.bin --fwimage /intel/fwimage.bin --osdnx /intel/osdnx.bin --osimage /intel/osimage.bin --gpflags 80000001
    sprintf(arg0.get(),"xfstk-dldr-api");
    sprintf(arg1.get(),"--fwdnx");
    sprintf(arg2.get(),"%s",fwdnx);
    sprintf(arg3.get(),"--fwimage");
    sprintf(arg4.get(),"%s",fwimage);
    sprintf(arg5.get(), "--osdnx");
    sprintf(arg6.get(), "%s", osdnx ? osdnx : "");
    sprintf(arg7.get(), "--osimage");
    sprintf(arg8.get(), "%s", osimage);
    sprintf(arg9.get(), "--gpflags");
    sprintf(arg10.get(), "%s", gpflags);
    sprintf(arg11.get(), "--debuglevel");
    QString str = QString::number(xfstkfactoryinterface->debuglevel, 16);
    sprintf(arg12.get(), "%s", str.toLatin1().data() );
    sprintf(arg13.get(),"--softfuse");
    sprintf(arg14.get(), "%s", softfuse);
    sprintf(arg15.get(), "--verbose");
    tmpargv[0] = &arg0[0];
    tmpargv[1] = &arg1[0];
    tmpargv[2] = &arg2[0];
    tmpargv[3] = &arg3[0];
    tmpargv[4] = &arg4[0];
    tmpargv[5] = &arg5[0];
    tmpargv[6] = &arg6[0];
    tmpargv[7] = &arg7[0];
    tmpargv[8] = &arg8[0];
    tmpargv[9] = &arg9[0];
    tmpargv[10] = &arg10[0];
    tmpargv[11] = &arg11[0];
    tmpargv[12] = &arg12[0];
    tmpargv[13] = &arg13[0];
    tmpargv[14] = &arg14[0];
    tmpargv[15] = &arg15[0];
    if(miscdnxenable)
    {
        sprintf(arg16.get(),"--miscdnx");
        sprintf(arg17.get(),"%s",this->miscdnx);
        tmpargv[16] = &arg16[0];
        tmpargv[17] = &arg17[0];
        tmpargc +=2;
    }
    if(xfstkfactoryinterface->wipeifwi)
    {
        sprintf(arg18.get(),"--wipeifwi");
        tmpargv[tmpargc] = &arg18[0];
        tmpargc++;
    }
    // Detect the device type and create the downloader interfaces
    int devicedetected = 0;
    while(!xfstkfactoryinterface->EnumerateDevices()) {
        xfstklogmessage(const_cast<char*>("XFSTK-PROGRESS--0"),this->physclientdata);
        sprintf(message.get(),"\nXFSTK-STATUS--Detecting Intel Device - Attempt #%d\n",devicedetected);
        xfstklogmessage(message.get(),this->physclientdata);
        xfstklogmessage(const_cast<char*>("XFSTK-PROGRESS--5"),this->physclientdata);
        if(devicedetected > this->retrycount) {
            xfstklogmessage(const_cast<char* >("XFSTK-STATUS--Aborting download process."), const_cast<void* >(this->physclientdata));
            xfstklogmessage(const_cast<char* >("XFSTK-PROGRESS--100"), const_cast<void* >(this->physclientdata));
            xfstkfactoryinterface->ClearAllLists();
            return false;
        }
        devicedetected++;
        sleeper.sleep(1);
    }
    //Parse the options from the user
    if(!xfstkfactoryinterface->SetOptions(tmpargc,tmpargv)) {
        printf("XFSTK: Download options could not be parsed correctly.\n");
        printf("XFSTK: Please connect only a single SoC device and cycle device power.\n");
        printf("XFSTK: Aborting download process.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    //Bind all interfaces together
    if(!xfstkfactoryinterface->BindInterfaces()) {
        printf("XFSTK: Binding failed for Download, Device, and Options interfaces.\n");
        printf("XFSTK: Aborting download process.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    xfstklogmessage(const_cast<char* >("Initiating download..."), const_cast<void* >(this->physclientdata));
    //Execute the download
    xfstkfactoryinterface->retrycount = this->retrycount;
    if(!xfstkfactoryinterface->ExecuteDownloadSerial()) {
        printf("XFSTK: Download operation encountered errors.\n");
        printf("XFSTK: Please verify fw/os image integrity and reprovision target.\n");
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    printf( "\nXFSTK: Transfer Completed Successfully.\n");
    if ((gpflags[9]==0x31)||(gpflags[9]==0x35))
        xfstklogmessage(const_cast<char* >("Success: Download of FW and OS Completed."), const_cast<void* >(this->physclientdata));
    else
        xfstklogmessage(const_cast<char* >("Success: Download of FW Completed."), const_cast<void* >(this->physclientdata));
    xfstkfactoryinterface->ClearAllLists();
    this->releaseinterface();
    return true;
}
bool xfstkdldrapi::downloadmtfwosasync(char *fwdnx, char *fwimage, char *osdnx, char* osimage, char* gpflags, char* usbsn)
{
    bool passfail = false;
    boost::scoped_array<char> message (new char[1024]);
    LastError er;
    xfstksleep sleeper;
    xfstkdldrfactory* xfstkfactoryinterface = (xfstkdldrfactory*)xfstkfactoryhandle;
    this->showversion();
    //Print options
    sprintf(message.get(),"\nfwdnx -- %s\n", fwdnx);
    xfstklogmessage(message.get(),this->physclientdata);
    sprintf(message.get(),"\nfwimage -- %s\n", fwimage);
    xfstklogmessage(message.get(),this->physclientdata);
    sprintf(message.get(),"\nosdnx -- %s\n", osdnx ? osdnx : "");
    xfstklogmessage(message.get(),this->physclientdata);
    sprintf(message.get(),"osimage -- %s\n", osimage);
    xfstklogmessage(message.get(),this->physclientdata);
    if(this->miscdnxenable)
    {
        sprintf(message.get(),"miscdnx -- %s\n", this->miscdnx);
        xfstklogmessage(message.get(),this->physclientdata);
    }
    sprintf(message.get(),"gpflags -- %s\n", gpflags);
    xfstklogmessage(message.get(),this->physclientdata);
    sprintf(message.get(),"usbsn -- %s\n", usbsn);
    xfstklogmessage(message.get(),this->physclientdata);
    if(!this->interfaceavailable()) {
        sprintf(message.get(),"\nUSBSN: %s -- Interface is not available...\n", usbsn);
        xfstklogmessage(message.get(),this->physclientdata);
        return false;
    }
    if(!this->claiminterface()) {
        sprintf(message.get(),"\nUSBSN: %s -- Cann't claim interface...\n", usbsn);
        xfstklogmessage(message.get(),this->physclientdata);
        return false;
    }
    if(physstatuspfn != NULL) {
        sprintf(message.get(),"\nUSBSN: %s -- Initiating download...\n", usbsn);
        xfstklogmessage(message.get(),this->physclientdata);
    }
    //Execute the download
    passfail = xfstkfactoryinterface->ExecuteDownloadSingleAsync(fwdnx, fwimage, osdnx, osimage, gpflags, usbsn, retrycount);
    xfstkfactoryinterface->GetLastError(&er);
    if((passfail==false) || (er.error_code != 0)) {
        sprintf(message.get(),"\nUSBSN: %s -- Download operation encountered errors.\n", usbsn);
        xfstklogmessage(message.get(),this->physclientdata);
        sprintf(message.get(),"\nUSBSN: %s -- Please verify fw/os image integrity and reprovision target.\n", usbsn);
        xfstklogmessage(message.get(),this->physclientdata);
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    printf( "\nXFSTK: Transfer Completed Successfully.\n");
    sprintf(message.get(),"\nUSBSN: %s -- Success: Transfer Completed Successfully\n", usbsn);
    xfstklogmessage(message.get(),this->physclientdata);
    xfstkfactoryinterface->ClearAllLists();
    this->releaseinterface();
    return true;
}
bool xfstkdldrapi::downloadmtfwosasync(char *fwdnx, char *fwimage, char *osdnx, char* osimage, char* gpflags, char* usbsn, char* softfuse)
{
    bool passfail = false;
    boost::scoped_array<char> message (new char[1024]);
    LastError er;
    xfstksleep sleeper;
    xfstkdldrfactory* xfstkfactoryinterface = (xfstkdldrfactory*)xfstkfactoryhandle;
    this->showversion();
    //Print options
    sprintf(message.get(),"\nfwdnx -- %s\n", fwdnx);
    xfstklogmessage(message.get(),this->physclientdata);
    sprintf(message.get(),"\nfwimage -- %s\n", fwimage);
    xfstklogmessage(message.get(),this->physclientdata);
    sprintf(message.get(),"\nosdnx -- %s\n", osdnx ? osdnx : "");
    xfstklogmessage(message.get(),this->physclientdata);
    sprintf(message.get(),"osimage -- %s\n", osimage);
    if(this->miscdnxenable)
    {
        sprintf(message.get(),"miscdnx -- %s\n", this->miscdnx);
        xfstklogmessage(message.get(),this->physclientdata);
    }
    xfstklogmessage(message.get(),this->physclientdata);
    sprintf(message.get(),"gpflags -- %s\n", gpflags);
    xfstklogmessage(message.get(),this->physclientdata);
    sprintf(message.get(),"usbsn -- %s\n", usbsn);
    xfstklogmessage(message.get(),this->physclientdata);
    sprintf(message.get(),"softfuse -- %s\n", softfuse);
    xfstklogmessage(message.get(),this->physclientdata);
    if(!this->interfaceavailable()) {
        sprintf(message.get(),"\nUSBSN: %s -- Interface is not available...\n", usbsn);
        xfstklogmessage(message.get(),this->physclientdata);
        return false;
    }
    if(!this->claiminterface()) {
        sprintf(message.get(),"\nUSBSN: %s -- Cann't claim interface...\n", usbsn);
        xfstklogmessage(message.get(),this->physclientdata);
        return false;
    }
    if(physstatuspfn != NULL) {
        sprintf(message.get(),"\nUSBSN: %s -- Initiating download...\n", usbsn);
        xfstklogmessage(message.get(),this->physclientdata);
    }
    //Execute the download
    passfail = xfstkfactoryinterface->ExecuteDownloadSingleAsync(fwdnx, fwimage, osdnx, osimage, gpflags, usbsn, softfuse, retrycount);
    xfstkfactoryinterface->GetLastError(&er);
    if((passfail==false) || (er.error_code != 0)) {
        sprintf(message.get(),"\nUSBSN: %s -- Download operation encountered errors.\n", usbsn);
        xfstklogmessage(message.get(),this->physclientdata);
        sprintf(message.get(),"\nUSBSN: %s -- Please verify fw/os image integrity and reprovision target.\n", usbsn);
        xfstklogmessage(message.get(),this->physclientdata);
        xfstkfactoryinterface->ClearAllLists();
        return false;
    }
    printf( "\nXFSTK: Transfer Completed Successfully.\n");
    sprintf(message.get(),"\nUSBSN: %s -- Success: Transfer Completed Successfully\n", usbsn);
    xfstklogmessage(message.get(),this->physclientdata);
    xfstkfactoryinterface->ClearAllLists();
    this->releaseinterface();
    return true;
}
bool xfstkdldrapi::registerstatuscallback(xfstkstatuspfn statuspfn, void *clientdata)
{
    xfstkdldrfactory* xfstkfactoryinterface = (xfstkdldrfactory*)xfstkfactoryhandle;
    if(!this->interfaceavailable()) {
        return false;
    }
    if(!this->claiminterface()) {
        return false;
    }
	if(statuspfn != NULL) {
            physstatuspfn = (XfstkStatusPfn)statuspfn;
            physclientdata = clientdata;
            xfstkfactoryinterface->SetStatusCallback(statuspfn, physclientdata);
	}
    this->releaseinterface();
    return true;
}
int xfstkdldrapi::getavailabletargets()
{
	QMutexLocker locker(&apiusbdevicemutex);
    xfstkdldrfactory* xfstkfactoryinterface = (xfstkdldrfactory*)xfstkfactoryhandle;
    int devicedetected = xfstkfactoryinterface->ScanForDevices();
    return devicedetected;
}
int xfstkdldrapi::getavailabletargets(unsigned long devicetype)
{
        QMutexLocker locker(&apiusbdevicemutex);
    xfstkdldrfactory* xfstkfactoryinterface = (xfstkdldrfactory*)xfstkfactoryhandle;
    int devicedetected = xfstkfactoryinterface->ScanForDevices(devicetype);
    return devicedetected;
}
int xfstkdldrapi::getavailabletargets(SoCDevices * socdevices)
{
    QMutexLocker locker(&apiusbdevicemutex);
    xfstkdldrfactory* xfstkfactoryinterface = (xfstkdldrfactory*)xfstkfactoryhandle;
    int devicedetected = xfstkfactoryinterface->GetNumberOfAttachedDevices(socdevices);
    return devicedetected;
}
bool xfstkdldrapi::interfaceavailable()
{
    //Since only one device will can into one API instance this is not required
    return binterfaceavailable;
}
bool xfstkdldrapi::claiminterface()
{
    //Since only one device will can into one API instance this is not required
    if(binterfaceavailable) {
        binterfaceavailable = false;
        binterfaceclaimed = true;
        return true;
    }
    return false;
}
void xfstkdldrapi::releaseinterface()
{
    binterfaceclaimed = false;
    binterfaceavailable = true;
    return;
}
void xfstkdldrapi::usbdevicemutexlock()
{
    apiusbdevicemutex.lock();
    return;
}
void xfstkdldrapi::usbdevicemutexunlock()
{
    apiusbdevicemutex.unlock();
    return;
}
void xfstkdldrapi::getversion(char* version)
{
    memcpy(version, DOWNLOADER_VERSION, 5);
    return;
}
string xfstkdldrapi::getfullversion(void) const
{
    return std::string(DOWNLOADER_VERSION);
}
bool xfstkdldrapi::getlasterror(LastError* er)
{
    xfstkdldrfactory* xfstkfactoryinterface = (xfstkdldrfactory*)xfstkfactoryhandle;
	return xfstkfactoryinterface->GetLastError(er);
}
void xfstkdldrapi::showversion(void)
{
    char* message = new char[256];
    memset(message,0,256);
    sprintf(message,"\nXFSTK Downloader API %s \nCopyright (c) 2014 Intel Corporation\n", DOWNLOADER_VERSION);
    xfstklogmessage(message,this->physclientdata);
    delete[] message;
}
//Exported C funcs for dynamic loading at client side
xfstkdldrapi* xfstkdldrapi_alloc()
{
    return new xfstkdldrapi;
}
void xfstkdldrapi_dealloc(xfstkdldrapi* obj)
{
    if (obj != NULL)
    {
        delete obj;
        obj = NULL;
    }
}
