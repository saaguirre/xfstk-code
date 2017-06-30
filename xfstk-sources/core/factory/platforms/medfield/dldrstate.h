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
#ifndef DLDRSTATE_H
#define DLDRSTATE_H
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include <string.h>
#include "medfieldutils.h"
#include "../../common/xfstktypes.h"
#include "../../interfaces/ivisitor.h"
#include "medfieldfw.h"
#include "medfieldos.h"
#include "../../interfaces/idevice.h"
#include "../../interfaces/idownloader.h"
#include "dldrhandler.h"

using namespace std;

//Dldr state defines, make it similiar to ach code
#define DLDR_STATE_INVALID     0x5354494EULL //"STIN"
#define DLDR_STATE_FW_NORMAL   0x5354464EULL //"STFN"
#define DLDR_STATE_FW_MISC     0x5354464DULL //"STFM"  //For none virgin eMMC
#define DLDR_STATE_OS_NORMAL   0x53544f4EULL //"STON"  //For none virgin eMMC
#define DLDR_STATE_OS_MISC     0x53544f4DULL //"STOM"  //For none virgin eMMC//For misc dnx
#define DLDR_STATE_FW_WIPE     0x53544657ULL //"STFW"  //For wipe out eMMC to make it like a virgin emmc
#define DLDR_STATE_BHC_ERRR    0x53544243ULL //"STBC"  //For Battery Health Check Error

#define DLDR_FW_TOTAL_STEPS_MFD 25

typedef std::map<unsigned long long, int> ErrorMapType;
typedef std::map<unsigned long long, IBaseVisitable<> *> HandleMapType;

class DldrState: public IVisitor
        , public Visitor<FwHandleER00>
        , public Visitor<FwHandleHLT0>
        , public Visitor<FwHandleDFRM>
        , public Visitor<FwHandleDxxM>
        , public Visitor<FwHandleDXBL>
        , public Visitor<FwHandleRUPHS>
        , public Visitor<FwHandleRUPH>
        , public Visitor<FwHandleDMIP>
        , public Visitor<FwHandleLOFW>
        , public Visitor<FwHandleHIFW>
        , public Visitor<FwHandlePSFW1>
        , public Visitor<FwHandlePSFW2>
        , public Visitor<FwHandleSSFW>
        , public Visitor<FwHandleHLT$>
        , public Visitor<FwHandleMFLD>
        , public Visitor<FwHandleCLVT>
        , public Visitor<FwHandleSuCP>
        , public Visitor<FwHandleRTBD>
        , public Visitor<FwHandleVEDFW>
        , public Visitor<FwHandleSSBIOS>
        , public Visitor<FwHandleIFW1>
        , public Visitor<FwHandleIFW2>
        , public Visitor<FwHandleIFW3>
        , public Visitor<FwHandleRESET>
        , public Visitor<OsHandleDORM>
        , public Visitor<OsHandleOSIPSZ>
        , public Visitor<OsHandleROSIP>
        , public Visitor<OsHandleDONE>
        , public Visitor<OsHandleRIMG>
        , public Visitor<OsHandleEOIU>
        , public Visitor<StHandleFwNormal>
        , public Visitor<StHandleFwMisc>
        , public Visitor<StHandleFwWipe>
        , public Visitor<StHandleOsNormal>
        , public Visitor<StHandleOsMisc>
        , public Visitor<ErHandleLogError>
        , public Visitor<ErHandleERRR>
{
public:
    DldrState();
    ~DldrState();

    bool Init(IDevice * usbdev, MedfieldUtils* utils);
    bool DoUpdate(char* fname_dnx_fw, char* fname_fw_image, char* fname_dnx_os, \
                  char* fname_os_image, char* fname_dnx_misc, unsigned long gpflags, \
				  unsigned long usbdelayms, bool ifwiwipeenable);
    bool GetOsStatus();
    bool GetFwStatus();
    bool GetIFWIStatus();
    bool GetLastError(last_error* er);
    bool IsProvisionFailed();
    bool IsGPPReset();
    bool IsOsOnly();
    bool IsFwState();
    bool IsOsState();

    virtual void Visit(FwHandleER00& hdlr);
    virtual void Visit(FwHandleHLT0& hdlr);
    virtual void Visit(FwHandleDFRM& hdlr);
    virtual void Visit(FwHandleDxxM& hdlr);
    virtual void Visit(FwHandleDXBL& hdlr);
    virtual void Visit(FwHandleRUPHS& hdlr);
    virtual void Visit(FwHandleRUPH& hdlr);
    virtual void Visit(FwHandleDMIP& hdlr);
    virtual void Visit(FwHandleLOFW& hdlr);
    virtual void Visit(FwHandleHIFW& hdlr);
    virtual void Visit(FwHandlePSFW1& hdlr);
    virtual void Visit(FwHandlePSFW2& hdlr);
    virtual void Visit(FwHandleSSFW& hdlr);
    virtual void Visit(FwHandleHLT$& hdlr);
    virtual void Visit(FwHandleMFLD& hdlr);
    virtual void Visit(FwHandleCLVT& hdlr);
    virtual void Visit(FwHandleSuCP& hdlr);
    virtual void Visit(FwHandleRTBD& hdlr);
    virtual void Visit(FwHandleVEDFW& hdlr);
    virtual void Visit(FwHandleSSBIOS& hdlr);
    virtual void Visit(FwHandleIFW1& hdlr);
    virtual void Visit(FwHandleIFW2& hdlr);
    virtual void Visit(FwHandleIFW3& hdlr);
    virtual void Visit(FwHandleRESET& hdlr);
    virtual void Visit(OsHandleDORM& hdlr);
    virtual void Visit(OsHandleOSIPSZ& hdlr);
    virtual void Visit(OsHandleROSIP& hdlr);
    virtual void Visit(OsHandleDONE& hdlr);
    virtual void Visit(OsHandleRIMG& hdlr);
    virtual void Visit(OsHandleEOIU& hdlr);
    virtual void Visit(StHandleFwNormal& hdlr);
    virtual void Visit(StHandleFwMisc& hdlr);
    virtual void Visit(StHandleFwWipe& hdlr);
    virtual void Visit(StHandleOsNormal& hdlr);
    virtual void Visit(StHandleOsMisc& hdlr);
    virtual void Visit(ErHandleLogError& hdlr);
    virtual void Visit(ErHandleERRR& hdlr);

private:
    bool WriteOutPipe(unsigned char* pbuf, uint32 size);
    bool ReadInPipe(unsigned char* pbuf, uint32 size);
    unsigned long long ReadInAck();
    bool StartFw();
    bool StartOs();
    bool HandleNoSize();
    void CheckFile(char *filename);
    bool FwDXBL();
    bool OsDXBL();
    void LogError(unsigned long errorcode);
    void StartLogTime();
    void EndlogTime();
    void LogProgress();
    void SleepMs(int delay);

    bool StateMachine();
    bool Start();
    unsigned long long GetOpCode();
    void GotoState(unsigned long long state);
    bool UsbDevInit();

    char* m_fname_dnx_fw;
    char* m_fname_fw_image;
    char* m_fname_dnx_misc;
    char* m_fname_dnx_os;
    char* m_fname_os_image;
    unsigned long m_gpflags;
    IDevice * m_usbdev;
    MedfieldUtils* m_utils;
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
    bool m_os_done;
    bool m_abort;
    bool m_GPPReset;
    bool m_state_change;
    bool m_ifwiwipe_enable;


    MedfieldFW* m_mfld_fw;
    MedfieldOS* m_mfld_os;

    unsigned long long m_dldr_state;

    bool m_ifwi_done;

    bool m_ROSIP_data_flag;
    unsigned long m_delay_ms;

    last_error m_last_error;

    HandleMapType m_bulk_ack_map;
    ErrorMapType m_fw_error_map;
    DldrState(const DldrState&);
    DldrState& operator=(const DldrState&);
};

#endif //DLDRSTATE_H
