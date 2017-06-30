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
#ifndef IDEVICE_H
#define IDEVICE_H

enum DeviceTransportType {
    XFSTK_USB20 = 1,
    XFSTK_WD_USB20 = 2,
    XFSTK_SERIAL = 3,
    XFSTK_SOCKET = 4,
    XFSTK_ITP = 5,
    XFSTK_USB30 = 6,
    XFSTK_WD_USB30 = 7,
    XFSTK_DEDI_PROG = 8,
    XFSTK_NOT_SPECIFIED = 666
};

struct DeviceInfo {
    char usbsn[128];
    unsigned long root_hub;
    unsigned long port[5];

};
struct DeviceInfos {
    int number_devices;
    DeviceInfo soc_device[32];
};

#ifdef XFSTK_OS_WIN
typedef struct DeviceCallBack
{
    DeviceInfos* device;
    void (*attach)(void* data);
    void (*detach)(void* data);
    void * clientdata;
}DeviceCallBack;
#endif

class IGenericDevice
{
public:
    virtual ~IGenericDevice() {}
    virtual int GetNumberAvailableDevices() = 0;
    virtual int GetNumberAvailableDevices(DeviceInfos * socdevices) = 0;
    virtual void* GetDeviceHandle(int DeviceNumber) = 0;
    virtual bool SetDeviceHandle(void *DeviceHandle) = 0;
    virtual bool SetTransport(DeviceTransportType Transport) = 0;
    virtual void GetUsbsn(char * usbsn) = 0;
    virtual int SetUsbTimeoutDelay(int timeoutMS) = 0;
private:
};

class IDevice : public IGenericDevice
{
public:
    virtual ~IDevice() {}
    virtual bool Open() = 0;
    virtual bool Init() = 0;
    virtual bool Write(void *pBuffer, unsigned int bufferSize) = 0;
    virtual bool Read(void *pBuffer, unsigned int bufferSize) = 0;
    virtual bool GetAck(void *szBuff, unsigned int *bytes_rxed) = 0;
    virtual int Detect() = 0;
    virtual bool Abort() = 0;
    virtual bool IsCorrectProtocol(void *szBuff, unsigned int *bytes_rxed) = 0;
    virtual bool SetUtilityInstance(void *UtilityInstance) = 0;
    virtual bool IsSupportedDevice(unsigned short vendorid, unsigned short productid) = 0;
};

#endif // IDEVICE_H
