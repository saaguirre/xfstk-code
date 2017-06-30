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
#ifndef CLOVERVIEWPLUSDEVICE_H
#define CLOVERVIEWPLUSDEVICE_H
#include "../../interfaces/idevice.h"
#include "cloverviewplusutils.h"
#include <stdio.h>
#include <string>
#include <memory.h>

#ifdef XFSTK_OS_WIN
#include "../../interfaces/idevicefactory.h"
#include <wddevicefactory.h>
#define CLOVERVIEWPLUS_DEVICE_ID 0x4
#else
#include <usb.h>
#if 0
#include <libusb.h>
#endif
#endif

#define INTEL_VENDOR_ID   0x8086
#define CLOVERVIEWPLUS_PRODUCT_ID  0x09EE
#define USBSN_BUFFER_SIZE 128
#define MAX_SOC_DEVICES 32

class CloverviewPlusDevice : public IDevice
{
public:
    CloverviewPlusDevice();
    ~CloverviewPlusDevice();
    int GetNumberAvailableDevices();
    int GetNumberAvailableDevices(DeviceInfos * socdevices);
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

	CloverviewPlusDevice( const CloverviewPlusDevice& );
	CloverviewPlusDevice& operator=( const CloverviewPlusDevice& );
};

#ifdef XFSTK_OS_WIN

#else
class CloverviewPlusUSB20Device : public IDevice
{
public:
    CloverviewPlusUSB20Device();
    ~CloverviewPlusUSB20Device();
    int GetNumberAvailableDevices();
    int GetNumberAvailableDevices(DeviceInfos * socdevices);
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
    CloverviewPlusUtils *libutils;
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

class CloverviewPlusSerialDevice : public IDevice
{
public:
    CloverviewPlusSerialDevice();
    int GetNumberAvailableDevices();
    int GetNumberAvailableDevices(DeviceInfos * socdevices);
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


#endif // CLOVERVIEWPLUSDEVICE_H
