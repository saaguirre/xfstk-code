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
#include <map>
#include <utility> 
#include <sys/stat.h>

#if defined XFSTK_OS_WIN
#include <windows.h>
#endif
#include <string.h>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include "common/scoped_file.h"
#include "cloverviewplusmessages.h"
#include "clvpdldrstate.h"
#include "../softfuses/softfusesMessages.h"
#include "../softfuses/softfusesfw.h"
#include <boost/smart_ptr/scoped_array.hpp>

#define USB_READ_WRITE_DELAY_MS 0

extern CPSTR CloverviewPlus_error_code_array [MAX_ERROR_CODE_CLOVERVIEWPLUS];  //JG - This is really used but it ijust a data array that we can just read.

#define FILENAME "emmc_dump2.bin"
#define BLOCKSIZE          512
#define NUMBLOCKS          6
#define EMMC_START_ADDR    0
#define USER_PARTITION     0
#define BOOT_PARTITION_1   1
#define BOOT_PARTITION_2   2
#define EXT_CSD_MODE       3
#define BUF_SIZE           256*1024 // 256K
#define ACK_MESSAGE_SIZE   4
#define DNER_MESSAGE_SIZE  4
#define EXTCSD_RETURN_SIZE 512
#define EOIO_MESSAGE_SIZE   20

namespace
{
    typedef  struct{
        char string[4];
        unsigned int blockSize;
        unsigned int offsetInEmmc;
        unsigned int numBlocks;
    } txHeader_t;

    const unsigned int SECONDS_PER_YEAR  = 31556952; // (365.2425 days/year) (24 hours/day) (60 minutes/hour) (60 seconds/minute) =  31556952 seconds/year
    const unsigned int SECONDS_PER_MONTH = 2629746;  // SECONDS_PER_YEAR/12
    const unsigned int SECONDS_PER_DAY   = 86400;    // (24 hours/day) (60 minutes/day) (60 seconds/minute)
    const unsigned int SECONDS_PER_HOUR  = 3600;     // (60 minutes/day) (60 seconds/minute)
    const unsigned int MAX_SECONDS       = 0xFFFFFFFF;     // 0xFFFFFFFF seconds

    const int MAX_KEY_COUNT = 16;

    unsigned int convertDurationToSeconds(std::string duration)
    {
        // Duration Supported format shall be in a string which
        //    starts with a numeric number following by h/d/m/y
        //    (h for hour, d for day, m for month and y for year).
        //    For example, if one provide 17d, it means the token
        //    string will expire in 17 days. If the value of the
        //    duration is zero (i.e. 0h,0d, 0m or 0y), it means the
        //    token will never expire.


        //"trim" - use the erase-remove idiom to delete the white space.
        duration.erase(std::remove(duration.begin(), duration.end(), ' '), duration.end() );

        //get the expiry format - h, d, m, y
        const char format = duration[duration.size() -1];

        //get the time in string format and convert it to UINT.
        const std::string sTime = duration.substr(0, duration.size()-1);
        std::stringstream ss(sTime);
        unsigned int time = 0;
        ss >> time;
        unsigned int seconds = 0xFFFFFFFF;

        //convert the expiration duration to seconds.
        switch(format)
        {
        case 'h': //convert hours to seconds
            if(time <= (MAX_SECONDS / SECONDS_PER_HOUR)) //check overflow, A <= 0xFFFFFFFF/B
                seconds = time * SECONDS_PER_HOUR;
            break;
        case 'd': //convert days to seconds
            if(time <= (MAX_SECONDS / SECONDS_PER_DAY))
                seconds = time * SECONDS_PER_DAY;
            break;
        case 'm': //convert months to seconds
            if(time <= (MAX_SECONDS / SECONDS_PER_MONTH))
                seconds = time * SECONDS_PER_MONTH;
            break;
        case 'y': //convert years to seconds
            if(time <= (MAX_SECONDS / SECONDS_PER_YEAR))
                seconds = time * SECONDS_PER_YEAR;
            break;
        default:
            std::string msg("Unknown expiration duration format. Please use h, d, m, y. (e.g.  '10d' represents 10 days)");
            throw std::runtime_error(msg);
            break;
        }

        return seconds;
    }

    unsigned int convertHexStringToInteger(const std::string& stringValue)
    {
        unsigned int integerValue = 0xff;
        try
        {
            if(!stringValue.empty())
            {
                size_t found = stringValue.find("x");

                if(found != std::string::npos)
                {
                    std::string newStringValue = stringValue.substr(found+1);
                    std::stringstream ss(newStringValue);
                    ss << std::hex << newStringValue.c_str();
                    ss>>integerValue;
                }
                else
                {
                    integerValue = atoi(stringValue.c_str());
                }
            }
        }
        catch(std::exception& e )
        {
            std::cout << "Error: "<< e.what() << std::endl;
        }


        return integerValue;
    }

    bool writeTokenToFile(const std::string& filename, const std::string& uid, const std::string& expiry, bool isExpiryEnabled)
    {
        std::stringstream ss1, ss2;
        for(std::size_t i=0; i<uid.length(); ++i)
        {
            unsigned int idata = static_cast<unsigned char> (uid[i]);
            ss1 << std::hex << std::setfill('0') << std::setw(2) << idata;
        }
        for(std::size_t i=0; i<expiry.length(); ++i)
        {
            unsigned int idata = static_cast<unsigned char> (expiry[i]);
            ss2 << std::hex << std::setfill('0') << std::setw(2) << idata;
        }
        try
        {
            {
                scoped_file tokenfile(filename.c_str(), "wb");
                tokenfile.write(ss1.str().c_str(), sizeof(char), 32);
            }


            if(isExpiryEnabled)
            {
                {
                    scoped_file tokenfile(filename.c_str(), "a");
                    tokenfile.write("\n", sizeof(char), 1);

                }
                {
                    scoped_file tokenfile(filename.c_str(), "ab");
                    tokenfile.write(ss2.str().c_str(), sizeof(char), 16);
                }

            }

            return true;
        }
        catch(std::exception& e)
        {
            std::cout << "Error: " << e.what() <<std::endl;
        }

        return false;
    }

    bool writeTokenToUnsignedDnXAtOffset(const std::string& filename, const std::string& token, unsigned int offset, bool isExpiryEnabled)
    {
        try
        {
            scoped_file tokenfile(filename.c_str(),"r+b");
            tokenfile.seek( offset , SEEK_SET );
            if(isExpiryEnabled)
            {
                tokenfile.write(token.c_str(), sizeof(char), 24);
            }
            else
            {
                tokenfile.write(token.c_str(), sizeof(char), 16);
            }
            return true;
        }
        catch(std::exception& e)
        {
            std::cout << "Error: " << e.what() <<std::endl;
        }
        return false;
    }
}

//ClvpDldrState Implementation
ClvpDldrState::ClvpDldrState()
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
    m_fwtotalsteps = DLDR_FW_TOTAL_STEPS_CLVP;//12;
    m_b_IDRQ = false;
    m_fw_done = false;
    m_ifwi_done = false;
    m_os_done = false;
    m_abort = false;
    m_GPPReset = false;
    m_b_DnX_OS = 0;
    m_sf_error = false;
    m_sf_retry = false;

    m_dldr_state = DLDR_STATE_INVALID;
    m_ROSIP_data_flag = false;

    m_mfld_fw = NULL;
    m_mfld_os = NULL;
    m_mfld_softfuses = NULL;
    m_fname_softfuses_bin = "BLANK.bin";

    m_delay_ms = USB_READ_WRITE_DELAY_MS;
    m_emmc_fname_signed_dnx = "";
    m_emmc_outfile = "";
    m_emmc_unsigned_fw_dnx = "";
    m_emmc_token_offset = "";
    m_emmc_expirationdur = "";
    m_emmc_partition = 0;
    m_emmc_blocksize = 0;
    m_emmc_blockcount = 0;
    m_emmc_offset = 0;
    m_emmc_umip_enabled = false;
    m_emmc_register_token_enabled = false;
    m_usb_delayms = USB_READ_WRITE_DELAY_MS;;
    m_perform_emmc_dump = false;

    // FW Upgrade Ack values, index pair map
    int i = 0;
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_INVALID_PING, new ClvpFwHandleER00));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_HLT0, new ClvpFwHandleHLT0));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_DFRM, new ClvpFwHandleDFRM));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_DSKF, new ClvpFwHandleDSKF));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_DxxM, new ClvpFwHandleDxxM));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_DXBL, new ClvpFwHandleDXBL));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_READY_UPH_SIZE, new ClvpFwHandleRUPHS));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_READY_SFUSE_UPH_SIZE, new ClvpFwHandleRSUPHS));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_READY_UPH, new ClvpFwHandleRUPH));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_READY_SFUSE_UPH, new ClvpFwHandleRSUPH));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_DMIP, new ClvpFwHandleDMIP));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_LOFW, new ClvpFwHandleLOFW));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_HIFW, new ClvpFwHandleHIFW));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_PSFW1, new ClvpFwHandlePSFW1));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_PSFW2, new ClvpFwHandlePSFW2));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_SSFW, new ClvpFwHandleSSFW));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_UPDATE_SUCESSFUL, new ClvpFwHandleHLT$));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_MFLD, new ClvpFwHandleMFLD));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_CLVT, new ClvpFwHandleCLVT));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_PATCH, new ClvpFwHandleSuCP));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_RTBD,  new ClvpFwHandleRTBD));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_VEDFW, new ClvpFwHandleVEDFW));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_SSBS, new ClvpFwHandleSSBIOS));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_IFW1, new ClvpFwHandleIFW1));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_IFW2, new ClvpFwHandleIFW2));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_IFW3, new ClvpFwHandleIFW3));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_GPP_RESET, new ClvpFwHandleRESET));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_DORM, new ClvpOsHandleDORM));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_OSIPSZ, new ClvpOsHandleOSIPSZ));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ROSIP, new ClvpOsHandleROSIP));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_DONE, new ClvpOsHandleDONE));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_RIMG, new ClvpOsHandleRIMG));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_EOIU, new ClvpOsHandleEOIU));
    // eMMC dump
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_RDY$, new ClvpEmmcDumpRDY$));
    m_bulk_ack_map.insert(std::make_pair(EMMC_DUMP_ACK, new ClvpEmmcDump$ACK));
    m_bulk_ack_map.insert(std::make_pair(EMMC_DUMP_NACK, new ClvpEmmcDumpNACK));
    m_bulk_ack_map.insert(std::make_pair(CLVP_EMMC_DUMP_STATE_TRANSFER, new ClvpStEmmcDumpTransfer));
    m_bulk_ack_map.insert(std::make_pair(EMMC_DUMP_ECSD, new ClvpEmmcDumpECSD));
    m_bulk_ack_map.insert(std::make_pair(EMMC_DUMP_SPR$, new ClvpEmmcDumpSPR$));
    m_bulk_ack_map.insert(std::make_pair(EMMC_DUMP_REQB, new ClvpEmmcDumpREQB));
    m_bulk_ack_map.insert(std::make_pair(EMMC_DUMP_EOIO, new ClvpEmmcDumpEOIO));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER40, new ClvpEmmcDumpER40));

    //Set up device error code handler
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER01, new ClvpErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER02, new ClvpErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER03, new ClvpErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER04, new ClvpErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER10, new ClvpErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER11, new ClvpErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER12, new ClvpErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER13, new ClvpErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER15, new ClvpErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER16, new ClvpErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER17, new ClvpErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER18, new ClvpErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER20, new ClvpErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER21, new ClvpErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER22, new ClvpErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER25, new ClvpErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER30, new ClvpErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER32, new ClvpErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER14, new ClvpErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ERRR, new ClvpErHandleERRR));

    //Setup state map
    m_bulk_ack_map.insert(std::make_pair(DLDR_STATE_FW_NORMAL, new ClvpStHandleFwNormal));
    m_bulk_ack_map.insert(std::make_pair(DLDR_STATE_FW_MISC, new ClvpStHandleFwMisc));
    m_bulk_ack_map.insert(std::make_pair(DLDR_STATE_FW_WIPE, new ClvpStHandleFwWipe));
    m_bulk_ack_map.insert(std::make_pair(DLDR_STATE_OS_NORMAL, new ClvpStHandleOsNormal));
    m_bulk_ack_map.insert(std::make_pair(DLDR_STATE_OS_MISC, new ClvpStHandleOsMisc));
    m_bulk_ack_map.insert(std::make_pair(DLDR_STATE_SOFT_FUSES, new ClvpStHandleSoftFuses));

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
    i=45;
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER30, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER32, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER14, i++));
    m_last_error.error_code = 1;
    strcpy(m_last_error.error_message, "Init error");

}

ClvpDldrState::~ClvpDldrState()
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
    if(m_mfld_softfuses)
    {
        delete m_mfld_softfuses;
        m_mfld_softfuses = NULL;
    }

}

bool ClvpDldrState::Init(IDevice * usbdev, CloverviewPlusUtils* utils)
{
    if(!m_b_usbinitok) {
        m_usbdev = usbdev;
        m_utils = utils;
        m_b_usbinitok = true;
    }
    m_ROSIP_data_flag = false; 

    return true;
}
bool ClvpDldrState::UsbDevInit(void)
{
    m_b_usbinitok = false;
    m_b_usbinitok = m_usbdev->Open();
    return m_b_usbinitok;
}

void ClvpDldrState::SetOptions(CloverviewPlusOptions *Options)
{
    DeviceSpecificOptions = Options;
}

bool ClvpDldrState::DoEmmcUpdate(char* emmc_fname_signed_dnx, std::string emmc_outfile, \
                       std::string emmc_token_offset, std::string emmc_expirationdur, int emmc_partition, \
                       long emmc_blocksize,long emmc_blockcount, long emmc_offset, bool emmc_umip_enabled, \
                       unsigned int usb_delayms)
{
    m_fw_done = false;
    m_GPPReset = false;
    m_abort = false;
    m_state_change = false;

    if(DeviceSpecificOptions->IsEmmcRegisterTokenEnabled())
    {
        return doRegisterToken();
    }
    else
    {
        m_fname_dnx_fw = emmc_fname_signed_dnx;
        m_emmc_outfile = emmc_outfile;
        m_emmc_token_offset = emmc_token_offset;
        m_emmc_expirationdur = emmc_expirationdur;
        m_emmc_partition = emmc_partition;
        m_emmc_blocksize = emmc_blocksize;
        m_emmc_blockcount = emmc_blockcount;
        m_emmc_offset = emmc_offset;
        m_emmc_umip_enabled = emmc_umip_enabled;
        m_delay_ms = usb_delayms;
        m_perform_emmc_dump = true;
        m_ifwiwipe_enable = false;

        return StateMachine();
    }
}

bool ClvpDldrState::DoUpdate(char* fname_dnx_fw, char* fname_fw_image, char* fname_softfuses_bin, char* fname_dnx_os, \
                         char* fname_os_image, char* fname_dnx_misc, unsigned long gpflags, \
                         unsigned long usbdelayms, bool ifwiwipeenable)
{
    bool ret = true;
    m_fname_dnx_fw = fname_dnx_fw;
    m_fname_fw_image = fname_fw_image;
    m_fname_softfuses_bin = fname_softfuses_bin;
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
    m_sf_flag = false;

    // Contour fix XFSTK_V1-BUG-287
    if((this->m_utils->FileSize(m_fname_dnx_os) != 0) && (this->m_gpflags & 0x00000002))
    {
        this->LogError(0x00000000BAADF00D, "\nGP flag's bit 2 set is not compatible with OS flashing on this target");
        return false;
    }



    ret = StateMachine();
    return ret;

}

bool ClvpDldrState::GetOsStatus()
{
    return m_os_done;
}

bool ClvpDldrState::GetFwStatus()
{
    return m_fw_done;
}

bool ClvpDldrState::GetSFStatus()
{
    return m_sf_error;
}

bool ClvpDldrState::GetSFRetryStatus()
{
    return m_sf_retry;
}

bool ClvpDldrState::GetIFWIStatus()
{
    return m_ifwi_done;
}

bool ClvpDldrState::IsFwState()
{
    return (m_dldr_state == DLDR_STATE_FW_NORMAL || m_dldr_state == DLDR_STATE_FW_MISC);
}

bool ClvpDldrState::IsOsState()
{
    return (m_dldr_state == DLDR_STATE_OS_NORMAL || m_dldr_state == DLDR_STATE_OS_MISC);
}

bool ClvpDldrState::IsProvisionFailed()
{
    return m_abort;
}

bool ClvpDldrState::IsGPPReset()
{
    return m_GPPReset;
}

bool ClvpDldrState::IsOsOnly()
{
    return (m_b_DnX_OS || (this->m_utils->FileSize(m_fname_fw_image) == 0));
}

bool ClvpDldrState::WriteOutPipe(unsigned char* pbuf, uint32 size)
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

bool ClvpDldrState::ReadInPipe(unsigned char* pbuf, uint32 size)
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

unsigned long long ClvpDldrState::ReadInAck()
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
        //ReInit();
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

unsigned long long ClvpDldrState::GetOpCode()
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

void ClvpDldrState::CheckFile(char *filename)
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

bool ClvpDldrState::HandleNoSize()
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    int ret_code = 0;
    dnx_data* fwdata = NULL;

    if (m_dldr_state == DLDR_STATE_SOFT_FUSES)
    {
        fwdata = m_mfld_softfuses->GetSoftFusesFileData(SFUSE_BIN_SUP0H);
    }
    else
        fwdata = m_mfld_fw->GetFwImageData(FW_DATA_DNX0H_CLVP);

    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size)) {
        ret_code = 1; //USB error
        LogError(ret_code);
    }
    return (ret_code ? false:true);
}

void ClvpDldrState::LogError(unsigned long errorcode)
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    if(errorcode == 0xBAADF00D)
    {
        m_last_error.error_code = errorcode;
        this->m_utils->u_log(LOG_STATUS, "Error Firmware/OS Image Not initialized");
        m_abort = true;
    }
    if(errorcode < static_cast<unsigned long>(MAX_ERROR_CODE_CLOVERVIEWPLUS)) {
        if((errorcode) == 0 || (errorcode == 45 || (errorcode == 46))) {
            this->m_utils->u_log(LOG_STATUS, "Error Code: %d - %s", errorcode, CloverviewPlus_error_code_array[errorcode]);
        } else {
            this->m_utils->u_log(LOG_STATUS,"Error Code: %d - %s", errorcode, CloverviewPlus_error_code_array[errorcode]);
            m_abort = true;
        }
        m_last_error.error_code = errorcode;
        strcpy(m_last_error.error_message,CloverviewPlus_error_code_array[errorcode]);
    }
}

void ClvpDldrState::LogError(int errorcode, std::string msg)
{
    m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);

    if(errorcode == 0) {
        m_utils->u_log(LOG_STATUS, "Error Code: %d - %s", errorcode, msg.c_str());
    } else {
        m_utils->u_log(LOG_STATUS,"Error Code: %d - %s", errorcode, msg.c_str());
        m_abort = true;
    }
    m_last_error.error_code = errorcode;

    char * str = new char[msg.size() + 1];
    std::copy(msg.begin(), msg.end(), str);
    str[msg.size()] = '\0';
    strcpy(m_last_error.error_message, str);
    delete[] str;
}

bool ClvpDldrState::GetLastError(last_error* er)
{
    er->copy(m_last_error);
    return true;
}
void ClvpDldrState::StartLogTime()
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

void ClvpDldrState::EndlogTime()
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

void ClvpDldrState::LogProgress()
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    m_fwprogress = (int)((m_fwstepsdone/m_fwtotalsteps)*100);
    this->m_utils->u_log(LOG_PROGRESS, "%d",((m_fwstepsdone>m_fwtotalsteps)? 100:m_fwprogress));
}

bool ClvpDldrState::FwDXBL()
{   
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    int ret_code = 0;

    dnx_data* fwdata = NULL;
    if(!m_mfld_fw)
    {
        LogError(0xBADF00D);
        return false;
    }
    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_DNXD_CLVP);
    this->m_utils->u_log(LOG_FWUPGRADE, "Sending FW Dnx data...%x", fwdata);

    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size))
    {
        ret_code = 1; //USB error
        LogError(ret_code);
    }
    return ret_code ? false:true;
}

bool ClvpDldrState::FwDSKF()
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    int ret_code = 0;

    dnx_data* fwdata = NULL;

    if(!m_mfld_softfuses)
    {
        LogError(0xBAADF00D);
        return false;
    }

    fwdata = m_mfld_softfuses->GetSoftFusesFileData(SFUSE_BIN_KEY_FILE);
    this->m_utils->u_log(LOG_FWUPGRADE, "Sending Key File Data...%x", fwdata);

    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size))
    {
        ret_code = 1; //USB error
        LogError(ret_code);
    }
    return ret_code ? false:true;
}

bool ClvpDldrState::OsDXBL()
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    int ret_code = 0;

    dnx_data* fwdata = NULL;

    if(!m_mfld_os)
    {
        LogError(0xBAADF00D);
        return false;
    }

    fwdata = m_mfld_os->GetOsDnX();

    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size))
    {
        ret_code = 1; //USB error
        LogError(ret_code);
    }
    return ret_code ? false:true;
}

bool ClvpDldrState::StartFw()
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    int ret_code = 0;
    dnx_data* fwdata = NULL;

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

    this->m_utils->u_log(LOG_FWUPGRADE, "CLVP Total number of calculated steps : %f",m_fwtotalsteps);
    // Start fw update
    this->m_utils->u_log(LOG_FWUPGRADE, "sending 6 DWORDS of DnxFW size and checksum...");
    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_DNXH_CLOVERVIEWPLUS);
    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size))
    {
        ret_code = 1; //USB error
        LogError(ret_code);
    }
    return (ret_code ? false:true);
}

bool ClvpDldrState::StartSoftfuses()
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    int ret_code = 0;
    dnx_data* SFdata = NULL;
    if(!m_mfld_softfuses)
    {
        LogError(0xBAADF00D);
        return false;
    }
    this->m_utils->u_log(LOG_FWUPGRADE, "Sending Softfuses UPH size DWORD ...");
    SFdata = m_mfld_softfuses->GetSoftFusesFileData(SFUSE_DATA_RSUPH);
    if(SFdata && !WriteOutPipe(SFdata->data, SFdata->size))
    {
        ret_code = 1; //USB error
        LogError(ret_code);
    }
    return (ret_code ? false:true);
}

bool ClvpDldrState::StartOs()
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


void ClvpDldrState::Visit(ClvpFwHandleER00& )
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

void ClvpDldrState::Visit(ClvpFwHandleHLT0& )
{
    if(m_dldr_state == DLDR_STATE_SOFT_FUSES)
    {
        this->m_utils->u_log(LOG_OPCODE, "Softfuse Binary file size = 0 or Number of keys = 0");
        m_fw_done = true;
    }
    else
    // --- FW file has no size
    {
        this->m_utils->u_log(LOG_OPCODE, "HLT0 - DnX FW or IFWI Size = 0");
        m_fw_done = true;
    }
}

void ClvpDldrState::Visit(ClvpFwHandleDSKF& )
{
    bool ret = true;
    m_abort = false;
    this->m_utils->u_log(LOG_OPCODE, "Sending Softfuses Key File ...");
#if 0
    gettimeofday(&m_start_time, NULL);
#endif

    ret = FwDSKF();

    if(!ret) {
        m_abort = true;
    }
}

void ClvpDldrState::Visit(ClvpFwHandleDFRM& )
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
    }
    else {
        //Start virgin part FW download state
        GotoState(DLDR_STATE_FW_NORMAL);
    }

    if(!ret) {
        m_abort = true;
    }
}

void ClvpDldrState::Visit(ClvpFwHandleDxxM& )
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
    else if(m_perform_emmc_dump || m_b_DnX_OS || strlen(m_fname_dnx_misc) > 3) //Only if misc dnx file is provided
        GotoState(DLDR_STATE_FW_MISC);
    else
        GotoState(DLDR_STATE_FW_NORMAL);

    if(!ret) {
        m_abort = true;
    }
}


void ClvpDldrState::Visit(ClvpFwHandleDXBL& )
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

    std::string s(const_cast<char*>(m_fname_softfuses_bin.c_str()));
    if (((s != "BLANK.bin") && (s != "") && (!s.empty())) && (!(m_gpflags&0x100)) && (m_sf_flag == false) && ((m_dldr_state != DLDR_STATE_OS_NORMAL) && (m_dldr_state != DLDR_STATE_OS_MISC))){
        GotoState(DLDR_STATE_SOFT_FUSES);
    }

    EndlogTime();
    LogProgress();

    if(!ret) {
        m_abort = true;
    }
}

void ClvpDldrState::Visit(ClvpFwHandleRUPHS& )
{
    int ret_code = 0;
    dnx_data* fwdata = NULL;

    m_fwstepsdone++;

    StartLogTime();

    if (m_dldr_state == DLDR_STATE_SOFT_FUSES)
    {
        m_dldr_state = DLDR_STATE_FW_NORMAL;
        if (m_sf_flag)
            this->m_utils->u_log(LOG_FWUPGRADE, "Softfuse provisioning completed. Proceeding to FW recovery...");
        else
            this->m_utils->u_log(LOG_FWUPGRADE, "GPP write-protected. Proceeding to FW recovery...");
    }

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

void ClvpDldrState::Visit(ClvpFwHandleRSUPHS& )
{
    if (m_dldr_state != DLDR_STATE_SOFT_FUSES)
    {
        int ret_code = 0;
        ULONG zero_init = 0;
        m_dldr_state = DLDR_STATE_SOFT_FUSES;
        if (!WriteOutPipe((unsigned char*)&zero_init, 4))
        {
            ret_code = 1;
            LogError(ret_code);
        }
        this->m_utils->u_log(LOG_STATUS, "Softfuses Binary absent. Please provide binary.");
        if(ret_code) {
            m_abort = true;
        }

    }
    else
    {
        int ret_code = 0;
        dnx_data* fwdata = NULL;
        m_abort = false;
        m_fwstepsdone++;

        if(!m_mfld_softfuses)
        {
            LogError(0xBAADF00D);
            return;
        }

        m_sf_flag = true;

        StartLogTime();

        this->m_utils->u_log(LOG_FWUPGRADE, "Sending Softfuses Update Profile Hdr Size...");
        fwdata = m_mfld_softfuses->GetSoftFusesFileData(SFUSE_DATA_RSUPHS);

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
}

void ClvpDldrState::Visit(ClvpFwHandleRUPH& )
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

void ClvpDldrState::Visit(ClvpFwHandleRSUPH& )
{
    int ret_code = 0;
    dnx_data* fwdata = NULL;
    m_abort = false;

    if(!m_mfld_softfuses)
    {
        LogError(0xBAADF00D);
        return;
    }

    m_fwstepsdone++;

    StartLogTime();

    this->m_utils->u_log(LOG_FWUPGRADE, "Sending Softfuses Update Profile Hdr ...");
    fwdata = m_mfld_softfuses->GetSoftFusesFileData(SFUSE_DATA_RSUPH);

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

void ClvpDldrState::Visit(ClvpFwHandleRESET& )
{
    StartLogTime();

    this->m_utils->u_log(LOG_FWUPGRADE, "FW: Received RESET...");

    EndlogTime();
    LogProgress();

    m_fw_done = true;
    m_GPPReset = true;
}
void ClvpDldrState::Visit(ClvpFwHandleDMIP& )
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

void ClvpDldrState::Visit(ClvpFwHandleLOFW& )
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

void ClvpDldrState::Visit(ClvpFwHandleHIFW& )
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

void ClvpDldrState::Visit(ClvpFwHandlePSFW1& )
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
            remainingBytes = NULL;

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
        //release resources
        if(remainingBytes)
            delete [] remainingBytes;

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

void ClvpDldrState::Visit(ClvpFwHandlePSFW2& )
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
                remainingBytes = NULL;

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
        //release resources
        if(remainingBytes)
            delete [] remainingBytes;

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

void ClvpDldrState::Visit(ClvpFwHandleSSFW& )
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
                remainingBytes = NULL;

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
        //release resources
        if(remainingBytes)
            delete [] remainingBytes;

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

void ClvpDldrState::Visit(ClvpFwHandleHLT$& )
{
    // --- ACK Received: HLT$ ---
    if(m_perform_emmc_dump)
    {
        this->m_utils->u_log(LOG_FWUPGRADE, "FW: Emmc dump completed...");
        m_fw_done = true;
        return;
    }

    if(m_dldr_state == DLDR_STATE_SOFT_FUSES)
    {
        //set m_fw_done = false and do nothing else or set m_abort = false
        //m_fw_done = false;
        m_abort = true;
    }
    else {
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

}

void ClvpDldrState::Visit(ClvpFwHandleMFLD& )
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


void ClvpDldrState::Visit(ClvpFwHandleCLVT& )
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

void ClvpDldrState::Visit(ClvpFwHandleSuCP& )
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
                remainingBytes = NULL;

                m_romPatchArrayPtr = 0;
                //Last bytes were sent reset the 128KB block count
                m_numRomPatchChunks = m_romPatch128KBChunks;
            } else
            {
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
        //release resources
        if(remainingBytes)
            delete [] remainingBytes;

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

void ClvpDldrState::Visit(ClvpFwHandleRTBD& )
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
            {
                if (m_dldr_state == DLDR_STATE_BHC_ERRR)
                {
                    m_abort=true;
                }
                throw 0;
            }
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
            throw 44;
        }

        //printf("\n\n***********Battery Data***************\n0x%x BATI in hex \n0x%x Header Size \n0x%x Header Revision \n0x%x XOR Checksum \n0x%x Battery Voltage \n0x%x Battery Coulomb Counter\n***********Battery Data***************\n\n",BattData.BATI,BattData.HeaderSize,BattData.HeaderRevis,BattData.XorCheck,BattData.voltage,BattData.coulombCount);
        this->m_utils->u_log(LOG_STATUS, "BATI: Battery power is too low, charging up ...");

        //The voltage conversion from raw data is not accurate, will need upate it later.
        this->m_utils->u_log(LOG_STATUS, "BATI:%x: Battery voltage: 0x%x(%fV) The threshold: 0x%x(%fV)", batt_data.BATI,batt_data.voltage, 3.6*batt_data.voltage/191, 0xbf, 3.6);
        } //try
    catch( int errorcode ) {
        ret_code = errorcode;
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Exception raised: ErrorCode %d", __FUNCTION__, errorcode);
        if(errorcode == 0)
        {
            if((m_dldr_state == DLDR_STATE_FW_NORMAL)||(m_dldr_state == DLDR_STATE_FW_MISC)||(m_dldr_state == DLDR_STATE_FW_WIPE))
                this->m_utils->u_log(LOG_STATUS, "Battery power is okay, continue FW download ...");
            else if((m_dldr_state == DLDR_STATE_OS_NORMAL)||(m_dldr_state == DLDR_STATE_OS_MISC))
                this->m_utils->u_log(LOG_STATUS, "Battery power is okay, continue OS download ...");
            else if(m_dldr_state == DLDR_STATE_BHC_ERRR)
                this->m_utils->u_log(LOG_STATUS, "Battery power is okay, BHC is done ...");
        }
        else
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

    if(ret_code) {
        m_abort = true;
    }
}

void ClvpDldrState::Visit(ClvpFwHandleVEDFW& )
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
                remainingBytes = NULL;

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
        //release resources
        if(remainingBytes)
            delete [] remainingBytes;

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

void ClvpDldrState::Visit(ClvpFwHandleSSBIOS& )
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
                remainingBytes = NULL;

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
        //release resources
        if(remainingBytes)
            delete [] remainingBytes;

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

void ClvpDldrState::Visit(ClvpFwHandleIFW1& )
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

void ClvpDldrState::Visit(ClvpFwHandleIFW2& )
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

void ClvpDldrState::Visit(ClvpFwHandleIFW3& )
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

void ClvpDldrState::Visit(ClvpOsHandleDORM& )
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

void ClvpDldrState::Visit(ClvpOsHandleOSIPSZ& )
{
}

void ClvpDldrState::Visit(ClvpOsHandleROSIP& )
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

void ClvpDldrState::Visit(ClvpOsHandleDONE& )
{
}

void ClvpDldrState::Visit(ClvpOsHandleRIMG& )
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

void ClvpDldrState::Visit(ClvpOsHandleEOIU& )
{
    int ret_code = 0;
    UCHAR DFN[4] = "DFN";

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

void ClvpDldrState::Visit(ClvpStHandleFwNormal& )
{
    bool ret = false;

    StartLogTime();

    this->m_utils->u_log(LOG_FWUPGRADE, "Changing to DLDR_STATE_FW_NORMAL ...");

    m_mfld_fw = new CloverviewPlusFW;

    if(m_mfld_fw->Init(m_fname_dnx_fw, m_fname_fw_image, m_utils, m_gpflags, false, m_perform_emmc_dump))
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

    this->m_utils->u_log(LOG_FWUPGRADE, "End of StHandleFwNormal ");

    EndlogTime();

    if(!ret) {
        m_abort = true;
    }
}

void ClvpDldrState::Visit(ClvpStHandleFwMisc& )
{
    bool ret = false;

    StartLogTime();

    this->m_utils->u_log(LOG_FWUPGRADE, "Changing to DLDR_STATE_FW_MISC ...");

    //Set the state machine to virgin emmc download
    //Need to set this first, since mfld_fw.init could fail because of fw blank.bin files.
    m_dldr_state = DLDR_STATE_FW_MISC;

    m_mfld_fw = new CloverviewPlusFW;

    if(m_mfld_fw->Init(m_fname_dnx_fw, m_fname_fw_image, m_utils, m_gpflags, false, m_perform_emmc_dump) && !m_b_DnX_OS)
    {
        //Send DnX FW size header data to target device
        //This will start the FW state machine
        this->m_utils->u_log(LOG_STATUS, "FW(Miscdnx) download is in progress ... ");

        ret = StartFw();
    } else if(m_b_DnX_OS || (this->m_utils->FileSize(m_fname_dnx_misc) == 0) || (this->m_utils->FileSize(m_fname_fw_image) == 0)) {
        ret = HandleNoSize();// == BULK_ACK_HLT0)
        ret = true;
    }

    this->m_utils->u_log(LOG_FWUPGRADE, "End of StHandleFwMisc ");

    EndlogTime();

    if(!ret) {
        m_abort = true;
    }
}

void ClvpDldrState::Visit(ClvpStHandleFwWipe& )
{
    bool ret = false;
    unsigned long temp_gpflags = 0;

    StartLogTime();

    this->m_utils->u_log(LOG_FWUPGRADE, "Changing to DLDR_STATE_FW_WIPE ...");

    //Set the state machine to virgin emmc download
    //Need to set this first, since mfld_fw.init could fail because of fw blank.bin files.
    m_dldr_state = DLDR_STATE_FW_WIPE;

    m_mfld_fw = new CloverviewPlusFW;

    //Set gpflags to do a FW cold reset
    temp_gpflags = m_gpflags | 0x2;

    if(m_mfld_fw->Init(m_fname_dnx_fw, m_fname_fw_image, m_utils, temp_gpflags, true, m_perform_emmc_dump) && !m_b_DnX_OS)
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

    this->m_utils->u_log(LOG_FWUPGRADE, "End of StHandleFwWipe ");

    EndlogTime();

    if(!ret) {
        m_abort = true;
    }
}

void ClvpDldrState::Visit(ClvpStHandleOsNormal& )
{
    bool ret = false;
    int ret_code = 0;

    StartLogTime();

    this->m_utils->u_log(LOG_OS, "Changing to DLDR_STATE_OS_NORMAL ...");

    m_mfld_os = new CloverviewPlusOS;

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

    this->m_utils->u_log(LOG_OS, "End of StHandleOsNormal ");

    EndlogTime();

    if(!ret) {
        m_abort = true;
    }
}

void ClvpDldrState::Visit(ClvpStHandleOsMisc& )
{
    bool ret = false;
    int ret_code = 0;

    StartLogTime();

    this->m_utils->u_log(LOG_OS, "Changing to DLDR_STATE_OS_MISC ...");

    m_mfld_os = new CloverviewPlusOS;

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

    this->m_utils->u_log(LOG_OS, "End of StHandleOsMisc ");

    EndlogTime();

    if(!ret) {
        m_abort = true;
    }
}

void ClvpDldrState::Visit(ClvpStHandleSoftFuses &)
{
    bool ret = false;
    StartLogTime();

    this->m_utils->u_log(LOG_OS, "Changing to DLDR_STATE_SOFT_FUSES ...");

    m_mfld_softfuses = new softfusesFW;
    m_dldr_state = DLDR_STATE_SOFT_FUSES;
    ret = m_mfld_softfuses->Init(const_cast<char*>(m_fname_softfuses_bin.c_str()),m_utils);
    if(ret)
    {
        //Send DnX OS size header data to target device
        //This will start the OS state machine
        this->m_utils->u_log(LOG_STATUS, "Soft fusing is in progress ... ");
    }
    if((this->m_utils->FileSize(const_cast<char*>(m_fname_softfuses_bin.c_str())) == 0)||(m_mfld_softfuses->softFusesDataSize <= 0)) {
        ret = HandleNoSize();
    }

    this->m_utils->u_log(LOG_OS, "End of StHandleSoftfuses ");

    EndlogTime();

    if(!ret) {
        m_abort = true;
    }

}

void ClvpDldrState::Visit(ClvpErHandleLogError& )
{
    ErrorMapType::iterator iter;
    bool temp_abort;
    if(m_achCode) {
        iter = m_fw_error_map.find(m_achCode);
        //handle ER1x differently if coming from soft fuses state - i.e. purge state machine
        if (m_dldr_state == DLDR_STATE_SOFT_FUSES)
        {
            m_fw_done = true;
            temp_abort = false;
        }
        else
            temp_abort = true;
    if (iter != m_fw_error_map.end())
        LogError(iter->second);
    } else {
        this->m_utils->u_log(LOG_STATUS, "Unknown ACK code, aborting ...");
        temp_abort = true;
    }
    m_abort = temp_abort;
}

void ClvpDldrState::Visit(ClvpErHandleERRR& )
{
    int ret_code = 0;
    ULONG preamble_msg = PREAMBLE_BMRQ;
    m_dldr_state=DLDR_STATE_BHC_ERRR;
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

void ClvpDldrState::Visit(ClvpStEmmcDumpTransfer& )
{
    m_utils->u_log(LOG_STATUS, "Transfer started");

    std::string outfile;
    int partition = 0;
    int blockCount = 0;
    int blockSize = 0;
    int offsetInEmmc = 0;
    bool bRegisterToken = false;
    string tokenOffset = "0x0108";

    outfile = m_emmc_outfile;
    if (m_emmc_umip_enabled)
    {
        partition = 0;
        blockCount = 1;
        blockSize = 65536;
        offsetInEmmc = 512;
    }
    else
    {
        partition = m_emmc_partition;
        blockCount = m_emmc_blockcount;
        blockSize = m_emmc_blocksize;
        offsetInEmmc = m_emmc_offset;
        bRegisterToken = m_emmc_register_token_enabled;
        tokenOffset = m_emmc_token_offset;
     }

    const char * ofname = outfile.c_str();
    unsigned int receivedCount=0;   //  16     *  131072  = 2097152
    unsigned int expectedToReceive = blockCount*blockSize;

    bool success = true;
    int status = 0;

    boost::scoped_array<char> tmp (new char[BUF_SIZE]);
    txHeader_t* txHeader = (txHeader_t*) (tmp.get());


    if( partition == 3 ) {  // ECSD

        m_utils->u_log(LOG_STATUS, "opening file for IO\r\n");
        try
        {
            scoped_file file(ofname,"wb");

            blockSize = 512;

            m_utils->u_log(LOG_STATUS, "Sending ECSD\n");
            strcpy(tmp.get(), "ECSD"); // Send request for user partition
            txHeader = (txHeader_t*) (tmp.get());
            txHeader->blockSize = blockSize;
            txHeader->offsetInEmmc = offsetInEmmc;

            success = m_usbdev->Write(tmp.get(), sizeof(txHeader_t));

            if (!success) {
                printf("\nError writing ECSD");
                return;
            }

            success = m_usbdev->Read(tmp.get(), blockSize);

            if(tmp[0] == 'E' && tmp[1] == 'R' && tmp[2] == '4' && tmp[3] == '6')
            {
                LogError(46, "\nFailure reading ECSD from eMMC.");
                return;
            }

            if (!success)
            {
                printf("\nError reading ECSD response");
                fflush(stdout);
                m_abort = true;
                return;
            }

            file.write(tmp.get(), sizeof(char), blockSize);
        }
        catch(std::exception& e)
        {
            std::cout << "Error: "<< e.what() << std::endl;
            m_abort = true;
            return;
        }

        transferComplete();
        return;
    }

    // Set the partition:
    m_utils->u_log(LOG_STATUS, "Sending SPR$");
    strcpy(tmp.get(), "SPR$");
    txHeader->blockSize = partition;

    success = m_usbdev->Write(tmp.get(), sizeof(txHeader_t));
    if (!success) {
        this->m_utils->u_log(LOG_STATUS, "Error writing SPR$");
        m_abort = true;
        return;
    }

    //Token Verification
    success = m_usbdev->Read(tmp.get(), 4);
    if (!success) {
        m_utils->u_log(LOG_STATUS, "Error reading SPR$ response.");
        m_abort = true;
        return;
    }
    if(tmp[0] == 'R' && tmp[1] == 'D' && tmp[2] == 'Y' && tmp[3] == '$')
    {
        m_utils->u_log(LOG_STATUS, "SPR$ Success");
    }
    else if(tmp[0] == 'E' && tmp[1] == 'R' && tmp[2] == '4' && tmp[3] == '2')
    {
        LogError(42, "\nPartition access not allowed.");
        return;
    }
    else if(tmp[0] == 'E' && tmp[1] == 'R' && tmp[2] == '4' && tmp[3] == '3')
    {
        LogError(43, "\nToken mismatched.");
        return;
    }

    else if(tmp[0] == 'E' && tmp[1] == 'R' && tmp[2] == '4' && tmp[3] == '4')
    {
        LogError(44, "\nToken has expired.");
        return;
    }
    else if(tmp[0] == 'E' && tmp[1] == 'R' && tmp[2] == '4' && tmp[3] == '5')
    {
        LogError(45, "\nFailure during eMMC partition switching.");
        return;
    }
    else
    {
        printf("\nUnkown command.\n");
        m_abort = true;
        return;
    }

    try // Request and receive data from target:
    {

        scoped_file file(outfile.c_str(),"wb");

        while(blockCount) {
            strcpy(tmp.get(), "REQB");
            txHeader = (txHeader_t*) (tmp.get());
            txHeader->blockSize = blockSize;
            txHeader->offsetInEmmc = offsetInEmmc;

            success = m_usbdev->Write(tmp.get(), sizeof(txHeader_t));

            if( !success) {
                 this->m_utils->u_log(LOG_STATUS, "Error writing REQB");
                 m_abort = true;
                 status = -1;
                 break;
            }

            success = m_usbdev->Read(tmp.get(), blockSize);

            if (!success)
            {
                printf("\nError reading");
                m_abort = true;
                fflush(stdout);
                status = -1;
                break;
            }
            if(tmp[0] == 'E' && tmp[1] == 'R' && tmp[2] == '4' && tmp[3] == '1')
            {
                LogError(41, "\nFailure reading from eMMC or invalid block size.");
                fflush(stdout);
                status = -1;
                break;
            }
            file.write(tmp.get(), sizeof(char), blockSize);
            this->m_utils->u_log(LOG_STATUS, "Number of blocks remaining: %d",blockCount);
            offsetInEmmc += blockSize;
            receivedCount += blockSize;
            blockCount--;
        }
    }
    catch(std::exception& e)
    {
        std::cout << "Error: "<< e.what() << std::endl;
        m_abort = true;
        return;
    }

    m_utils->u_log(LOG_STATUS, "received: %X / %X\n\r", receivedCount, expectedToReceive );

    //check to see progress:
    if(receivedCount >= expectedToReceive){
        m_utils->u_log(LOG_STATUS, "We have received %d bytes - done\n", receivedCount);
        status = 0;
    }

    if( ! status ) {
        // Done!
        transferComplete();

        }else if ( -1 == status ){
            printf( "read error...\n");
            m_abort = true;
            return;
        }
    else {
        printf("some other error...\n");
        m_abort = true;
        return;
    }

}

void ClvpDldrState::transferComplete()
{
    char tmp[BUF_SIZE];
    unsigned int* pDW = (unsigned int*)tmp;

    bool success = false;
    m_utils->u_log(LOG_STATUS, "EMMCDUMP Transfers completed");

    ///////////////////////////
    strcpy(tmp, "STAT");

    success = m_usbdev->Write(tmp, sizeof(txHeader_t));
    if( !success)
    {
         printf("\nError sending STAT");
    }
    success = m_usbdev->Read(tmp, EOIO_MESSAGE_SIZE);

    if (!success)
    {
        printf("\nError reading EOIO message... aborting");
        m_abort = true;
    }
    else
    {
        if( tmp[0] == 'I' && tmp[1] == 'N' && tmp[2] == 'F' && tmp[3] == 'O' ) {
                m_utils->u_log(LOG_STATUS, "Received STAT - transfer complete\n");
                m_utils->u_log(LOG_STATUS, "Number of errors: %d\n",pDW[1]);
                m_utils->u_log(LOG_STATUS, "Number of transfers: %d\n",pDW[2]);
                m_utils->u_log(LOG_STATUS, "Number of unknown commands received: %d\n",pDW[3]);
                m_utils->u_log(LOG_STATUS, "Number of retries due to unknown commands: %d\n",pDW[4]);
                m_fw_done = true;
        }else{
                printf("unexpected message received\n");
                m_abort = true;
        }
    }

    strcpy(tmp, "DONE");

    success = m_usbdev->Write(tmp, sizeof(txHeader_t));

    if( !success)
    {
         printf("\nError sending DONE");
    }
}

void ClvpDldrState::Visit(ClvpEmmcDump$ACK& )
{
    m_utils->u_log(LOG_STATUS, "eMMC Dump $ACK");
    GotoState(CLVP_EMMC_DUMP_STATE_TRANSFER);
}

void ClvpDldrState::Visit(ClvpEmmcDumpNACK& )
{
    m_utils->u_log(LOG_STATUS, "eMMC Dump NACK");
    m_abort = true;
}

void ClvpDldrState::Visit(ClvpEmmcDumpECSD& )
{
    m_utils->u_log(LOG_STATUS, "eMMC Dump ECSD");
}

void ClvpDldrState::Visit(ClvpEmmcDumpREQB& )
{
    m_utils->u_log(LOG_STATUS, "eMMC Dump REQB");
}

void ClvpDldrState::Visit(ClvpEmmcDumpEOIO& )
{
    m_utils->u_log(LOG_STATUS, "eMMC Dump EOIO");
    printf("\nDONE<->EOIO handshake completed...");
    m_fw_done = true;
}

void ClvpDldrState::Visit(ClvpEmmcDumpRDY$& )
{
    m_utils->u_log(LOG_STATUS, "eMMC Dump $ACK");
    GotoState(CLVP_EMMC_DUMP_STATE_TRANSFER);
}
void ClvpDldrState::Visit(ClvpEmmcDumpER40& )
{
    LogError(40, "\nInitialize eMMCHW and Card failed.");
}

bool ClvpDldrState::Start()
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

void ClvpDldrState::GotoState(unsigned long long state)
{
    m_dldr_state = state;
    m_state_change = true;
}

void ClvpDldrState::SleepMs(int delay)
{
#if defined XFSTK_OS_WIN
        Sleep(delay);
#else
        usleep(1000*delay);
#endif
}

bool ClvpDldrState::doRegisterToken()
{
    const int SN_BUF_SIZE = 16;
    char sn[SN_BUF_SIZE];
    m_usbdev->GetUsbsn(sn);

    const int TIME_BUF_SIZE = 4;
    unsigned int expirationDurationSeconds = 0;
    if(!DeviceSpecificOptions->GetEmmcExpirationDur().empty()) //handle expiry
    {
        try
        {
            expirationDurationSeconds = convertDurationToSeconds(DeviceSpecificOptions->GetEmmcExpirationDur());
        }
        catch(std::exception& e)
        {
            cout << "\n" << e.what() << "\n";
            return false;
        }
    }
    else
    {
        m_utils->u_log(LOG_STATUS, "\nWarning expiration duration not set. Expiry will not be written to file.\n");
    }

    unsigned char minTime[TIME_BUF_SIZE] = {0xFF, 0xFF, 0xFF, 0xFF};
    unsigned char maxTime[TIME_BUF_SIZE] = {0xFF, 0xFF, 0xFF, 0xFF};

    if(expirationDurationSeconds == 0)
    {
        m_utils->u_log(LOG_STATUS, "\nToken expiry disabled.\n");
    }
    else if(expirationDurationSeconds == MAX_SECONDS)
    {
        printf("\nWarning: Expiration duration exceeds maximum time. Token expiry disabled.\n");
    }
    else //read counter and calculate expiry time
    {
        m_utils->u_log(LOG_STATUS, "Sending IDRQ...");
        ULONG preamble_msg = PREAMBLE_IDRQ;
        if(!WriteOutPipe((PUCHAR)&preamble_msg, 4))
        {
            this->m_utils->u_log(LOG_STATUS, "IDRQ Write Error");
            return 0;
        }
        SleepMs(m_delay_ms);
        //read in header, keys, and error code
        char data[BUF_SIZE];
        if(!m_usbdev->Read(data,MAX_KEY_COUNT*8*sizeof(DWORD) + 2*sizeof(DWORD)))
        {
            this->m_utils->u_log(LOG_STATUS, "IDRQ Read Error");
            return 0;
        }

        const int numOfkeys = *(data+3);

        //skip header [1 dword], skip hash keys [numkeys * 8 dwords], skip error code [1 dword]
        memcpy(&minTime,(data+sizeof(DWORD)+(8*numOfkeys)*sizeof(DWORD)+sizeof(DWORD)),sizeof(DWORD));

        std::stringstream ss1;
        int i = TIME_BUF_SIZE;
        while(i != 0)
        {
            i--;
            unsigned int idata = static_cast<unsigned char> (minTime[i]);
            ss1 << std::hex << idata;
        }
        unsigned int minTimeValue = 0;
        ss1 >> std::hex >> minTimeValue;

        unsigned int maxTimeValue = 0;
        if(minTimeValue == MAX_SECONDS)
        {
            printf("Warning: Expiration duration cannot be set. Token expiry disabled.");
            for(int i =0; i < TIME_BUF_SIZE; ++i)
            {
                maxTime[i] = 0xFF;
            }
        }
        else //calculate max time given expiration duration
        {
            maxTimeValue = minTimeValue + expirationDurationSeconds;

            //check overflow
            if(maxTimeValue < minTimeValue)
            {
                printf("Warning: Expiration duration cannot be set. Token expiry disabled.");
                for(int i =0; i < TIME_BUF_SIZE; ++i)
                {
                    minTime[i] = 0xFF;
                    maxTime[i] = 0xFF;
                }
            }
            else //set max time
            {
                std::stringstream ss2;
                for(int i=0; i<TIME_BUF_SIZE; ++i){
                    unsigned int byte = maxTimeValue & 0xFF;
                    maxTime[i] = byte;
                    maxTimeValue >>= 8;
                }
            }
        }
    }

    std::string token;
    std::string s1(sn, sn+SN_BUF_SIZE);
    std::string s2(minTime, minTime+ sizeof(DWORD));
    std::string s3(maxTime, maxTime+ sizeof(DWORD));
    token = s1 + s2 + s3;

    if(!DeviceSpecificOptions->GetEmmcFile().empty())
    {
        if(writeTokenToFile(DeviceSpecificOptions->GetEmmcFile(), s1, s2+s3, !DeviceSpecificOptions->GetEmmcExpirationDur().empty()))
        {
            printf("Token written to file.\n");
        }
        else
        {
            printf("Error writing token to file.\n");
        }
    }

    if(!DeviceSpecificOptions->GetEmmcUnsignedFwDNX().empty())
    {
        unsigned int iOffset = convertHexStringToInteger(DeviceSpecificOptions->GetEmmcTokenOffset());
        if(writeTokenToUnsignedDnXAtOffset(DeviceSpecificOptions->GetEmmcUnsignedFwDNX(), token, iOffset, !DeviceSpecificOptions->GetEmmcExpirationDur().empty()))
        {
            printf("Token written to unsigned DnX.\n");
        }
        else
        {
            printf("Error writing token to unsigned DnX.\n");
        }
    }

    return true;
}

bool ClvpDldrState::StateMachine()
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
                strcpy(this->m_last_error.error_message, CloverviewPlus_error_code_array[4]);
            }
        }

        if(m_abort) {
            if (m_dldr_state == DLDR_STATE_SOFT_FUSES)
            {
                m_sf_retry = true;
            }
            ret = false;
            break;
        }

        if(m_fw_done) {
            if (m_dldr_state == DLDR_STATE_SOFT_FUSES)
            {
                m_sf_error = true;
                ret = true;
                break;
            }
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

