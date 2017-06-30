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
#ifndef CLV_DLDRSTATE_H
#define CLV_DLDRSTATE_H
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include <string.h>
#include "cloverviewutils.h"
#include "../../common/xfstktypes.h"
#include "./interfaces/ivisitor.h"
#include "cloverviewfw.h"
#include "cloverviewos.h"
#include "../../interfaces/idevice.h"
#include "../../interfaces/idownloader.h"
#include "clvdldrhandler.h"

using namespace std;

//Dldr state defines, make it similiar to ach code
#define DLDR_STATE_INVALID     0x5354494EULL //"STIN"
#define DLDR_STATE_FW_NORMAL   0x5354464EULL //"STFN"
#define DLDR_STATE_FW_MISC     0x5354464DULL //"STFM"  //For none virgin eMMC
#define DLDR_STATE_OS_NORMAL   0x53544f4EULL //"STON"  //For none virgin eMMC
#define DLDR_STATE_OS_MISC     0x53544f4DULL //"STOM"  //For none virgin eMMC//For misc dnx
#define DLDR_STATE_FW_WIPE     0x53544657ULL //"STFW"  //For wipe out eMMC to make it like a virgin emmc

#define DLDR_FW_TOTAL_STEPS_CLV 23

typedef std::map<unsigned long long, int> ErrorMapType;
typedef std::map<unsigned long long, IBaseVisitable<> *> HandleMapType;

class ClvDldrState: public IVisitor
        , public Visitor<ClvFwHandleER00>
        , public Visitor<ClvFwHandleHLT0>
        , public Visitor<ClvFwHandleDFRM>
        , public Visitor<ClvFwHandleDxxM>
        , public Visitor<ClvFwHandleDXBL>
        , public Visitor<ClvFwHandleRUPHS>
        , public Visitor<ClvFwHandleRUPH>
        , public Visitor<ClvFwHandleDMIP>
        , public Visitor<ClvFwHandleLOFW>
        , public Visitor<ClvFwHandleHIFW>
        , public Visitor<ClvFwHandlePSFW1>
        , public Visitor<ClvFwHandlePSFW2>
        , public Visitor<ClvFwHandleSSFW>
        , public Visitor<ClvFwHandleHLT$>
        , public Visitor<ClvFwHandleMFLD>
        , public Visitor<ClvFwHandleCLVT>
        , public Visitor<ClvFwHandleSuCP>
        , public Visitor<ClvFwHandleRTBD>
        , public Visitor<ClvFwHandleVEDFW>
        , public Visitor<ClvFwHandleSSBIOS>
        , public Visitor<ClvFwHandleIFW1>
        , public Visitor<ClvFwHandleIFW2>
        , public Visitor<ClvFwHandleIFW3>
        , public Visitor<ClvFwHandleRESET>
        , public Visitor<ClvOsHandleDORM>
        , public Visitor<ClvOsHandleOSIPSZ>
        , public Visitor<ClvOsHandleROSIP>
        , public Visitor<ClvOsHandleDONE>
        , public Visitor<ClvOsHandleRIMG>
        , public Visitor<ClvOsHandleEOIU>
        , public Visitor<ClvStHandleFwNormal>
        , public Visitor<ClvStHandleFwMisc>
        , public Visitor<ClvStHandleFwWipe>
        , public Visitor<ClvStHandleOsNormal>
        , public Visitor<ClvStHandleOsMisc>
        , public Visitor<ClvErHandleLogError>
        , public Visitor<ClvErHandleERRR>

{
public:
    ClvDldrState();
    ~ClvDldrState();

    bool Init(IDevice * usbdev, CloverviewUtils* utils);
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

    virtual void Visit(ClvFwHandleER00& hdlr);
    virtual void Visit(ClvFwHandleHLT0& hdlr);
    virtual void Visit(ClvFwHandleDFRM& hdlr);
    virtual void Visit(ClvFwHandleDxxM& hdlr);
    virtual void Visit(ClvFwHandleDXBL& hdlr);
    virtual void Visit(ClvFwHandleRUPHS& hdlr);
    virtual void Visit(ClvFwHandleRUPH& hdlr);
    virtual void Visit(ClvFwHandleDMIP& hdlr);
    virtual void Visit(ClvFwHandleLOFW& hdlr);
    virtual void Visit(ClvFwHandleHIFW& hdlr);
    virtual void Visit(ClvFwHandlePSFW1& hdlr);
    virtual void Visit(ClvFwHandlePSFW2& hdlr);
    virtual void Visit(ClvFwHandleSSFW& hdlr);
    virtual void Visit(ClvFwHandleHLT$& hdlr);
    virtual void Visit(ClvFwHandleMFLD& hdlr);
    virtual void Visit(ClvFwHandleCLVT& hdlr);
    virtual void Visit(ClvFwHandleSuCP& hdlr);
    virtual void Visit(ClvFwHandleRTBD& hdlr);
    virtual void Visit(ClvFwHandleVEDFW& hdlr);
    virtual void Visit(ClvFwHandleSSBIOS& hdlr);
    virtual void Visit(ClvFwHandleIFW1& hdlr);
    virtual void Visit(ClvFwHandleIFW2& hdlr);
    virtual void Visit(ClvFwHandleIFW3& hdlr);
    virtual void Visit(ClvFwHandleRESET& hdlr);
    virtual void Visit(ClvOsHandleDORM& hdlr);
    virtual void Visit(ClvOsHandleOSIPSZ& hdlr);
    virtual void Visit(ClvOsHandleROSIP& hdlr);
    virtual void Visit(ClvOsHandleDONE& hdlr);
    virtual void Visit(ClvOsHandleRIMG& hdlr);
    virtual void Visit(ClvOsHandleEOIU& hdlr);
    virtual void Visit(ClvStHandleFwNormal& hdlr);
    virtual void Visit(ClvStHandleFwMisc& hdlr);
    virtual void Visit(ClvStHandleFwWipe& hdlr);
    virtual void Visit(ClvStHandleOsNormal& hdlr);
    virtual void Visit(ClvStHandleOsMisc& hdlr);
    virtual void Visit(ClvErHandleLogError& hdlr);
    virtual void Visit(ClvErHandleERRR& hdlr);

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
    CloverviewUtils* m_utils;
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


    CloverviewFW* m_mfld_fw;
    CloverviewOS* m_mfld_os;

    unsigned long long m_dldr_state;

    bool m_ROSIP_data_flag;
    unsigned long m_delay_ms;

    last_error m_last_error;

    bool m_ifwi_done;

    HandleMapType m_bulk_ack_map;
    ErrorMapType m_fw_error_map;
    ClvDldrState(const ClvDldrState&);
    ClvDldrState& operator=(const ClvDldrState&);
};

#endif //CLV_DLDRSTATE_H
