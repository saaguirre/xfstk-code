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
#ifndef CLVPDLDRSTATE_H
#define CLVPDLDRSTATE_H
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include <string.h>
#include "cloverviewplusutils.h"
#include "../../common/xfstktypes.h"
#include "../../interfaces/ivisitor.h"
#include "cloverviewplusfw.h"
#include "cloverviewplusos.h"
#include "../../interfaces/idevice.h"
#include "../../interfaces/idownloader.h"
#include "clvpdldrhandler.h"
#include "../softfuses/softfusesdldrhandler.h"

using namespace std;


class softfusesFW;

//Dldr state defines, make it similiar to ach code
#define DLDR_STATE_INVALID     0x5354494EULL //"STIN"
#define DLDR_STATE_FW_NORMAL   0x5354464EULL //"STFN"
#define DLDR_STATE_FW_MISC     0x5354464DULL //"STFM"  //For none virgin eMMC
#define DLDR_STATE_OS_NORMAL   0x53544f4EULL //"STON"  //For none virgin eMMC
#define DLDR_STATE_OS_MISC     0x53544f4DULL //"STOM"  //For none virgin eMMC//For misc dnx
#define DLDR_STATE_FW_WIPE     0x53544657ULL //"STFW"  //For wipe out eMMC to make it like a virgin emmc
#define DLDR_STATE_SOFT_FUSES  0x53545346ULL //"STSF"
#define DLDR_STATE_BHC_ERRR    0x53544243ULL //"STBC"  //For Battery Health Check Error

#define DLDR_FW_TOTAL_STEPS_CLVP 23

typedef std::map<unsigned long long, int> ErrorMapType;
typedef std::map<unsigned long long, IBaseVisitable<> *> HandleMapType;

class ClvpDldrState: public IVisitor
        , public Visitor<ClvpFwHandleER00>
        , public Visitor<ClvpFwHandleHLT0>
        , public Visitor<ClvpFwHandleDSKF>
        , public Visitor<ClvpFwHandleDFRM>
        , public Visitor<ClvpFwHandleDxxM>
        , public Visitor<ClvpFwHandleDXBL>
        , public Visitor<ClvpFwHandleRUPHS>
        , public Visitor<ClvpFwHandleRSUPHS>
        , public Visitor<ClvpFwHandleRUPH>
        , public Visitor<ClvpFwHandleRSUPH>
        , public Visitor<ClvpFwHandleDMIP>
        , public Visitor<ClvpFwHandleLOFW>
        , public Visitor<ClvpFwHandleHIFW>
        , public Visitor<ClvpFwHandlePSFW1>
        , public Visitor<ClvpFwHandlePSFW2>
        , public Visitor<ClvpFwHandleSSFW>
        , public Visitor<ClvpFwHandleHLT$>
        , public Visitor<ClvpFwHandleMFLD>
        , public Visitor<ClvpFwHandleCLVT>
        , public Visitor<ClvpFwHandleSuCP>
        , public Visitor<ClvpFwHandleRTBD>
        , public Visitor<ClvpFwHandleVEDFW>
        , public Visitor<ClvpFwHandleSSBIOS>
        , public Visitor<ClvpFwHandleIFW1>
        , public Visitor<ClvpFwHandleIFW2>
        , public Visitor<ClvpFwHandleIFW3>
        , public Visitor<ClvpFwHandleRESET>
        , public Visitor<ClvpOsHandleDORM>
        , public Visitor<ClvpOsHandleOSIPSZ>
        , public Visitor<ClvpOsHandleROSIP>
        , public Visitor<ClvpOsHandleDONE>
        , public Visitor<ClvpOsHandleRIMG>
        , public Visitor<ClvpOsHandleEOIU>
        , public Visitor<ClvpStHandleFwNormal>
        , public Visitor<ClvpStHandleFwMisc>
        , public Visitor<ClvpStHandleFwWipe>
        , public Visitor<ClvpStHandleOsNormal>
        , public Visitor<ClvpStHandleOsMisc>
        , public Visitor<ClvpErHandleLogError>
        , public Visitor<ClvpErHandleERRR>
        , public Visitor<ClvpStHandleSoftFuses>

{
public:
    ClvpDldrState();
    ~ClvpDldrState();

    bool Init(IDevice * usbdev, CloverviewPlusUtils* utils);
    bool DoUpdate(char* fname_dnx_fw, char* fname_fw_image, char* fname_softfuses_bin, char* fname_dnx_os, \
                  char* fname_os_image, char* fname_dnx_misc, unsigned long gpflags, \
				  unsigned long usbdelayms, bool ifwiwipeenable);
    bool GetOsStatus();
    bool GetFwStatus();
    bool GetSFStatus();
    bool GetSFRetryStatus();
    bool GetIFWIStatus();
    bool GetLastError(last_error* er);
    bool IsProvisionFailed();
    bool IsGPPReset();
    bool IsOsOnly();
    bool IsFwState();
    bool IsOsState();

    virtual void Visit(ClvpFwHandleER00& hdlr);
    virtual void Visit(ClvpFwHandleHLT0& hdlr);
    virtual void Visit(ClvpFwHandleDSKF& hdlr);
    virtual void Visit(ClvpFwHandleDFRM& hdlr);
    virtual void Visit(ClvpFwHandleDxxM& hdlr);
    virtual void Visit(ClvpFwHandleDXBL& hdlr);
    virtual void Visit(ClvpFwHandleRUPHS& hdlr);
    virtual void Visit(ClvpFwHandleRSUPHS& hdlr);
    virtual void Visit(ClvpFwHandleRUPH& hdlr);
    virtual void Visit(ClvpFwHandleRSUPH& hdlr);
    virtual void Visit(ClvpFwHandleDMIP& hdlr);
    virtual void Visit(ClvpFwHandleLOFW& hdlr);
    virtual void Visit(ClvpFwHandleHIFW& hdlr);
    virtual void Visit(ClvpFwHandlePSFW1& hdlr);
    virtual void Visit(ClvpFwHandlePSFW2& hdlr);
    virtual void Visit(ClvpFwHandleSSFW& hdlr);
    virtual void Visit(ClvpFwHandleHLT$& hdlr);
    virtual void Visit(ClvpFwHandleMFLD& hdlr);
    virtual void Visit(ClvpFwHandleCLVT& hdlr);
    virtual void Visit(ClvpFwHandleSuCP& hdlr);
    virtual void Visit(ClvpFwHandleRTBD& hdlr);
    virtual void Visit(ClvpFwHandleVEDFW& hdlr);
    virtual void Visit(ClvpFwHandleSSBIOS& hdlr);
    virtual void Visit(ClvpFwHandleIFW1& hdlr);
    virtual void Visit(ClvpFwHandleIFW2& hdlr);
    virtual void Visit(ClvpFwHandleIFW3& hdlr);
    virtual void Visit(ClvpFwHandleRESET& hdlr);
    virtual void Visit(ClvpOsHandleDORM& hdlr);
    virtual void Visit(ClvpOsHandleOSIPSZ& hdlr);
    virtual void Visit(ClvpOsHandleROSIP& hdlr);
    virtual void Visit(ClvpOsHandleDONE& hdlr);
    virtual void Visit(ClvpOsHandleRIMG& hdlr);
    virtual void Visit(ClvpOsHandleEOIU& hdlr);
    virtual void Visit(ClvpStHandleFwNormal& hdlr);
    virtual void Visit(ClvpStHandleFwMisc& hdlr);
    virtual void Visit(ClvpStHandleFwWipe& hdlr);
    virtual void Visit(ClvpStHandleOsNormal& hdlr);
    virtual void Visit(ClvpStHandleOsMisc& hdlr);
    virtual void Visit(ClvpStHandleSoftFuses& hdlr);
    virtual void Visit(ClvpErHandleLogError& hdlr);
    virtual void Visit(ClvpErHandleERRR& hdlr);

private:
    bool WriteOutPipe(unsigned char* pbuf, uint32 size);
    bool ReadInPipe(unsigned char* pbuf, uint32 size);
    unsigned long long ReadInAck();
    bool StartFw();
    bool StartOs();
    bool StartSoftfuses();
    bool HandleNoSize();
    void CheckFile(char *filename);
    bool FwDXBL();
    bool FwDSKF();
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
    char* m_fname_softfuses_bin;
    unsigned long m_gpflags;
    IDevice * m_usbdev;
    CloverviewPlusUtils* m_utils;
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
    bool m_sf_flag;
    bool m_sf_error;
    bool m_sf_retry;


    CloverviewPlusFW* m_mfld_fw;
    CloverviewPlusOS* m_mfld_os;
    softfusesFW* m_mfld_softfuses;

    unsigned long long m_dldr_state;

    bool m_ifwi_done;

    bool m_ROSIP_data_flag;
    unsigned long m_delay_ms;

    last_error m_last_error;

    HandleMapType m_bulk_ack_map;
    ErrorMapType m_fw_error_map;

    ClvpDldrState(const ClvpDldrState&);
    ClvpDldrState& operator=(const ClvpDldrState&);
};

#endif //CLVPDLDRSTATE_H
