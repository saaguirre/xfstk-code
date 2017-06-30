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

extern QMutex apiusbdevicemutex;

xfstksleep::xfstksleep(QObject *parent) :
    QThread(parent)
{
}
void xfstksleep::sleep(unsigned long secs)
{
    QThread::sleep(secs);
}
void xfstksleep::msleep(unsigned long msecs)
{
    QThread::msleep(msecs);
}
void xfstksleep::usleep(unsigned long usecs)
{
    QThread::usleep(usecs);
}

xfstkdldrfactory::xfstkdldrfactory()
{
    char error[] = "Unknown error!\n";
    this->XfstkDldrFactory = NULL;
    this->XfstkDldrFactory = new XfstkFactory();
    this->EnumeratedDeviceType = XFSTK_NODEVICE;
    this->XfstkDeviceWorking = NULL;
    this->XfstkDownloaderList.clear();
    this->XfstkDeviceList.clear();
    this->XfstkOptionsList.clear();
    this->XfstkWinDriverDeviceWorking.clear();
    this->StatusCallback = NULL;
    this->miscdnxenable = false;
	this->softfuse = new char[2048];
    this->miscdnx.reset(new char[2048]);
    memset(this->miscdnx.get(),0,2048);
    memcpy(this->miscdnx.get(),"BLANK.BIN",strlen("BLANK.BIN"));
    this->softfuseinclude = false;

    this->transporttype = XFSTK_USB20;
	this->scudevicetype = XFSTK_NODEVICE;
    this->wipeifwi = false;
    this->lasterror.error_code = 13;
    memset(this->lasterror.error_message,0,sizeof(this->lasterror.error_message));
    memcpy(this->lasterror.error_message,error,strlen(error));
    this->debuglevel = 0xFFFFFFFF;
    this->usbdelayms = 500;
    this->expandedOptions = 0;
    idrqBuffSize = NULL;
    maxIdrqBuffSize = 0;
    idrqBuffer = NULL;
#if defined XFSTK_OS_WIN
    this->WinDriverDeviceFactory = new WDDeviceFactory();
#endif
}
xfstkdldrfactory::~xfstkdldrfactory()
{
    this->ClearAllLists();
    if(this->XfstkDeviceWorking) {
        delete this->XfstkDeviceWorking;
        this->XfstkDeviceWorking = NULL;
    }
    if(this->XfstkDldrFactory) {
        delete this->XfstkDldrFactory;
        this->XfstkDldrFactory = NULL;
    }
#if defined XFSTK_OS_WIN
    if(this->WinDriverDeviceFactory){
        delete this->WinDriverDeviceFactory;
        this->WinDriverDeviceFactory = NULL;
    }
#endif
    if( this->softfuse)
    {
        delete [] this->softfuse;
        this->softfuse = NULL;
    }
}

bool xfstkdldrfactory::ClearAllLists()
{
    IGenericDevice *xfstkdevice = NULL;
    IOptions *xfstkoptions = NULL;
    IDownloader *xfstkdownloader = NULL;

    int listlength = 0;
    int counter = 0;
    listlength = this->XfstkDownloaderList.length();
    for(counter = 0; counter < listlength; counter++) {
        xfstkdownloader = this->XfstkDownloaderList.takeAt(0);
        if(xfstkdownloader != NULL) {
            delete xfstkdownloader;
            xfstkdownloader = NULL;
            this->XfstkDownloaderList.removeAt(0);
        }
    }
    this->XfstkDownloaderList.clear();

    xfstkdevice = NULL;
    xfstkoptions = NULL;
    xfstkdownloader = NULL;

    listlength = 0;
    counter = 0;
    listlength = this->XfstkDeviceList.length();
    for(counter = 0; counter < listlength; counter++) {
        xfstkdevice = this->XfstkDeviceList.takeAt(0);
        if(xfstkdevice != NULL) {
            delete xfstkdevice;
            xfstkdevice = NULL;
        }
    }
    this->XfstkDeviceList.clear();

    xfstkdevice = NULL;
    xfstkoptions = NULL;
    xfstkdownloader = NULL;

    listlength = 0;
    counter = 0;
    listlength = this->XfstkOptionsList.length();
    for(counter = 0; counter < listlength; counter++) {
        xfstkoptions = this->XfstkOptionsList.takeAt(0);
        if(xfstkoptions != NULL) {
            delete xfstkoptions;
            xfstkoptions = NULL;
        }
    }
    this->XfstkOptionsList.clear();

    return true;

}
bool xfstkdldrfactory::ExecuteDownloadSerial(const char *usbsn)
{
    int numberofdevices = 0;
    int counter = 0;
    IGenericDevice *xfstkdevice = NULL;
    IDownloader *xfstkdownloader = NULL;
    last_error er;
    bool passfail = false;
    bool usbsnTargetFlashed = false;
    char message[512];
    memset(message,0,sizeof(message));

    //Before we attempt to download ensure that all interfaces are available to complete the task
    numberofdevices = this->GetNumberOfAttachedDevices();
    if(numberofdevices != XFSTK_ERROR) {

        for(counter = 0; counter < numberofdevices; counter++) {

            //Get the device interface
            xfstkdevice = NULL;
            xfstkdevice = this->XfstkDeviceList.at(counter);
            if(xfstkdevice == NULL) {
                this->ClearAllLists();
                return false;
            }

            if(usbsn != NULL)
            {
                xfstkdevice->GetUsbsn(message);
                if(strcmp(message,usbsn) != 0)
                {
                    break;
                }
                usbsnTargetFlashed = true;
            }
            //Get the downloader interface
            xfstkdownloader = NULL;
            xfstkdownloader = this->XfstkDownloaderList.at(counter);
            if(xfstkdownloader == NULL) {
                this->ClearAllLists();
                return false;
            }
            else {
                passfail = true;
                int counter = 1;
                //Begin the update process and loop until complete
                while(!xfstkdownloader->UpdateTarget()) {
                    xfstkdownloader->GetLastError(&er);
                    //If error code is not successful or a connection error
                    if(er.error_code > 1)
                    {
                        this->lasterror.error_code = er.error_code;
                        memset(this->lasterror.error_message,0,sizeof(this->lasterror.error_message));
                        strcpy(this->lasterror.error_message,er.error_message);
                        return false;
                    }

                    this->sleeper.sleep(10);
                    void *tmphandle = NULL;

                    while(tmphandle == NULL) {
                        memset(message,0,sizeof(message));
                        tmphandle = NULL;
                        tmphandle = xfstkdevice->GetDeviceHandle(0);
                        sprintf(&message[0],"XFSTK-STATUS--Reconnecting to device - Attempt #%d",counter);
                        if(this->StatusCallback != NULL)
                            this->StatusCallback(&message[0],this->ClientData);
                        counter++;

                        this->sleeper.sleep(1);

                        if(counter > this->retrycount) {
                            sprintf(&message[0],"XFSTK-STATUS--Timeout to detect the device, make sure to charge up your phone before flashing\n");
                            if(this->StatusCallback != NULL)
                                this->StatusCallback(&message[0],this->ClientData);
                            this->lasterror.error_code = er.error_code;
                            memset(this->lasterror.error_message,0,sizeof(this->lasterror.error_message));
                            strcpy(this->lasterror.error_message,er.error_message);

                            return false;
                        }
                    }
                    xfstkdevice->SetDeviceHandle(tmphandle);
                }
                if(this->idrqBuffer != NULL)
                {
                    *this->idrqBuffSize = xfstkdownloader->GetResponse(this->idrqBuffer,this->maxIdrqBuffSize);
                }
                last_error tmperror;
                xfstkdownloader->GetLastError(&tmperror);
                this->lasterror.error_code = tmperror.error_code;
                memset(this->lasterror.error_message,0,sizeof(this->lasterror.error_message));
                memcpy(this->lasterror.error_message,tmperror.error_message,strlen(tmperror.error_message));
                if(!xfstkdownloader->GetStatus()) {
                    passfail = false;
                } else {
                    //Download completed for one device, need to break the loop
                    break;
                }
                if(usbsnTargetFlashed)
                    return passfail;
            }
        }
    }

    return passfail;
}

bool xfstkdldrfactory::BindInterfaces()
{
    int numberofdevices = 0;
    int counter = 0;

    IGenericDevice *xfstkdevice = NULL;
    IOptions *xfstkoptions = NULL;
    IDownloader *xfstkdownloader = NULL;

    //Before we attempt to download ensure that all interfaces are available to complete the task
    numberofdevices = this->GetNumberOfAttachedDevices();
    if(numberofdevices != XFSTK_ERROR) {

        for(counter = 0; counter < numberofdevices; counter++) {

            //Get the downloader interface
            xfstkdownloader = NULL;
            xfstkdownloader = this->XfstkDownloaderList.at(counter);
            if(xfstkdownloader == NULL) {
                this->ClearAllLists();
                return false;
            }

            //Get the options interface and bind it to the downloader
            xfstkoptions = NULL;
            xfstkoptions = this->XfstkOptionsList.at(counter);
            if(xfstkoptions) {
                if(!xfstkdownloader->SetOptions(xfstkoptions)) {
                    this->ClearAllLists();
                    return false;
                }
            }
            else {
                this->ClearAllLists();
                return false;
            }

            //Get the device and bind it to the downloader
            xfstkdevice = NULL;
            xfstkdevice = this->XfstkDeviceList.at(counter);
            if(xfstkdevice)
            {
                if(!xfstkdownloader->SetDevice(xfstkdevice))
                {
                    this->ClearAllLists();
                    return false;

                }
                int timeout = xfstkoptions->GetReadWriteTimeout();
                xfstkdevice->SetUsbTimeoutDelay(timeout);
            }

        }

        return true;

    }

    this->ClearAllLists();
    return false;
}

bool xfstkdldrfactory::SetOptions(int argc, char* argv[])
{
    IOptions *xfstkoptions = NULL;
    int listlength = 0;
    int counter = 0;
    listlength = this->XfstkOptionsList.length();
    if(listlength == 0) {
        return false;
    }
    for(counter = 0; counter < listlength; counter++) {
        xfstkoptions = NULL;
        xfstkoptions = this->XfstkOptionsList.at(counter);
        if(xfstkoptions != NULL) {
            xfstkoptions->Parse(argc,argv);
        }
        else {
            return false;
        }
    }
    return true;
}
int xfstkdldrfactory::GetNumberOfAttachedDevices()
{
    int lengthdevices = 0;
    int lengthdownloaders = 0;
    int lengthoptions = 0;
    int lengthtest = 0;

    lengthdevices = this->XfstkDeviceList.length();
    lengthdownloaders = this->XfstkDownloaderList.length();
    lengthoptions = this->XfstkOptionsList.length();

    lengthtest = lengthdevices - lengthdownloaders - lengthoptions;

    if((lengthtest + lengthdevices == 0) && (lengthtest + lengthdownloaders == 0) && (lengthtest + lengthoptions == 0)) {
        return lengthdevices;
    }

    this->ClearAllLists();

    return XFSTK_ERROR;
}

int xfstkdldrfactory::GetNumberOfAttachedDevices(SoCDevices * socdevices)
{
    int totaldevices = 0;
    unsigned long devicetype = XFSTK_NODEVICE;

    socdevices->number_devices = 0;

    //Enumerate for any attached device.  This enumeration is looking for the first available device.

    for(int i = 0; i < XFSTK_MAXDEVICE; i++) {
        switch(i) {
        this->XfstkDeviceWorking = NULL;
        case MOORESTOWN:
            devicetype = MOORESTOWN;
            break;
        case MEDFIELD:
            devicetype = MEDFIELD;
            break;
        case CLOVERVIEW:
            devicetype = CLOVERVIEW;
            break;
        case CLOVERVIEWPLUS:
            devicetype = CLOVERVIEWPLUS;
            break;
        case MERRIFIELD:
            devicetype = MERRIFIELD;
            break;
        case MOOREFIELD:
            devicetype = MOOREFIELD;
            break;
        case BAYTRAIL:
            devicetype = BAYTRAIL;
            break;
        case CARBONCANYON:
            devicetype = CARBONCANYON;
            break;
        default:
            devicetype = XFSTK_NODEVICE;
            break;
        }

        //Attempt to create a device for each possible target type.

        this->XfstkDeviceWorking = this->XfstkDldrFactory->CreateDevice(devicetype);
        if(this->XfstkDeviceWorking) {

            //Dynamic transport "could" be specified via cmdline and IOptions...

            if(this->XfstkDeviceWorking->SetTransport(this->transporttype)) {
                totaldevices = this->XfstkDeviceWorking->GetNumberAvailableDevices((DeviceInfos*)socdevices);
            }
            delete this->XfstkDeviceWorking;
            this->XfstkDeviceWorking = NULL;

			//If find device then need to break out
			if(totaldevices > 0)
				break;

        }
    }
    return totaldevices;
}

bool xfstkdldrfactory::CreateAllRequiredInterfaces(unsigned long devicetype, int totalnumdevices)
{
    IGenericDevice *xfstkdevice = NULL;
    IOptions *xfstkoptions = NULL;
    IDownloader *xfstkdownloader = NULL;

    //If zero devices enumerated exit without creating any interfaces.
    if(devicetype == XFSTK_NODEVICE) {
        this->ClearAllLists();
        return false;
    }

    for(int i = 0; i < totalnumdevices; i++) {
        //Create a download options interface
        xfstkoptions = NULL;

        if(this->expandedOptions!=0)
        {
            xfstkoptions = this->XfstkDldrFactory->CreateDownloaderOptions(this->expandedOptions);

        }else
        {
            xfstkoptions = this->XfstkDldrFactory->CreateDownloaderOptions(devicetype);
        }


        if(xfstkoptions == NULL) {
            this->ClearAllLists();
            return false;
        }
        else {
           this->XfstkOptionsList.append(xfstkoptions);
        }

        //Create a device interface to communicate with the SoC
        xfstkdevice = NULL;
        xfstkdevice = this->XfstkDldrFactory->CreateDevice(devicetype);
        if(xfstkdevice == NULL) {
            this->ClearAllLists();
            return false;
        }
        else {
            this->XfstkDeviceList.append(xfstkdevice);
        }

        //Configure the device interafce with transport type and physical device instance (use first device "0")
        xfstkdevice->SetTransport(this->transporttype);
        void *tmphandle = xfstkdevice->GetDeviceHandle(i);
        xfstkdevice->SetDeviceHandle(tmphandle);

        //Create the downloader interface for the current device type
        xfstkdownloader = NULL;

        if(this->expandedOptions!=0)
        {
            xfstkdownloader = this->XfstkDldrFactory->CreateDownloader(this->expandedOptions,devicetype);

        }else
        {
            xfstkdownloader = this->XfstkDldrFactory->CreateDownloader(devicetype);
        }



        if(xfstkdownloader == NULL) {
            this->ClearAllLists();
            return false;
        }
        else {
            xfstkdownloader->SetStatusCallback(this->StatusCallback, this->ClientData);
            this->XfstkDownloaderList.append(xfstkdownloader);
        }
    }

    return true;
}
#if defined XFSTK_OS_WIN
bool xfstkdldrfactory::SetDeviceCallBackInstances(void* deviceCallBack)
{
    bool devicesInstances = false;
    unsigned long devicetype = XFSTK_NODEVICE;
    this->ClearAllLists();

    //Enumerate for any attached device.
    //if platform is non zero, then enumerate that platform
    for(int i = 0; i < XFSTK_MAXDEVICE; i++) {
        switch(i) {
        case MOORESTOWN:
            devicetype = MOORESTOWN;
            break;
        case MEDFIELD:
            devicetype = MEDFIELD;
            break;
        case CLOVERVIEW:
            devicetype = CLOVERVIEW;
            break;
        case CLOVERVIEWPLUS:
            devicetype = CLOVERVIEWPLUS;
            break;
        case MERRIFIELD:
            devicetype = MERRIFIELD;
            break;
        case MOOREFIELD:
            devicetype = MOOREFIELD;
            break;
        case BAYTRAIL:
            devicetype = BAYTRAIL;
            break;
        case CARBONCANYON:
            devicetype = CARBONCANYON;
            break;
         default:
            devicetype = XFSTK_NODEVICE;
            break;
        }


        //Attempt to create a device for each possible target type.
        this->XfstkWinDriverDeviceWorking.append(this->WinDriverDeviceFactory->CreateDevice(devicetype,deviceCallBack));

        devicesInstances = devicesInstances || (this->XfstkWinDriverDeviceWorking.at(i) ? true: false);
    }
    return devicesInstances;
}
bool xfstkdldrfactory::SetDeviceCallBackInstances(void* deviceCallBack,unsigned long ProductId)
{
    bool devicesInstances = false;
    unsigned long devicetype = XFSTK_NODEVICE;
    this->ClearAllLists();


    //Enumerate for any attached device.
    //if platform is non zero, then enumerate that platform
    for(int i = 0; i < XFSTK_MAXDEVICE; i++) {
        switch(i) {
        case MOORESTOWN:
            devicetype = MOORESTOWN;
            break;
        case MEDFIELD:
            devicetype = MEDFIELD;
            break;
        case CLOVERVIEW:
            devicetype = CLOVERVIEW;
            break;
        case CLOVERVIEWPLUS:
            devicetype = CLOVERVIEWPLUS;
            break;
        case MERRIFIELD:
            devicetype = MERRIFIELD;
            break;
        case MOOREFIELD:
            devicetype = MOOREFIELD;
            break;
        case BAYTRAIL:
            devicetype = BAYTRAIL;
            break;
        case CARBONCANYON:
            devicetype = CARBONCANYON;
            break;
         default:
            devicetype = XFSTK_NODEVICE;
            break;
        }

        if(this->XfstkWinDriverDeviceWorking.count() < XFSTK_MAXDEVICE)
            this->XfstkWinDriverDeviceWorking.append(NULL);

        //Attempt to create a device for each possible target type.
        if((ProductId == devicetype) && (devicetype != XFSTK_NODEVICE))
        {
            this->XfstkWinDriverDeviceWorking.replace(devicetype,this->WinDriverDeviceFactory->CreateDevice(devicetype,deviceCallBack));
            if(this->XfstkWinDriverDeviceWorking.at(devicetype))
                devicesInstances = true;
        }
    }
    return devicesInstances;
}
#endif



bool xfstkdldrfactory::EnumerateDevices()
{

    int totaldevices = 0;
    unsigned long devicetype = XFSTK_NODEVICE;
    this->ClearAllLists();

    //Enumerate for any attached device.  This enumeration is looking for the first available device.

    for(int i = 0; i < XFSTK_MAXDEVICE; i++) {
        switch(i) {
        this->XfstkDeviceWorking = NULL;
        case MOORESTOWN:
            devicetype = MOORESTOWN;
            break;
        case MEDFIELD:
            devicetype = MEDFIELD;
            break;
        case CLOVERVIEW:
            devicetype = CLOVERVIEW;
            break;
        case CLOVERVIEWPLUS:
            devicetype = CLOVERVIEWPLUS;
            break;
        case MERRIFIELD:
            devicetype = MERRIFIELD;
            break;
        case MOOREFIELD:
            devicetype = MOOREFIELD;
            break;
        case BAYTRAIL:
            devicetype = BAYTRAIL;
            break;
        case CARBONCANYON:
            devicetype = CARBONCANYON;
            break;
        default:
            devicetype = XFSTK_NODEVICE;
            break;
        }


        //Attempt to create a device for each possible target type.

        this->XfstkDeviceWorking = this->XfstkDldrFactory->CreateDevice(devicetype);
        if(this->XfstkDeviceWorking) {

            //Dynamic transport "could" be specified via cmdline and IOptions...

            if(this->XfstkDeviceWorking->SetTransport(this->transporttype)) {
                totaldevices = totaldevices + this->XfstkDeviceWorking->GetNumberAvailableDevices();
            }
            delete this->XfstkDeviceWorking;
            this->XfstkDeviceWorking = NULL;

            //If we found a device make a note of it and create all required interfaces needed to provision
            if(totaldevices > 0) {
                this->EnumeratedDeviceType = devicetype;
                return this->CreateAllRequiredInterfaces(this->EnumeratedDeviceType, totaldevices);
            }
        }
        //Since we have MFLD and CLV a0 using the same devices ID, we will bail out as long as we find a valid device
        //And we suppose we will never to download to two different type of devices at the same time.
        if(totaldevices > 0)
            break;
    }
    return false;
}
int xfstkdldrfactory::ScanForDevices()
{
    int totaldevices = 0;
    unsigned long devicetype = XFSTK_NODEVICE;

    //Enumerate for any attached device.  This enumeration is looking for the first available device.

    for(int i = 0; i < XFSTK_MAXDEVICE; i++) {
        switch(i) {
        this->XfstkDeviceWorking = NULL;
        case MOORESTOWN:
            devicetype = MOORESTOWN;
            break;
        case MEDFIELD:
            devicetype = MEDFIELD;
            break;
        case CLOVERVIEW:
            devicetype = CLOVERVIEW;
            break;
        case CLOVERVIEWPLUS:
            devicetype = CLOVERVIEWPLUS;
            break;
        case MERRIFIELD:
            devicetype = MERRIFIELD;
            break;
        case MOOREFIELD:
            devicetype = MOOREFIELD;
            break;
        case BAYTRAIL:
            devicetype = BAYTRAIL;
            break;
        case CARBONCANYON:
            devicetype = CARBONCANYON;
            break;
        default:
            devicetype = XFSTK_NODEVICE;
            break;
        }

        //Attempt to create a device for each possible target type.

        this->XfstkDeviceWorking = this->XfstkDldrFactory->CreateDevice(devicetype);
        if(this->XfstkDeviceWorking) {

            //Dynamic transport "could" be specified via cmdline and IOptions...

            if(this->XfstkDeviceWorking->SetTransport(this->transporttype)) {
                totaldevices = totaldevices + this->XfstkDeviceWorking->GetNumberAvailableDevices();
            }
            delete this->XfstkDeviceWorking;
            this->XfstkDeviceWorking = NULL;

        }
    }
    return totaldevices;
}

int xfstkdldrfactory::ScanForDevices(unsigned long devicetype)
{
    int totaldevices = 0;
    //Attempt to create a device for each possible target type.

    this->XfstkDeviceWorking = this->XfstkDldrFactory->CreateDevice(devicetype);
    if(this->XfstkDeviceWorking) {

        //Dynamic transport "could" be specified via cmdline and IOptions...
        if(devicetype == BAYTRAIL)
        {
            this->XfstkDeviceWorking->SetTransport(XFSTK_DEDI_PROG);
            totaldevices = this->XfstkDeviceWorking->GetNumberAvailableDevices();
        }
        else if(this->XfstkDeviceWorking->SetTransport(this->transporttype)) {
            totaldevices = this->XfstkDeviceWorking->GetNumberAvailableDevices();
        }
        delete this->XfstkDeviceWorking;
        this->XfstkDeviceWorking = NULL;

    }
    return totaldevices;
}

bool xfstkdldrfactory::SetStatusCallback(XfstkStatusPfn StatusCallback, void* clientdata)
{
    this->StatusCallback = StatusCallback;
    this->ClientData = clientdata;
    return true;
}

void xfstkdldrfactory::SetTransportType(DeviceTransportType transporttype)
{
    this->transporttype = transporttype;
}

void xfstkdldrfactory::SetIdrqResponse(unsigned char *buffer, int& maxsize)
{
    this->idrqBuffer = buffer;
    this->idrqBuffSize = &maxsize;
    this->maxIdrqBuffSize = maxsize;
}

bool xfstkdldrfactory::GetLastError(LastError *er)
{
    char def_error[] = "Unknown error!\n";
	
    er->error_code = this->lasterror.error_code;
	if(er->error_code > MAX_ERROR_CODE) {
        er->error_code = 13; //Unknown error
        memset(er->error_message,0,sizeof(er->error_message));
        memcpy(er->error_message, def_error,strlen(def_error));
	} else {

        memset(er->error_message,0,sizeof(er->error_message));
        memcpy(er->error_message, this->lasterror.error_message, strlen(this->lasterror.error_message));
	}
	return true;
}


bool xfstkdldrfactory::ExecuteDownloadSingleAsync(char *fwdnx, char *fwimage, char *osdnx, char* osimage, char* gpflags, char* usbsn, int retrycount)
{
    xfstkdldrthread *xfstkkillthread = NULL;
    xfstkdldrthreadobj *xfstkkillobj = NULL;
    int donetargets = 0;
    int failtargets = 0;
    bool passfail = true;
    char message[512];
    int counter = 0;
    bool devicedetected = false;
    int threadlistlength = 0;
    char myusbsn[128];
    bool threadrunning = false;
    char error_message[] = "Can not find device handle.\n";

    IGenericDevice *xfstkdevice = NULL;

    int totalnumtargets = 1;
    try {
        unsigned long devicetype = XFSTK_NODEVICE;
        //Scan connected target devices, loop until find one device or timeout
        while(1) {
            for(int i = 0; i < XFSTK_MAXDEVICE; i++) {
                switch(i) {
                    this->XfstkDeviceWorking = NULL;
                case MOORESTOWN:
                    devicetype = MOORESTOWN;
                    break;
                case MEDFIELD:
                    devicetype = MEDFIELD;
                    break;
                case CLOVERVIEW:
                    devicetype = CLOVERVIEW;
                    break;
                case CLOVERVIEWPLUS:
                    devicetype = CLOVERVIEWPLUS;
                    break;
                case MERRIFIELD:
                    devicetype = MERRIFIELD;
                    break;
                case MOOREFIELD:
                    devicetype = MOOREFIELD;
                    break;
                case BAYTRAIL:
                    devicetype = BAYTRAIL;
                    break;
                default:
                    devicetype = XFSTK_NODEVICE;
                    break;
                }

                if(devicetype != XFSTK_NODEVICE)
                {
                    //Create a device interface to communicate with the SoC
                    xfstkdevice = this->XfstkDldrFactory->CreateDevice(devicetype);
                    if(xfstkdevice != NULL)
                    {
                        //Configure the device interface with transport type and physical device instance
                        if(xfstkdevice->SetTransport(this->transporttype))
                        {
                            if(xfstkdevice->GetNumberAvailableDevices() > 0)
                            {
                                //Here as soon as one device is detected, will break out
                                //Suppose only one type of platform is plugged in at a time.
                                devicedetected = true;
                                break;
                            }
                        }
                        delete xfstkdevice;
                        xfstkdevice = NULL;
                    }
                }
            }

            if(counter++ > retrycount) {
                passfail = false;
                this->lasterror.error_code = 7; //Can not find device handle
                memset(this->lasterror.error_message,0,sizeof(this->lasterror.error_message));
                memcpy(this->lasterror.error_message,error_message,strlen(error_message));
                break;
            }

            if(devicedetected)
                break;
            sprintf(&message[0],"\nUSBSN: %s -- Attemp %d times to scan devices ...\n", usbsn, counter);
            if(this->StatusCallback != NULL)
                this->StatusCallback(message,this->ClientData);
            this->sleeper.msleep(200);
        }

        counter = 0;

        if(xfstkdevice != NULL) {

            //looping and find all devices, start a thread as soong as one device is found
            while(1) {

                if(donetargets == totalnumtargets) {
                    break;
                } else {
                    if(counter++ > retrycount) {
                        passfail = false;
                        this->lasterror.error_code = 7; //
                        memset(this->lasterror.error_message,0,sizeof(this->lasterror.error_message));
                        memcpy(this->lasterror.error_message,error_message,strlen(error_message));

                        sprintf(&message[0],"\nUSBSN: %s--FW/OS download timeout aborting...\n", usbsn);
                        if(this->StatusCallback != NULL)
                            this->StatusCallback(&message[0],this->ClientData);
                        break;
                    }
                    sprintf(&message[0],"\nUSBSN: %s -- Attemp %d times to start download ...\n", usbsn, counter);
                    if(this->StatusCallback != NULL)
                        this->StatusCallback(message,this->ClientData);
                    this->sleeper.msleep(200);
               }

                if(!threadrunning) {
                     for (int j = 0; j < xfstkdevice->GetNumberAvailableDevices(); j++){
                        void *tmphandle = xfstkdevice->GetDeviceHandle(j);
                        if(tmphandle) {
                            xfstkdevice->SetDeviceHandle(tmphandle);
                            memset(myusbsn, 0, 128);
                            xfstkdevice->GetUsbsn(myusbsn);
                        }

                        if((tmphandle != NULL)   \
                           && ((string)myusbsn == (string)usbsn) \
                           && !this->DeviceHandleList.contains(tmphandle)) {

                            sprintf(&message[0],"\nUSBSN: %s -- Starting downloading thread...\n", usbsn);
                            if(this->StatusCallback != NULL)
                                this->StatusCallback(message,this->ClientData);

                            this->DeviceHandleList.append(tmphandle);
                            xfstkdldrthread *xfstknewthread = NULL;
                            xfstknewthread = new xfstkdldrthread();
                            xfstkdldrthreadobj *xfstknewthreadobj = NULL;
                            xfstknewthreadobj = new xfstkdldrthreadobj();
                            if(xfstknewthread) {
                                xfstkmtthreadlist.append(xfstknewthread);
                            }

                            if(xfstknewthreadobj) 
							{
                                xfstkthreadobjlist.append(xfstknewthreadobj);
                                xfstknewthreadobj->isDebug = 0xffffffff; //enable all
                                xfstknewthreadobj->retrycount = retrycount;
								xfstknewthreadobj->physstatuspfn = StatusCallback;
								xfstknewthreadobj->handle = tmphandle;
								xfstknewthreadobj->devicetype = devicetype;
								xfstknewthreadobj->debuglevel = debuglevel;
								xfstknewthreadobj->usbdelayms = usbdelayms;
								xfstknewthreadobj->wipeifwi = wipeifwi;
								xfstknewthreadobj->physclientdata = ClientData;
								xfstknewthreadobj->miscdnxenable = this->miscdnxenable;
								memcpy(xfstknewthreadobj->miscdnx,this->miscdnx.get(),strlen(this->miscdnx.get()));
								memcpy(xfstknewthreadobj->usbsn, usbsn, strlen(usbsn));

								xfstknewthreadobj->configuredownloader(fwdnx, fwimage, osdnx, osimage, gpflags, NULL, totalnumtargets);

								if( xfstknewthread ) // pointer check before use
								{ 
									xfstknewthread->setobj(xfstknewthreadobj);
									xfstknewthread->start();  //THIS LAUNCHES THE THREAD
									//Since we are handling only one device here, we can wait here
									xfstknewthread->wait();
									threadrunning = true;
								}
							
								if(xfstknewthreadobj->lastError.error_code == 0xBAADF00D)
								{
									this->lasterror.error_code = xfstknewthreadobj->lastError.error_code;
									memset(this->lasterror.error_message,0,sizeof(this->lasterror.error_message));
									strcpy(this->lasterror.error_message, xfstknewthreadobj->lastError.error_message);
									return false;
								}
							}
                            //Some times it will detect two device handle with the same usbsn
                            //only one handle device, so break out to avoid start two download.
                            break;
                        }
                    }
                }

                //remove the device that is done
                threadlistlength = xfstkmtthreadlist.length();
                for(int j = 0; j < threadlistlength; j++) {
                    xfstkkillthread = xfstkmtthreadlist.at(j);
                    xfstkkillobj = xfstkthreadobjlist.at(j);

					if( xfstkkillobj ) // pointer check before use
					{
						if(xfstkkillobj->osisdone ==  true){
							donetargets++;
							passfail = true;
						}

						if(xfstkkillobj->downloadfailed ==  true){
							failtargets++;
							passfail = false;
							this->sleeper.msleep(100);
						}
					}

                    //CLEAN UP ALL THE THREADS
                    if(xfstkkillobj && xfstkkillthread) {
                        if(xfstkkillobj->fwisdone ==  true
                           || xfstkkillobj->osisdone ==  true
                           || xfstkkillobj->downloadfailed ==  true) {
                            threadrunning = false;
                            //need to remove from the device list
                            xfstkkillthread->obj = NULL;
                            int temptargetindex = this->DeviceHandleList.indexOf(xfstkkillobj->handle);
                            this->DeviceHandleList.takeAt(temptargetindex);

                            xfstkkillthread = xfstkmtthreadlist.takeAt(j);
                            xfstkkillobj = xfstkthreadobjlist.takeAt(j);
                            this->lasterror.error_code = xfstkkillobj->lastError.error_code;
                            memset(this->lasterror.error_message,0,sizeof(this->lasterror.error_message));
                            memcpy(this->lasterror.error_message,xfstkkillobj->lastError.error_message,strlen(xfstkkillobj->lastError.error_message));
                            delete xfstkkillthread;
                            delete xfstkkillobj;
                            this->sleeper.msleep(100);
                            if(!failtargets)
                                break;

                        }
                    }
                }


            }
        } else {
            passfail = false;
        }

        //Do some cleanup if anything is left in the lists
        if(xfstkdevice) {
            int threadlength = xfstkmtthreadlist.length();
            for(int j = 0; j < threadlength; j++) {
                xfstkkillthread = xfstkmtthreadlist.takeAt(0);
                xfstkkillobj = xfstkthreadobjlist.takeAt(0);
                this->lasterror.error_code = xfstkkillobj->lastError.error_code;
                memset(this->lasterror.error_message,0,sizeof(this->lasterror.error_message));
                memcpy(this->lasterror.error_message,xfstkkillobj->lastError.error_message,strlen(xfstkkillobj->lastError.error_message));

                delete xfstkkillthread;
                xfstkkillthread = NULL;
                delete xfstkkillobj;
                xfstkkillobj = NULL;
            }

            threadlength = this->DeviceHandleList.length();
            for(int j = 0; j < threadlength; j++) {
                this->DeviceHandleList.takeAt(0);
            }

            delete xfstkdevice;
            xfstkdevice = NULL;
        }
    }
    catch ( ... ) {
        string tmpmessage = (string)"\nUSBSN: " + (string)usbsn + (string)"--FW/OS download caused an exception!\n";
        if(this->StatusCallback != NULL)
            this->StatusCallback((char*)tmpmessage.c_str(),this->ClientData);

        this->sleeper.msleep(200);
    }

    return passfail;
}


bool xfstkdldrfactory::ExecuteDownloadSingleAsync(char *fwdnx, char *fwimage, char *osdnx, char* osimage, char* gpflags, char* usbsn, char* softfuse, int retrycount)
{
    xfstkdldrthread *xfstkkillthread = NULL;
    xfstkdldrthreadobj *xfstkkillobj = NULL;
    int donetargets = 0;
    int failtargets = 0;
    bool passfail = true;
    char message[512];
    int counter = 0;
    bool devicedetected = false;
    int threadlistlength = 0;
    char myusbsn[128];
    bool threadrunning = false;
    char error_message[] = "Can not find device handle.\n";

    IGenericDevice *xfstkdevice = NULL;
   
    int totalnumtargets = 1;
    try {
        unsigned long devicetype = XFSTK_NODEVICE;
        //Scan connected target devices, loop until find one device or timeout
        while(1) {
            for(int i = 0; i < XFSTK_MAXDEVICE; i++) {
                switch(i) {
                    this->XfstkDeviceWorking = NULL;
                case MOORESTOWN:
                    devicetype = MOORESTOWN;
                    break;
                case MEDFIELD:
                    devicetype = MEDFIELD;
                    break;
                case CLOVERVIEW:
                    devicetype = CLOVERVIEW;
                    break;
                case CLOVERVIEWPLUS:
                    devicetype = CLOVERVIEWPLUS;
                    break;
                case MERRIFIELD:
                    devicetype = MERRIFIELD;
                    break;
                case MOOREFIELD:
                    devicetype = MOOREFIELD;
                    break;
                case BAYTRAIL:
                    devicetype = BAYTRAIL;
                    break;
                default:
                    devicetype = XFSTK_NODEVICE;
                    break;
                }
                
                if(devicetype != XFSTK_NODEVICE)
                {
                    //Create a device interface to communicate with the SoC
                    xfstkdevice = this->XfstkDldrFactory->CreateDevice(devicetype);
                    if(xfstkdevice != NULL)
                    {
                        //Configure the device interface with transport type and physical device instance
                        if(xfstkdevice->SetTransport(this->transporttype))
                        {
                            if(xfstkdevice->GetNumberAvailableDevices() > 0)
                            {
                                //Here as soon as one device is detected, will break out
                                //Suppose only one type of platform is plugged in at a time.
                                devicedetected = true;
                                break;
                            }
                        }
                        delete xfstkdevice;
                        xfstkdevice = NULL;
                    }
                }
            }
            
            if(counter++ > retrycount) {
                passfail = false;
                this->lasterror.error_code = 7; //Can not find device handle
                memset(this->lasterror.error_message,0,sizeof(this->lasterror.error_message));
                memcpy(this->lasterror.error_message,error_message,strlen(error_message));
                break;
            }
            
            if(devicedetected)
                break;
            sprintf(&message[0],"\nUSBSN: %s -- Attemp %d times to scan devices ...\n", usbsn, counter);
            if(this->StatusCallback != NULL)
                this->StatusCallback(message,this->ClientData);
            this->sleeper.msleep(200);
        }
        
        counter = 0;
        
        if(xfstkdevice != NULL) {
            
            //looping and find all devices, start a thread as soong as one device is found
            while(1) {
                
                if(donetargets == totalnumtargets) {
                    break;
                } else {
                    if(counter++ > retrycount) {
                        passfail = false;
                        this->lasterror.error_code = 7; //
                        memset(this->lasterror.error_message,0,sizeof(this->lasterror.error_message));
                        memcpy(this->lasterror.error_message,error_message,strlen(error_message));

                        sprintf(&message[0],"\nUSBSN: %s--FW/OS download timeout aborting...\n", usbsn);
                        if(this->StatusCallback != NULL)
                            this->StatusCallback(&message[0],this->ClientData);
                        break;
                    }
                    sprintf(&message[0],"\nUSBSN: %s -- Attemp %d times to start download ...\n", usbsn, counter);
                    if(this->StatusCallback != NULL)
                        this->StatusCallback(message,this->ClientData);
		            this->sleeper.msleep(200);
               }
                
                if(!threadrunning) {
                     for (int j = 0; j < xfstkdevice->GetNumberAvailableDevices(); j++){
                        void *tmphandle = xfstkdevice->GetDeviceHandle(j);
                        if(tmphandle) {
                            xfstkdevice->SetDeviceHandle(tmphandle);
                            memset(myusbsn, 0, 128);
                            xfstkdevice->GetUsbsn(myusbsn);
                        }
                        
                        if((tmphandle != NULL)   \
                           && ((string)myusbsn == (string)usbsn) \
                           && !this->DeviceHandleList.contains(tmphandle)) {
                            
                            sprintf(&message[0],"\nUSBSN: %s -- Starting downloading thread...\n", usbsn);
                            if(this->StatusCallback != NULL)
                                this->StatusCallback(message,this->ClientData);
                            
                            this->DeviceHandleList.append(tmphandle);
                            xfstkdldrthread *xfstknewthread = NULL;
                            xfstknewthread = new xfstkdldrthread();
                            xfstkdldrthreadobj *xfstknewthreadobj = NULL;
                            xfstknewthreadobj = new xfstkdldrthreadobj();
                            if(xfstknewthread) {
                                xfstkmtthreadlist.append(xfstknewthread);
                            }

                            if( xfstknewthreadobj ) 
							{
                                xfstkthreadobjlist.append(xfstknewthreadobj);
                                xfstknewthreadobj->isDebug = 0xffffffff; //enable all
                                xfstknewthreadobj->retrycount = retrycount;
								xfstknewthreadobj->physstatuspfn = StatusCallback;
								xfstknewthreadobj->handle = tmphandle;
								xfstknewthreadobj->devicetype = devicetype;
								xfstknewthreadobj->debuglevel = debuglevel;
								xfstknewthreadobj->usbdelayms = usbdelayms;
								xfstknewthreadobj->wipeifwi = wipeifwi;
								xfstknewthreadobj->physclientdata = ClientData;
								xfstknewthreadobj->miscdnxenable = this->miscdnxenable;
								memcpy(xfstknewthreadobj->miscdnx,this->miscdnx.get(),strlen(this->miscdnx.get()));
								memcpy(xfstknewthreadobj->usbsn, usbsn, strlen(usbsn));
                            
								xfstknewthreadobj->configuredownloader(fwdnx, fwimage, osdnx, osimage, gpflags, softfuse, NULL, totalnumtargets);
                            
								if( xfstknewthread )
								{
									xfstknewthread->setobj(xfstknewthreadobj);
									xfstknewthread->start();  //THIS LAUNCHES THE THREAD
									//Since we are handling only one device here, we can wait here
									xfstknewthread->wait();
									threadrunning = true;
								}

								if(xfstknewthreadobj->lastError.error_code == 0xBAADF00D)
								{
									this->lasterror.error_code = xfstknewthreadobj->lastError.error_code;
									memset(this->lasterror.error_message,0,sizeof(this->lasterror.error_message));
									strcpy(this->lasterror.error_message, xfstknewthreadobj->lastError.error_message);
									return false;
								}
							}
                            //Some times it will detect two device handle with the same usbsn
                            //only one handle device, so break out to avoid start two download.
                            break;
                        }
                        
                    }
                }
                
                //remove the device that is done
                threadlistlength = xfstkmtthreadlist.length();
                for(int j = 0; j < threadlistlength; j++) {
                    xfstkkillthread = xfstkmtthreadlist.at(j);
                    xfstkkillobj = xfstkthreadobjlist.at(j);
                    
					if( xfstkkillobj ) // pointer check before use
					{
						if(xfstkkillobj->osisdone ==  true){
							donetargets++;
							passfail = true;
						}
					}
                    
					if( xfstkkillobj ) // pointer check before use
					{
						if(xfstkkillobj->downloadfailed ==  true){
							failtargets++;
							passfail = false;
							this->sleeper.msleep(100);
						}
					}
                    
                    //CLEAN UP ALL THE THREADS
                    if(xfstkkillobj && xfstkkillthread) {
                        if(xfstkkillobj->fwisdone ==  true
                           || xfstkkillobj->osisdone ==  true
                           || xfstkkillobj->downloadfailed ==  true) {
                            threadrunning = false;
                            //need to remove from the device list
                            xfstkkillthread->obj = NULL;
                            int temptargetindex = this->DeviceHandleList.indexOf(xfstkkillobj->handle);
                            this->DeviceHandleList.takeAt(temptargetindex);
                            
                            xfstkkillthread = xfstkmtthreadlist.takeAt(j);
                            xfstkkillobj = xfstkthreadobjlist.takeAt(j);
                            this->lasterror.error_code = xfstkkillobj->lastError.error_code;
                            memset(this->lasterror.error_message,0,sizeof(this->lasterror.error_message));
                            memcpy(this->lasterror.error_message,xfstkkillobj->lastError.error_message,strlen(xfstkkillobj->lastError.error_message));
							delete xfstkkillthread;
                            xfstkkillthread = NULL;
                            delete xfstkkillobj;
                            xfstkkillobj = NULL;
                            this->sleeper.msleep(100);
                            if(!failtargets)
                                break;
                            
                        }
                    }
                }
                
                
            }
        } else {
            passfail = false;
        }
        
        //Do some cleanup if anything is left in the lists
        if(xfstkdevice) {
            int threadlength = xfstkmtthreadlist.length();
            for(int j = 0; j < threadlength; j++) {
                xfstkkillthread = xfstkmtthreadlist.takeAt(0);
                xfstkkillobj = xfstkthreadobjlist.takeAt(0);
                this->lasterror.error_code = xfstkkillobj->lastError.error_code;
                memset(this->lasterror.error_message,0,sizeof(this->lasterror.error_message));
                memcpy(this->lasterror.error_message,xfstkkillobj->lastError.error_message,strlen(xfstkkillobj->lastError.error_message));

                
                delete xfstkkillthread;
                xfstkkillthread = NULL;
                delete xfstkkillobj;
                xfstkkillobj = NULL;
            }
            
            threadlength = this->DeviceHandleList.length();
            for(int j = 0; j < threadlength; j++) {
                this->DeviceHandleList.takeAt(0);
            }
            
            delete xfstkdevice;
            xfstkdevice = NULL;
        }
    }
    catch ( ... ) {
        string tmpmessage = (string)"\nUSBSN: " + (string)usbsn + (string)"--FW/OS download caused an exception!\n";
        if(this->StatusCallback != NULL)
            this->StatusCallback((char*)tmpmessage.c_str(),this->ClientData);
        
        this->sleeper.msleep(200);
    }
    
    return passfail;
}


