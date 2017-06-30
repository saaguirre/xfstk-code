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
#ifndef MERRIFIELDDEVICE_H
#define MERRIFIELDDEVICE_H
#include "../../interfaces/idevice.h"
#include "merrifieldutils.h"
#include <stdio.h>
#include <string>
#include <memory.h>
#include <map>
#include <sstream>

#ifdef XFSTK_OS_WIN
#include "../../interfaces/idevicefactory.h"
#include <wddevicefactory.h>
#include "windows.h"
#define MERRIFIELD_DEVICE_ID 0x5
#define MOOREFIELD_DEVICE_ID 0x8
#define CARBONCANYON_DEVICE_ID 0xA
#else
#define MERRIFIELD_DEVICE_ID 0x5
#define MOOREFIELD_DEVICE_ID 0x8
#define CARBONCANYON_DEVICE_ID 0xA
#include <usb.h>
#if 0
#include <libusb.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
typedef void* HANDLE;
#endif

#define INTEL_VENDOR_ID   0x8086
#define MERRIFIELD_PRODUCT_ID  0xe005
#define MOOREFIELD_PRODUCT_ID  0x0A2C
#define CARBONCANYON_PRODUCT_ID 0x0A52
#define USBSN_BUFFER_SIZE 128
#define MAX_SOC_DEVICES 32

class MerrifieldDevice : public IDevice
{
public:
    MerrifieldDevice(unsigned long device=0);
    ~MerrifieldDevice();
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
    int SetUsbTimeoutDelay(int timeoutMS)\
    { return this->PhysicalTransportDevice->SetUsbTimeoutDelay(timeoutMS);}
    int serialComPort;

private:
    IDevice *PhysicalTransportDevice;
    unsigned long _device;
#if defined XFSTK_OS_WIN
    IDeviceFactory *deviceFactory;
#endif

	MerrifieldDevice( const MerrifieldDevice& );
	MerrifieldDevice& operator=( const MerrifieldDevice& );
};

#ifdef XFSTK_OS_WIN

#else
class MerrifieldUSB20Device : public IDevice
{
public:
    MerrifieldUSB20Device(unsigned long device=0);
    ~MerrifieldUSB20Device();
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
    int SetUsbTimeoutDelay(int timeoutMS){ USBTIMEOUT = timeoutMS; return USBTIMEOUT;}
private:
    bool VerifyDeviceHandle(struct usb_device* Handle);
    void* CurrentDeviceHandle;
    MerrifieldUtils *libutils;
    int USBTIMEOUT;
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
#if 0
class MerrifieldUSB30Device : public IDevice
{
public:
    MerrifieldUSB30Device(unsigned long device=0);
    ~MerrifieldUSB30Device();
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
    int SetUsbTimeoutDelay(int timeoutMS){ USBTIMEOUT = timeoutMS; return USBTIMEOUT;}
private:
    bool VerifyDeviceHandle(struct usb_device* Handle);
    void* CurrentDeviceHandle;
    MerrifieldUtils *libutils;
    int USBTIMEOUT;
    int target;
    int numinits;
    int reqnuminits;
    void FindDevice(void);
   libusb_device_handle *dev_handle;
   libusb_interface_descriptor interfaceDescript;
   libusb_device *dev;
   libusb_context *context;
    unsigned char in_pipe;
    unsigned char out_pipe;
    unsigned int pkt_size;
    char usbsn[USBSN_BUFFER_SIZE];
    unsigned short id_vendor;
    unsigned short id_product;
};
#endif

class MerrifieldSerialDevice : public IDevice
{
public:
    MerrifieldSerialDevice();
    int GetNumberAvailableDevices();
    int GetNumberAvailableDevices(DeviceInfos * ){return 0;}
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
    int SetUsbTimeoutDelay(int timeoutMS){ return timeoutMS;}
private:
    HANDLE CurrentDeviceHandle;
    MerrifieldUtils *libutils;
    unsigned short id_vendor;
    unsigned short id_product;
    int baudrate;
    HANDLE OpenCom(int port, int baud);
    void CloseCom(HANDLE handle);
    int ReadCom(unsigned char* strBf, int size, HANDLE handle);
    int WriteCom(unsigned char* strBf, int size, HANDLE handle);
    int serialPort;
    #ifndef XFSTK_OS_WIN
        struct termios newComSetting, oldComSetting;
        int comHandle;
    #endif
};
#endif // MerrifieldDEVICE_H
