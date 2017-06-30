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
#include "xfstkdownloader.h"

#if defined XFSTK_OS_WIN
	#include <windows.h>
    #include "interfaces/idevice.h"
#else
#if 0
#include <libusb.h>
#endif
#endif


XfstkDownloader::XfstkDownloader()
{
    //Initialize all member variables an create a factory instance

    this->enumerateddevicetype = XFSTK_NODEVICE;
    this->xfstkfactory = NULL;
    this->xfstkdevice = NULL;
    this->xfstkoptions = NULL;
    this->xfstkdownloader = NULL;
    this->expandedOptions = 0;
    this->xfstkfactory = new XfstkFactory();
}
XfstkDownloader::~XfstkDownloader()
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
bool XfstkDownloader::SetStatusCallback(XfstkStatusPfn StatusPfn, void *ClientData)
{
    if(this->xfstkdevice != NULL && this->xfstkdownloader != NULL && this->xfstkoptions != NULL) {
        return this->xfstkdownloader->SetStatusCallback(StatusPfn,ClientData);
    }
    return false;
}

unsigned long XfstkDownloader::GetEnumDeviceType()
{
    return enumerateddevicetype;
}

bool XfstkDownloader::ExecuteDownload()
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
            last_error er;
            this->xfstkdownloader->GetLastError(&er);
            //If not successful or not connection error exit
            if(er.error_code > 1)
                return false;

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
bool XfstkDownloader::CreateAllRequiredInterfaces(unsigned long devicetype)
{
    //If zero devices enumerated exit without creating any interfaces.
    if(this->enumerateddevicetype == XFSTK_NODEVICE) {
        return false;
    }
    //Create a device interface to communicate with the SoC
    this->xfstkdevice = NULL;
    this->xfstkdevice = this->xfstkfactory->CreateDevice(devicetype);
    if(this->xfstkdevice == NULL) {
        return false;
    }

    //Configure the device interafce with transport type and physical device instance (use first device "0")
    this->xfstkdevice->SetTransport(this->xfstkoptions->GetTransportType());
    void *tmphandle = this->xfstkdevice->GetDeviceHandle(0);
    this->xfstkdevice->SetDeviceHandle(tmphandle);


    //Create a download options interface
    this->xfstkoptions = NULL;
    if(this->expandedOptions !=0)
    {
        this->xfstkoptions = this->xfstkfactory->CreateDownloaderOptions(this->expandedOptions);
    }
    else
    {
    this->xfstkoptions = this->xfstkfactory->CreateDownloaderOptions(devicetype);
    }
    if(this->xfstkoptions == NULL) {
        return false;
    }


    //Create the downloader interface for the current device type
    this->xfstkdownloader = NULL;
    if(this->expandedOptions!=0)
    {
        this->xfstkdownloader = this->xfstkfactory->CreateDownloader(this->expandedOptions,devicetype);

    }
    else
    {
    this->xfstkdownloader = this->xfstkfactory->CreateDownloader(devicetype);
    }
    if(this->xfstkdownloader == NULL) {
        return false;
    }

    return true;
}

bool XfstkDownloader::ParseCommandline(int argc, char* argv[])
{
    //Parse the commandline from the user
    bool retval = false;

    if(argc>1)
    {
        if(!strcmp(argv[1],"emmcdump"))
        {
            this->expandedOptions = EMMC_DUMP;
        }
        else if(!strcmp(argv[1],"spi"))
        {
            this->expandedOptions = BAYTRAIL;
        }
        else if(!strcmp(argv[1],"hashverify"))
        {
            this->expandedOptions = HASHVERIFY;
        }
    }

    if(this->expandedOptions != 0) {
        this->xfstkoptions = this->xfstkfactory->CreateDownloaderOptions(this->expandedOptions);

        this->xfstkoptions->Parse(argc-1,&argv[1]);
        retval = this->xfstkoptions->IsActionRequired();
    }else if(this->xfstkoptions != NULL) {

        this->xfstkoptions->Parse(argc,argv);
        return true;
    }
    else {
        this->xfstkoptions = this->xfstkfactory->CreateDownloaderOptions(MEDFIELD);
        if(this->xfstkoptions)
        {
            this->xfstkoptions->Parse(argc,argv);
            retval = this->xfstkoptions->IsActionRequired();
        }

    }
    return retval;
}

bool XfstkDownloader::EnumerateDevice()
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
        case BAYTRAIL:
            deveicetype = BAYTRAIL;
            break;
        case CARBONCANYON:
            deveicetype = CARBONCANYON;
            break;
        default:
            deveicetype = XFSTK_NODEVICE;
            break;
        }

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


