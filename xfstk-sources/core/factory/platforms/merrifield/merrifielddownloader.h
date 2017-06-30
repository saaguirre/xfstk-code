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
#ifndef MERRIFIELDDOWNLOADER_H
#define MERRIFIELDDOWNLOADER_H
#include "../../interfaces/idownloader.h"
#include "merrifieldoptions.h"
#include "merrifieldmessages.h"
#include "merrifieldutils.h"
#include "mrfdldrstate.h"

#define OSIP_PARTITIONTABLE_SIZE    0x200

#if defined XFSTK_OS_WIN
#ifndef HAVE_STRUCT_TIMESPEC
#define HAVE_STRUCT_TIMESPEC 1
enum clockid_t {
    CLOCK_REALTIME,
    TIMER_ABSTIME,
    CLOCK_MONOTONIC
};

struct timespec {
    long tv_sec;
    long tv_nsec;
};

#endif /*HAVE_STRUCT_TIMESPEC*/
#endif

class MerrifieldDownloader : public IDownloader
{
public:
    MerrifieldDownloader();
    bool SetOptions(IOptions *options);
    bool SetDevice(IGenericDevice *device);
    bool UpdateTarget();
    bool GetStatus();
    bool GetLastError(last_error* er);
    bool Cleanup();
    bool SetStatusCallback(void(*StatusPfn)(char* Status, void* ClientData), void* ClientData);
    int GetResponse(unsigned char* buffer,int maxsize=0);
    MerrifieldOptions *getDeviceOptions(){return DeviceSpecificOptions;}

private:
    bool wehateblankbin;
    bool SetDebugLevel(unsigned long DebugLevel);
    float time_elapsed;
    void ReInit();
    void Init();
    int            write_out_pipe(unsigned char* pbuf, unsigned int size);
    int            read_in_pipe(unsigned char* pbuf, unsigned int size);
    unsigned long long int       read_in_ack();
    void            do_abort();
    void            _check_file(char *filename);
    void            _log_ack(unsigned long long int ackMsg);
    bool            bserialconfigured;
    bool            bUSBconfigured;
    bool            bsocketconfigured;
    bool            b_usbinitok;
    bool            abort;
    IDevice *CurrentDownloaderDevice;
    MerrifieldUtils   libutils;
    mrfdldrstate m_dldr_state;
    unsigned int    b_DnX_OS;
    int b_osdone;
    int b_fwdone;
    int b_provisionfailed;
    bool b_provisionhasstarted;
    int b_continue_to_OS;
    int b_IDRQ;
    IOptions *CurrentDownloaderOptions;
    MerrifieldOptions *DeviceSpecificOptions;
    void do_emmc_update(MerrifieldOptions* options);
    void do_update(MerrifieldOptions* options);
    int handle_no_size(unsigned int gpflags);
    int handle_fw_no_size(unsigned int gpflags);
    int firmware_upgrade_routine(char* dnx_fw, char* fw_image, unsigned int gpflags);
    int os_image_routine(char* dnx_os, unsigned int gpflags);
    int DnXOS_recovery_routine(char* dnx_os, unsigned int gpflags);
    unsigned int  get_Opp_Code();
    bool isosdone();
    bool isfwdone();
    void cleanup();
    int clock_gettime(clockid_t clk_id, struct timespec *res);

#pragma pack(push,1)
typedef struct{
    unsigned short  cmd;
    unsigned short  status;
    unsigned long   data;
} mfd_pkt_t;
#pragma pack(pop)
};

#endif // MerrifieldDOWNLOADER_H
