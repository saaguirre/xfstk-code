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
#include <stdexcept>
#include "common/scoped_file.h"
#include <boost/smart_ptr/scoped_array.hpp>
#include "merrifieldmessages.h"
#include "mrfdldrstate.h"
#include <fstream>
#include <QTime>
#include <string>

#define USB_READ_WRITE_DELAY_MS 0

extern CPSTR Merrifield_error_code_array [MAX_ERROR_CODE_MERRIFIIELD];

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
                tokenfile.write(ss1.str().c_str(), sizeof(char), 64);
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
                tokenfile.write(token.c_str(), sizeof(char), 40);
            }
            else
            {
                tokenfile.write(token.c_str(), sizeof(char), 32);
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

mrfdldrstate::mrfdldrstate()
{
    m_usbdev = NULL;
    m_utils = NULL;

    m_b_usbinitok = false;

    m_gpflags = 0;
    m_achCode = 0;

    m_ifwiArrayPtr = 0;
    m_numIfwiChunks = 0;
    m_ifwi128KBChunks = 0;
    m_residualIfwiBytes = 0;

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
    directcsdbStatus = 0;

    m_delay_ms = USB_READ_WRITE_DELAY_MS;
    m_emmc_fname_signed_dnx = NULL;
    m_emmc_outfile = "";
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
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_INVALID_PING, new MrfdFwHandleER00));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_HLT0, new MrfdFwHandleHLT0));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_DFRM, new MrfdFwHandleDFRM));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_DxxM, new MrfdFwHandleDxxM));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_DXBL, new MrfdFwHandleDXBL));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_READY_UPH_SIZE, new MrfdFwHandleRUPHS));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_READY_UPH, new MrfdFwHandleRUPH));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_DCFI00, new MrfdFwHandleDCFI00));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_DIFWI, new MrfdFwHandleDIFWI));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_UPDATE_SUCESSFUL, new MrfdFwHandleHLT$));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_MFLD, new MrfdFwHandleMFLD));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_CLVT, new MrfdFwHandleCLVT));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_RTBD,  new MrfdFwHandleRTBD));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_GPP_RESET, new MrfdFwHandleRESET));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_DORM, new MrfdOsHandleDORM));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_OSIPSZ, new MrfdOsHandleOSIPSZ));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ROSIP, new MrfdOsHandleROSIP));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_DONE, new MrfdOsHandleDONE));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_RIMG, new MrfdOsHandleRIMG));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_EOIU, new MrfdOsHandleEOIU));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_DCSDB,new MrfdFwHandleDCSDB));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_UCSDB,new MrfdFwHandleUCSDB));

    // eMMC dump
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_RDY$, new MrfdEmmcDumpRDY$));
    m_bulk_ack_map.insert(std::make_pair(EMMC_DUMP_ACK, new MrfdEmmcDump$ACK));
    m_bulk_ack_map.insert(std::make_pair(EMMC_DUMP_NACK, new MrfdEmmcDumpNACK));
    m_bulk_ack_map.insert(std::make_pair(MERR_EMMC_DUMP_STATE_TRANSFER, new MrfdStEmmcDumpTransfer));
    m_bulk_ack_map.insert(std::make_pair(EMMC_DUMP_ECSD, new MrfdEmmcDumpECSD));
    m_bulk_ack_map.insert(std::make_pair(EMMC_DUMP_SPR$, new MrfdEmmcDumpSPR$));
    m_bulk_ack_map.insert(std::make_pair(EMMC_DUMP_REQB, new MrfdEmmcDumpREQB));
    m_bulk_ack_map.insert(std::make_pair(EMMC_DUMP_EOIO, new MrfdEmmcDumpEOIO));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER40, new MrfdEmmcDumpER40));

    //Set up device error code handler
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER01, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER02, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER03, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER04, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER10, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER11, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER12, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER13, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER15, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER16, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER17, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER18, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER20, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER21, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER22, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER25, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER26, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER04, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER06, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER07, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ERRR, new MrfdErHandleERRR));

    //PMIC Provisioning
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER90, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER91, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER92, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER93, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER94, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER95, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER96, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER97, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER98, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER99, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER9A, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER9B, new MrfdErHandleLogError));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_PP90, new MrfdHandleLogDevice));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_PP91, new MrfdHandleLogDevice));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_PPOK, new MrfdHandleLogDevice));


    //Setup state map
    m_bulk_ack_map.insert(std::make_pair(DLDR_STATE_FW_NORMAL, new MrfdStHandleFwNormal));
    m_bulk_ack_map.insert(std::make_pair(DLDR_STATE_FW_MISC, new MrfdStHandleFwMisc));
    m_bulk_ack_map.insert(std::make_pair(DLDR_STATE_FW_WIPE, new MrfdStHandleFwWipe));
    m_bulk_ack_map.insert(std::make_pair(DLDR_STATE_OS_NORMAL, new MrfdStHandleOsNormal));
    m_bulk_ack_map.insert(std::make_pair(DLDR_STATE_OS_MISC, new MrfdStHandleOsMisc));

    //Error ack code and error code index map
    i = 19; //Check error code string array for index.
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER01, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER02, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER03, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER10, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER11, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER12, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER13, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER20, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER21, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER22, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER25, i++));
    i = 40;
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER04, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER06, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER07, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER26, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER90, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER91, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER92, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER93, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER94, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER95, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER96, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER97, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER98, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER99, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER9A, i++));
    m_fw_error_map.insert(std::make_pair(BULK_ACK_ER9B, i++));

    m_fw_log_map.insert(std::make_pair(BULK_ACK_PP90, i++));
    m_fw_log_map.insert(std::make_pair(BULK_ACK_PP91, i++));
    m_fw_log_map.insert(std::make_pair(BULK_ACK_PPOK, i++));

    m_last_error.error_code = 1;
    strcpy(m_last_error.error_message, "Init error");

}

mrfdldrstate::~mrfdldrstate()
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

bool mrfdldrstate::Init(IDevice * usbdev, MerrifieldUtils* utils)
{
    if(!m_b_usbinitok) {
        m_usbdev = usbdev;
        m_utils = utils;
        m_b_usbinitok = true;
    }
    m_ROSIP_data_flag = false;

    return true;
}
bool mrfdldrstate::UsbDevInit(void)
{
    m_b_usbinitok = false;
    m_b_usbinitok = m_usbdev->Open();
    return m_b_usbinitok;
}

void mrfdldrstate::SetOptions(MerrifieldOptions *Options)
{
    DeviceSpecificOptions = Options;
}

bool mrfdldrstate::DoEmmcUpdate(char* emmc_fname_signed_dnx, std::string emmc_outfile, \
                       std::string emmc_token_offset, std::string emmc_expirationdur, int emmc_partition, \
                       long emmc_blocksize,long emmc_blockcount, long emmc_offset, bool emmc_umip_enabled, \
                       unsigned int usb_delayms)
{
    m_fw_done = false;
    m_GPPReset = false;
    m_abort = false;
    m_state_change = false;

    if(DeviceSpecificOptions->IsRegisterToken())
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
        directcsdbStatus = false;
        csdbStatus = " ";


        return StateMachine();
    }
}


bool mrfdldrstate::DoUpdate(char* fname_dnx_fw, char* fname_fw_image, char* fname_dnx_os, \
                         char* fname_os_image, char* fname_dnx_misc, unsigned long gpflags, \
                         unsigned long usbdelayms, bool ifwiwipeenable, char* miscbin)
{
    bool ret = true;
    m_fname_dnx_fw = fname_dnx_fw;
    m_fname_fw_image = fname_fw_image;
    m_fname_dnx_misc = fname_dnx_misc;
    m_fname_dnx_os = fname_dnx_os;
    m_fname_os_image = fname_os_image;
    m_fname_bin_misc = miscbin;
    m_gpflags = gpflags;
    m_fw_done = false;
    m_os_done = false;
    m_GPPReset = false;
    m_abort = false;
    m_state_change = false;
    m_delay_ms = usbdelayms;
    m_ifwi_done = false;
    m_ifwiwipe_enable = ifwiwipeenable;
    ResponseBuffSize = 0;

    //Start downloader state machine
    ret = StateMachine();

    return ret;
}

int mrfdldrstate::getResponseBuffer(unsigned char *idrqbuf,int maxsize)
{
    int size = (maxsize > ResponseBuffSize) ? ResponseBuffSize : maxsize;
    if(size)
    {
        memset(idrqbuf,0,maxsize);
        memcpy(idrqbuf, this->ResponseBuff.get(),size);
    }
    return size;
}

bool mrfdldrstate::GetOsStatus()
{
    return m_os_done;
}

bool mrfdldrstate::SetIDRQstatus(bool idrq)
{
    this->m_b_IDRQ = idrq;
    return this->m_b_IDRQ;
}

void mrfdldrstate::SetCsdbStatus(string csdb, BYTE direct)
{
    this->csdbStatus = csdb;
    this->directcsdbStatus = direct;
}

bool mrfdldrstate::GetFwStatus()
{
    return m_fw_done;
}

bool mrfdldrstate::GetIFWIStatus()
{
    return m_ifwi_done;
}

bool mrfdldrstate::IsFwState()
{
    return (m_dldr_state == DLDR_STATE_FW_NORMAL || m_dldr_state == DLDR_STATE_FW_MISC || m_dldr_state == DLDR_STATE_FW_WIPE);
}

bool mrfdldrstate::IsOsState()
{
    return (m_dldr_state == DLDR_STATE_OS_NORMAL || m_dldr_state == DLDR_STATE_OS_MISC);
}

bool mrfdldrstate::IsProvisionFailed()
{
    return m_abort;
}

bool mrfdldrstate::IsGPPReset()
{
    return m_GPPReset;
}

bool mrfdldrstate::IsOsOnly()
{
    return ((this->m_utils->FileSize(m_fname_fw_image) == 0));
}

bool mrfdldrstate::WriteOutPipe(unsigned char* pbuf, uint32 size)
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

bool mrfdldrstate::ReadInPipe(unsigned char* pbuf, uint32 size)
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

unsigned long long mrfdldrstate::ReadInAck()
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

unsigned long long mrfdldrstate::GetOpCode()
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

void mrfdldrstate::CheckFile(char *filename)
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

bool mrfdldrstate::HandleNoSize()
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    int ret_code = 0;
    dnx_data* fwdata = NULL;

    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_DNX0H_MRFD);
    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size)) {
        ret_code = 1; //USB error
        LogError(ret_code);
    }
    return (ret_code ? false:true);
}

void mrfdldrstate::LogError(unsigned long errorcode)
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    if(errorcode == 0xBADF00D)
    {
        m_abort = true;
        m_last_error.error_code = errorcode;
        this->m_utils->u_log(LOG_STATUS, "Error Firmware/OS Image Not initialized");
    }

    if(errorcode == 0x2468)
    {
        m_abort = true;
        m_last_error.error_code = errorcode;
        this->m_utils->u_log(LOG_STATUS, "Invalid Progress bar initialization");
    }
    if(errorcode < MAX_ERROR_CODE_MERRIFIIELD)
    {
        if(errorcode == 0)
        {
            this->m_utils->u_log(LOG_STATUS, "Error Code: %d - %s", errorcode, Merrifield_error_code_array[errorcode]);
        }
        else
        {
            this->m_utils->u_log(LOG_STATUS,"Error Code: %d - %s", errorcode, Merrifield_error_code_array[errorcode]);
            m_abort = true;
        }
        m_last_error.error_code = errorcode;
        strcpy(m_last_error.error_message, Merrifield_error_code_array[errorcode]);
    }
}

void mrfdldrstate::LogError(int errorcode, std::string msg)
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

bool mrfdldrstate::GetLastError(last_error* er)
{
    er->copy(m_last_error);
    return true;
}
void mrfdldrstate::StartLogTime()
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

void mrfdldrstate::EndlogTime()
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

void mrfdldrstate::LogProgress()
{
    static int counter = 0; //used to calculate m_stepsdone...use debugger to retrieve

    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    m_stepsdone++; //increment counter everytime we log progress
    m_progress = (int)((m_stepsdone/m_totalsteps)*100);
    this->m_utils->u_log(LOG_PROGRESS, "%d",((m_stepsdone>m_totalsteps)? 100:m_progress));
    counter++;
}

void mrfdldrstate::InitStepsDone()
{
    m_stepsdone = 0;
}

void mrfdldrstate::InitializeProgressBar(int Downloadtype)
{
    // progress bar depends on how many steps are required
    // use file size to determine what we are downloading
    if(Downloadtype == DOWNLOAD_FW_PROGRESS)
    {
        m_totalsteps = DLDR_FW_TOTAL_STEPS; //fw only progress bar
        m_stepsdone = 0;
    }

    else if(Downloadtype == DOWNLOAD_OS_PROGRESS)
    {
        m_totalsteps = DLDR_OS_TOTAL_STEPS; //fw only progress bar
        m_stepsdone = 0;
    }
    else
    {
        LogError(0x2468);
    }
}

void mrfdldrstate::FinishProgressBar()
{

    if(( m_totalsteps == DLDR_FW_TOTAL_STEPS)&&(m_fw_done == 1)&&(m_dldr_state == DLDR_STATE_FW_NORMAL))
    {
            m_stepsdone = m_totalsteps; //fw only finish
    }
    if(( m_totalsteps == DLDR_OS_TOTAL_STEPS)&&(m_os_done == 1)&&(m_dldr_state == DLDR_STATE_OS_NORMAL))
    {
            m_stepsdone = m_totalsteps; // os only finish
    }
    LogProgress(); //will advance progress bar to 100%
}

bool mrfdldrstate::FwDXBL()
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    int ret_code = 0;

    LogProgress();
    dnx_data* fwdata = NULL;
    if(!m_mfld_fw)
    {
        LogError(0xBADF00D);
        return false;
    }
    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_DNXD_MRFD);
    this->m_utils->u_log(LOG_FWUPGRADE, "Sending FW Dnx data...%x", fwdata);

    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size))
    {
        ret_code = 1; //USB error
        LogError(ret_code);
    }
    return ret_code ? false:true;
}

bool mrfdldrstate::OsDXBL()
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    int ret_code = 0;

    LogProgress();
    dnx_data* fwdata = NULL;
    if(!m_mfld_os)
    {
        LogError(0xBADF00D);
        return false;
    }

    fwdata = m_mfld_os->GetOsDnX();

    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size))
    {
        ret_code = 1; //USB error
        LogError(ret_code);
    }
#ifdef USB30_POC_OSDNX
            if(!ret_code) {
                SleepMs(20000);
                m_usbdev->Abort();
                void *tmphandle = NULL;
                int counter = 0;
                while(tmphandle == NULL) {
                    tmphandle = NULL;
                    if (!m_usbdev->Open()) {
                        this->m_utils->u_log(LOG_STATUS, "error openning usb device\n");
                        return false;
                    }
                    tmphandle = m_usbdev->GetDeviceHandle(0);
                    counter++;
                    SleepMs(1000);
                    this->m_utils->u_log(LOG_STATUS, "XFSTK-STATUS--Sleep one second and try to detect the device a gain...\n");
                    if(counter > 20) {
                        this->m_utils->u_log(LOG_STATUS, "XFSTK-STATUS--Timeout to detect the device, make sure to charge up your phone before flashing\n");
                        ret_code = 1;
                    }
                }
                m_usbdev->SetDeviceHandle(tmphandle);
            }
#endif
    return ret_code ? false:true;
}
bool mrfdldrstate::StartFw()
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    int ret_code = 0;
    dnx_data* fwdata = NULL;
    if(!m_mfld_fw)
    {
        LogError(0xBADF00D);
        return false;
    }

    // Start fw update
    this->m_utils->u_log(LOG_FWUPGRADE, "sending 6 DWORDS of DnxFW size and checksum...");
    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_DNXH_MERRIFIELDUTILS);
    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size))
    {
        ret_code = 1; //USB error
        LogError(ret_code);
    }

    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_IFWI);
    if(!fwdata)
    {
        this->m_utils->u_log(LOG_FWUPGRADE, "Error FW_DATA_IFWI not found...");
        LogError(0xBADF00D);
        return false;
    }
    m_numIfwiChunks = fwdata->size / ONE28_K;
    m_residualIfwiBytes = fwdata->size - (m_numIfwiChunks * ONE28_K);
    //if there are residual bytes then we want to send chunks +1
    m_ifwi128KBChunks = m_residualIfwiBytes ? ++m_numIfwiChunks : m_numIfwiChunks;

    LogProgress();
    return (ret_code ? false:true);
}

bool mrfdldrstate::StartOs()
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    int ret_code = 0;
    dnx_data* fwdata = NULL;

    this->m_utils->u_log(LOG_OS, "sending 6 DWORDS of DnxOS size and checksum...");

    fwdata = m_mfld_os->GetOsDnXHdr();

    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size))
    {
        ret_code = 1; //USB error
        LogError(ret_code);
    }
    LogProgress();
    return (ret_code ? false:true);
}

void mrfdldrstate::Visit(MrfdFwHandleER00& )
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

void mrfdldrstate::Visit(MrfdFwHandleHLT0 &)
{
    // --- FW file has no size
    this->m_utils->u_log(LOG_OPCODE, "HLT0 - DnX FW or IFWI Size = 0");
    m_fw_done = true;
}

void mrfdldrstate::Visit(MrfdStEmmcDumpTransfer& )
{
    m_utils->u_log(LOG_STATUS, "Transfer started");

    std::string outfile;
    int partition = 0;
    int blockCount = 0;
    int blockSize = 0;
    int offsetInEmmc = 0;
    bool bRegisterToken = false;
    string tokenOffset = "0x010C";

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

void mrfdldrstate::transferComplete()
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

void mrfdldrstate::Visit(MrfdEmmcDump$ACK& )
{
    m_utils->u_log(LOG_STATUS, "eMMC Dump $ACK");
    GotoState(MERR_EMMC_DUMP_STATE_TRANSFER);
}

void mrfdldrstate::Visit(MrfdEmmcDumpNACK& )
{
    m_utils->u_log(LOG_STATUS, "eMMC Dump NACK");
    m_abort = true;
}

void mrfdldrstate::Visit(MrfdEmmcDumpECSD& )
{
    m_utils->u_log(LOG_STATUS, "eMMC Dump ECSD");
}

void mrfdldrstate::Visit(MrfdEmmcDumpREQB& )
{
    m_utils->u_log(LOG_STATUS, "eMMC Dump REQB");
}

void mrfdldrstate::Visit(MrfdEmmcDumpEOIO& )
{
    m_utils->u_log(LOG_STATUS, "eMMC Dump EOIO");
    printf("\nDONE<->EOIO handshake completed...");
    m_fw_done = true;
}

void mrfdldrstate::Visit(MrfdEmmcDumpRDY$& )
{
    m_utils->u_log(LOG_STATUS, "eMMC Dump $ACK");
    GotoState(MERR_EMMC_DUMP_STATE_TRANSFER);
}
void mrfdldrstate::Visit(MrfdEmmcDumpER40& )
{
    LogError(40, "\nInitialize eMMCHW and Card failed.");
 }


void mrfdldrstate::Visit(MrfdFwHandleDFRM& )
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    bool ret = true;

    //init the progress bar to zero state at begining on download
    InitializeProgressBar(DOWNLOAD_FW_PROGRESS);

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

void mrfdldrstate::Visit(MrfdFwHandleDxxM& )
{
    //init the progress bar to zero state at begining on download
    InitializeProgressBar(DOWNLOAD_FW_PROGRESS);

    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
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
    LogProgress();
}

void mrfdldrstate::Visit(MrfdFwHandleDXBL& )
{
    bool ret = true;
    StartLogTime();

    if(m_dldr_state == DLDR_STATE_FW_NORMAL  \
            || m_dldr_state == DLDR_STATE_FW_MISC \
            || m_dldr_state == DLDR_STATE_FW_WIPE)
    {

        this->m_utils->u_log(LOG_FWUPGRADE, "FW: Sending DnX ...");
        ret = FwDXBL();
    } else
    {
        if(m_mfld_os && !m_mfld_os->validDnX())
        {
            LogError(0xBADF00D,"No OS DnX Has Been Provided");
            ret = false;
        }
        else
        {
            this->m_utils->u_log(LOG_OS, "OS: Sending DnX ...");
            ret = OsDXBL();
        }
    }

    EndlogTime();

    if(!ret)
    {
        m_abort = true;
    }
    LogProgress();
}

void mrfdldrstate::Visit(MrfdFwHandleRUPHS& )
{
    int ret_code = 0;
    dnx_data* fwdata = NULL;
    if(!m_mfld_fw)
    {
        LogError(0xBADF00D);
        return;
    }

    StartLogTime();

    this->m_utils->u_log(LOG_FWUPGRADE, "FW: Sending FW Update Profile Hdr Size...");
    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_RUPHS);

    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size))
    {
        ret_code = 1; //USB error
        LogError(ret_code);
    }

    EndlogTime();

    if(ret_code) {
        m_abort = true;
    }
    LogProgress();
}

void mrfdldrstate::Visit(MrfdFwHandleRUPH& )
{
    int ret_code = 0;
    dnx_data* fwdata = NULL;
    if(!m_mfld_fw)
    {
        LogError(0xBADF00D);
        return;
    }


    StartLogTime();

    this->m_utils->u_log(LOG_FWUPGRADE, "FW: Sending FW Update Profile Hdr...");
    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_RUPH);

    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size))
    {
        ret_code = 1; //USB error
        LogError(ret_code);
    }
#ifndef XFSTK_OS_WIN
    if(this->csdbStatus != " ")
        SleepMs(500);
#endif
    EndlogTime();

    if(ret_code) {
        m_abort = true;
    }
    LogProgress();
}



void mrfdldrstate::Visit(MrfdFwHandleUCSDB& )
{

    this->m_utils->u_log(LOG_USB, "%s", __FUNCTION__);
    BYTE cmd_code = 0;
    DWORD ret_code = 0;
    int csdb_size = 0;
    boost::scoped_array<unsigned char> buffer(new unsigned char[MAX_CSDB_PAYLOAD]);
    memset(buffer.get(),0,MAX_CSDB_PAYLOAD);
    StartLogTime();
    this->m_utils->u_log(LOG_STATUS, "FW: Receiving Refreshed CSDB ...");
    //Read the uploaded CSDB
    if(ReadInPipe(buffer.get(),MAX_CSDB_PAYLOAD))
    {
        memcpy(&cmd_code,buffer.get() + CSDB_CMD_OFFSET , sizeof(BYTE));
        memcpy(&ret_code,buffer.get() + CSDB_RESULT_OFFSET , sizeof(DWORD));
        memcpy(&csdb_size,buffer.get() + CSDB_SIZE_OFFSET , sizeof(DWORD));

        //if there is a payload, dump to file
        if(csdb_size && (csdb_size - CSDB_HEADER_SIZE) > 0)
        {
            QString curTime = QDateTime::currentDateTime().toString()\
                    .replace(":","_").replace(" ","_");
            std::string currentTime = "./csdb_payload_" + \
                    curTime.toStdString() +".bin";
            try
            {
                scoped_file fileout(currentTime.c_str(),"wb");
                if(fileout.valid())
                {
                    fileout.write(buffer.get(),1,csdb_size);
                    this->m_utils->u_log(LOG_STATUS, "CSDB payload dumped to file: %s",currentTime.c_str());
                }
                else
                {
                    this->m_utils->u_log(LOG_STATUS, "Unable to dump CSDB payload");
                }
            }
            catch(std::exception& e)
            {
                this->m_utils->u_log(LOG_STATUS, "Error dumping CSDB payload: %s", e.what());
            }
        }

        //if the final csdb bit has been set, then send a halt command to chaabi
        if((this->directcsdbStatus & FINALCSDB_MASK))
        {
            dnx_data* fwdata = NULL;
            if(!m_mfld_fw)
            {
                LogError(0xBADF00D);
                return;
            }
            //read in the last csdb payload
            fwdata = m_mfld_fw->GetFwImageData(FW_DATA_CSDB);
            if(!fwdata)
            {
                this->m_utils->u_log(LOG_FWUPGRADE, "Error FW_DATA_CSDB not found...");
                LogError(0xBADF00D);
                return;
            }
            BYTE haltCode = 0x1;
            //replace the command with halt command
            memcpy(fwdata->data + CSDB_CMD_OFFSET, &haltCode, sizeof(haltCode));
            this->GotoState(BULK_ACK_DCSDB);
        }
        else
        {
            m_fw_done = true;
        }
        if(ret_code != 0)
        {
            this->m_last_error.error_code = ret_code;
            strcpy(this->m_last_error.error_message, "CSDB returned nonzero result status");
            this->m_utils->u_log(LOG_STATUS, "CSDB returned nonzero result status of:0x%X",ret_code);
        }
        this->ResponseBuff.swap(buffer);
        this->ResponseBuffSize = csdb_size;
    }else
    {
        this->m_abort = true;
    }
    LogProgress();
}

void mrfdldrstate::Visit(MrfdFwHandleDCSDB& )
{
    int ret_code = 0;
    dnx_data* fwdata = NULL;
    if(!m_mfld_fw)
    {
        LogError(0xBADF00D);
        return;
    }
    StartLogTime();
    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_CSDB);

    if(!fwdata)
    {
        this->m_utils->u_log(LOG_FWUPGRADE, "Error FW_DATA_CSDB not found...");
        LogError(0xBADF00D);
        return;
    }
    this->m_utils->u_log(LOG_STATUS, "FW: Sending CSDB of size %d K ...",(fwdata->size/1024));


    if(!WriteOutPipe(fwdata->data, fwdata->size))
    {
        ret_code = 1; //USB error
        LogError(ret_code);
    }

    EndlogTime();
    if(ret_code) {
        m_abort = true;
    }
    LogProgress();
}

void mrfdldrstate::Visit(MrfdFwHandleDCFI00& )
{
    int ret_code = 0;
    dnx_data* fwdata = NULL;
    if(!m_mfld_fw && IsFwState())
    {
        LogError(0xBADF00D);
        return;
    }

    if(!m_mfld_os && IsOsState())
    {
        LogError(0xBADF00D);
        return;
    }

    StartLogTime();
    this->m_utils->u_log(LOG_STATUS, "FW: Sending DnX DCFI00 ...");
    if(IsFwState())
        fwdata = m_mfld_fw->GetFwImageData(FW_DATA_CHFI00);
    else if(IsOsState())
    {
        if(!m_mfld_os->validDnX())
        {
            LogError(0xBADF00D,"No OS DnX Has Been Provided");
            ret_code = false;
        }
        else
        {
            fwdata = m_mfld_os->GetChaabiDnX();
        }
    }
    else
    {
        this->m_utils->u_log(LOG_STATUS, "Unable to determine downloader state ...");
        LogError(0xBADF00D);
        return;
    }
    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size))
    {
        ret_code = 1; //USB error
        LogError(ret_code);
    }

    EndlogTime();

    if(ret_code) {
        m_abort = true;
    }
    LogProgress();
}

void mrfdldrstate::Visit(MrfdFwHandleDIFWI& )
{
    int ret_code = 0;
    dnx_data* fwdata = NULL;
    if(!m_mfld_fw)
    {
        LogError(0xBADF00D);
        return;
    }

    StartLogTime();
    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_IFWI);

    if(!fwdata)
    {
        this->m_utils->u_log(LOG_FWUPGRADE, "Error FW Data not found...");
        LogError(0xBADF00D);
        return;
    }

    //If there are residual bytes and 1 chunk left to send
    if(m_residualIfwiBytes && (1 == m_numIfwiChunks))
    {
        UCHAR byteBlock[ONE28_K];
        for(int i = 0; i < (int)m_residualIfwiBytes; i++)
        {
            byteBlock[i] = *(fwdata->data + m_ifwiArrayPtr);
            m_ifwiArrayPtr++;
        }
        if(fwdata && !WriteOutPipe(byteBlock, ONE28_K))
        {
            ret_code = 1; //USB error
            LogError(ret_code);
        }
        EndlogTime();
        LogProgress();
        m_ifwiArrayPtr = 0;
        m_numIfwiChunks = m_ifwi128KBChunks;

    }
    else
    {
        UCHAR byteBlock[ONE28_K];
        for(int i = 0; i < ONE28_K; i++)
        {
            byteBlock[i] = *(fwdata->data + m_ifwiArrayPtr);
            m_ifwiArrayPtr++;
        }
        if(fwdata && !WriteOutPipe(byteBlock, ONE28_K))
        {
            ret_code = 1; //USB error
            LogError(ret_code);
        }
        EndlogTime();
        m_numIfwiChunks--;
        if(m_numIfwiChunks == 0) {
            m_numIfwiChunks = m_ifwi128KBChunks;
            m_ifwiArrayPtr = 0;
        }
    }
    LogProgress();
}

void mrfdldrstate::Visit(MrfdFwHandleRESET& )
{
    StartLogTime();

    this->m_utils->u_log(LOG_FWUPGRADE, "FW: Received RESET...");

    EndlogTime();

    m_fw_done = true;
    m_GPPReset = true;
    LogProgress();
}

void mrfdldrstate::Visit(MrfdFwHandleHLT$& )
{
    // --- ACK Received: HLT$ ---
    if(m_perform_emmc_dump)
    {
        this->m_utils->u_log(LOG_FWUPGRADE, "FW: Emmc dump completed...");
    }
    else
    {
        this->m_utils->u_log(LOG_FWUPGRADE, "FW: Firmware update completed...");
    }
#if 0
    gettimeofday(&m_end_time, NULL);
    m_time_elapsed = get_time_elapse(&m_start_time, &m_end_time);
    this->m_utils->u_log(LOG_FWUPGRADE, "FW: IFWI download used %ds.", m_time_elapsed);
#endif
    m_fw_done = true;
    LogProgress();
}

void mrfdldrstate::Visit(MrfdFwHandleMFLD& )
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
    LogProgress();
}


void mrfdldrstate::Visit(MrfdFwHandleCLVT& )
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

void mrfdldrstate::Visit(MrfdFwHandleRTBD& )
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

        //data = ReadInAck();
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
            throw runtime_error("Error: Unexpected Battery signature");
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

void mrfdldrstate::Visit(MrfdOsHandleDORM& )
{
    bool ret = true;

    //init the progress bar to zero state at begining on download
    InitializeProgressBar(DOWNLOAD_OS_PROGRESS);

    this->m_utils->u_log(LOG_OS, "OS:  Handle OS download...");

    m_b_DnX_OS = 0;
    m_b_DnX_OS = m_gpflags & 0x20;
#if 0
    gettimeofday(&m_start_time, NULL);
#endif
    if(m_b_DnX_OS)
    {
        //Start Misc OS dowload state
        GotoState(DLDR_STATE_OS_MISC);
    } else if(m_gpflags & 0x1)
    {
        //Start normal OS dowload state
        GotoState(DLDR_STATE_OS_NORMAL);
    } else
    {
        this->m_utils->u_log(LOG_STATUS, "OS: Got DORM but OS recovery was not requested, booted into fw without fw dnx?");
        ret = false;
    }

    if(!ret) {
        m_abort = true;
    }
    LogProgress();
}

void mrfdldrstate::Visit(MrfdOsHandleOSIPSZ& )
{
}

void mrfdldrstate::Visit(MrfdOsHandleROSIP& )
{
    int ret_code = 0;
    dnx_data* fwdata = NULL;
    if(!m_mfld_os)
    {
        LogError(0xBADF00D);
        return;
    }

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
    LogProgress();
}

void mrfdldrstate::Visit(MrfdOsHandleDONE& )
{
}

void mrfdldrstate::Visit(MrfdOsHandleRIMG& )
{
    int ret_code = 0;
    dnx_data* fwdata = NULL;
    if(!m_mfld_os)
    {
        LogError(0xBADF00D);
        return;
    }

    fwdata = m_mfld_os->GetOsImageDataChunk();
    if(fwdata)
    {
        StartLogTime();

        this->m_utils->u_log(LOG_OS, "sending data to USB...");
        if(!WriteOutPipe(fwdata->data, fwdata->size))
        {
            ret_code = 39;  // do_usb_writes failed sending OS data
            LogError(ret_code);
        }

        this->m_utils->RIMGChunkSize = 0;

        // free pkt_buffer to re-allocate next size on next phase
        m_mfld_os->ReleaseOsImageDataChunk();

        EndlogTime();
    } else {
        this->m_utils->u_log(LOG_OS, "OS data buffer is NULL...");
        ret_code = 1;
    }
    this->m_utils->u_log(LOG_OS, "End of RIMG ");

    if(ret_code) {
        m_abort = true;
    }
    LogProgress();
}

void mrfdldrstate::Visit(MrfdOsHandleEOIU& )
{
    int ret_code = 0;
    UCHAR DFN[4] = "DFN";
    if(!m_mfld_os)
    {
        LogError(0xBADF00D);
        return;
    }

    StartLogTime();
    this->m_utils->u_log(LOG_OS, "EOIU Received...");


    // write out pipe
    this->m_utils->u_log(LOG_OS, "sending DFN...");

    if(!WriteOutPipe(DFN, sizeof(DFN)))
    {
        this->m_utils->u_log(LOG_OS, "After DFN is sent, if status is bad it will be ignored");
    }
#if 0
    //if there are more OS images
    if(!m_b_DnX_OS && os_data_size != 0)
    {

        this->m_utils->u_log(LOG_STATUS, "EOIU received but os_data_size is not zero: %d", os_data_size);
        this->m_utils->u_log(LOG_STATUS, "There are more OS images ...");
        this->m_utils->u_log(LOG_STATUS, "Allow 6 minutes to receive next ROSIP ...");
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
            time_counter++;
        }while(time_counter <= timeout && data != BULK_ACK_ROSIP);
        this->m_utils->u_log(LOG_STATUS,"OS: TIMEOUT waiting for ROSIP!! ");

    }
    else
    {
#endif
#if 0
        //Log time elapsed for OS download
        gettimeofday(&m_end_time, NULL);
        m_time_elapsed = get_time_elapse(&m_start_time, &m_end_time);
        this->m_utils->u_log(LOG_OS, "OS: Operating system download used %ds.", m_time_elapsed);
#endif
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

    EndlogTime();
    if(ret_code) {
        m_abort = true;
    }
    LogProgress();
}

void mrfdldrstate::Visit(MrfdStHandleFwNormal& )
{
    bool ret = false;

    StartLogTime();

    this->m_utils->u_log(LOG_FWUPGRADE, "Changing to DLDR_STATE_FW_NORMAL ...");

    m_mfld_fw = new MerrifieldFW;

    if(m_mfld_fw->Init(m_fname_dnx_fw, m_fname_fw_image,m_fname_bin_misc ,csdbStatus , m_utils, m_gpflags, false,m_perform_emmc_dump))
    {
        //Set the state machine to virgin emmc download
        m_dldr_state = DLDR_STATE_FW_NORMAL;
        this->m_utils->u_log(LOG_STATUS, "FW download is in progress ... ");
        //Send DnX FW size header data to target device
        //This will start the FW state machine
        ret = StartFw();
    }
    else if(this->m_utils->FileSize(m_fname_dnx_fw) == 0) {
        ret = HandleNoSize();
    }

    this->m_utils->u_log(LOG_FWUPGRADE, "End of MrfdStHandleFwNormal ");

    EndlogTime();
    if(!ret) {
        m_abort = true;
    }
    LogProgress();
}

void mrfdldrstate::Visit(MrfdStHandleFwMisc& )
{
    bool ret = false;

    StartLogTime();

    this->m_utils->u_log(LOG_FWUPGRADE, "Changing to DLDR_STATE_FW_MISC ...");

    //Set the state machine to virgin emmc download
    //Need to set this first, since mfld_fw.init could fail because of fw blank.bin files.
    m_dldr_state = DLDR_STATE_FW_MISC;

    m_mfld_fw = new MerrifieldFW;

    if(m_mfld_fw->Init(m_fname_dnx_fw, m_fname_fw_image,m_fname_bin_misc, csdbStatus\
            , m_utils, m_gpflags, false,m_perform_emmc_dump))
    {
        //Send DnX FW size header data to target device
        //This will start the FW state machine
        this->m_utils->u_log(LOG_STATUS, "FW(Miscdnx) download is in progress ... ");

        ret = StartFw();
    }
    else if((this->m_utils->FileSize(m_fname_dnx_misc) == 0) || (this->m_utils->FileSize(m_fname_fw_image) == 0))
    {
        ret = HandleNoSize();
        ret = true;
    }

    this->m_utils->u_log(LOG_FWUPGRADE, "End of MrfdStHandleFwMisc ");

    EndlogTime();

    if(!ret) {
        m_abort = true;
    }
    LogProgress();
}

void mrfdldrstate::Visit(MrfdStHandleFwWipe& )
{
    bool ret = false;
    unsigned long temp_gpflags = 0;

    StartLogTime();

    this->m_utils->u_log(LOG_FWUPGRADE, "Changing to DLDR_STATE_FW_WIPE ...");

    //Set the state machine to virgin emmc download
    //Need to set this first, since mfld_fw.init could fail because of fw blank.bin files.
    m_dldr_state = DLDR_STATE_FW_WIPE;

    m_mfld_fw = new MerrifieldFW;

    //Set gpflags to do a FW cold reset
    temp_gpflags = m_gpflags | 0x2;

    if(m_mfld_fw->Init(m_fname_dnx_fw, m_fname_fw_image,m_fname_bin_misc,\
            csdbStatus , m_utils, temp_gpflags, true, m_perform_emmc_dump))
    {
        //Send DnX FW size header data to target device
        //This will start the FW state machine
        this->m_utils->u_log(LOG_STATUS, "FW Wipe IFWI download is in progress ... ");
        ret = StartFw();

        m_ifwi_done = true;
    } else if((this->m_utils->FileSize(m_fname_dnx_fw) == 0)) {
        ret = HandleNoSize();// == BULK_ACK_HLT0)
        ret = true;
    }

    this->m_utils->u_log(LOG_FWUPGRADE, "End of MrfdStHandleFwWipe ");

    EndlogTime();

    if(!ret) {
        m_abort = true;
    }
    LogProgress();
}

void mrfdldrstate::Visit(MrfdStHandleOsNormal& )
{
    bool ret = false;
    int ret_code = 0;

    StartLogTime();

    this->m_utils->u_log(LOG_OS, "Changing to DLDR_STATE_OS_NORMAL ...");

    m_mfld_os = new MerrifieldOS;

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

            void *tmphandle = NULL;
            int counter = 0;

            while(tmphandle == NULL) {
                tmphandle = NULL;
                tmphandle = m_usbdev->GetDeviceHandle(0);
                counter++;

                SleepMs(1000);
                this->m_utils->u_log(LOG_STATUS, "XFSTK-STATUS--Sleep one second and try to detect the device a gain...\n");

                if(counter > 20) {
                    this->m_utils->u_log(LOG_STATUS, "XFSTK-STATUS--Timeout to detect the device, make sure to charge up your phone before flashing\n");
                    ret = false;
                }
            }
            m_usbdev->SetDeviceHandle(tmphandle);
            m_b_IDRQ = false;
            ret = Start();
        } else {
            if(m_mfld_os->validDnX())
            {
                 ret = StartOs();
            }
            else
            {
                ret = true;
                this->m_utils->u_log(LOG_STATUS, "Warning: No DnX Provided");
            }
        }
#else
        if(m_mfld_os->validDnX())
        {
             ret = StartOs();
        }
        else
        {
            ret = true;
            this->m_utils->u_log(LOG_STATUS, "Warning: No DnX Provided");
        }
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

    this->m_utils->u_log(LOG_OS, "End of MrfdStHandleOsNormal ");

    EndlogTime();
    if(!ret) {
        m_abort = true;
    }
    LogProgress();
}

void mrfdldrstate::Visit(MrfdStHandleOsMisc& )
{
    bool ret = false;
    int ret_code = 0;

    StartLogTime();

    this->m_utils->u_log(LOG_OS, "Changing to DLDR_STATE_OS_MISC ...");

    m_mfld_os = new MerrifieldOS;

    ret = m_mfld_os->Init(m_fname_dnx_os, m_fname_os_image, m_utils, m_gpflags);
    if(ret)
    {
        //Send DnX OS size header data to target device
        //This will start the OS state machine
        m_dldr_state = DLDR_STATE_OS_MISC;
        this->m_utils->u_log(LOG_STATUS, "POS(LPDDR) download is in progress ... ");

        if(m_mfld_os->validDnX())
        {
             ret = StartOs();
        }
        else
        {
            ret = true;
            this->m_utils->u_log(LOG_STATUS, "Warning: No DnX Provided");
        }
    }
    else if(this->m_utils->FileSize(m_fname_dnx_os) == 0) {
        dnx_data* fwdata = m_mfld_os->GetNoSizeData();
        if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size)) {
            ret_code = 1; //USB error
            LogError(ret_code);
        }
        ret = (ret_code ? false:true);
    }

    this->m_utils->u_log(LOG_OS, "End of MrfdStHandleOsMisc ");

    EndlogTime();
    if(!ret) {
        m_abort = true;
    }
    LogProgress();
}

void mrfdldrstate::Visit(MrfdErHandleLogError& )
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


//Added for PMIC provision, this state simply relays a message from the device
void mrfdldrstate::Visit(MrfdHandleLogDevice& )
{
    if(m_achCode)
    {
        if(m_fw_log_map.find(m_achCode) != m_fw_log_map.end())
        {
            unsigned long logCode = m_fw_log_map[m_achCode];
            if(logCode < MAX_ERROR_CODE_MERRIFIIELD)
            {
                this->m_utils->u_log\
                        (LOG_STATUS,"%s", Merrifield_error_code_array[logCode]);
            }
        }
    }
    else
    {
        this->m_utils->u_log(LOG_STATUS, "Unknown ACK code... 0x%X",m_achCode );
    }
}

void mrfdldrstate::Visit(MrfdErHandleERRR& )
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

bool mrfdldrstate::IDRQresponse()
{
    idrqBuffer responseBuff;
    string responseBuffStrs[15];
    boost::scoped_array<unsigned char> tmpkeyBIN;
    bool write;

    fstream IDRQFile;
    write = strcmp(m_fname_bin_misc , "BLANK.bin") != 0;
    if(write)
    {
        IDRQFile.open(this->m_fname_bin_misc,ios::out | ios::binary);

        if(!IDRQFile.is_open())
        {
            this->m_utils->u_log(LOG_STATUS,"Error Opening File");
            return false;
        }
    }
    memset(&responseBuff,0,sizeof(responseBuff));
    if(!this->m_usbdev->Read(reinterpret_cast<void*>(&responseBuff),sizeof(responseBuff)))
    {
        //error
        this->m_utils->u_log(LOG_STATUS,"Error Reading IDRQ response");
		IDRQFile.close();
        return false;
    }
    this->ResponseBuffSize = sizeof(responseBuff);
    this->ResponseBuff.reset(new unsigned char[ResponseBuffSize]);
    memcpy(this->ResponseBuff.get(), reinterpret_cast<void*>(&responseBuff),ResponseBuffSize);

    responseBuffStrs[0] = string(reinterpret_cast<char*>(&responseBuff.preamble),sizeof(DWORD));
    if(write)
        IDRQFile.write(reinterpret_cast<char*>(&responseBuff.preamble),sizeof(DWORD));

    if((responseBuffStrs[0].compare("MRFD")!=0) && (responseBuffStrs[0].compare("MOOR")!=0))
    {
        //preamble error
        this->m_utils->u_log(LOG_STATUS,"Error: Preamble 0x%x not equal to 'MRFD' or 'MOOR' ", responseBuff.preamble);
    }

    tmpkeyBIN.reset((new unsigned char[1*sizeof(DWORD)]));
    memcpy(tmpkeyBIN.get(),reinterpret_cast<void*>(&responseBuff.Status),sizeof(DWORD));
    this->bin2ascii(responseBuffStrs[14],tmpkeyBIN.get(),sizeof(DWORD));
    if(responseBuffStrs[14] != "00000000")
    {
        this->m_utils->u_log(LOG_STATUS,"Warning: Device responded with error code 0x%X",responseBuff.Status);
        this->m_last_error.error_code = responseBuff.Status;
        strcpy(this->m_last_error.error_message, "IDRQ Error");
    }

    //HASH keys
    for(int i = 1; i<9; i++)
    {
        tmpkeyBIN.reset(new unsigned char[4*sizeof(DWORD)]);
        memcpy(tmpkeyBIN.get(),reinterpret_cast<void*>(&responseBuff.hashkeys[i-1]),4*sizeof(DWORD));
        this->bin2ascii(responseBuffStrs[i],tmpkeyBIN.get(), KEYSIZE_MRFD);
        if(write)
            IDRQFile.write(reinterpret_cast<char*>(&responseBuff.hashkeys[i-1]),4*sizeof(DWORD));

        this->m_utils->u_log(LOG_STATUS," Hashkey %d: %s",i,responseBuffStrs[i].c_str());
    }

    //PartSpecificIDVariable
    tmpkeyBIN.reset(new unsigned char[4*sizeof(DWORD)]);
    memcpy(tmpkeyBIN.get(),reinterpret_cast<void*>(&responseBuff.PartSpecificIDVariable),4*sizeof(DWORD));
    if(write)
        IDRQFile.write(reinterpret_cast<char*>(&responseBuff.PartSpecificIDVariable),4*sizeof(DWORD));

    this->bin2ascii(responseBuffStrs[9],tmpkeyBIN.get(),4*sizeof(DWORD));
    this->m_utils->u_log(LOG_STATUS,"PartSpecificIDVariable: %s",responseBuffStrs[9].c_str());



    //PartSpecificIDConstant
    tmpkeyBIN.reset(new unsigned char[4*sizeof(DWORD)]);
    memcpy(tmpkeyBIN.get(),reinterpret_cast<void*>(&responseBuff.PartSpecificIDConstant),4*sizeof(DWORD));

    if(write)
        IDRQFile.write(reinterpret_cast<char*>(&responseBuff.PartSpecificIDConstant),4*sizeof(DWORD));

    this->bin2ascii(responseBuffStrs[10],tmpkeyBIN.get(),4*sizeof(DWORD));
    this->m_utils->u_log(LOG_STATUS,"PartSpecificIDConstant: %s",responseBuffStrs[10].c_str());


    // PMICTimeBase
    tmpkeyBIN.reset(new unsigned char[sizeof(DWORD)]);
    memcpy(tmpkeyBIN.get(),reinterpret_cast<void*>(&responseBuff.PMICTimeBase),sizeof(DWORD));

    if(write)
        IDRQFile.write(reinterpret_cast<char*>(&responseBuff.PMICTimeBase),sizeof(DWORD));

    this->bin2ascii(responseBuffStrs[11],tmpkeyBIN.get(),sizeof(DWORD));
    this->m_utils->u_log(LOG_STATUS,"PMICTimeBase: %s",responseBuffStrs[11].c_str());


    //PMICNonce
    tmpkeyBIN.reset(new unsigned char[4*sizeof(DWORD)]);
    memcpy(tmpkeyBIN.get(),reinterpret_cast<void*>(&responseBuff.PMICNonce),4*sizeof(DWORD));

    if(write)
        IDRQFile.write(reinterpret_cast<char*>(&responseBuff.PMICNonce),4*sizeof(DWORD));

    this->bin2ascii(responseBuffStrs[12],tmpkeyBIN.get(),4*sizeof(DWORD));
    this->m_utils->u_log(LOG_STATUS,"PMICNonce: %s",responseBuffStrs[12].c_str());


    //LiveNonce
    tmpkeyBIN.reset(new unsigned char[4*sizeof(DWORD)]);
    memcpy(tmpkeyBIN.get(),reinterpret_cast<void*>(&responseBuff.LiveNonce),4*sizeof(DWORD));

    if(write)
        IDRQFile.write(reinterpret_cast<char*>(&responseBuff.LiveNonce),4*sizeof(DWORD));

    this->bin2ascii(responseBuffStrs[13],tmpkeyBIN.get(),4*sizeof(DWORD));
    this->m_utils->u_log(LOG_STATUS,"LiveNonce: %s",responseBuffStrs[13].c_str());

    if(write)
    {
        IDRQFile.write(reinterpret_cast<char*>(&responseBuff.Status),sizeof(DWORD));
    }

	IDRQFile.close();
    return true;

}


bool mrfdldrstate::bin2ascii(std::string& ascii, unsigned char* bin, int size)
{
    bool retval = true;
    std::stringstream str;
    for(int i = 0;i<size;i++)
    {
        //Convert the hex values into a hex string
        str << std::hex << uppercase <<std::setfill('0')<< std::setw(2) << short((*(bin+(size-i-1))));
    }
    ascii = str.str();
    str.flush();
    return retval;
}



bool mrfdldrstate::Start()
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    INT ret_code = 0;
    ULONG preamble_msg = PREAMBLE_DNER;

    if(m_b_IDRQ)
    {
        this->m_utils->u_log(LOG_OPCODE, "Sending IDRQ...");
        preamble_msg = PREAMBLE_IDRQ;
    }
    else if((!(this->directcsdbStatus & INITCSDB_MASK)) && !m_perform_emmc_dump)
    {
        m_mfld_fw = new MerrifieldFW;

        if(m_mfld_fw->Init(m_fname_dnx_misc, m_fname_fw_image,m_fname_bin_misc, \
                    csdbStatus , m_utils, m_gpflags, false, m_perform_emmc_dump))
        {
            GotoState(BULK_ACK_DCSDB);
            return true;
        }
        else
        {
           this->m_utils->u_log(LOG_STATUS, "Failed to init DnX and CSDB, aborting ...\n");
           return false;
        }
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

void mrfdldrstate::GotoState(unsigned long long state)
{
    m_dldr_state = state;
    m_state_change = true;
}

void mrfdldrstate::SleepMs(int delay)
{
#if defined XFSTK_OS_WIN
        Sleep(delay);
#else
        usleep(1000*delay);
#endif
}

bool mrfdldrstate::doRegisterToken()
{
    const int SN_BUF_SIZE = 32;
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

bool mrfdldrstate::StateMachine()
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
    if(m_b_IDRQ)
    {
        ret = this->IDRQresponse();
        m_fw_done = true;
        m_os_done = true;
        if(ret)
        {
            m_last_error.error_code = 0;
           //temp solution
            strcpy(m_last_error.error_message, "IDRQ Successful");
        }
    }
    else
    {
        while(1) {

            ackcode = GetOpCode();

            iter1 = m_bulk_ack_map.find(ackcode);
            if (iter1 != m_bulk_ack_map.end() ) {
                iter1->second->Accept(*this);
            } else {
                this->m_utils->u_log(LOG_STATUS, "Unknown Ack code:%x, aborting ...\n", ackcode);
                m_abort = true;
                ret = false;
            }

            if(m_abort) {
                ret = false;
                break;
            }

            if(m_fw_done) {
                FinishProgressBar();
                ret = true;
                LogError(0);
                break;
            }

            if(m_os_done) {
                FinishProgressBar();
                ret = true;
                LogError(0);
                break;
            }
        }
    }

    return ret;
}

