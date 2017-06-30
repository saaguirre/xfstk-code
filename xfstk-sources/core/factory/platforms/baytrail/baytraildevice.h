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
#ifndef BAYTRAILDEVICE_H
#define BAYTRAILDEVICE_H
#include "../../interfaces/idevice.h"
#include "baytrailutils.h"
#include "baytraildownloader.h"
#include <stdio.h>
#include <string>
#include <memory.h>

#ifdef XFSTK_OS_WIN
#include "../../interfaces/idevicefactory.h"
#include <wddevicefactory.h>
#define BAYTRAIL_DEVICE_ID 0x9
#else
#include <usb.h>
#endif

#define INTEL_VENDOR_ID   0x8086
#define BAYTRAIL_PRODUCT_ID  0xe009
#define USBSN_BUFFER_SIZE 128
#define MAX_SOC_DEVICES 32

class BaytrailDevice : public IDevice, public QObject
{
public:
    BaytrailDevice(QObject *parent = 0);
    ~BaytrailDevice();

    //from object: IGenericDevice
    int GetNumberAvailableDevices();
    int GetNumberAvailableDevices(DeviceInfos * socdevices);
    void* GetDeviceHandle(int DeviceNumber);
    bool SetDeviceHandle(void *DeviceHandle);
    bool SetTransport(DeviceTransportType Transport);
    void GetUsbsn(char * usbsn);

    //from object: IDEVICE
    bool Open();
    bool Init();
    bool Write(void *pBuffer, unsigned int bufferSize);
    bool Read(void *szBuff, unsigned int bufferSize);
    bool GetAck(void *szBuff, unsigned int *bytes_rxed);
    int Detect();
    bool Abort();
    bool IsCorrectProtocol(void *szBuff, unsigned int *bytes_rxed);
    bool SetUtilityInstance(void *UtilityInstance);
    bool IsSupportedDevice(unsigned short vendorid, unsigned short productid);
    int SetUsbTimeoutDelay(int timeoutMS){ return timeoutMS; }
private:
    last_error m_last_error;
    IDevice *PhysicalTransportDevice;
    QProcess *device_process;
    BaytrailUtils   libutils;
#if defined XFSTK_OS_WIN
    IDeviceFactory *deviceFactory;
#endif
};

#endif // BAYTRAILDEVICE_H
