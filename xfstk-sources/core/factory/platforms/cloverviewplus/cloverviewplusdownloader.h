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
#ifndef CLOVERVIEWPLUSDOWNLOADER_H
#define CLOVERVIEWPLUSDOWNLOADER_H
#include "../../interfaces/idownloader.h"
#include "cloverviewplusoptions.h"
#include "cloverviewplusmessages.h"
#include "cloverviewplusutils.h"
#include "clvpdldrstate.h"

class CloverviewPlusDownloader : public IDownloader
{
public:
    CloverviewPlusDownloader();
    bool SetOptions(IOptions *options);
    bool SetDevice(IGenericDevice *device);
    bool UpdateTarget();
    bool GetStatus();
    bool GetLastError(last_error* er);
    bool Cleanup();
    bool SetStatusCallback(void(*StatusPfn)(char* Status, void* ClientData), void* ClientData);
    int GetResponse(unsigned char* ,int ){return false;}
private:
    bool wehateblankbin;
    bool SetDebugLevel(unsigned long DebugLevel);
    float time_elapsed;
    void ReInit();
    void Init();
    void            do_abort();
    bool            bserialconfigured;
    bool            bUSBconfigured;
    bool            bsocketconfigured;
    bool            b_usbinitok;
    IDevice *CurrentDownloaderDevice;
    CloverviewPlusUtils   libutils;
    ClvpDldrState m_dldr_state;
    unsigned int    b_DnX_OS;
    int b_osdone;
    int b_fwdone;
    int b_provisionfailed;
    bool b_provisionhasstarted;
    int b_continue_to_OS;
    int b_IDRQ;
    IOptions *CurrentDownloaderOptions;
    CloverviewPlusOptions *DeviceSpecificOptions;
    void do_emmc_update(CloverviewPlusOptions* options);
    void do_update(CloverviewPlusOptions* options);

    void cleanup();
    bool abort;

#pragma pack(push,1)
typedef struct{
    unsigned short  cmd;
    unsigned short  status;
    unsigned long   data;
} mfd_pkt_t;
#pragma pack(pop)
};

#endif // CLOVERVIEWPLUSDOWNLOADER_H
