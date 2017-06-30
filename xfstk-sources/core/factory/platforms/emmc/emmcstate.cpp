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
#include <string>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <algorithm>
#include "emmcmessages.h"
#include "emmcstate.h"
#include "common/scoped_file.h"
#include <boost/smart_ptr/scoped_array.hpp>

#define USB_READ_WRITE_DELAY_MS 0

extern CPSTR Emmc_error_code_array [MAX_ERROR_CODE];  

#define FILENAME "emmc_dump2.bin"
#define BLOCKSIZE	512
#define NUMBLOCKS	6
#define EMMC_START_ADDR 0
#define USER_PARTITION	0
#define BOOT_PARTITION_1  1
#define BOOT_PARTITION_2  2
#define EXT_CSD_MODE	3

#define BUF_SIZE 128*1024 // 128K
#define ACK_MESSAGE_SIZE	4
#define DNER_MESSAGE_SIZE	4
#define EXTCSD_RETURN_SIZE	512
#define EOIO_MESSAGE_SIZE	20

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


//EmmcDumpState Implementation
EmmcDumpState::EmmcDumpState():
    m_gpflags(0),
    m_usbdev(NULL),
    m_utils(NULL),
    m_b_usbinitok(false),
    m_achCode(0),
    m_numChunks(0),
    m_numResidentChunks(0),
    m_numRomPatchChunks(0),
    m_secFwArrayPtr(0),
    m_sec2FwArrayPtr(0),
    m_sec3FwArrayPtr(0),
    m_romPatchArrayPtr(0),
    m_vedFwArrayPtr(0),
    m_fwprogress(0),
    m_fwstepsdone(0),
    m_fwtotalsteps(DLDR_FW_TOTAL_STEPS_EMMC),//12),
    m_b_IDRQ(false),
    m_b_DnX_OS(0),
    m_abort(false),
    m_GPPReset(false),
    m_dldr_state(DLDR_STATE_INVALID),
    m_ROSIP_data_flag(false),
    m_delay_ms(USB_READ_WRITE_DELAY_MS)
{


    // FW Upgrade Ack values, index pair map
    int i = 0;

    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_DxxM, new EmmcDumpFwHandleDxxM));
    m_bulk_ack_map.insert(std::make_pair(DLDR_STATE_FW_MISC, new EmmcDumpStHandleFwMisc));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_DXBL, new EmmcDumpFwHandleDXBL));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_RDY$, new EmmcDumpRDY$));
    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_UPDATE_SUCESSFUL, new EmmcDumpFwHandleHLT$));

    //eMMC Dump map
    m_bulk_ack_map.insert(std::make_pair(EMMC_DUMP_ACK, new EmmcDump$ACK));
    m_bulk_ack_map.insert(std::make_pair(EMMC_DUMP_NACK, new EmmcDumpNACK));
    m_bulk_ack_map.insert(std::make_pair(EMMC_DUMP_STATE_TRANSFER, new StEmmcDumpTransfer));
    m_bulk_ack_map.insert(std::make_pair(EMMC_DUMP_ECSD, new EmmcDumpECSD));
    m_bulk_ack_map.insert(std::make_pair(EMMC_DUMP_SPR$, new EmmcDumpSPR$));
    m_bulk_ack_map.insert(std::make_pair(EMMC_DUMP_REQB, new EmmcDumpREQB));
    m_bulk_ack_map.insert(std::make_pair(EMMC_DUMP_EOIO, new EmmcDumpEOIO));

    m_bulk_ack_map.insert(std::make_pair(BULK_ACK_ER40, new EmmcDumpER40));



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

EmmcDumpState::~EmmcDumpState()
{
    // clear the entries in the map
    m_bulk_ack_map.clear();
    m_fw_error_map.clear();
}

bool EmmcDumpState::Init(IDevice * usbdev, EmmcUtils* utils)
{
    if(!m_b_usbinitok) {
        m_usbdev = usbdev;

        m_utils = utils;
        m_b_usbinitok = true;
    }
    m_ROSIP_data_flag = false; 

    return true;
}

void EmmcDumpState::SetOptions(EmmcOptions *Options)
{
    DeviceSpecificOptions = Options;
}

bool EmmcDumpState::UsbDevInit(void)
{
    m_b_usbinitok = false;
    m_b_usbinitok = m_usbdev->Open();
    return m_b_usbinitok;
}

bool EmmcDumpState::DoUpdate(char* fname_dnx_misc, unsigned long usbdelayms)
{
    m_fw_done = false;
    m_GPPReset = false;
    m_abort = false;
    m_state_change = false;
    m_fwstepsdone = 0;

    if(DeviceSpecificOptions->IsRegisterToken())
    {
        return doRegisterToken();
    }
    else
    {
        m_fname_dnx_misc = fname_dnx_misc;
        m_fw_done = false;
        m_GPPReset = false;
        m_abort = false;
        m_state_change = false;
        m_delay_ms = usbdelayms;
        m_fwstepsdone = 0;

        //Start downloader state machine
        return StateMachine();
    }

}

bool EmmcDumpState::IsProvisionFailed()
{
    return m_abort;
}

bool EmmcDumpState::IsGPPReset()
{
    return m_GPPReset;
}

bool EmmcDumpState::WriteOutPipe(unsigned char* pbuf, uint32 size)
{
    bool result = true;
    m_utils->u_log(LOG_USB, "%s %s", __FUNCTION__, m_utils->makestring(pbuf, size).c_str());
    if(m_b_usbinitok) {
        m_utils->u_log(LOG_USB, "%s %d", __FUNCTION__, m_delay_ms);

        SleepMs(m_delay_ms);
        if(!m_usbdev->Write(pbuf,size)) {
            return false;
        }
    }
    else {
        return false;
    }
    return result;
}

bool EmmcDumpState::ReadInPipe(unsigned char* pbuf, uint32 size)
{
    bool result = true;
    m_utils->u_log(LOG_USB, "%s %s", __FUNCTION__, m_utils->makestring(pbuf, size).c_str());
    if(m_b_usbinitok) {
        SleepMs(m_delay_ms);
        if(!m_usbdev->Read(pbuf,size)) {
            return false;
        }
    }
    else {
        return false;
    }
    return result;
}

unsigned long long EmmcDumpState::ReadInAck()
{
    m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    unsigned long long  rev_data = 0;
    uint32 bytes_rxed = 0;
    boost::scoped_array<unsigned char> szBuff (new unsigned char[MAX_BUFFER_SIZE + 1]);
    memset(szBuff.get(),0,MAX_BUFFER_SIZE + 1);

    bool result = true;

    if(m_b_usbinitok) {
        SleepMs(m_delay_ms);
        result = m_usbdev->GetAck(szBuff.get(), &bytes_rxed);
    }

    if(result == false) {
        LogError(4);
        return 0;
        //ReInit();
    }

    m_achCode = m_utils->scan_string_for_protocol((char *)szBuff.get());

    rev_data = m_utils->endian_swap(m_achCode);
    string output;
    if(bytes_rxed == 4) {
        uint32 tmp = (uint32)rev_data;
        output = m_utils->makestring((unsigned char*)(&tmp), 4);
    }
    else {
        output = m_utils->makestring((unsigned char*)(&rev_data), 8);
    }

    return m_achCode;
}

unsigned long long EmmcDumpState::GetOpCode()
{
    m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
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

void EmmcDumpState::CheckFile(char *filename)
{
    m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    FILE *fp;
    m_abort = false;

    fp = fopen(filename, "rb" );
    if ( fp == NULL) {
        m_utils->u_abort("File %s cannot be opened", filename);
        m_abort = true;
    }
    struct stat file_info;
    if(stat(filename, &file_info)) {
        m_utils->u_abort("Failed to stat file: %s", filename);
        m_abort = true;
    }

    if(fp)
        fclose(fp);
}

void EmmcDumpState::LogError(int errorcode)
{
    m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    if(errorcode < MAX_ERROR_CODE) {
        if(errorcode == 0) {
            m_utils->u_log(LOG_STATUS, "Error Code: %d - %s", errorcode, Emmc_error_code_array[errorcode]);
        } else {
            m_utils->u_log(LOG_STATUS,"Error Code: %d - %s", errorcode, Emmc_error_code_array[errorcode]);
            m_abort = true;
        }
        m_last_error.error_code = errorcode;
        strcpy(m_last_error.error_message, Emmc_error_code_array[errorcode]);
    }
}
void EmmcDumpState::LogError(int errorcode, std::string msg)
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

bool EmmcDumpState::GetLastError(last_error* er)
{
    er->copy(m_last_error);
    return true;
}
void EmmcDumpState::StartLogTime()
{
#if 0
    string strtime;
    m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    if(m_b_measure_usb_time) {
        gettimeofday(&m_start_time, NULL);
        gettime(&m_start_time, NULL, &strtime);
        m_utils->u_log(LOG_FWUPGRADE, "%s -- Start time stamp: %s", __FUNCTION__, strtime.c_str());
    }
#endif
}

void EmmcDumpState::EndlogTime()
{
#if 0
    string strtime;
    m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    if(m_b_measure_usb_time) {
        gettimeofday(&m_end_time, NULL);
        m_time_elapsed += get_time_elapse(&m_start_time, &m_end_time);
        gettime(&m_start_time, NULL, &strtime);
        m_utils->u_log(LOG_FWUPGRADE, "%s -- End time stamp: %s", __FUNCTION__, strtime.c_str());
    }
#endif
}

void EmmcDumpState::LogProgress()
{
    m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    m_fwprogress = (int)((m_fwstepsdone/m_fwtotalsteps)*100);
    m_utils->u_log(LOG_PROGRESS, "%d",m_fwprogress);
}

void EmmcDumpState::Visit(StEmmcDumpTransfer& )
{
    m_utils->u_log(LOG_STATUS, "Transfer started");

    std::string outfile;
    std::string ufwdnxFile;
    int partition = 0;
    int blockCount = 0;
    int blockSize = 0;
    int offsetInEmmc = 0;
    bool bRegisterToken = false;
    string tokenOffset = "0x0108";
    if (DeviceSpecificOptions){
        outfile = DeviceSpecificOptions->GetFile();
        ufwdnxFile = DeviceSpecificOptions->GetUnsignedFwDnxFile();
        if (DeviceSpecificOptions->GetUmipdumpValue())
        {
            partition = 0;
            blockCount = 1;
            blockSize = 65536;
            offsetInEmmc = 512;
        }
        else
        {
            partition = DeviceSpecificOptions->GetPartition();
            blockCount = DeviceSpecificOptions->GetBlockCount();
            blockSize = DeviceSpecificOptions->GetBlockSize();
            offsetInEmmc = DeviceSpecificOptions->GetOffset();
            bRegisterToken = DeviceSpecificOptions->IsRegisterToken();
            tokenOffset = DeviceSpecificOptions->GetTokenOffset();
        }
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

void EmmcDumpState::transferComplete()
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

void EmmcDumpState::Visit(EmmcDump$ACK& )
{
    m_utils->u_log(LOG_STATUS, "eMMC Dump $ACK");
    GotoState(EMMC_DUMP_STATE_TRANSFER);
}

void EmmcDumpState::Visit(EmmcDumpNACK& )
{
    m_utils->u_log(LOG_STATUS, "eMMC Dump NACK");
    m_abort = true;
}

void EmmcDumpState::Visit(EmmcDumpECSD& )
{
    m_utils->u_log(LOG_STATUS, "eMMC Dump ECSD");
}

void EmmcDumpState::Visit(EmmcDumpREQB& )
{
    m_utils->u_log(LOG_STATUS, "eMMC Dump REQB");
}

void EmmcDumpState::Visit(EmmcDumpEOIO& )
{
    m_utils->u_log(LOG_STATUS, "eMMC Dump EOIO");
    printf("\nDONE<->EOIO handshake completed...");
    m_fw_done = true;
}

void EmmcDumpState::Visit(EmmcDumpFwHandleDxxM& )
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    bool ret = true;
    this->m_utils->u_log(LOG_OPCODE, "FW: Handle None virgin part DnX ...");

    m_b_DnX_OS = 0;
    m_b_DnX_OS = (m_gpflags & 0x20);
#if 0
    gettimeofday(&m_start_time, NULL);
#endif

    if(m_b_DnX_OS || strlen(m_fname_dnx_misc) > 3) //Only if misc dnx file is provided
        GotoState(DLDR_STATE_FW_MISC);

    if(!ret) {
        m_abort = true;
    }
}
void EmmcDumpState::Visit(EmmcDumpStHandleFwMisc& )
{
    bool ret = false;

    StartLogTime();

    this->m_utils->u_log(LOG_FWUPGRADE, "Changing to DLDR_STATE_FW_MISC ...");

    //Set the state machine to virgin emmc download
    //Need to set this first, since mfld_fw.init could fail because of fw blank.bin files.
    m_dldr_state = DLDR_STATE_FW_MISC;

    m_mfld_fw = new EmmcFW;

    if(m_mfld_fw->Init(m_fname_dnx_misc, m_fname_fw_image, m_utils, m_gpflags, false) && !m_b_DnX_OS)
    {
        //Send DnX FW size header data to target device
        //This will start the FW state machine
        this->m_utils->u_log(LOG_STATUS, "FW(Miscdnx) download is in progress ... ");

        ret = StartFw();
    } else if(m_b_DnX_OS || (this->m_utils->FileSize(m_fname_dnx_misc) == 0)) {
        //AMB ret = HandleNoSize();// == BULK_ACK_HLT0)
        ret = true;
    }

    this->m_utils->u_log(LOG_FWUPGRADE, "End of StHandleFwMisc ");

    EndlogTime();

    if(!ret) {
        m_abort = true;
    }
}
void EmmcDumpState::Visit(EmmcDumpFwHandleDXBL& )
{
    bool ret = true;

    m_fwstepsdone++;

    StartLogTime();

    if(m_dldr_state == DLDR_STATE_FW_NORMAL  \
            || m_dldr_state == DLDR_STATE_FW_MISC \
            || m_dldr_state == DLDR_STATE_FW_WIPE) {
        this->m_utils->u_log(LOG_FWUPGRADE, "FW: Sending DnX ...");
        ret = FwDXBL();
    }
    EndlogTime();
    LogProgress();

    if(!ret) {
        m_abort = true;
    }
}
void EmmcDumpState::Visit(EmmcDumpRDY$& )
{
    m_utils->u_log(LOG_STATUS, "eMMC Dump $ACK");
    GotoState(EMMC_DUMP_STATE_TRANSFER);
}
void EmmcDumpState::Visit(EmmcDumpFwHandleHLT$& )
{
    // --- ACK Received: HLT$ ---
#if 0
    gettimeofday(&m_end_time, NULL);
    m_time_elapsed = get_time_elapse(&m_start_time, &m_end_time);
    this->m_utils->u_log(LOG_FWUPGRADE, "FW: IFWI download used %ds.", m_time_elapsed);
#endif
    m_fwstepsdone = DLDR_FW_TOTAL_STEPS_EMMC;
    m_fw_done = true;

    m_fwprogress = (int)((m_fwstepsdone/m_fwtotalsteps)*100);
    this->m_utils->u_log(LOG_PROGRESS, "%d",((m_fwstepsdone>m_fwtotalsteps)? 100:m_fwprogress));

}
void EmmcDumpState::Visit(EmmcDumpER40& )
{
    LogError(40, "\nInitialize eMMCHW and Card failed.");
 }

bool EmmcDumpState::StartFw()
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

    // Start fw update
    this->m_utils->u_log(LOG_FWUPGRADE, "sending 6 DWORDS of DnxFW size and checksum...");
    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_DNXH_EMMC);
    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size))
    {
        ret_code = 1; //USB error
        LogError(ret_code);
    }
    return (ret_code ? false:true);
}
bool EmmcDumpState::FwDXBL()
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    int ret_code = 0;

    dnx_data* fwdata = NULL;
    fwdata = m_mfld_fw->GetFwImageData(FW_DATA_DNXD_EMMC);
    this->m_utils->u_log(LOG_FWUPGRADE, "Sending FW Dnx data...%x", fwdata);

    if(fwdata && !WriteOutPipe(fwdata->data, fwdata->size))
    {
        ret_code = 1; //USB error
        LogError(ret_code);
    }
    return ret_code ? false:true;
}
bool EmmcDumpState::Start()
{
    m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    INT ret_code = 0;
    ULONG preamble_msg = PREAMBLE_DNER;

    // --- Send DnER:  ---
    preamble_msg = PREAMBLE_DNER;
    m_utils->u_log(LOG_OPCODE, "Sending DnER...");

    if(!WriteOutPipe((PUCHAR)&preamble_msg, 4))
    {
        ret_code = 1; //USB error
        LogError(ret_code);
    }

    return ret_code ? false:true;
}

void EmmcDumpState::GotoState(unsigned long long state)
{
    m_dldr_state = state;
    m_state_change = true;
}

void EmmcDumpState::SleepMs(int delay)
{
#if defined XFSTK_OS_WIN
        Sleep(delay);
#else
        usleep(1000*delay);
#endif
}

bool EmmcDumpState::doRegisterToken()
{
    const int SN_BUF_SIZE = 16;
    char sn[SN_BUF_SIZE];
    m_usbdev->GetUsbsn(sn);

    const int TIME_BUF_SIZE = 4;
    unsigned int expirationDurationSeconds = 0;
    if(!DeviceSpecificOptions->GetExpirationDuration().empty()) //handle expiry
    {
        try
        {
            expirationDurationSeconds = convertDurationToSeconds(DeviceSpecificOptions->GetExpirationDuration());
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

    if(!DeviceSpecificOptions->GetFile().empty())
    {
        if(writeTokenToFile(DeviceSpecificOptions->GetFile(), s1, s2+s3, !DeviceSpecificOptions->GetExpirationDuration().empty()))
        {
            printf("Token written to file.\n");
        }
        else
        {
            printf("Error writing token to file.\n");
        }
    }

    if(!DeviceSpecificOptions->GetUnsignedFwDnxFile().empty())
    {
        unsigned int iOffset = convertHexStringToInteger(DeviceSpecificOptions->GetTokenOffset());
        if(writeTokenToUnsignedDnXAtOffset(DeviceSpecificOptions->GetUnsignedFwDnxFile(), token, iOffset, !DeviceSpecificOptions->GetExpirationDuration().empty()))
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

bool EmmcDumpState::StateMachine()
{
    bool ret = true;
    HandleMapType::iterator iter1;
    unsigned long long ackcode = 0;

    m_abort = false;

    //Start the downloader state machine
    if(!Start()) {
        m_utils->u_log(LOG_STATUS, "Failed to start eMMC Dump state machine, aborting ...\n");
        return false;
    }

    //Looping until the download is done or fail.
    while(1) {

        ackcode = GetOpCode();

        iter1 = m_bulk_ack_map.find(ackcode);
        if (iter1 != m_bulk_ack_map.end() ) {
            iter1->second->Accept(*this);
        } else {
            m_utils->u_log(LOG_STATUS, "Unknown Ack code:%x, aborting ...\n", ackcode);
            m_abort = true;
            ret = false;
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

    }
    
    return ret;
}
