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
#ifndef EMMCDOWNLOADER_H
#define EMMCDOWNLOADER_H
#include "../../interfaces/idownloader.h"
#include "emmcoptions.h"
#include "emmcmessages.h"
#include "emmcutils.h"
#include "emmcstate.h"
#include "../cloverview/cloverviewdevice.h"
#include "../cloverview/cloverviewutils.h"
#include "../cloverviewplus/cloverviewplusutils.h"
#include "../cloverviewplus/cloverviewplusdevice.h"
#include "../medfield/medfieldutils.h"
#include "../medfield/medfielddevice.h"
#include "../merrifield/merrifielddevice.h"
#include "../merrifield/merrifieldutils.h"

#define MOORESTOWN          0x1
#define MEDFIELD            0x2
#define CLOVERVIEW          0x3
#define CLOVERVIEWPLUS      0x4
#define MERRIFIELD          0x5
#define EMMC_DUMP           0x6
#define HASHVERIFY          0x7
#define MOOREFIELD          0x8
#define BAYTRAIL            0x9
#define XFSTK_MAXDEVICE     0xB
#define XFSTK_NODEVICE      0x666

class EmmcDownloader : public IDownloader
{
public:
    EmmcDownloader(unsigned long Device);
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
    //USBDevice       usb_dev;
    IDevice *CurrentDownloaderDevice;


    EmmcDumpState m_dldr_state;

    unsigned int    b_DnX_OS;
    int b_osdone;
    int b_fwdone;
    int b_provisionfailed;
    bool b_provisionhasstarted;
    int b_continue_to_OS;
    int b_IDRQ;
    IOptions *CurrentDownloaderOptions;
    EmmcOptions *DeviceSpecificOptions;
    void do_update(EmmcOptions* options);
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

    unsigned long devicetype;
    EmmcUtils   libutils;
    CloverviewPlusUtils clputils;
    CloverviewUtils clvutils;
    MedfieldUtils mfdUtils;
    MerrifieldUtils mrfUtils;
    bool SetUtilityInstance();

#pragma pack(push,1)
typedef struct{
    unsigned short  cmd;
    unsigned short  status;
    unsigned long   data;
} mfd_pkt_t;
#pragma pack(pop)
};

#endif // EMMCDOWNLOADER_H
