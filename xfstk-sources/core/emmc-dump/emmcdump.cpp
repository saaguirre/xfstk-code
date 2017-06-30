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
#include "emmcdump.h"
#if defined XFSTK_OS_WIN
#include <windows.h>
#include <wddevicefactory.h>
#endif

EmmcDump::EmmcDump()
{
    //Initialize all member variables an create a factory instance

    this->enumerateddevicetype = XFSTK_NODEVICE;
    this->xfstkfactory = NULL;
    this->xfstkdevice = NULL;
    this->xfstkoptions = NULL;
    this->xfstkdownloader = NULL;
    this->xfstkfactory = new XfstkFactory();
}
EmmcDump::~EmmcDump()
{
    //Cleanup all interfaces

    if(this->xfstkfactory) {
        delete this->xfstkfactory;
        this->xfstkfactory = NULL;
    }
    if(this->xfstkdevice) {
        delete this->xfstkdevice;
        this->xfstkdevice = NULL;
    }
    if(this->xfstkoptions) {
        delete this->xfstkoptions;
        this->xfstkoptions = NULL;
    }
    if(this->xfstkdownloader) {
        delete this->xfstkdownloader;
        this->xfstkdownloader = NULL;
    }
}
bool EmmcDump::SetStatusCallback(XfstkStatusPfn StatusPfn, void *ClientData)
{
    if(this->xfstkdevice != NULL && this->xfstkdownloader != NULL && this->xfstkoptions != NULL) {
        return this->xfstkdownloader->SetStatusCallback(StatusPfn,ClientData);
    }
    return false;
}

bool EmmcDump::ExecuteDownload()
{
    bool RetVal = true;
    int counter = 0;

    //Before we attempt to download ensure that all interfaces are available to complete the task
    if(this->xfstkdevice != NULL && this->xfstkdownloader != NULL && this->xfstkoptions != NULL) {

        //Bind options to the downloader
        if(!this->xfstkdownloader->SetOptions(this->xfstkoptions)) {
            RetVal = false;
        }
        //Bind device to the downloader
        if(!this->xfstkdownloader->SetDevice(this->xfstkdevice)) {
            RetVal = false;
        }
        //Begin the update process and loop until complete
        while(!this->xfstkdownloader->UpdateTarget()) {

#if defined XFSTK_OS_WIN
                Sleep(5000);
#else
                sleep(5);
#endif

            void *tmphandle = NULL;
            while(tmphandle == NULL) {
                tmphandle = NULL;
                tmphandle = this->xfstkdevice->GetDeviceHandle(0);
                counter++;
#if defined XFSTK_OS_WIN
                Sleep(1000);
#else
                sleep(1);
#endif
                printf("XFSTK-STATUS--Reconnecting to device - Attempt #%d\n",counter);
                if(counter > 20) {
                    return false;
                }
            }
            this->xfstkdevice->SetDeviceHandle(tmphandle);
        }

        //Cleanup after the download has finished
        if(!this->xfstkdownloader->Cleanup()) {
            RetVal = false;
        }
    }
    return RetVal;
}
bool EmmcDump::CreateAllRequiredInterfaces(unsigned long devicetype)
{
    //If zero devices enumerated exit without creating any interfaces.
    if(this->enumerateddevicetype == XFSTK_NODEVICE) {
        return false;
    }

    //Create a download options interface
    this->xfstkoptions = NULL;
    this->xfstkoptions = this->xfstkfactory->CreateDownloaderOptions(devicetype);
    if(this->xfstkoptions == NULL) {
        return false;
    }

    //Create a device interface to communicate with the SoC
    this->xfstkdevice = NULL;
    this->xfstkdevice = this->xfstkfactory->CreateDevice(devicetype);
    if(this->xfstkdevice == NULL) {
        return false;
    }

    //Configure the device interafce with transport type and physical device instance (use first device "0")
    this->xfstkdevice->SetTransport(XFSTK_USB20);
    void *tmphandle = this->xfstkdevice->GetDeviceHandle(0);
    this->xfstkdevice->SetDeviceHandle(tmphandle);

    //Create the downloader interface for the current device type
    this->xfstkdownloader = NULL;
    this->xfstkdownloader = this->xfstkfactory->CreateDownloader(devicetype);
    if(this->xfstkdownloader == NULL) {
        return false;
    }

    return true;
}

bool EmmcDump::ParseCommandline(int argc, char* argv[])
{
    //Parse the commandline from the user
    bool retval = false;
    if(this->xfstkoptions != NULL) {
        this->xfstkoptions->Parse(argc,argv);
        return true;
    }
    else {
        IOptions *tempxfstkoptions = NULL;
        tempxfstkoptions = this->xfstkfactory->CreateDownloaderOptions(EMMC_DUMP);
        if(tempxfstkoptions) {
            tempxfstkoptions->Parse(argc,argv);
            retval = tempxfstkoptions->IsActionRequired();
            delete tempxfstkoptions;
            tempxfstkoptions = NULL;
        }
    }
    return retval;
}

bool EmmcDump::EnumerateDevice()
{

    int totaldevices = 0;
    unsigned long deveicetype = XFSTK_NODEVICE;

    //Enumerate for any attached device.  This enumeration is looking for the first available device.

    for(int i = 0; i < XFSTK_MAXDEVICE; i++) {
        switch(i) {
        this->xfstkdevice = NULL;
        case MOORESTOWN:
            deveicetype = MOORESTOWN;
            break;
        case MEDFIELD:
            deveicetype = MEDFIELD;
            break;
        case CLOVERVIEW:
            deveicetype = CLOVERVIEW;
            break;
        case CLOVERVIEWPLUS:
            deveicetype = CLOVERVIEWPLUS;
            break;
        case MERRIFIELD:
            deveicetype = MERRIFIELD;
            break;
        case MOOREFIELD:
            deveicetype = MOOREFIELD;
            break;
        default:
            deveicetype = XFSTK_NODEVICE;
            break;
        }

        //Attempt to create a device for each possible target type.

        this->xfstkdevice = this->xfstkfactory->CreateDevice(deveicetype);
        if(this->xfstkdevice) {


            if(this->xfstkdevice->SetTransport(this->xfstkoptions->GetTransportType())) {
                totaldevices = totaldevices + this->xfstkdevice->GetNumberAvailableDevices();
            }
            delete this->xfstkdevice;
            this->xfstkdevice = NULL;

            //If we found a device make a note of it an create all required interfaces needed to provision
            if(totaldevices > 0) {
                this->enumerateddevicetype = deveicetype;
                this->CreateAllRequiredInterfaces(enumerateddevicetype);
                return true;
            }
        }
    }
    return false;
}
