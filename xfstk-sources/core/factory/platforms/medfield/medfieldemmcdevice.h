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
#ifndef MEDFIELDEMMCDEVICE_H
#define MEDFIELDEMMCDEVICE_H
#include "../../interfaces/idevice.h"
#include "medfieldutils.h"
#include "../emmc/emmcutils.h"
#include <stdio.h>
#include <string>
#include <memory.h>

#ifdef XFSTK_OS_WIN
#include "../../interfaces/idevicefactory.h"
#include <wddevicefactory.h>
#define MEDFIELD_DEVICE_ID 0x2
#else
#include <usb.h>
#endif

#define INTEL_VENDOR_ID   0x8086
#define MEDFIELD_PRODUCT_ID  0xe004
#define USBSN_BUFFER_SIZE 128
#define MAX_SOC_DEVICES 32

class MedfieldEmmcDevice : public IDevice
{
public:
    MedfieldEmmcDevice();
    ~MedfieldEmmcDevice();
    int GetNumberAvailableDevices();
#if defined XFSTK_OS_WIN
    int GetNumberAvailableDevices(DeviceInfos * socdevices);
#endif
    void* GetDeviceHandle(int DeviceNumber);
    bool SetDeviceHandle(void *DeviceHandle);
    bool SetTransport(DeviceTransportType Transport);
    bool Open();
    bool Init();
    bool Write(void *pBuffer, unsigned int bufferSize);
    bool Read(void *szBuff, unsigned int bufferSize);
    bool GetAck(void *szBuff, unsigned int *bytes_rxed);
    int Detect();
    bool Abort();
    bool IsCorrectProtocol(void *szBuff, unsigned int *bytes_rxed);
    bool SetUtilityInstance(void *UtilityInstance);
    void GetUsbsn(char * usbsn);
    bool IsSupportedDevice(unsigned short vendorid, unsigned short productid);
private:
    IDevice *PhysicalTransportDevice;
#if defined XFSTK_OS_WIN
    IDeviceFactory *deviceFactory;
#endif
};

#ifdef XFSTK_OS_WIN

#else
class MedfieldEmmcUSB20Device : public IDevice
{
public:
    MedfieldEmmcUSB20Device();
    ~MedfieldEmmcUSB20Device();
    int GetNumberAvailableDevices();
#if defined XFSTK_OS_WIN
    int GetNumberAvailableDevices(DeviceInfos * socdevices);
#endif
    void* GetDeviceHandle(int DeviceNumber);
    bool SetDeviceHandle(void *DeviceHandle);
    bool SetTransport(DeviceTransportType ) {return true;}
    bool Open();
    bool Init();
    bool Write(void *pBuffer, unsigned int bufferSize);
    bool Read(void *szBuff, unsigned int bufferSize);
    bool GetAck(void *szBuff, unsigned int *bytes_rxed);
    int Detect();
    bool Abort();
    bool IsCorrectProtocol(void *szBuff, unsigned int *bytes_rxed);
    bool SetUtilityInstance(void *UtilityInstance);
    void GetUsbsn(char * usbsn);
    bool IsSupportedDevice(unsigned short vendorid, unsigned short productid);
private:
    bool VerifyDeviceHandle(struct usb_device* Handle);
    void* CurrentDeviceHandle;
    EmmcUtils *libutils;
    static const int USBTIMEOUT = 5000;
    struct usb_bus *bus;
    struct usb_device *dev;
    struct usb_device *dev_port;
    struct usb_device *dev_extern_mt;
    struct usb_bus *busses;
    int target;
    int numinits;
    int reqnuminits;
    void FindDevice(void);
    usb_dev_handle *dev_handle;
    unsigned int in_pipe;
    unsigned int out_pipe;
    unsigned int pkt_size;
    char usbsn[USBSN_BUFFER_SIZE];
    unsigned short id_vendor;
    unsigned short id_product;
};
#endif

class MedfieldEmmcSerialDevice : public IDevice
{
public:
    MedfieldEmmcSerialDevice();
    int GetNumberAvailableDevices();
#if defined XFSTK_OS_WIN
    int GetNumberAvailableDevices(DeviceInfos * socdevices){ Q_UNUSED(socdevices); return 0;}
#endif
    void* GetDeviceHandle(int DeviceNumber);
    bool SetDeviceHandle(void *DeviceHandle);
    bool SetTransport(DeviceTransportType ) {return true;}
    bool Open();
    bool Init();
    bool Write(void *pBuffer, unsigned int bufferSize);
    bool Read(void *szBuff, unsigned int bufferSize);
    bool GetAck(void *szBuff, unsigned int *bytes_rxed);
    int Detect();
    bool Abort();
    bool IsCorrectProtocol(void *szBuff, unsigned int *bytes_rxed);
    bool SetUtilityInstance(void *UtilityInstance);
    void GetUsbsn(char * usbsn);
    bool IsSupportedDevice(unsigned short vendorid, unsigned short productid);
private:
    void* CurrentDeviceHandle;
    unsigned short id_vendor;
    unsigned short id_product;
};


#endif // MEDFIELDEMMCDEVICE_H
