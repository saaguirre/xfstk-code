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
#ifndef CLOVERVIEWDOWNLOADER_H
#define CLOVERVIEWDOWNLOADER_H
#include "../../interfaces/idownloader.h"
#include "cloverviewoptions.h"
#include "cloverviewmessages.h"
#include "cloverviewutils.h"
#include "clvdldrstate.h"

class CloverviewDownloader : public IDownloader
{
public:
    CloverviewDownloader();
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
    CloverviewUtils   libutils;

    ClvDldrState m_dldr_state;

    unsigned int    b_DnX_OS;
    int b_osdone;
    int b_fwdone;
    int b_provisionfailed;
    bool b_provisionhasstarted;
    int b_continue_to_OS;
    int b_IDRQ;
    IOptions *CurrentDownloaderOptions;
    CloverviewOptions *DeviceSpecificOptions;
    void do_update(CloverviewOptions* options);
    void do_update(
        char * dnx_fw_name,
        char * fw_image_name,
        char * dnx_os_name,
        char * os_image_name,
        char * dnx_misc_name,
        unsigned int gpflags
        );
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

#endif // CLOVERVIEWDOWNLOADER_H
