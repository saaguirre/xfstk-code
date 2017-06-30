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
#include "emmcdownloader.h"
#include <cstdio>
#include <vector>


#if defined XFSTK_OS_WIN
#include <windows.h>
#endif

#include "../../common/xfstktypes.h"

using namespace std;
extern CPSTR Emmc_error_code_array [MAX_ERROR_CODE];  

EmmcDownloader::EmmcDownloader(unsigned long Device):
    CurrentDownloaderDevice(NULL),
    b_DnX_OS(0),
    b_provisionfailed(false),
    b_provisionhasstarted(false),
    CurrentDownloaderOptions(NULL),
    DeviceSpecificOptions(NULL),
    abort(false),
    devicetype(Device)
{
    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
}
bool EmmcDownloader::SetOptions(IOptions *options)
{
    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    bool RetVal = false;
    if(options != NULL) {
        this->CurrentDownloaderOptions = options;
        this->DeviceSpecificOptions = (EmmcOptions *) options;
        if(this->DeviceSpecificOptions->IsVerbose()) {
            this->SetDebugLevel(this->DeviceSpecificOptions->GetDebugLevel());
        }
        RetVal = true;
    }
    return RetVal;
}

bool EmmcDownloader::SetDevice(IGenericDevice *device)
{
    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    bool RetVal = false;
    if(device != NULL && this->CurrentDownloaderOptions !=NULL) {
        this->CurrentDownloaderDevice = (IDevice *) device;
        RetVal = this->SetUtilityInstance();
    }
    return RetVal;
}

bool EmmcDownloader::SetUtilityInstance()
{
    int retval;

    switch(this->devicetype)
    {
         case MEDFIELD :

            retval = this->CurrentDownloaderDevice->SetUtilityInstance((void*) (&(this->mfdUtils)));

            break;
         case CLOVERVIEW  :
            retval = this->CurrentDownloaderDevice->SetUtilityInstance((void*) (&(this->clvutils)));


            break;
         case CLOVERVIEWPLUS :
            retval = this->CurrentDownloaderDevice->SetUtilityInstance((void*) (&(this->clputils)));


            break;

         case MERRIFIELD      :
            retval = this->CurrentDownloaderDevice->SetUtilityInstance((void*) (&(this->mrfUtils)));

            break;
         default:
            retval = this->CurrentDownloaderDevice->SetUtilityInstance((void*) (&(this->libutils)));
            break;

    }

    return retval;
}

bool EmmcDownloader::UpdateTarget()
{
    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    bool RetVal = false;
    if(this->CurrentDownloaderOptions == NULL ||
            this->CurrentDownloaderDevice == NULL) {
        return RetVal;
    }

    this->Init();

    if(!this->b_usbinitok)
        return false;
    this->do_update(this->DeviceSpecificOptions);
    this->libutils.u_log(LOG_STATUS, "EMMC-DUMP: Download completed.");
    return true;
}

bool EmmcDownloader::GetStatus()
{
    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    if(this->CurrentDownloaderOptions == NULL ||
       this->CurrentDownloaderDevice == NULL) {
        return 0x0;
    }
    return !(this->b_provisionfailed);
}

bool EmmcDownloader::GetLastError(last_error* er)
{
    return m_dldr_state.GetLastError(er);
}

bool EmmcDownloader::SetDebugLevel(unsigned long DebugLevel)
{
    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    if(DebugLevel > 0) {
        this->libutils.isDebug = DebugLevel;
    }
    return true;
}

bool EmmcDownloader::SetStatusCallback(void(*StatusPfn)(char* Status, void* ClientData), void* ClientData)
{
    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    this->libutils.u_setstatuspfn(StatusPfn,ClientData);
    return true;
}

bool EmmcDownloader::Cleanup()
{
    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    return true;
}
void EmmcDownloader::Init()
{
    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    b_usbinitok = false;
    bool result = true;
    result = this->CurrentDownloaderDevice->Open();
    this->b_usbinitok = result;
}
void EmmcDownloader::ReInit()
{
    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    this->CurrentDownloaderDevice->Init();
    this->CurrentDownloaderDevice->Open();
#if defined XFSTK_OS_WIN
    Sleep(1000);
#else
    sleep(1);
#endif
    this->CurrentDownloaderDevice->Init();
    this->CurrentDownloaderDevice->Open();
}

void EmmcDownloader::do_abort()
{
    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    this->CurrentDownloaderDevice->Abort();
}

void EmmcDownloader::cleanup()
{
    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
}

void EmmcDownloader::do_update(EmmcOptions* options)
{
    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    m_dldr_state.Init(this->CurrentDownloaderDevice, &this->libutils);
    m_dldr_state.SetOptions(options);
    m_dldr_state.DoUpdate((PSTR)options->GetMiscDnxPath(),
                          options->GetUsbdelayms());
    this->do_abort();
}
