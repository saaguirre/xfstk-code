/*
    Copyright (C) 2015  Intel Corporation

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
#include "xfstkdldrfactory.h"
#include "xfstkdldrthread.h"
#include <boost/smart_ptr/scoped_array.hpp>
#include <cstdio>

xfstkdldrthreadobj::xfstkdldrthreadobj(QObject *parent) :
    QObject(parent)
{
    this->isDebug = 0;
    this->time_elapsed = 0;
    this->retrycount = 40;
    this->fwisdone = false;
    this->osisdone = false;
    this->miscdnxenable = false;
    this->downloadfailed = false;
    memset(this->usbsn,0,sizeof(this->usbsn));
    memset(this->miscdnx,0,sizeof(this->miscdnx));
    memset(this->lastError.error_message,0,sizeof(this->lastError.error_message));

	this->fwdnx = NULL;
    this->fwimage = NULL;
    this->osdnx = NULL;
    this->osimage = NULL;
    this->gpflags = NULL;
    this->handle = NULL;
    this->softfuse = NULL;
}

void xfstkdldrthreadobj::configuredownloader(char *fwdnx, char *fwimage, char *osdnx, char* osimage, char* gpflags, char* usbsn, int numtargets)
{
    this->fwdnx = fwdnx;
    this->fwimage = fwimage;
    this->osdnx = osdnx;
    this->osimage = osimage;
    this->gpflags = gpflags;
    this->numtargets = numtargets;
    this->bFwOnly = false;
    this->bFwOs = false;
    this->bOsOnly = false;
    if(usbsn)
        memcpy(this->usbsn, usbsn, sizeof(usbsn));
}

void xfstkdldrthreadobj::configuredownloader(char *fwdnx, char *fwimage, char *osdnx, char* osimage, char* gpflags, char* softfuse, char* usbsn, int numtargets)
{
    this->fwdnx = fwdnx;
    this->fwimage = fwimage;
    this->osdnx = osdnx;
    this->osimage = osimage;
    this->gpflags = gpflags;
    this->softfuse = softfuse;
    this->numtargets = numtargets;
    this->bFwOnly = false;
    this->bFwOs = false;
    this->bOsOnly = false;
    if(usbsn)
        memcpy(this->usbsn, usbsn, sizeof(usbsn));
}

bool xfstkdldrthreadobj::downloadmtfwosthread()
{
    IFactory * xfstkdldrfactory = NULL;
    IGenericDevice *xfstkdevice = NULL;
    IOptions *xfstkoptions = NULL;
    IDownloader *xfstkdownloader = NULL;

    int tmpargc = 18;
    char* tmpargv[21];
    bool passfail = false;
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
    boost::scoped_array<char> arg19(new char[XFSTK_PARAM_LENGTH]);
    boost::scoped_array<char> arg20(new char[XFSTK_PARAM_LENGTH]);
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
	QString str = QString::number(this->debuglevel, 16);
    sprintf(arg12.get(), "%s", str.toLatin1().data());
    sprintf(arg13.get(), "--usbdelayms");
    str = QString::number(this->usbdelayms, 10);
    sprintf(arg14.get(), "%s", str.toLatin1().data());
    sprintf(arg15.get(),"--verbose");
    sprintf(arg16.get(),"--softfuse");
    sprintf(arg17.get(),"%s",softfuse);

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
    tmpargv[16] = &arg16[0];
    tmpargv[17] = &arg17[0];
    if(miscdnxenable)
    {
        sprintf(arg18.get(),"--miscdnx");
        sprintf(arg19.get(),"%s",this->miscdnx);
        tmpargv[tmpargc++] = &arg18[0];
        tmpargv[tmpargc++] = &arg19[0];
    }

    if(this->wipeifwi)
    {
        sprintf(arg20.get(), "--wipeifwi");
        tmpargv[tmpargc++] = &arg20[0];
    }


    //Create a device interface to communicate with the SoC
    xfstkdldrfactory = new XfstkFactory();
    xfstkdevice = NULL;

	if( xfstkdldrfactory )
		xfstkdevice = xfstkdldrfactory->CreateDevice(devicetype);
    if(xfstkdevice == NULL) {
        passfail = false;
        goto cleanup;
    }
    else {


        //Configure the device interafce with transport type and physical device instance
        xfstkdevice->SetTransport(XFSTK_USB20);
        xfstkdevice->SetDeviceHandle(handle);

        //Create a download options interface
        xfstkoptions = NULL;
        xfstkoptions = xfstkdldrfactory->CreateDownloaderOptions(devicetype);
        if(xfstkoptions == NULL) {
            passfail = false;
            goto cleanup;
        } else {
            xfstkoptions->Parse(tmpargc, tmpargv);
            if(!xfstkoptions->IsActionRequired()) {
                passfail = false;
                goto cleanup;
            }
        }

        //Create the downloader interface for the current device type
        xfstkdownloader = NULL;
        xfstkdownloader = xfstkdldrfactory->CreateDownloader(devicetype);
        if(xfstkdownloader == NULL) {
            passfail = false;
            goto cleanup;
        }
        else {
            xfstkdownloader->SetStatusCallback(this->physstatuspfn, this->physclientdata);
            //Get the options interface and bind it to the downloader
            if(xfstkoptions) {
                if(!xfstkdownloader->SetOptions(xfstkoptions)) {
                    passfail = false;
                    goto cleanup;
                }
            }
            else {
                passfail = false;
                goto cleanup;
            }

            //Get the device and bind it to the downloader
            if(xfstkdevice) {
                if(!xfstkdownloader->SetDevice(xfstkdevice))
                {
                    passfail = false;
                    goto cleanup;
                }
                int timeout = xfstkoptions->GetReadWriteTimeout();
                xfstkdevice->SetUsbTimeoutDelay(timeout);
            }
        }
    }
    passfail = xfstkdownloader->UpdateTarget();
    //Cleanup after the download has finished
    //Set the right status for the thread
    if(passfail) {
        sleeper.sleep(1);
        this->osisdone = true;
    } else {
        if(xfstkdownloader->GetStatus()) {
            //Wait at least 10 seconds before the device is alive again
            sleeper.sleep(10);
            this->fwisdone = true;

        } else {
            this->downloadfailed =true;
        }
    }

    //Get Last error

    xfstkdownloader->GetLastError(&this->tmpError);
    this->lastError.error_code = this->tmpError.error_code;
    memcpy(this->lastError.error_message,this->tmpError.error_message,strlen(this->tmpError.error_message));

cleanup:

    if(xfstkoptions)
    {
        delete xfstkoptions;
        xfstkoptions = NULL;
    }
    if(xfstkdevice)
    {
        delete xfstkdevice;
        xfstkdevice = NULL;
    }
    if(xfstkdownloader)
    {
        delete xfstkdownloader;
        xfstkdownloader = NULL;
    }
    if(xfstkdldrfactory)
    {
        delete xfstkdldrfactory;
        xfstkdldrfactory = NULL;
    }

    return passfail;
}

void xfstkdldrthreadobj::go()
{
    this->downloadmtfwosthread();
}

xfstkdldrthread::xfstkdldrthread(QObject *parent) :
    QThread(parent)
{
}
xfstkdldrthread::~xfstkdldrthread()
{
    wait();
}

void xfstkdldrthread::setobj(xfstkdldrthreadobj *obj)
{
    this->obj = obj;
}
void xfstkdldrthread::run()
{
    this->obj->go();
    this->done();
    exit();
}
