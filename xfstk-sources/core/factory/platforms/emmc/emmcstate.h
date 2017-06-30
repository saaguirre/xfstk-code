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
#ifndef EMMCSTATE_H
#define EMMCSTATE_H
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include <string.h>
#include "emmcutils.h"
#include "../../common/xfstktypes.h"
#include "../../interfaces/ivisitor.h"
#include "../../interfaces/idevice.h"
#include "../../interfaces/idownloader.h"
#include "emmchandler.h"
#include "emmcoptions.h"
#include "../cloverview/cloverviewdevice.h"
#include "../cloverview/cloverviewutils.h"
#include "../cloverviewplus/cloverviewplusutils.h"
#include "../cloverviewplus/cloverviewplusdevice.h"
#include "../medfield/medfieldutils.h"
#include "../medfield/medfielddevice.h"
#include "../merrifield/merrifielddevice.h"
#include "../merrifield/merrifieldutils.h"
#include "emmcfw.h"
using namespace std;

//Dldr state defines, make it similiar to ach code
#define DLDR_STATE_INVALID     0x5354494EULL //"STIN"
#define DLDR_STATE_FW_NORMAL   0x5354464EULL //"STFN"
#define DLDR_STATE_FW_MISC     0x5354464DULL //"STFM"  //For none virgin eMMC
#define DLDR_STATE_OS_NORMAL   0x53544f4EULL //"STON"  //For none virgin eMMC
#define DLDR_STATE_OS_MISC     0x53544f4DULL //"STOM"  //For none virgin eMMC//For misc dnx
#define DLDR_STATE_FW_WIPE     0x53544657ULL //"STFW"  //For wipe out eMMC to make it like a virgin emmc

#define DLDR_FW_TOTAL_STEPS_EMMC 23

#define EMMC_DUMP_STATE_TRANSFER 0x53545458ULL //"STTX"

typedef std::map<unsigned long long, int> ErrorMapType;
typedef std::map<unsigned long long, IBaseVisitable<> *> HandleMapType;

class EmmcDumpState: public IVisitor
        , public Visitor<StEmmcDumpTransfer>
        , public Visitor<EmmcDump$ACK>
        , public Visitor<EmmcDumpNACK>
        , public Visitor<EmmcDumpECSD>
        , public Visitor<EmmcDumpREQB>
        , public Visitor<EmmcDumpEOIO>
        , public Visitor<EmmcDumpFwHandleDxxM>
        , public Visitor<EmmcDumpStHandleFwMisc>
        , public Visitor<EmmcDumpFwHandleDXBL>
        , public Visitor<EmmcDumpRDY$>
        , public Visitor<EmmcDumpFwHandleHLT$>
        , public Visitor<EmmcDumpER40>


{
public:
    EmmcDumpState();
    ~EmmcDumpState();

    bool Init(IDevice * usbdev, EmmcUtils* utils);
    bool DoUpdate(char* fname_dnx_misc, unsigned long usbdelayms);
    bool GetOsStatus();
    bool GetFwStatus();
    bool GetLastError(last_error* er);
    bool IsProvisionFailed();
    bool IsGPPReset();
    void SetOptions(EmmcOptions *Options);
    void SetUtils(EmmcOptions *Options);

    virtual void Visit(StEmmcDumpTransfer& hdlr);
    virtual void Visit(EmmcDump$ACK& hdlr);
    virtual void Visit(EmmcDumpNACK& hdlr);
    virtual void Visit(EmmcDumpECSD& hdlr);
    virtual void Visit(EmmcDumpREQB& hdlr);
    virtual void Visit(EmmcDumpEOIO& hdlr);
    virtual void Visit(EmmcDumpFwHandleDxxM& hdlr);
    virtual void Visit(EmmcDumpStHandleFwMisc& hdlr);
    virtual void Visit(EmmcDumpFwHandleDXBL& hdlr);
    virtual void Visit(EmmcDumpRDY$& hdlr);
    virtual void Visit(EmmcDumpFwHandleHLT$& hdlr);
    virtual void Visit(EmmcDumpER40& hldr);


private:
    bool doRegisterToken();
    void transferComplete();
    bool WriteOutPipe(unsigned char* pbuf, uint32 size);
    bool ReadInPipe(unsigned char* pbuf, uint32 size);
    unsigned long long ReadInAck();
    bool StartFw();
    bool FwDXBL();
    void CheckFile(char *filename);
    void LogError(int errorcode);
    void LogError(int errorcode, std::string msg);
    void StartLogTime();
    void EndlogTime();
    void LogProgress();
    void SleepMs(int delay);

    bool StateMachine();

    bool Start();
    unsigned long long GetOpCode();
    void GotoState(unsigned long long state);
    bool UsbDevInit();

    EmmcFW* m_mfld_fw;
    char* m_fname_dnx_fw;
    char* m_fname_fw_image;
    char* m_fname_dnx_misc;
    char* m_fname_dnx_os;
    char* m_fname_os_image;
    unsigned long m_gpflags;
    EmmcOptions *DeviceSpecificOptions;
    IDevice * m_usbdev;
    EmmcUtils* m_utils;
    bool m_b_usbinitok;
    unsigned long long  m_achCode;

    //Calculations for iCache FW
    unsigned long m_numChunks;
    //keep a static copy of iCache 128KB chunks
    unsigned long m_iCacheNum128KBChunks;
    unsigned long m_residualBytes;

    //Calculations for Resident FW
    unsigned long m_numResidentChunks;
    //keep a static copy of Resident 128KB chunks
    unsigned long m_residentNum128KBChunks;
    unsigned long m_residualResidentBytes;

    //Calculations for Extended FW
    unsigned long m_numExtChunks;
    //keep a static copy of Ext. FW 128KB chunks
    unsigned long m_extFWNum128KBChunks;
    unsigned long m_residualExtBytes;

    //Calculations for Rom Patch
    unsigned long m_numRomPatchChunks;
    unsigned long m_romPatch128KBChunks;
    unsigned long m_residualRomPatchBytes;

    //Calculations for VED FW
    unsigned long m_numVedFwChunks;
    unsigned long m_vedFw128KBChunks;
    unsigned long m_residualVedFwBytes;

    //Keep track the buffer position pointer
    unsigned long m_secFwArrayPtr;
    unsigned long m_sec2FwArrayPtr;
    unsigned long m_sec3FwArrayPtr;
    unsigned long m_romPatchArrayPtr;
    unsigned long m_vedFwArrayPtr;

    int m_fwprogress;
    float m_fwstepsdone;
    float m_fwtotalsteps;
    bool m_b_IDRQ;
    unsigned long  m_b_DnX_OS;

    bool m_fw_done;
    bool m_abort;
    bool m_GPPReset;
    bool m_state_change;

    unsigned long long m_dldr_state;

    bool m_ROSIP_data_flag;
    unsigned long m_delay_ms;

    last_error m_last_error;

    HandleMapType m_bulk_ack_map;
    ErrorMapType m_fw_error_map;

    EmmcUtils   libutils;
    EmmcDumpState(const EmmcDumpState&);
    EmmcDumpState& operator=(const EmmcDumpState&);

};

#endif //EMMCSTATE_H
