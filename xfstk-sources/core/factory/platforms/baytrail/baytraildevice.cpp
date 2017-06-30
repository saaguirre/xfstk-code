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
#include "baytraildevice.h"
#include "../../common/xfstktypes.h"
#include "../../common/xfstkcommon.h"
extern IntelSoCGlobalDeviceState IntelSoCGlobalDeviceCurrentState;
extern CPSTR Baytrail_error_code_array [BAYTRAIL_MAX_ERROR_CODE];

BaytrailDevice::BaytrailDevice(QObject *parent) :
                               QObject(parent)
{
#ifdef XFSTK_OS_WIN
    deviceFactory = NULL;
#endif
    this->PhysicalTransportDevice = NULL;
    device_process = new QProcess(this);
    if(!device_process)
    {
        this->m_last_error.error_code = 12;
        strcpy(this->m_last_error.error_message, Baytrail_error_code_array[12]);
    }
}

BaytrailDevice::~BaytrailDevice()
{
    if(this->PhysicalTransportDevice != NULL)
    {
        delete this->PhysicalTransportDevice;
        this->PhysicalTransportDevice = NULL;
    }
    if(device_process)
    {
        delete device_process;
        device_process = NULL;
    }
#ifdef XFSTK_OS_WIN
    if(deviceFactory)
    {
        delete deviceFactory;
        deviceFactory = NULL;
    }
#endif
}

#if defined XFSTK_OS_WIN
int BaytrailDevice::GetNumberAvailableDevices()
{
    int NumDevs = 0;
    // Have DediProg detect flash chip for identification
    QString cmdline = "dpcmd.exe -i --vcc 2 --detect";

    // Execute the dedi prog downloader now
    device_process->start(cmdline);
    if(!device_process->waitForStarted(5000)) //wait up to 5 seconds to start
    {
        this->libutils.u_log(LOG_STATUS, "dpcmd.exe failed to start. Check DediProg's installation");
        this->m_last_error.error_code = 0xBAADF00D; //will retry 20 times in enumeration loop
        strcpy(this->m_last_error.error_message, "dpcmd.exe failed to start. Check DediProg's installation");
        return 0; // retrun zero devices available
    }
    device_process->waitForFinished();
    QByteArray outData = device_process->readAllStandardOutput();

    int retCode = device_process->exitCode();
    switch(retCode)
    {
    case 0:
        if(outData.contains("W25Q64DW"))
        {
            this->libutils.u_log( LOG_STATUS, "Detected SPI Flash W25Q64DW");

            NumDevs = 1;
        }
        else
        {
            this->libutils.u_log(LOG_STATUS, "No SPI Flash W25Q64DW detected");
            NumDevs = 0;
        }
        break;
    case 99:
        if(outData.contains("not connected"))
        {
            this->libutils.u_log( LOG_STATUS, "SPI SW detected, but program cable not connected");

        }
        else
        {
            this->libutils.u_log( LOG_STATUS, "SPI SW detected but completed with unknown error");

        }
        NumDevs = 0;
        break;
    default:
        this->libutils.u_log( LOG_STATUS, "SPI device not detected.");
        NumDevs = 0;
        break;
    }
    return NumDevs;
    }
#else
int BaytrailDevice::GetNumberAvailableDevices()
{
    int NumDevs = 0;

     QString cmdline = "flashrom -p dediprog:1.8V";
    // Execute the dedi prog downloader now
    device_process->start(cmdline);
    if(!device_process->waitForStarted(5000)) //wait up to 5 seconds to start
    {
        this->libutils.u_log(LOG_STATUS, "flashrom failed to start. Check flashrom's installation");
        this->m_last_error.error_code = 0xBAADF00D; //will retry 20 times in enumeration loop
        strcpy(this->m_last_error.error_message, "flashrom failed to start. Check flashrom's installation");
        return 0; // retrun zero devices available
    }
    device_process->waitForFinished();
    QByteArray outData = device_process->readAllStandardOutput();

    if(outData.contains("W25Q64.W"))
    {
        this->libutils.u_log( LOG_STATUS, "Detected SPI Flash W25Q64DW");
        NumDevs = 1;
    }
    else
    {
        this->libutils.u_log(LOG_STATUS, "No SPI Flash W25Q64DW detected, check USB cable and ribbon cable");
        NumDevs = 0;
     }

    return NumDevs;
}
#endif

int BaytrailDevice::GetNumberAvailableDevices(DeviceInfos* )
{
    int RetVal = 0;
    return RetVal;
}

bool BaytrailDevice::SetUtilityInstance(void *)
{
    return false;
}

void* BaytrailDevice::GetDeviceHandle(int )
{
    void *RetVal = NULL;
    return RetVal;
}
bool BaytrailDevice::SetDeviceHandle(void *)
{
    bool RetVal = true;
    return RetVal;
}

bool BaytrailDevice::SetTransport(DeviceTransportType Transport)
{
    bool RetVal = false;
    switch(Transport)
    {
        // nothing to setup on DedgiProg SF100
        case XFSTK_DEDI_PROG:
            RetVal = true;
            break;
        case XFSTK_USB20:
        case XFSTK_WD_USB20:
        case XFSTK_SERIAL:
        case XFSTK_SOCKET:
        case XFSTK_ITP:
        case XFSTK_USB30:
        case XFSTK_WD_USB30:
        case XFSTK_NOT_SPECIFIED:
        default:
            this->m_last_error.error_code = 13;
            strcpy(this->m_last_error.error_message, Baytrail_error_code_array[13]);
            RetVal = false;
            break;
    }
    return RetVal;
}

bool BaytrailDevice::Open()
{

    return false;
}

bool BaytrailDevice::Init()
{

    return false;
}

bool BaytrailDevice::Abort()
{

    return false;
}

bool BaytrailDevice::Write(void *, unsigned int )
{

    return false;
}

bool BaytrailDevice::Read(void *, unsigned int )
{

    return false;
}

bool BaytrailDevice::GetAck(void *, unsigned int *)
{

    return false;
}

int BaytrailDevice::Detect()
{
  ;
    return 0;
}

bool BaytrailDevice::IsCorrectProtocol(void *, unsigned int *)
{

    return false;
}

bool BaytrailDevice::IsSupportedDevice(unsigned short , unsigned short )
{
    return false;
}

void BaytrailDevice::GetUsbsn(char * usbsn)
{
    this->PhysicalTransportDevice->GetUsbsn(usbsn);
}
