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
#ifndef MRFDLDRSTATE_H
#define MRFDLDRSTATE_H
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <string.h>
#include "merrifieldutils.h"
#include "../../common/xfstktypes.h"
#include "../../interfaces/ivisitor.h"
#include "merrifieldfw.h"
#include "merrifieldos.h"
#include "../../interfaces/idevice.h"
#include "../../interfaces/idownloader.h"
#include "mrfdldrhandler.h"
#include "boost/scoped_array.hpp"
#define KEYSIZE_MRFD 4*sizeof(DWORD)

using namespace std;

#define CSDB_BYPASS_STATUS "777"
#define CSDB_RESULT_OFFSET 12
#define CSDB_CMD_OFFSET 5
#define CSDB_SIZE_OFFSET 8

//Dldr state defines, make it similiar to ach code
#define DLDR_STATE_INVALID     0x5354494EULL //"STIN"
#define DLDR_STATE_FW_NORMAL   0x5354464EULL //"STFN"
#define DLDR_STATE_FW_MISC     0x5354464DULL //"STFM"  //For none virgin eMMC
#define DLDR_STATE_OS_NORMAL   0x53544f4EULL //"STON"  //For none virgin eMMC
#define DLDR_STATE_OS_MISC     0x53544f4DULL //"STOM"  //For none virgin eMMC//For misc dnx
#define DLDR_STATE_FW_WIPE     0x53544657ULL //"STFW"  //For wipe out eMMC to make it like a virgin emmc
#define DLDR_STATE_BHC_ERRR    0x53544243ULL //"STBC"  //For Battery Health Check Error


#define DLDR_FW_TOTAL_STEPS      72
#define DLDR_OS_TOTAL_STEPS       9
#define DOWNLOAD_FW_PROGRESS      1
#define DOWNLOAD_OS_PROGRESS      2

typedef std::map<unsigned long long, int> ErrorMapType;
typedef std::map<unsigned long long, IBaseVisitable<> *> HandleMapType;
//typedef unsigned long DWORD;

class mrfdldrstate: public IVisitor
        , public Visitor<MrfdFwHandleER00>
        , public Visitor<MrfdFwHandleHLT0>
        , public Visitor<MrfdFwHandleDFRM>
        , public Visitor<MrfdFwHandleDxxM>
        , public Visitor<MrfdFwHandleDXBL>
        , public Visitor<MrfdFwHandleRUPHS>
        , public Visitor<MrfdFwHandleRUPH>
        , public Visitor<MrfdFwHandleDCFI00>
        , public Visitor<MrfdFwHandleDIFWI>
        , public Visitor<MrfdFwHandleHLT$>
        , public Visitor<MrfdFwHandleMFLD>
        , public Visitor<MrfdFwHandleCLVT>
        , public Visitor<MrfdFwHandleRTBD>
        , public Visitor<MrfdFwHandleRESET>
        , public Visitor<MrfdOsHandleDORM>
        , public Visitor<MrfdOsHandleOSIPSZ>
        , public Visitor<MrfdOsHandleROSIP>
        , public Visitor<MrfdOsHandleDONE>
        , public Visitor<MrfdOsHandleRIMG>
        , public Visitor<MrfdOsHandleEOIU>
        , public Visitor<MrfdStHandleFwNormal>
        , public Visitor<MrfdStHandleFwMisc>
        , public Visitor<MrfdStHandleFwWipe>
        , public Visitor<MrfdStHandleOsNormal>
        , public Visitor<MrfdStHandleOsMisc>
        , public Visitor<MrfdErHandleLogError>
        , public Visitor<MrfdErHandleERRR>
        , public Visitor<MrfdFwHandleDCSDB>
        , public Visitor<MrfdFwHandleUCSDB>
{
public:
    mrfdldrstate();
    ~mrfdldrstate();

    bool Init(IDevice * usbdev, MerrifieldUtils* utils);
    bool DoUpdate(char* fname_dnx_fw, char* fname_fw_image, char* fname_dnx_os, \
                  char* fname_os_image, char* fname_dnx_misc, unsigned long gpflags, \
                  unsigned long usbdelayms, bool ifwiwipeenable, char *miscbin);
    bool GetOsStatus();
    bool SetIDRQstatus(bool idrq);
    bool GetFwStatus();
    bool GetIFWIStatus();
    bool GetLastError(last_error* er);
    bool IsProvisionFailed();
    bool IsGPPReset();
    bool IsOsOnly();
    bool IsFwState();
    bool IsOsState();
    int getResponseBuffer(unsigned char* idrqbuf,int maxsize);
    void SetCsdbStatus(string csdb, bool direct);
    void InitStepsDone();
    void LogProgress();

    virtual void Visit(MrfdFwHandleER00& hdlr);
    virtual void Visit(MrfdFwHandleHLT0& hdlr);//
    virtual void Visit(MrfdFwHandleDFRM& hdlr);//
    virtual void Visit(MrfdFwHandleDxxM& hdlr);//
    virtual void Visit(MrfdFwHandleDXBL& hdlr);//
    virtual void Visit(MrfdFwHandleRUPHS& hdlr);//
    virtual void Visit(MrfdFwHandleRUPH& hdlr);//
    virtual void Visit(MrfdFwHandleDCFI00& hdlr);//BULK_ACK_DCFI00
    virtual void Visit(MrfdFwHandleDIFWI& hdlr);//BULK_ACK_DIFWI
    virtual void Visit(MrfdFwHandleHLT$& hdlr);//
    virtual void Visit(MrfdFwHandleMFLD& hdlr);
    virtual void Visit(MrfdFwHandleCLVT& hdlr);
    virtual void Visit(MrfdFwHandleRTBD& hdlr);//
    virtual void Visit(MrfdFwHandleRESET& hdlr);
    virtual void Visit(MrfdOsHandleDORM& hdlr);//
    virtual void Visit(MrfdOsHandleOSIPSZ& hdlr);
    virtual void Visit(MrfdOsHandleROSIP& hdlr);//
    virtual void Visit(MrfdOsHandleDONE& hdlr);
    virtual void Visit(MrfdOsHandleRIMG& hdlr);//
    virtual void Visit(MrfdOsHandleEOIU& hdlr);//
    virtual void Visit(MrfdStHandleFwNormal& hdlr);//
    virtual void Visit(MrfdStHandleFwMisc& hdlr);//
    virtual void Visit(MrfdStHandleFwWipe& hdlr);//
    virtual void Visit(MrfdStHandleOsNormal& hdlr);//
    virtual void Visit(MrfdStHandleOsMisc& hdlr);//
    virtual void Visit(MrfdErHandleLogError& hdlr);//
    virtual void Visit(MrfdErHandleERRR& hdlr);
    virtual void Visit(MrfdFwHandleDCSDB& hdlr);
    virtual void Visit(MrfdFwHandleUCSDB& hdlr);

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
    void FinishProgressBar();
    void InitializeProgressBar(int Downloadtype);
    void SleepMs(int delay);

    bool StateMachine();
    bool Start();
    bool IDRQresponse();
    bool bin2ascii(std::string& ascii, unsigned char* bin, int size);
    unsigned long long GetOpCode();
    void GotoState(unsigned long long state);
    bool UsbDevInit();

    char* m_fname_dnx_fw;
    char* m_fname_fw_image;
    char* m_fname_dnx_misc;
    char* m_fname_dnx_os;
    char* m_fname_os_image;
    char* m_fname_bin_misc;
    string csdbStatus;
    bool directcsdbStatus;
    unsigned long m_gpflags;
    IDevice * m_usbdev;
    MerrifieldUtils* m_utils;
    bool m_b_usbinitok;
    unsigned long long  m_achCode;

    int m_progress;
    float m_stepsdone;
    float m_totalsteps;
    bool m_b_IDRQ;
    unsigned long  m_b_DnX_OS;

    bool m_fw_done;
    bool m_os_done;
    bool m_abort;
    bool m_GPPReset;
    bool m_state_change;
    bool m_ifwiwipe_enable;
    unsigned int m_ifwiArrayPtr;
    unsigned int m_numIfwiChunks;
    unsigned int m_ifwi128KBChunks;
    unsigned int m_residualIfwiBytes;

    MerrifieldFW* m_mfld_fw;
    MerrifieldOS* m_mfld_os;

    unsigned long long m_dldr_state;

    bool m_ifwi_done;

    bool m_ROSIP_data_flag;
    unsigned long m_delay_ms;

    last_error m_last_error;

    HandleMapType m_bulk_ack_map;
    ErrorMapType m_fw_error_map;
    boost::scoped_array<unsigned char> ResponseBuff;
    int ResponseBuffSize;
    mrfdldrstate(const mrfdldrstate&);
    mrfdldrstate& operator=(const mrfdldrstate&);
};


struct idrqBuffer
{
    DWORD preamble;
    DWORD hashkeys[8][4];
    DWORD PartSpecificIDVariable[4];
    DWORD PartSpecificIDConstant[4];
    DWORD PMICTimeBase;
    DWORD PMICNonce[4];
    DWORD LiveNonce[4];
    DWORD Status;
};
#endif // MRFmrfdldrstate_H
