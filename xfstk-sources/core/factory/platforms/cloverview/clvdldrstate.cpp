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
#include <map>
#include <utility> 
#include <sys/stat.h>

#if defined XFSTK_OS_WIN
#include <windows.h>
#endif
#include <exception>
#include <stdexcept>
#include "cloverviewmessages.h"
#include "clvdldrstate.h"
#include <boost/smart_ptr/scoped_array.hpp>

#define USB_READ_WRITE_DELAY_MS 0

extern CPSTR Cloverview_error_code_array [MAX_ERROR_CODE]; 

//ClvDldrState Implementation
ClvDldrState::ClvDldrState()
{
    m_usbdev = NULL;
    m_utils = NULL;

    m_b_usbinitok = false;

    m_gpflags = 0;
    m_achCode = 0;

    m_numChunks = 0;
    m_numResidentChunks = 0;
    m_numRomPatchChunks = 0;
    m_secFwArrayPtr = 0;
    m_sec2FwArrayPtr = 0;
    m_sec3FwArrayPtr = 0;
    m_romPatchArrayPtr = 0;
    m_vedFwArrayPtr = 0;

    m_fwstepsdone = 0;
    m_fwprogress = 0;
    m_fwtotalsteps = DLDR_FW_TOTAL_STEPS_CLV;//12;
    m_b_IDRQ = false;
    m_fw_done = false;
    m_ifwi_done = false;
    m_os_done = false;
    m_abort = false;
    m_GPPReset = false;
    m_b_DnX_OS = 0;


    m_dldr_state = DLDR_STATE_INVALID;
    m_ROSIP_data_flag = false;

    m_mfld_fw = NULL;
    m_mfld_os = NULL;

    m_delay_ms = USB_READ_WRITE_DELAY_MS;

    // FW Upgrade Ack values, index pair map
    int i = 0;
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_INVALID_PING, new ClvFwHandleER00));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_HLT0, new ClvFwHandleHLT0));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_DFRM, new ClvFwHandleDFRM));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_DxxM, new ClvFwHandleDxxM));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_DXBL, new ClvFwHandleDXBL));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_READY_UPH_SIZE, new ClvFwHandleRUPHS));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_READY_UPH, new ClvFwHandleRUPH));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_DMIP, new ClvFwHandleDMIP));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_LOFW, new ClvFwHandleLOFW));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_HIFW, new ClvFwHandleHIFW));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_PSFW1, new ClvFwHandlePSFW1));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_PSFW2, new ClvFwHandlePSFW2));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_SSFW, new ClvFwHandleSSFW));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_UPDATE_SUCESSFUL, new ClvFwHandleHLT$));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_MFLD, new ClvFwHandleMFLD));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_CLVT, new ClvFwHandleCLVT));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_PATCH, new ClvFwHandleSuCP));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_RTBD,  new ClvFwHandleRTBD));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_VEDFW, new ClvFwHandleVEDFW));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_SSBS, new ClvFwHandleSSBIOS));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_IFW1, new ClvFwHandleIFW1));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_IFW2, new ClvFwHandleIFW2));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_IFW3, new ClvFwHandleIFW3));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_GPP_RESET, new ClvFwHandleRESET));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_DORM, new ClvOsHandleDORM));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_OSIPSZ, new ClvOsHandleOSIPSZ));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ROSIP, new ClvOsHandleROSIP));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_DONE, new ClvOsHandleDONE));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_RIMG, new ClvOsHandleRIMG));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_EOIU, new ClvOsHandleEOIU));

    //Set up device error code handler
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER01, new ClvErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER02, new ClvErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER03, new ClvErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER04, new ClvErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER10, new ClvErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER11, new ClvErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER12, new ClvErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER13, new ClvErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER15, new ClvErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER16, new ClvErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER17, new ClvErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER18, new ClvErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER20, new ClvErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER21, new ClvErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER22, new ClvErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER25, new ClvErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ERRR, new ClvErHandleERRR));

    //Setup state map
    m_bulk_ack_map.insert(std::make_pair(DLDR_STATE_FW_NORMAL, new ClvStHandleFwNormal));
    m_bulk_ack_map.insert(std::make_pair(DLDR_STATE_FW_MISC, new ClvStHandleFwMisc));
    m_bulk_ack_map.insert(std::make_pair(DLDR_STATE_FW_WIPE, new ClvStHandleFwWipe));
    m_bulk_ack_map.insert(std::make_pair(DLDR_STATE_OS_NORMAL, new ClvStHandleOsNormal));
    m_bulk_ack_map.insert(std::make_pair(DLDR_STATE_OS_MISC, new ClvStHandleOsMisc));

    //Error ack code and error code index map
    i = 19; //Check error code string array for index.
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER01, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER02, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER03, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER04, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER10, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER11, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER12, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER13, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER15, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER16, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER17, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER18, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER20, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER21, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER22, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER25, i++));
    m_last_error.error_code = 1;
    strcpy(m_last_error.error_message, "Init error");

}

ClvDldrState::~ClvDldrState()
{
    // clear the entries in the map
    for(HandleMapType::iterator it=m_bulk_ack_map.begin(); it != m_bulk_ack_map.end(); it++)
        delete (*it).second;
    m_bulk_ack_map.clear();
    m_fw_error_map.clear();

    if(m_mfld_fw)
    {
        delete m_mfld_fw;
        m_mfld_fw = NULL;
    }
    if(m_mfld_os)
    {
        delete m_mfld_os;
        m_mfld_os = NULL;
    }
}

bool ClvDldrState::Init(IDevice * usbdev, CloverviewUtils* utils)
{
    if(!m_b_usbinitok) {
        m_usbdev = usbdev;
        m_utils = utils;
        m_b_usbinitok = true;
    }
    m_ROSIP_data_flag = false; 

    return true;
}
bool ClvDldrState::UsbDevInit(void)
{
    m_b_usbinitok = false;
    m_b_usbinitok = m_usbdev->Open();
    return m_b_usbinitok;
}
bool ClvDldrState::DoUpdate(char* fname_dnx_fw, char* fname_fw_image, char* fname_dnx_os, \
                         char* fname_os_image, char* fname_dnx_misc, unsigned long gpflags, \
						 unsigned long usbdelayms, bool ifwiwipeenable)
{
    bool ret = true;
    m_fname_dnx_fw = fname_dnx_fw;
    m_fname_fw_image = fname_fw_image;
    m_fname_dnx_misc = fname_dnx_misc;
    m_fname_dnx_os = fname_dnx_os;
    m_fname_os_image = fname_os_image;
    m_gpflags = gpflags;
    m_fw_done = false;
    m_ifwi_done = false;
    m_os_done = false;
    m_GPPReset = false;
    m_abort = false;
    m_state_change = false;
    m_delay_ms = usbdelayms;
    m_ifwiwipe_enable = ifwiwipeenable;
    m_fwstepsdone = 0;

    //Start downloader state machine
    ret = StateMachine();

    return ret;
}

bool ClvDldrState::GetOsStatus()
{
    return m_os_done;
}

bool ClvDldrState::GetFwStatus()
{
    return m_fw_done;
}

bool ClvDldrState::GetIFWIStatus()
{
    return m_ifwi_done;
}

bool ClvDldrState::IsFwState()
{
    return (m_dldr_state == DLDR_STATE_FW_NORMAL || m_dldr_state == DLDR_STATE_FW_MISC);
}

bool ClvDldrState::IsOsState()
{
    return (m_dldr_state == DLDR_STATE_OS_NORMAL || m_dldr_state == DLDR_STATE_OS_MISC);
}

bool ClvDldrState::IsProvisionFailed()
{
    return m_abort;
}

bool ClvDldrState::IsGPPReset()
{
    return m_GPPReset;
}

bool ClvDldrState::IsOsOnly()
{
    return (m_b_DnX_OS || (this->m_utils->FileSize(m_fname_fw_image) == 0));
}

bool ClvDldrState::WriteOutPipe(unsigned char* pbuf, uint32 size)
{
    bool result = true;
    this->m_utils->u_log(LOG_USB, "%s %s", __FUNCTION__, this->m_utils->makestring(pbuf, size).c_str());
    if(this->m_b_usbinitok) {
		this->m_utils->u_log(LOG_USB, "%s %d", __FUNCTION__, m_delay_ms);

        SleepMs(m_delay_ms);
        if(!this->m_usbdev->Write(pbuf,size)) {
            return false;
        }
    }
    else {
        return false;
    }
    return result;
}

bool ClvDldrState::ReadInPipe(unsigned char* pbuf, uint32 size)
{
    bool result = true;
    this->m_utils->u_log(LOG_USB, "%s %s", __FUNCTION__, this->m_utils->makestring(pbuf, size).c_str());
    if(this->m_b_usbinitok) {
        SleepMs(m_delay_ms);
        if(!this->m_usbdev->Read(pbuf,size)) {
            return false;
        }
    }
    else {
        return false;
    }
    return result;
}

unsigned long long ClvDldrState::ReadInAck()
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    unsigned long long  rev_data = 0;
    uint32 bytes_rxed = 0;
    boost::scoped_array<unsigned char> szBuff (new unsigned char[MAX_BUFFER_SIZE + 1]);
    memset(szBuff.get(),0,MAX_BUFFER_SIZE + 1);

    bool result = true;

    if(this->m_b_usbinitok) {
        SleepMs(m_delay_ms);
        result = this->m_usbdev->GetAck(szBuff.get(), &bytes_rxed);
        if(bytes_rxed == 0xBAADF00D)
            return bytes_rxed;
    }


    if(result == false) {
        LogError(4);
        return 0;
    }

    m_achCode = this->m_utils->scan_string_for_protocol((char *)szBuff.get());

    rev_data = this->m_utils->endian_swap(m_achCode);
    string output;
    if(bytes_rxed == 4) {
        uint32 tmp = (uint32)rev_data;
        output = this->m_utils->makestring((unsigned char*)(&tmp), 4);
    }
    else {
        output = this->m_utils->makestring((unsigned char*)(&rev_data), 8);
    }

    return m_achCode;
}

unsigned long long ClvDldrState::GetOpCode()
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    unsigned long long data = 0;

    //First deal SW state change, then deal HW state
    if(m_state_change == true) {
        data = m_dldr_state;
        m_state_change = false;
    } else if(m_ROSIP_data_flag) {
        data = BULK_ACK_ROSIP;
        m_ROSIP_data_flag = false;
    } else {
        data = ReadInAck();
    }
    return data;
}

void ClvDldrState::CheckFile(char *filename)
{
    
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    FILE *fp;
    m_abort = false;

    fp = fopen(filename, "rb" );
    if ( fp == NULL) {
        this->m_utils->u_abort("File %s cannot be opened", filename);
        this->m_abort = true;
    }
    struct stat file_info;
    if(stat(filename, &file_info)) {
        this->m_utils->u_abort("Failed to stat file: %s", filename);
        this->m_abort = true;
    }

    if(fp)
        fclose(fp);
}

bool ClvDldrState::HandleNoSize()
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    int ret_code = 0;
    dnx_data* fwdata = NULL;

    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_DNX0H_CLV);
    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size)) {
        ret_code = 1; //USB error
        LogError(ret_code);
    }
    return (ret_code ? false:true);
}

void ClvDldrState::LogError(unsigned long errorcode)
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    if(errorcode == 0xBAADF00D)
    {
        m_last_error.error_code = errorcode;
        this->m_utils->u_log(LOG_STATUS, "Error Firmware/OS Image Not initialized");
        m_abort = true;
    }
    if(errorcode < MAX_ERROR_CODE) {
        if(errorcode == 0) {
            this->m_utils->u_log(LOG_STATUS, "Error Code: %d - %s", errorcode, Cloverview_error_code_array[errorcode]);
        } else {
            this->m_utils->u_log(LOG_STATUS,"Error Code: %d - %s", errorcode, Cloverview_error_code_array[errorcode]);            
            m_abort = true;
        }
        m_last_error.error_code = errorcode;
        strcpy(m_last_error.error_message, Cloverview_error_code_array[errorcode]);
    }
}

bool ClvDldrState::GetLastError(last_error* er)
{
    er->copy(m_last_error);
    return true;
}
void ClvDldrState::StartLogTime()
{
#if 0
    string strtime;
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    if(m_b_measure_usb_time) {
        gettimeofday(&m_start_time, NULL);
        gettime(&m_start_time, NULL, &strtime);
        this->m_utils->u_log(LOG_FWUPGRADE, "%s -- Start time stamp: %s", __FUNCTION__, strtime.c_str());
    }
#endif
}

void ClvDldrState::EndlogTime()
{
#if 0
    string strtime;
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    if(m_b_measure_usb_time) {
        gettimeofday(&m_end_time, NULL);
        m_time_elapsed += get_time_elapse(&m_start_time, &m_end_time);
        gettime(&m_start_time, NULL, &strtime);
        this->m_utils->u_log(LOG_FWUPGRADE, "%s -- End time stamp: %s", __FUNCTION__, strtime.c_str());
    }
#endif
}

void ClvDldrState::LogProgress()
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    m_fwprogress = (int)((m_fwstepsdone/m_fwtotalsteps)*100);
    this->m_utils->u_log(LOG_PROGRESS, "%d",((m_fwstepsdone>m_fwtotalsteps)? 100:m_fwprogress));
}

bool ClvDldrState::FwDXBL()
{   
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    int ret_code = 0;

    if(!m_mfld_fw)
    {
        LogError(0xBAADF00D);
        return false;
    }

    dnx_data* fwdata = NULL;
    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_DNXD_CLV);
    this->m_utils->u_log(LOG_FWUPGRADE, "Sending FW Dnx data...%x", fwdata);

    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size))
    {
        ret_code = 1; //USB error
        LogError(ret_code);
    }
    return ret_code ? false:true;
}

bool ClvDldrState::OsDXBL()
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    int ret_code = 0;

    if(!m_mfld_os)
    {
        LogError(0xBAADF00D);
        return false;
    }

    dnx_data* fwdata = NULL;
    fwdata = m_mfld_os->GetOsDnX();

    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size))
    {
        ret_code = 1; //USB error
        LogError(ret_code);
    }
    return ret_code ? false:true;
}

bool ClvDldrState::StartFw()
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    int ret_code = 0;
    dnx_data* fwdata = NULL;

    if(!m_mfld_fw)
    {
        LogError(0xBAADF00D);
        return false;
    }

    //Prepare the start 128k chunk count

    //Calculations for iCache FW
    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_PSFW1);
    if(!fwdata)
    {
        this->m_utils->u_log(LOG_FWUPGRADE, "Error FW_DATA_PSFW1 not found...");
        LogError(0xBADF00D);
        return false;
    }
    m_numChunks = fwdata->size  / ONE28_K;
    //keep a static copy of iCache 128KB chunks
    m_iCacheNum128KBChunks = m_numChunks;
    m_residualBytes = fwdata->size - (m_numChunks * ONE28_K);
    //If the size is 128KB aligned need to adjust the chunk num since it index from 0
    if(m_residualBytes == 0) {
        m_numChunks--;
        m_iCacheNum128KBChunks = m_numChunks;
    }


    //Calculations for Resident FW
    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_PSFW2);
    if(!fwdata)
    {
        this->m_utils->u_log(LOG_FWUPGRADE, "Error FW_DATA_PSFW2 not found...");
        LogError(0xBADF00D);
        return false;
    }
    m_numResidentChunks = fwdata->size / ONE28_K;
    //keep a static copy of Resident 128KB chunks
    m_residentNum128KBChunks = m_numResidentChunks;
    m_residualResidentBytes = fwdata->size - (m_numResidentChunks * ONE28_K);
    //If the size is 128KB aligned need to adjust the chunk num since it index from 0
    if(m_residualResidentBytes == 0) {
        m_numResidentChunks--;
        m_residentNum128KBChunks = m_numResidentChunks;
    }

    //Calculations for Extended FW
    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_SSFW);
    if(!fwdata)
    {
        this->m_utils->u_log(LOG_FWUPGRADE, "Error FW_DATA_SSFW not found...");
        LogError(0xBADF00D);
        return false;
    }
    m_numExtChunks = fwdata->size / ONE28_K;
    //keep a static copy of Ext. FW 128KB chunks
    m_extFWNum128KBChunks = m_numExtChunks;
    m_residualExtBytes = fwdata->size - (m_numExtChunks * ONE28_K);
    //If the size is 128KB aligned need to adjust the chunk num since it index from 0
    if(m_residualExtBytes == 0) {
        m_numExtChunks--;
        m_extFWNum128KBChunks = m_numExtChunks;
    }

    //Calculations for Rom Patch
    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_PATCH);
    if(!fwdata)
    {
        this->m_utils->u_log(LOG_FWUPGRADE, "Error FW_DATA_PATCH not found...");
        LogError(0xBADF00D);
        return false;
    }
    m_numRomPatchChunks = fwdata->size / ONE28_K;
    m_romPatch128KBChunks = m_numRomPatchChunks;
    m_residualRomPatchBytes = fwdata->size - (m_romPatch128KBChunks * ONE28_K);
    //If the size is 128KB aligned need to adjust the chunk num since it index from 0
    if(m_residualRomPatchBytes == 0) {
        m_numRomPatchChunks--;
        m_romPatch128KBChunks = m_numRomPatchChunks;
    }

    //Calculations for VED FW
    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_VEDFW);
    if(!fwdata)
    {
        this->m_utils->u_log(LOG_FWUPGRADE, "Error FW_DATA_VEDFW not found...");
        LogError(0xBADF00D);
        return false;
    }
    m_numVedFwChunks = fwdata->size / ONE28_K;
    m_vedFw128KBChunks = m_numVedFwChunks;
    m_residualVedFwBytes = fwdata->size - (m_vedFw128KBChunks * ONE28_K);
    //If the size is 128KB aligned need to adjust the chunk num since it index from 0
    if(m_residualVedFwBytes == 0) {
        m_numVedFwChunks--;
        m_vedFw128KBChunks = m_numVedFwChunks;
    }

    // Update the Total no. of Steps based on the Number of Chunks
    if ( (m_numVedFwChunks * ONE28_K) > TWO_HUNDRED_KB)
    {
        //For CLV
        //     4--->i)DXBL ii)RUPHS iii)RUPH iv) HLT
        //   2 *---> for two Boot Partitions
        //     4--->i)DMIP ii)IFWI1 iii) IFWI2 iv) IFWI3
        //Chunks--->ICache/Extended/Resident/RomPatch
        //    +1---> as Chunks are indexed from 0
        //SS BIOS is in GPP and we update it once only
        m_fwtotalsteps = 4 + (2 *(4 + m_numChunks + 1 + m_numResidentChunks + 1 + m_numExtChunks + 1 + m_numRomPatchChunks + 1)) + m_numVedFwChunks;
    }
    else if( (m_numVedFwChunks * ONE28_K) <= TWO_HUNDRED_KB)
    {
        //For CLVP
        //     4--->i)DXBL ii)RUPHS iii)RUPH iv) HLT
        //   2 *---> for two Boot Partitions
        //     4--->i)DMIP ii)IFWI1 iii) IFWI2 iv) IFWI3
        //Chunks--->ICache/Extended/Resident/RomPatch/MSVDX
        //    +1---> as Chunks are indexed from 0
        m_fwtotalsteps = 4 + (2 *(4 + m_numChunks + 1 + m_numResidentChunks + 1 + m_numExtChunks + 1 + m_numRomPatchChunks + 1 + m_numVedFwChunks + 1 )) ;
    }
    this->m_utils->u_log(LOG_FWUPGRADE, "CLV Total number of calculated steps : %f",m_fwtotalsteps);
    // Start fw update
    this->m_utils->u_log(LOG_FWUPGRADE, "sending 6 DWORDS of DnxFW size and checksum...");
    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_DNXH_CLOVERVIEW);
    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size))
    {
        ret_code = 1; //USB error
        LogError(ret_code);
    }
    return (ret_code ? false:true);
}

bool ClvDldrState::StartOs()
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    int ret_code = 0;
    dnx_data* fwdata = NULL;

    if(!m_mfld_os)
    {
        LogError(0xBAADF00D);
        return false;
    }

    this->m_utils->u_log(LOG_OS, "sending 6 DWORDS of DnxOS size and checksum...");

    fwdata = m_mfld_os->GetOsDnXHdr();

    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size))
    {
        ret_code = 1; //USB error
        LogError(ret_code);
    }
    return (ret_code ? false:true);
}


void ClvDldrState::Visit(ClvFwHandleER00& )
{
    int ret_code = 0;
    ULONG preamble_msg = PREAMBLE_DNER;

    // --- Retries Sending Preamble: DnER ---
    this->m_utils->u_log(LOG_OPCODE, "Retry Sending DnER...");
    if(!WriteOutPipe((PUCHAR)&preamble_msg, 4))
        ret_code = 1;

    if(ret_code)
        m_abort = true;
 }

void ClvDldrState::Visit(ClvFwHandleHLT0& )
{
    // --- FW file has no size
    this->m_utils->u_log(LOG_OPCODE, "HLT0 - DnX FW or IFWI Size = 0");
    m_fw_done = true;
}

void ClvDldrState::Visit(ClvFwHandleDFRM& )
{
    bool ret = true;

    this->m_utils->u_log(LOG_OPCODE, "FW: Handle virgin part DnX ...");
#if 0
    gettimeofday(&m_start_time, NULL);
#endif
    if(m_ifwiwipe_enable) {
        this->m_utils->u_log(LOG_OPCODE, "FW: EMMC is virgin, no need to wipe ifwi, aborting ...");
        m_fw_done = true;
        m_ifwi_done = false;
    } else {
        //Start virgin part FW download state
        GotoState(DLDR_STATE_FW_NORMAL);
    }

    if(!ret) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvFwHandleDxxM& )
{
    bool ret = true;
    this->m_utils->u_log(LOG_OPCODE, "FW: Handle None virgin part DnX ...");

    m_b_DnX_OS = 0;
    m_b_DnX_OS = (m_gpflags & 0x20);
#if 0
    gettimeofday(&m_start_time, NULL);
#endif
    //If want to wipe out old ifwi image on emmc to make the emmc like virgin 
    if(m_ifwiwipe_enable)
        GotoState(DLDR_STATE_FW_WIPE);
    else if(m_b_DnX_OS || strlen(m_fname_dnx_misc) > 3) //Only if misc dnx file is provided
        GotoState(DLDR_STATE_FW_MISC);
    else
        GotoState(DLDR_STATE_FW_NORMAL);

    if(!ret) {
        m_abort = true;
    }
}


void ClvDldrState::Visit(ClvFwHandleDXBL& )
{
    bool ret = true;

    m_fwstepsdone++;

    StartLogTime();

    if(m_dldr_state == DLDR_STATE_FW_NORMAL  \
            || m_dldr_state == DLDR_STATE_FW_MISC \
            || m_dldr_state == DLDR_STATE_FW_WIPE) {
        this->m_utils->u_log(LOG_FWUPGRADE, "FW: Sending DnX ...");
        ret = FwDXBL();
    } else {
        this->m_utils->u_log(LOG_OS, "OS: Sending DnX ...");
        ret = OsDXBL();
    }

    EndlogTime();
    LogProgress();

    if(!ret) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvFwHandleRUPHS& )
{
    int ret_code = 0;
    dnx_data* fwdata = NULL;

    if(!m_mfld_fw)
    {
        LogError(0xBAADF00D);
        return;
    }

    m_fwstepsdone++;

    StartLogTime();

    this->m_utils->u_log(LOG_FWUPGRADE, "FW: Sending FW Update Profile Hdr Size...");
    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_RUPHS);

    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size))
    {
        ret_code = 1; //USB error
        LogError(ret_code);
    }

    EndlogTime();
    LogProgress();

    if(ret_code) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvFwHandleRUPH& )
{
    int ret_code = 0;
    dnx_data* fwdata = NULL;

    if(!m_mfld_fw)
    {
        LogError(0xBAADF00D);
        return;
    }

    m_fwstepsdone++;

    StartLogTime();

    this->m_utils->u_log(LOG_FWUPGRADE, "FW: Sending FW Update Profile Hdr...");
    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_RUPH);

    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size))
    {
        ret_code = 1; //USB error
        LogError(ret_code);
    }

    EndlogTime();
    LogProgress();

    if(ret_code) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvFwHandleRESET& )
{
    StartLogTime();

    this->m_utils->u_log(LOG_FWUPGRADE, "FW: Received RESET...");

    EndlogTime();
    LogProgress();

    m_fw_done = true;
    m_GPPReset = true;
}
void ClvDldrState::Visit(ClvFwHandleDMIP& )
{
    int ret_code = 0;
    dnx_data* fwdata = NULL;

    if(!m_mfld_fw)
    {
        LogError(0xBAADF00D);
        return;
    }

    m_fwstepsdone++;

    StartLogTime();

    this->m_utils->u_log(LOG_FWUPGRADE, "FW: Sending MIP...");
    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_DMIP);

    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size)) {
        ret_code = 1; //USB error
        LogError(ret_code);
    }

    EndlogTime();
    LogProgress();

    if(ret_code) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvFwHandleLOFW& )
{
    int ret_code = 0;
    dnx_data* fwdata = NULL;

    if(!m_mfld_fw)
    {
        LogError(0xBAADF00D);
        return;
    }

    m_fwstepsdone++;

    StartLogTime();

    this->m_utils->u_log(LOG_FWUPGRADE, "FW: Sending first 128k...");
    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_LOFW);

    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size)) {
        ret_code = 1; //USB error
        LogError(ret_code);
    }

    EndlogTime();
    LogProgress();

    if(ret_code) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvFwHandleHIFW& )
{
    int ret_code = 0;
    dnx_data* fwdata = NULL;

    if(!m_mfld_fw)
    {
        LogError(0xBAADF00D);
        return;
    }

    m_fwstepsdone++;

    StartLogTime();

    this->m_utils->u_log(LOG_FWUPGRADE, "FW: Sending second 128k...");
    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_HIFW);

    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size)) {
        ret_code = 1; //USB error
        LogError(ret_code);
    }

    EndlogTime();
    LogProgress();

    if(ret_code) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvFwHandlePSFW1& )
{
    int ret_code = 0;
    dnx_data* fwdata = NULL;

    if(!m_mfld_fw)
    {
        LogError(0xBAADF00D);
        return;
    }

    unsigned char* remainingBytes = NULL;
    try {
        m_fwstepsdone++;

        StartLogTime();

        fwdata = m_mfld_fw->GetFwImageData(FW_DATA_PSFW1);
        if(!fwdata)
        {
            LogError(0xBADF00D);
            throw runtime_error("Error FW_DATA_PSFW1 not found...");
        }

        this->m_utils->u_log(LOG_FWUPGRADE, "FW: Sending Primary Security FW 1 Chunk %d", m_numChunks);
        if(m_residualBytes && 0 == m_numChunks)
        {
            remainingBytes = new UCHAR [m_residualBytes];
            //DEBUG ONLY
            this->m_utils->u_log(LOG_FWUPGRADE, "Allocation done \n");
            for(int j = 0; j < (int)m_residualBytes; j++)
            {
                remainingBytes[j] = fwdata->data[m_secFwArrayPtr];
                m_secFwArrayPtr ++;
            }

            //DEBUG ONLY
            this->m_utils->u_log(LOG_FWUPGRADE, "Writing the Bytes \n");
            //Write residual bytes
            if(!WriteOutPipe(&remainingBytes[0], m_residualBytes))
            {
                // do_usb_writes failed on last chunk send.
                throw 42;
            }
            //release resources
            delete [] remainingBytes;
            m_secFwArrayPtr = 0;
            //Last bytes were sent reset the 128KB block count
            m_numChunks = m_iCacheNum128KBChunks;
        }
        else
        {
            //Write the 128KB chunk and decrement numChunks
            //copy in array the next 128KB
            UCHAR byteBlock[ONE28_K];
            for(int i = 0; i < ONE28_K; i++)
            {
                byteBlock[i] = fwdata->data[m_secFwArrayPtr];
                m_secFwArrayPtr++;
            }

            if(!WriteOutPipe(byteBlock, ONE28_K))
            {
                // do_usb_writes failed on second 128k send.
                throw 42;
            }

            //Last bytes were sent reset the 128KB block count
            if(m_numChunks == 0) {
                m_secFwArrayPtr = 0;
                //Last bytes were sent reset the 128KB block count
                m_numChunks = m_iCacheNum128KBChunks;
            } else {
                m_numChunks--;
            }
        }
    }
    catch( int errorcode ) {
        ret_code = errorcode;
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Exception raised: ErrorCode %d", __FUNCTION__, errorcode);
        LogError(errorcode);
    }
    catch(std::exception & e)
    {
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Exception raised %s", __FUNCTION__, e.what());
        ret_code = 1;
    }
    catch( ... ) {
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Unknown Exception raised", __FUNCTION__);
        ret_code = 1;
    }
    EndlogTime();
    LogProgress();

    if(ret_code) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvFwHandlePSFW2& )
{
    int ret_code = 0;
    dnx_data* fwdata = NULL;

    if(!m_mfld_fw)
    {
        LogError(0xBAADF00D);
        return;
    }

    m_fwstepsdone++;

    unsigned char* remainingBytes = NULL;
    try {
        StartLogTime();

        fwdata = m_mfld_fw->GetFwImageData(FW_DATA_PSFW2);
        if(!fwdata)
        {
            LogError(0xBADF00D);
            throw runtime_error("Error FW_DATA_PSFW2 not found...");
        }

        //keep a static copy of Resident 128KB chunks
        this->m_utils->u_log(LOG_FWUPGRADE, "FW: Sending Primary Security FW 2 Chunk %d", m_numResidentChunks);

        //					this->m_utils->u_log(LOG_FWUPGRADE, "sending Primary Security FW 2 Chunk %d", numResidentChunks);
        if(m_residualResidentBytes && 0 == m_numResidentChunks)
        {
            remainingBytes = new UCHAR [m_residualResidentBytes];
            if(remainingBytes) {
                for(int j = 0; j < (int)m_residualResidentBytes; j++)
                {
                    remainingBytes[j] = fwdata->data[m_sec2FwArrayPtr];
                    m_sec2FwArrayPtr ++;
                }

                //Write residual bytes
                if(!WriteOutPipe(&remainingBytes[0], m_residualResidentBytes))
                {
                    // do_usb_writes failed on last chunk send.
                    throw 42;
                }

                //release resources
                delete [] remainingBytes;
                m_sec2FwArrayPtr = 0;
                //Last bytes were sent reset the 128KB block count
                m_numResidentChunks = m_residentNum128KBChunks;
            } else {
                throw 5;
            }
        }
        else
        {
            //Write the 128KB chunk and decrement m_numResidentChunks

            //copy in array the next 128KB
            UCHAR byteBlock[ONE28_K];
            for(int i = 0; i < ONE28_K; i++)
            {
                byteBlock[i] = fwdata->data[m_sec2FwArrayPtr];
                m_sec2FwArrayPtr++;
            }

            if(!WriteOutPipe(byteBlock, ONE28_K))
            {
                // do_usb_writes failed on second 128k send.
                throw 42;
            }
            //Last bytes were sent reset the 128KB block count
            if(m_numResidentChunks == 0) {
                m_sec2FwArrayPtr = 0;
                //Last bytes were sent reset the 128KB block count
                m_numResidentChunks = m_residentNum128KBChunks;
             } else {
                m_numResidentChunks--;
            }

        }
    }
    catch( int errorcode ) {
        ret_code = errorcode;
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Exception raised: ErrorCode %d", __FUNCTION__, errorcode);
        LogError(errorcode);
    }
    catch(std::exception & e)
    {
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Exception raised %s", __FUNCTION__, e.what());
        ret_code = 1;
    }
    catch( ... ) {
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Unknown Exception raised", __FUNCTION__);
        ret_code = 1;
    }

    EndlogTime();
    LogProgress();

    if(ret_code) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvFwHandleSSFW& )
{
    int ret_code = 0;
    dnx_data* fwdata = NULL;

    if(!m_mfld_fw)
    {
        LogError(0xBAADF00D);
        return;
    }

    m_fwstepsdone++;

    unsigned char* remainingBytes = NULL;
    try {
        StartLogTime();

        fwdata = m_mfld_fw->GetFwImageData(FW_DATA_SSFW);
        if(!fwdata)
        {
            LogError(0xBADF00D);
            throw runtime_error("Error FW_DATA_SSFW not found...");
        }

        this->m_utils->u_log(LOG_FWUPGRADE, "FW: Sending secondary security fw  Chunk %d", m_numExtChunks);

        if(m_residualExtBytes && 0 == m_numExtChunks)
        {
            remainingBytes = new UCHAR [m_residualExtBytes];
            if(remainingBytes) {
                for(int j = 0; j < (int)m_residualExtBytes; j++)
                {
                    remainingBytes[j] = fwdata->data[m_sec3FwArrayPtr];
                    m_sec3FwArrayPtr ++;
                }

                //Write residual bytes
                if(!WriteOutPipe(&remainingBytes[0], m_residualExtBytes))
                {
                    // do_usb_writes failed on last chunk send.
                    throw 42;
                }

                //release resources
                delete [] remainingBytes;

                m_sec3FwArrayPtr = 0;
                //Last bytes were sent reset the 128KB block count
                m_numExtChunks = m_extFWNum128KBChunks;
            } else {
                throw 5;
            }
        }
        else
        {
            //Write the 128KB chunk and decrement m_numExtChunks

            //copy in array the next 128KB
            UCHAR byteBlock[ONE28_K];
            for(int i = 0; i < ONE28_K; i++)
            {
                byteBlock[i] = fwdata->data[m_sec3FwArrayPtr];
                m_sec3FwArrayPtr++;
            }

            if(!WriteOutPipe(byteBlock, ONE28_K))
            {
                // do_usb_writes failed on second 128k send.
                throw 42;
            }

            //Last bytes were sent reset the 128KB block count
            if(m_numExtChunks == 0) {
                m_sec3FwArrayPtr = 0;
                //Last bytes were sent reset the 128KB block count
                m_numExtChunks = m_extFWNum128KBChunks;
             } else {
                m_numExtChunks--;
            }
        }
    }
    catch( int errorcode ) {
        ret_code = errorcode;
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Exception raised: ErrorCode %d", __FUNCTION__, errorcode);
        LogError(errorcode);
    }
    catch(std::exception & e)
    {
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Exception raised %s", __FUNCTION__, e.what());
        ret_code = 1;
    }
    catch( ... ) {
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Unknown Exception raised", __FUNCTION__);
        ret_code = 1;
    }

    EndlogTime();
    LogProgress();

    if(ret_code) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvFwHandleHLT$& )
{
    // --- ACK Received: HLT$ ---
    this->m_utils->u_log(LOG_FWUPGRADE, "FW: Firmware update completed...");
#if 0
    gettimeofday(&m_end_time, NULL);
    m_time_elapsed = get_time_elapse(&m_start_time, &m_end_time);   
    this->m_utils->u_log(LOG_FWUPGRADE, "FW: IFWI download used %ds.", m_time_elapsed);
#endif
    m_fwstepsdone = m_fwtotalsteps;
    m_fw_done = true;
    
    m_fwprogress = (int)((m_fwstepsdone/m_fwtotalsteps)*100);
    this->m_utils->u_log(LOG_PROGRESS, "%d",((m_fwstepsdone>m_fwtotalsteps)? 100:m_fwprogress));

}

void ClvDldrState::Visit(ClvFwHandleMFLD& )
{
    int ret_code = 0;
    unsigned char* buffer = NULL;
    ULONG preamble_msg = 0;

    //IDRQ response will be 'MFLD'
    //followed by 32 bytes of data
    buffer = (unsigned char*)(&(this->m_utils->szIdrqData));
    for (int i=0; i<32; i++)
    {
        //printf("%x ", *buffer);
        buffer++;
        if ((i%8) == 7)
            printf("\n");
    }
    printf("===============\n");
    preamble_msg = PREAMBLE_DNER;
    if(!WriteOutPipe((PUCHAR)&preamble_msg, 4))
        ret_code = 1;

    if(ret_code) {
        m_abort = true;
    }
}


void ClvDldrState::Visit(ClvFwHandleCLVT& )
{
    int ret_code = 0;
    unsigned char* buffer = NULL;
    ULONG preamble_msg = 0;

    //IDRQ response will be 'CLVT'
    //followed by 32 bytes of data
    buffer = (unsigned char*)(&(this->m_utils->szIdrqData));
    for (int i=0; i<32; i++)
    {
        //printf("%x ", *buffer);
        buffer++;
        if ((i%8) == 7)
            printf("\n");
    }
    printf("===============\n");
    preamble_msg = PREAMBLE_DNER;
    if(!WriteOutPipe((PUCHAR)&preamble_msg, 4))
        ret_code = 1;

    if(ret_code) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvFwHandleSuCP& )
{
    int ret_code = 0;
    dnx_data* fwdata = NULL;

    if(!m_mfld_fw)
    {
        LogError(0xBAADF00D);
        return;
    }

    m_fwstepsdone++;

    //rom patch
    unsigned char* remainingBytes = NULL;

    this->m_utils->u_log(LOG_FWUPGRADE, "FW: Sending ROM Patch Chunk %d", m_numRomPatchChunks);

    try {
        StartLogTime();

        fwdata = m_mfld_fw->GetFwImageData(FW_DATA_PATCH);
        if(!fwdata)
        {
            LogError(0xBADF00D);
            throw runtime_error("Error FW_DATA_PATCH not found...");
        }

        if(m_residualRomPatchBytes && 0 == m_numRomPatchChunks)
        {
            remainingBytes = new UCHAR [m_residualRomPatchBytes];
            if(remainingBytes) {
                for(int j = 0; j < (int)m_residualRomPatchBytes; j++)
                {
                    remainingBytes[j] = fwdata->data[m_romPatchArrayPtr];
                    m_romPatchArrayPtr ++;
                }

                //Write residual bytes
                if(!WriteOutPipe(&remainingBytes[0], m_residualRomPatchBytes))
                {
                    // do_usb_writes failed on last chunk send.
                    throw 42;
                }

                //release resources
                delete [] remainingBytes;
                m_romPatchArrayPtr = 0;
                //Last bytes were sent reset the 128KB block count
                m_numRomPatchChunks = m_romPatch128KBChunks;
            } else {
                throw 5;
            }
        }
        else
        {
            //Write the 128KB chunk and decrement m_numRomPatchChunks

            //copy in array the next 128KB
            UCHAR byteBlock[ONE28_K];
            for(int i = 0; i < ONE28_K; i++)
            {
                byteBlock[i] = fwdata->data[m_romPatchArrayPtr];
                m_romPatchArrayPtr++;
            }

            if(!WriteOutPipe(byteBlock, ONE28_K))
            {
                // do_usb_writes failed on second 128k send.
                throw 42;
            }

            //Last bytes were sent reset the 128KB block count
            if(m_numRomPatchChunks == 0) {
                m_romPatchArrayPtr = 0;
                //Last bytes were sent reset the 128KB block count
                m_numRomPatchChunks = m_romPatch128KBChunks;
             } else {
                m_numRomPatchChunks--;
            }
        }
    }
    catch( int errorcode ) {
        ret_code = errorcode;
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Exception raised: ErrorCode %d", __FUNCTION__, errorcode);
        LogError(errorcode);
    }
    catch(std::exception & e)
    {
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Exception raised %s", __FUNCTION__, e.what());
        ret_code = 1;
    }
    catch( ... ) {
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Unknown Exception raised", __FUNCTION__);
        ret_code = 1;
    }

    EndlogTime();
    LogProgress();

    if(ret_code) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvFwHandleRTBD& )
{
    int ret_code = 0;
    struct bati_battery_data batt_data;
    uint32 bati_data_size = 0;
    BYTE sBuff[17] = {0};
    ULONG preamble_msg = 0;

    try {
        //SEND ACKNOWLEDGEMENT  "DBDS"
        preamble_msg = PREAMBLE_DBDS;
        if(!WriteOutPipe((PUCHAR)&preamble_msg, 4))
        {
            //abort if error occurs
            throw 1;
        }

        //read in either HLT0 or 4

        if(!ReadInPipe(sBuff, 4))
        {
            throw 6;
        }
        else
        {

            //First to check if it's HLT0 ack, if not then it should be the data size of BATI
            bati_data_size = (sBuff [0] <<24 ) + (sBuff [1] <<16) + (sBuff [2] << 8) + sBuff[3];
            if(bati_data_size == BULK_ACK_HLT0)
                throw 0;
            else
                bati_data_size = (sBuff [3] <<24 ) + (sBuff [2] <<16) + (sBuff [1] << 8) + sBuff[0];
        }

        //while device does not issue HLT0

        //SEND ACKNOWLEDGEMENT  "RRBD"
        preamble_msg = PREAMBLE_RRBD;
        if(!WriteOutPipe((PUCHAR)&preamble_msg, 4))

        {
            //abort if error occurs
            throw 1;
        }

        //Read back 16 bytes BATI data
        //if an error occurs while sending abort and display error
        if(!ReadInPipe(sBuff, bati_data_size*4))
        {
            throw 6;
        }

        batt_data.BATI = (sBuff [0] <<24 ) + (sBuff [1] <<16) + (sBuff [2] << 8) + sBuff[3];
        batt_data.header_size = (sBuff [5] << 8) + sBuff[4];
        batt_data.header_revis = sBuff[6];
        batt_data.xor_check = sBuff[7];
        batt_data.voltage = sBuff[8];
        batt_data.coulomb_count = sBuff[9];

        if(batt_data.BATI != BATI_SIGNATURE)
        {
            this->m_utils->u_log(LOG_FWUPGRADE, "BATI:%x: Battery voltage: 0x%x(%fV)", batt_data.BATI,batt_data.voltage, 4*4.692*batt_data.voltage);
            throw 39;
        }

        //printf("\n\n***********Battery Data***************\n0x%x BATI in hex \n0x%x Header Size \n0x%x Header Revision \n0x%x XOR Checksum \n0x%x Battery Voltage \n0x%x Battery Coulomb Counter\n***********Battery Data***************\n\n",BattData.BATI,BattData.HeaderSize,BattData.HeaderRevis,BattData.XorCheck,BattData.voltage,BattData.coulombCount);
        this->m_utils->u_log(LOG_STATUS, "BATI: Battery power is too low, charging up ...");
        this->m_utils->u_log(LOG_STATUS, "BATI:%x: Battery voltage: 0x%x(%fV) The threshold: 0x%x(%fV)", batt_data.BATI,batt_data.voltage, 4*4.692*batt_data.voltage, 0xbf, 3.585);
    }//try
    catch( int errorcode ) {
        ret_code = errorcode;
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Exception raised: ErrorCode %d", __FUNCTION__, errorcode);
        LogError(errorcode);
        this->m_utils->u_log(LOG_STATUS, "Battery power is okay, continue FW download ...");
    }
    catch(std::exception & e)
    {
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Exception raised %s", __FUNCTION__, e.what());
        ret_code = 1;
    }
    catch( ... ) {
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Unknown Exception raised", __FUNCTION__);
        ret_code = 1;
    }

    if(ret_code) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvFwHandleVEDFW& )
{
    int ret_code = 0;
    dnx_data* fwdata = NULL;

    if(!m_mfld_fw)
    {
        LogError(0xBAADF00D);
        return;
    }

    m_fwstepsdone++;

    unsigned char* remainingBytes = NULL;
    try {
        StartLogTime();

        fwdata = m_mfld_fw->GetFwImageData(FW_DATA_VEDFW);
        if(!fwdata)
        {
            LogError(0xBADF00D);
            throw runtime_error("Error FW_DATA_VEDFW not found...");
        }

        this->m_utils->u_log(LOG_FWUPGRADE, "sending VED FW Chunk %d", m_numVedFwChunks);

        if(m_residualVedFwBytes && 0 == m_numVedFwChunks)
        {
            remainingBytes = new UCHAR [m_residualVedFwBytes];
            if(remainingBytes) {
                for(int j = 0; j < (int)m_residualVedFwBytes; j++)
                {
                    remainingBytes[j] = fwdata->data[m_vedFwArrayPtr];
                    m_vedFwArrayPtr ++;
                }

                //Write residual bytes
                if(!WriteOutPipe(&remainingBytes[0], m_residualVedFwBytes))
                {
                    // do_usb_writes failed on last chunk send.
                    throw 42;
                }

                //release resources
                delete [] remainingBytes;
                m_vedFwArrayPtr = 0;
                //Last bytes were sent reset the 128KB block count
                m_numVedFwChunks = m_vedFw128KBChunks;
            } else {
                throw 5;
            }

        }
        else
        {
            //Write the 128KB chunk and decrement m_numVedFwChunks

            //copy in array the next 128KB
            UCHAR byteBlock[ONE28_K];
            for(int i = 0; i < ONE28_K; i++)
            {
                byteBlock[i] = fwdata->data[m_vedFwArrayPtr];
                m_vedFwArrayPtr++;
            }

            if(!WriteOutPipe(byteBlock, ONE28_K))
            {
                // do_usb_writes failed on second 128k send.
                throw 42;
            }

            //Last bytes were sent reset the 128KB block count
            if(m_numVedFwChunks == 0) {
                m_vedFwArrayPtr = 0;
                //Last bytes were sent reset the 128KB block count
                m_numVedFwChunks = m_vedFw128KBChunks;
             } else {
                m_numVedFwChunks--;
            }
        }
    }
    catch( int errorcode ) {
        ret_code = errorcode;
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Exception raised: ErrorCode %d", __FUNCTION__, errorcode);
        LogError(errorcode);
    }
    catch( ... ) {
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Unknown Exception raised", __FUNCTION__);
        ret_code = 1;
    }

    EndlogTime();
    LogProgress();

    if(ret_code) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvFwHandleSSBIOS& )
{
    int ret_code = 0;
    dnx_data* fwdata = NULL;

    if(!m_mfld_fw)
    {
        LogError(0xBAADF00D);
        return;
    }

    m_fwstepsdone++;

    unsigned char* remainingBytes = NULL;
    try {
        StartLogTime();

        fwdata = m_mfld_fw->GetFwImageData(FW_DATA_VEDFW);
        if(!fwdata)
        {
            LogError(0xBADF00D);
            throw runtime_error("Error FW_DATA_VEDFW not found...");
        }

        this->m_utils->u_log(LOG_FWUPGRADE, "sending Second Stage BIOS Chunk %d", m_numVedFwChunks);

        if(m_residualVedFwBytes && 0 == m_numVedFwChunks)
        {
            remainingBytes = new UCHAR [m_residualVedFwBytes];
            if(remainingBytes) {
                for(int j = 0; j < (int)m_residualVedFwBytes; j++)
                {
                    remainingBytes[j] = fwdata->data[m_vedFwArrayPtr];
                    m_vedFwArrayPtr ++;
                }

                //Write residual bytes
                if(!WriteOutPipe(&remainingBytes[0], m_residualVedFwBytes))
                {
                    // do_usb_writes failed on last chunk send.
                    throw 42;
                }

                //release resources
                delete [] remainingBytes;
                m_vedFwArrayPtr = 0;
                //Last bytes were sent reset the 128KB block count
                m_numVedFwChunks = m_vedFw128KBChunks;
            } else {
                throw 5;
            }

        }
        else
        {
            //Write the 128KB chunk and decrement m_numVedFwChunks

            //copy in array the next 128KB
            UCHAR byteBlock[ONE28_K];
            for(int i = 0; i < ONE28_K; i++)
            {
                byteBlock[i] = fwdata->data[m_vedFwArrayPtr];
                m_vedFwArrayPtr++;
            }

            if(!WriteOutPipe(byteBlock, ONE28_K))
            {
                // do_usb_writes failed on second 128k send.
                throw 42;
            }

            //Last bytes were sent reset the 128KB block count
            if(m_numVedFwChunks == 0) {
                m_vedFwArrayPtr = 0;
                //Last bytes were sent reset the 128KB block count
                m_numVedFwChunks = m_vedFw128KBChunks;
              } else {
                m_numVedFwChunks--;
            }
        }
    }
    catch( int errorcode ) {
        ret_code = errorcode;
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Exception raised: ErrorCode %d", __FUNCTION__, errorcode);
        LogError(errorcode);
    }
    catch( ... ) {
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Unknown Exception raised", __FUNCTION__);
        ret_code = 1;
    }

    EndlogTime();
    LogProgress();

    if(ret_code) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvFwHandleIFW1& )
{
    int ret_code = 0;
    dnx_data* fwdata = NULL;

    if(!m_mfld_fw)
    {
        LogError(0xBAADF00D);
        return;
    }

    m_fwstepsdone++;

    StartLogTime();

    this->m_utils->u_log(LOG_FWUPGRADE, "sending first 96k...");

    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_IFW1);
    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size)){
        // do_usb_writes failed on first 96k send.
        ret_code = 42;
        LogError(ret_code);
    }


    EndlogTime();
    LogProgress();

    if(ret_code) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvFwHandleIFW2& )
{
    int ret_code = 0;
    dnx_data* fwdata = NULL;

    if(!m_mfld_fw)
    {
        LogError(0xBAADF00D);
        return;
    }

    m_fwstepsdone++;

    StartLogTime();

    this->m_utils->u_log(LOG_FWUPGRADE, "sending second 96k...");

    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_IFW2);
    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size)){
        // do_usb_writes failed on second 96k send.
        ret_code = 42;
        LogError(ret_code);
    }

    EndlogTime();
    LogProgress();

    if(ret_code) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvFwHandleIFW3& )
{
    int ret_code = 0;
    dnx_data* fwdata = NULL;

    if(!m_mfld_fw)
    {
        LogError(0xBAADF00D);
        return;
    }

    m_fwstepsdone++;

    StartLogTime();

    this->m_utils->u_log(LOG_FWUPGRADE, "sending third 96k...");

    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_IFW3);
    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size)){
        // do_usb_writes failed on third 96k send.
        ret_code = 42;
        LogError(ret_code);
    }

    EndlogTime();
    LogProgress();

    if(ret_code) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvOsHandleDORM& )
{
    bool ret = true;

    this->m_utils->u_log(LOG_OS, "OS:  Handle OS download...");

    m_b_DnX_OS = 0;
    m_b_DnX_OS = m_gpflags & 0x20;
#if 0
    gettimeofday(&m_start_time, NULL);
#endif
    if(m_b_DnX_OS && (strlen(m_fname_dnx_misc) > 0)) {
        //Start Misc OS dowload state
        GotoState(DLDR_STATE_OS_MISC);
    } else if((m_gpflags & 0x1) && (strlen(m_fname_dnx_os) > 0)){
        //Start normal OS dowload state
        GotoState(DLDR_STATE_OS_NORMAL);
    } else {
        this->m_utils->u_log(LOG_STATUS, "OS: Got DORM but OS recovery was not requested, booted into fw without fw dnx?");
        ret = false;
    }

    if(!ret) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvOsHandleOSIPSZ& )
{
}

void ClvDldrState::Visit(ClvOsHandleROSIP& )
{
    int ret_code = 0;
    dnx_data* fwdata = NULL;

    if(!m_mfld_os)
    {
        LogError(0xBAADF00D);
        return;
    }

    m_fwstepsdone++;

    fwdata = m_mfld_os->GetOsipHdr();

    StartLogTime();

    this->m_utils->u_log(LOG_OS, "sending OSIP...");
    // 3) Send OSIP HDR --------------------------------------------------------
    //  -- allocate temp buffer for header
    //  -- copy to local buffer
    //  -- send header, move file pointer to end of header
    // Sending OSIP Header
    this->m_utils->u_log(LOG_OS, "sending OSIP header + Partion Table...");

    // write out pipe
    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size)){
        ret_code = 36; //"Error write out pipe for OSIP header"
        LogError(ret_code);
    }

    EndlogTime();

    if(ret_code) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvOsHandleDONE& )
{
}

void ClvDldrState::Visit(ClvOsHandleRIMG& )
{
    int ret_code = 0;
    dnx_data* fwdata = NULL;

    if(!m_mfld_os)
    {
        LogError(0xBAADF00D);
        return;
    }

    fwdata = m_mfld_os->GetOsImageDataChunk();

    StartLogTime();

    this->m_utils->u_log(LOG_OS, "sending data to USB...");
    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size))
    {
        ret_code = 39;  // do_usb_writes failed sending OS data
        LogError(ret_code);
    }

    this->m_utils->RIMGChunkSize = 0;

    // free pkt_buffer to re-allocate next size on next phase
    if(fwdata) {
        m_mfld_os->ReleaseOsImageDataChunk();
    }

    EndlogTime();

    this->m_utils->u_log(LOG_OS, "End of RIMG ");

    if(ret_code) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvOsHandleEOIU& )
{
    int ret_code = 0;
    UCHAR DFN[4] = "DFN";
    //TIME_T start_time, end_time;

    if(!m_mfld_os)
    {
        LogError(0xBAADF00D);
        return;
    }

    uint64 os_data_size = m_mfld_os->GetOsImageDataSize();

    StartLogTime();
    this->m_utils->u_log(LOG_OS, "EOIU Received...");

    if(!m_b_DnX_OS && os_data_size != 0)
    {
        ret_code = 12;
        LogError(ret_code);
    }

    // write out pipe
    this->m_utils->u_log(LOG_OS, "sending DFN...");

    if(!WriteOutPipe(DFN, sizeof(DFN)))
    {
        this->m_utils->u_log(LOG_OS, "After DFN is sent, if status is bad it will be ignored");
    }

    //if there are more OS images
    if(!m_b_DnX_OS && os_data_size != 0)
    {

        this->m_utils->u_log(LOG_STATUS, "EOIU received but os_data_size is not zero: %d", os_data_size);
        this->m_utils->u_log(LOG_STATUS, "There are more OS images ...");
        this->m_utils->u_log(LOG_STATUS, "Allow 6 minutes to receive next ROSIP ...");
        //Allow 6 minutes to receive next ROSIP
#if 1
        //TIME_T start_time, end_time;
        ULONGLONG timeout = 3600; //6 mins in 100ms interval
        ULONGLONG time_counter = 0;
        unsigned long long data;
        //within 6 mins then process ROSIP
        do
        {
            data = ReadInAck();
            if(data == BULK_ACK_ROSIP)
            {
                m_ROSIP_data_flag = true;
            }
           // clock_gettime(CLOCK_REALTIME, &end_time);
            SleepMs(100);
            time_counter++; //= get_time_elapse(start_time, end_time);
        }while(time_counter <= timeout && data != BULK_ACK_ROSIP);
        this->m_utils->u_log(LOG_STATUS,"OS: TIMEOUT waiting for ROSIP!! ");
#endif
    }
    else
    {
        int osprogress = 100;
#if 0
        //Log time elapsed for OS download        
        gettimeofday(&m_end_time, NULL);
        m_time_elapsed = get_time_elapse(&m_start_time, &m_end_time);
        this->m_utils->u_log(LOG_OS, "OS: Operating system download used %ds.", m_time_elapsed);
#endif
        this->m_utils->u_log(LOG_PROGRESS, "%d",osprogress);
        this->m_utils->u_log(LOG_OS, "OS: Operating system download completed.");

#ifdef AMI_POC
        //If state is in OS misc dnx state, then continue...
        if(m_dldr_state != DLDR_STATE_OS_MISC)
            m_os_done = 1;

        //If doing misc dnx, then need to reset the state to normal after it's done
        if(m_b_DnX_OS && m_dldr_state == DLDR_STATE_OS_MISC) {
            GotoState(DLDR_STATE_OS_NORMAL);
        }
#else
         m_os_done = 1;
#endif
         ret_code = 0;

         SleepMs(2000);
    }

    EndlogTime();

    if(ret_code) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvStHandleFwNormal& )
{
    bool ret = false;

    StartLogTime();

    this->m_utils->u_log(LOG_FWUPGRADE, "Changing to DLDR_STATE_FW_NORMAL ...");

    m_mfld_fw = new CloverviewFW;

    if(m_mfld_fw->Init(m_fname_dnx_fw, m_fname_fw_image, m_utils, m_gpflags, false))
    {
        //Set the state machine to virgin emmc download
        m_dldr_state = DLDR_STATE_FW_NORMAL;
        this->m_utils->u_log(LOG_STATUS, "FW download is in progress ... ");
        //Send DnX FW size header data to target device
        //This will start the FW state machine
        ret = StartFw();
    }
    else if(this->m_utils->FileSize(m_fname_dnx_fw) == 0) {
        ret = HandleNoSize();// == BULK_ACK_HLT0)
    }

    this->m_utils->u_log(LOG_FWUPGRADE, "End of ClvStHandleFwNormal ");

    EndlogTime();

    if(!ret) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvStHandleFwMisc& )
{
    bool ret = false;

    StartLogTime();

    this->m_utils->u_log(LOG_FWUPGRADE, "Changing to DLDR_STATE_FW_MISC ...");

    //Set the state machine to virgin emmc download
    //Need to set this first, since mfld_fw.init could fail because of fw blank.bin files.
    m_dldr_state = DLDR_STATE_FW_MISC;

    m_mfld_fw = new CloverviewFW;

    if(m_mfld_fw->Init(m_fname_dnx_misc, m_fname_fw_image, m_utils, m_gpflags, false) && !m_b_DnX_OS)
    {
        //Send DnX FW size header data to target device
        //This will start the FW state machine
        this->m_utils->u_log(LOG_STATUS, "FW(Miscdnx) download is in progress ... ");

        ret = StartFw();
    } else if(m_b_DnX_OS || (this->m_utils->FileSize(m_fname_dnx_misc) == 0) || (this->m_utils->FileSize(m_fname_fw_image) == 0)) {
        ret = HandleNoSize();// == BULK_ACK_HLT0)
        ret = true;
    }

    this->m_utils->u_log(LOG_FWUPGRADE, "End of ClvStHandleFwMisc ");

    EndlogTime();

    if(!ret) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvStHandleFwWipe& )
{
    bool ret = false;
    unsigned long temp_gpflags = 0;

    StartLogTime();

    this->m_utils->u_log(LOG_FWUPGRADE, "Changing to DLDR_STATE_FW_WIPE ...");

    //Set the state machine to virgin emmc download
    //Need to set this first, since mfld_fw.init could fail because of fw blank.bin files.
    m_dldr_state = DLDR_STATE_FW_WIPE;

    m_mfld_fw = new CloverviewFW;

    //Set gpflags to do a FW cold reset
    temp_gpflags = m_gpflags | 0x2;

    if(m_mfld_fw->Init(m_fname_dnx_fw, m_fname_fw_image, m_utils, temp_gpflags, true) && !m_b_DnX_OS)
    {
        //Send DnX FW size header data to target device
        //This will start the FW state machine
        this->m_utils->u_log(LOG_STATUS, "FW Wipe IFWI download is in progress ... ");
        ret = StartFw();
        m_ifwi_done = true;
    } else if(m_b_DnX_OS || (this->m_utils->FileSize(m_fname_dnx_fw) == 0)) {
        ret = HandleNoSize();// == BULK_ACK_HLT0)
        ret = true;
    }

    this->m_utils->u_log(LOG_FWUPGRADE, "End of ClvStHandleFwWipe ");

    EndlogTime();

    if(!ret) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvStHandleOsNormal& )
{
    bool ret = false;
    int ret_code = 0;

    StartLogTime();

    this->m_utils->u_log(LOG_OS, "Changing to DLDR_STATE_OS_NORMAL ...");

    m_mfld_os = new CloverviewOS;

    ret = m_mfld_os->Init(m_fname_dnx_os, m_fname_os_image, m_utils, m_gpflags);

    if(ret)
    {
        //Send DnX OS size header data to target device
        //This will start the OS state machine
        m_dldr_state = DLDR_STATE_OS_NORMAL;
        this->m_utils->u_log(LOG_STATUS, "POS download is in progress ... ");

#ifdef AMI_POC
        if(m_b_DnX_OS && strlen(m_fname_dnx_misc) > 3) {
            SleepMs(5000);

            //Since device may "randomly" re-enumerate on the bus or reboot...
            //detect a working handle by trying to grab the first device...
            //if we are flashing diff device types we probably should re-enumerate
            //here rather than expecting same dev type
            void *tmphandle = NULL;
            int counter = 0;

            while(tmphandle == NULL) {
                tmphandle = NULL;
                tmphandle = m_usbdev->GetDeviceHandle(0);
                counter++;

                SleepMs(1000);
                std::cout << "XFSTK-STATUS--Sleep one second and try to detect the device a gain...\n";

                if(counter > 20) {
                    std::cout << "XFSTK-STATUS--Timeout to detect the device, make sure to charge up your phone before flashing\n";
                    ret = false;
                }
            }
            m_usbdev->SetDeviceHandle(tmphandle);
            m_b_IDRQ = false;
            ret = Start();
        } else {
            ret = StartOs();
        }
#else
        ret = StartOs();
#endif
    }
    else if(this->m_utils->FileSize(m_fname_dnx_os) == 0) {
        dnx_data* fwdata = m_mfld_os->GetNoSizeData();
        if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size)) {
            ret_code = 1; //USB error
            LogError(ret_code);
        }
        ret = (ret_code ? false:true);
    }

    m_os_done = 0;

    this->m_utils->u_log(LOG_OS, "End of ClvStHandleOsNormal ");

    EndlogTime();

    if(!ret) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvStHandleOsMisc& )
{
    bool ret = false;
    int ret_code = 0;

    StartLogTime();

    this->m_utils->u_log(LOG_OS, "Changing to DLDR_STATE_OS_MISC ...");

    m_mfld_os = new CloverviewOS;

    ret = m_mfld_os->Init(m_fname_dnx_os, m_fname_dnx_misc, m_utils, m_gpflags);
    if(ret)
    {
        //Send DnX OS size header data to target device
        //This will start the OS state machine
        m_dldr_state = DLDR_STATE_OS_MISC;
        this->m_utils->u_log(LOG_STATUS, "POS(LPDDR) download is in progress ... ");

        ret = StartOs();
    }
    else if(this->m_utils->FileSize(m_fname_dnx_os) == 0) {
        dnx_data* fwdata = m_mfld_os->GetNoSizeData();
        if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size)) {
            ret_code = 1; //USB error
            LogError(ret_code);
        }
        ret = (ret_code ? false:true);
    }

    this->m_utils->u_log(LOG_OS, "End of ClvStHandleOsMisc ");

    EndlogTime();

    if(!ret) {
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvErHandleLogError& )
{
    ErrorMapType::iterator iter;
    if(m_achCode) {
        iter = m_fw_error_map.find(m_achCode);
        if(iter != m_fw_error_map.end()) {
            LogError(iter->second);
            m_abort = true;
        }
    } else {
        this->m_utils->u_log(LOG_STATUS, "Unknown ACK code, aborting ...");
        m_abort = true;
    }
}

void ClvDldrState::Visit(ClvErHandleERRR& )
{
    int ret_code = 0;
    ULONG preamble_msg = PREAMBLE_BMRQ;
    if(m_achCode) {

        this->m_utils->u_log(LOG_OPCODE, "Sending BMRQ...");
        if(!WriteOutPipe((PUCHAR)&preamble_msg, 4))
        {
            ret_code = 1; //USB error
            LogError(ret_code);
        }
    } else {
        this->m_utils->u_log(LOG_STATUS, "Unknown ACK code, aborting ...");
        m_abort = true;
    }

}

bool ClvDldrState::Start()
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    INT ret_code = 0;
    ULONG preamble_msg = PREAMBLE_DNER;

    if(m_b_IDRQ)
    {
        this->m_utils->u_log(LOG_OPCODE, "Sending IDRQ...");
        preamble_msg = PREAMBLE_IDRQ;
    }
    else
    {
        // --- Send DnER:  ---
        preamble_msg = PREAMBLE_DNER;
        this->m_utils->u_log(LOG_OPCODE, "Sending DnER...");
    }
    if(!WriteOutPipe((PUCHAR)&preamble_msg, 4))
    {
        ret_code = 1; //USB error
        LogError(ret_code);
    }
    return ret_code ? false:true;
}

void ClvDldrState::GotoState(unsigned long long state)
{
    m_dldr_state = state;
    m_state_change = true;
}

void ClvDldrState::SleepMs(int delay)
{
#if defined XFSTK_OS_WIN
        Sleep(delay);
#else
        usleep(1000*delay);
#endif
}

bool ClvDldrState::StateMachine()
{
    bool ret = true;
    HandleMapType::iterator iter1;
    unsigned long long ackcode = 0;

    m_abort = false;

    //Start the downloader state machine
    if(!Start()) {
        this->m_utils->u_log(LOG_STATUS, "Failed to start Dldr state machine, aborting ...\n");
        return false;
    }

    //Looping until the download is done or fail.
    while(1) {

        ackcode = GetOpCode();

        iter1 = m_bulk_ack_map.find(ackcode);
        if (iter1 != m_bulk_ack_map.end() ) {
            iter1->second->Accept(*this);
        } else {
            this->m_utils->u_log(LOG_STATUS, "Unknown Ack code:%x, aborting ...\n", ackcode);
            m_abort = true;
            ret = false;
            if(ackcode == 0xBAADF00D)
            {
                this->m_last_error.error_code = 0xBAADF00D;
                strcpy(this->m_last_error.error_message, Cloverview_error_code_array[4]);
            }
        }

        if(m_abort) {
            ret = false;
            break;
        }

        if(m_fw_done) {
            ret = true;
            LogError(0);
            break;
        }

        if(m_os_done) {
            ret = true;
            LogError(0);
            break;
        }
    }
    
    return ret;
}

