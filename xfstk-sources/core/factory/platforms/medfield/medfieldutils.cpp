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
#include <cstring>
#include <string>
#include "medfieldmessages.h"
#include "medfieldutils.h"
#include "../../common/xfstktypes.h"

using namespace std;

CPSTR Medfield_error_code_array [MAX_ERROR_CODE] = {
                    "Success",                                                                //0
                    "Connection error",                                                       //1
                    "Aborted by User",                                                        //2
                    "NACK Received: MVER",                                                    //3
                    "Error attempting to read ACK/NACK from IN Pipe",                         //4
                    "Library memory allocation failure",                                      //5
                    "Error reading from input file stream",                                   //6
                    "Can not open handle to device",                                          //7
                    "Can not open input file",                                                //8
                    "Can not retrieve file information",                                      //9
                    "FW/OS file wrong size",                                                  //10
                    "OS buffer == NULL",                                                      //11
                    "OS Recovery data size mismatch",                                         //12
                    "Unknown error code",                                                     //13
                    "do not have a valid handle to WinUSB lib",                               //14
                    "out_pipe is NULL",                                                       //15
                    "in_pipe is NULL",                                                        //16
                    "ER00 - Invalid Ping",                                                    //17
                    "HLT0 - DnX FW or IFWI Size = 0",                                         //18
                    "ER01 - DnX FW Sum Mismatch",                                             //19
                    "ER02 - DnX FW CPH Signature Error",                                      //20
                    "ER03 - DnX FW CPH Sum Mismatch",                                         //21
                    "ER04 - DnX binary is less than 1KB",                                     //22
                    "ER10 - DX_PUB_KEY_HASH_VALIDATION_FAILURE",                              //23
                    "ER11 - DX_INVALID_PRIMARY_VRL_MAGIC_NUM_VAL",                            //24
                    "ER12 - DX_VRL_PRIMARY_INTEGRITY_FAILURE",                                //25
                    "ER13 - DX_RECORD_INTEGRITY_FAILURE",                                     //26
                    "ER15 - Public Key Index is invalid - greater than 4",                    //27
                    "ER16 - VLR header size != ImageOffset in the VLR Header",                //28
                    "ER17 - &#8216;$DnX&#8217; Signature not found",                          //29
                    "ER18 - DnX image size in the CDPH is out of range - greater than 96KB",  //30
                    "ER20 - FW Update Profile Header Signature Error",                        //31
                    "ER21 - Primary or Secondary Security FW Size = 0",                       //32
                    "ER22 - FW Update Profile Header Sum Mismatch",                           //33
                    "ER25 - Storage Programming Error",                                       //34
                    "Error sending OSIP Size",                                                //35
                    "Error write out pipe for OSIP header",                                   //36
                    "Error when sending req size cmd",                                        //37
                    "Error when receiving ACK after sending req size cmd",                    //38
                    "Error when sending OS data",                                             //39
                    "Error when getting status from target after OS data sent",               //40
                    "Error when sending complete signal",                                     //41
                    "Error when sending PSFW/SSFW",                                           //42
                    "Error when sending DFN",                                                 //43
                    "Error when reading BATI"                                                 //44

};

MedfieldUtils::MedfieldUtils()
{
    this->RIMGChunkSize = 0;
    this->PSFWChunkID = 0;
    memset((this->szIdrqData),0,sizeof(this->szIdrqData));
    this->StepID = 0;
    this->validstatusclientdata = NULL;
    this->validstatuspfn = NULL;
    this->isDebug = 0x0;
    this->tmpmsg = new char[2048];
    memset((this->usbsn),0,128);
}
MedfieldUtils::~MedfieldUtils()
{
    if(this->tmpmsg)
        delete[] this->tmpmsg;
}

ULONGLONG MedfieldUtils::endian_swap(ULONGLONG x)
{
    if(x <= 0x000000FFFFFFFFULL){
        return (x>>24) |
                ((x<<8) & 0x0000000000FF0000ULL) |
                ((x>>8) & 0x000000000000FF00ULL) |
                ((x<<24)& 0x00000000FF000000ULL);
    }

    else if(x <= 0xFFFFFFFFFFULL)
        return (x>>32) |
                ((x<<16) & 0x00000000FF000000ULL) |
                (  x     & 0x0000000000FF0000ULL) |
                ((x>>16) & 0x000000000000FF00ULL) |
                ((x<<32) & 0x000000FF00000000ULL);
    else if(x <= 0xFFFFFFFFFFFFULL)
        return (x>>40) |
                ((x<<24) & 0x000000FF00000000ULL) |
                ((x<<8)  & 0x00000000FF000000ULL) |
                ((x>>8)  & 0x0000000000FF0000ULL) |
                ((x>>24) & 0x000000000000FF00ULL) |
                ((x<<40) & 0x0000FF0000000000ULL);
    else if(x <= 0xFFFFFFFFFFFFFFULL)
        return (x>>48) |
                ((x<<32) & 0x0000FF0000000000ULL) |
                ((x<<16) & 0x000000FF00000000ULL) |
                (   x    & 0x00000000FF000000ULL) |
                ((x>>16) & 0x0000000000FF0000ULL) |
                ((x>>32) & 0x000000000000FF00ULL) |
                ((x<<48) & 0x00FF000000000000ULL);
    else
        return (x>>56) |
                ((x<<40) & 0x00FF000000000000ULL) |
                ((x<<24) & 0x0000FF0000000000ULL) |
                ((x<<8)  & 0x000000FF00000000ULL) |
                ((x>>8)  & 0x00000000FF000000ULL) |
                ((x>>24) & 0x0000000000FF0000ULL) |
                ((x>>40) & 0x000000000000FF00ULL) |
                (x<<56);
}


int MedfieldUtils::strstr_lowercase_strip(char szBuff[], const char *keyword, int getChunkID,
                                                   bool RIMGSize, bool idrqData)
{
    int result = -1;
    int buff_len = strlen(szBuff);
    int keyword_len = strlen(keyword);
    int match_count = 0;
    int i = 0;
    bool found = false;;
    char chunkBuff[3] = {0};
    unsigned int RIMGBuff;

    for(i=0; i<buff_len; i++){
        if(tolower(szBuff[i]) != tolower(keyword[match_count]))
        {
            match_count = 0;
            result = -1;
        }
        if(tolower(szBuff[i]) == tolower(keyword[match_count]))
        {
            if(match_count == 0)
                result = i;
            match_count ++;
            if(match_count == keyword_len){
                found = true;
                break;
            }
        }
    }
    if(found)
    {
        if(getChunkID == 1)
        {
            memmove(chunkBuff, szBuff + result + keyword_len, 2);
            chunkBuff[2] = '\0';
            PSFWChunkID = atoi(chunkBuff);
            keyword_len += 2;
        }
        if(1 == RIMGSize)
        {
            memmove(&RIMGBuff, szBuff + result + keyword_len + 4, 4);
            u_log(LOG_UTIL, "RIMGBuff: %d", RIMGBuff);
            RIMGChunkSize = (ULONG)RIMGBuff;
            u_log(LOG_UTIL, "%s, %d, %d, RIMGChunkSize: %d",
                  szBuff, result, keyword_len, RIMGChunkSize);
        }
        //get 32 bytes after IDRQ
        if(idrqData == true)
        {
            memmove(&szIdrqData, szBuff + 4, 32);
            keyword_len += 32;
        }
    }
    else
        result = -1;

    return result;
}
ULONGLONG MedfieldUtils::scan_string_for_protocol(char szBuff[])
{
    ULONGLONG keywords_as_ulonglong[MAX_ACK_CODE_MEDFIELD] = {
        SERIAL_START,

                BULK_ACK_DFRM,
                BULK_ACK_DxxM,
                BULK_ACK_DORM,
                BULK_ACK_MFLD,
                BULK_ACK_CLVT,

                BULK_ACK_DXBL,
                BULK_ACK_READY_UPH_SIZE,
                BULK_ACK_READY_UPH,
                BULK_ACK_DMIP,
                BULK_ACK_LOFW,
                BULK_ACK_HIFW,
                BULK_ACK_PSFW1,
                BULK_ACK_PSFW2,
                BULK_ACK_SSFW,
                BULK_ACK_PATCH,
                BULK_ACK_VEDFW,
                BULK_ACK_SSBS,
                BULK_ACK_IFW1,
                BULK_ACK_IFW2,
                BULK_ACK_IFW3,
                BULK_ACK_GPP_RESET,
                //OS
                BULK_ACK_OSIPSZ,
                BULK_ACK_ROSIP,
                BULK_ACK_DONE,
                BULK_ACK_RIMG,
                BULK_ACK_EOIU,
                //Error codes
                BULK_ACK_UPDATE_SUCESSFUL,
                BULK_ACK_INVALID_PING,
                BULK_ACK_HLT0,
                BULK_ACK_ER01,
                BULK_ACK_ER02,
                BULK_ACK_ER03,
                BULK_ACK_ER04,
                BULK_ACK_ER10,
                BULK_ACK_ER11,
                BULK_ACK_ER12,
                BULK_ACK_ER13,
                BULK_ACK_ER15,
                BULK_ACK_ER16,
                BULK_ACK_ER17,
                BULK_ACK_ER18,
                BULK_ACK_ER20,
                BULK_ACK_ER21,
                BULK_ACK_ER22,
                BULK_ACK_ER25,
                BULK_ACK_ERRR,
                BULK_ACK_RTBD
        };

    int index = index_of_keyword(szBuff, StepID);
    if(index != -1){
        return keywords_as_ulonglong[index];
    }
    else
        return 0;
}

int MedfieldUtils::index_of_keyword(char szBuff[], int start_index)
{
    int result = -1;
    int getChunkID = 0;
    const char * keywords[MAX_ACK_CODE_MEDFIELD] = {
        "SoTx",

        "DFRM",
        "DxxM",
        "DORM",
        "MFLD",
        "CLVT",

        "DXBL",
        "RUPHS",
        "RUPH",
        "DMIP",
        "LOFW",
        "HIFW",
        "PSFW1",
        "PSFW2",
        "SSFW",
        "SuCP",
        "VEDFW",
        "SSBS",
        "IFW1",
        "IFW2",
        "IFW3",
        "RESET",
        //OS
        "OSIP Sz",
        "ROSIP",
        "DONE",
        "RIMG",
        "EOIU",
        //Error Codes
        "HLT$",
        "ER00",
        "HLT0",
        "ER01",
        "ER02",
        "ER03",
        "ER04",
        "ER10",
        "ER11",
        "ER12",
        "ER13",
        "ER15",
        "ER16",
        "ER17",
        "ER18",
        "ER20",
        "ER21",
        "ER22",
        "ER25",
        "ERRR",
        "RTBD"

    };

    for(int i=start_index; i< MAX_ACK_CODE_MEDFIELD; i++){
        if((i == 10) || (i == 11) || (i == 12))
            getChunkID = 1;
        else
            getChunkID = 0;
        if(keywords[i])
        {
            if(strstr_lowercase_strip(szBuff, keywords[i], getChunkID, 0)!= -1){
                result = i;
                break;
            }
        }
    }
    return result;

}

void MedfieldUtils::u_setstatuspfn(void(*physstatuspfn)(char* status, void* clientdata), void* clientdata)
{
    validstatuspfn = physstatuspfn;
    validstatusclientdata = clientdata;
}
void MedfieldUtils::u_apistatus(char * message)
{
    if(validstatuspfn != NULL) {
        validstatuspfn(message, validstatusclientdata);
    }
    return;
}
void MedfieldUtils::u_log(uint32 logLevel, string message, ...)
{
    string fmtMsg;
    fmtMsg = (string)message;
    if(logLevel == LOG_PROGRESS) {
        fmtMsg = (string)"XFSTK-PROGRESS--" + (string)"USBSN:" + (string)this->usbsn + (string)"--" + message + (string)"\n";
        fmtMsg_port = (string)"XFSTK-PROGRESS--" + (string)"USBSN:" + (string)this->usbsn + (string)"--" + message + (string)"\n";
    }
    else if(logLevel == LOG_STATUS) {
            fmtMsg = (string)"XFSTK-STATUS--" + (string)"USBSN:" + (string)this->usbsn + (string)"--" + message + (string)"\n";
            fmtMsg_port = (string)"XFSTK-STATUS--" + (string)"USBSN:" + (string)this->usbsn + (string)"--" + message + (string)"\n";
    }
    else {
        fmtMsg = (string)"XFSTK-LOG--" + (string)"USBSN:" + (string)this->usbsn + (string)"--" + message + (string)"\n";
        fmtMsg_port = (string)"XFSTK-LOG--" + (string)"USBSN:" + (string)this->usbsn + (string)"--" + message + (string)"\n";
    }
    if(validstatuspfn != NULL) {
        if (isDebug & logLevel)
        {

            memset(tmpmsg,0,2048);
            va_list ap;
            va_start(ap, message);
            vsprintf(tmpmsg,fmtMsg_port.c_str(), ap);
            //vprintf(fmtMsg_port.c_str(), ap);
            va_end(ap);
            validstatuspfn(tmpmsg,validstatusclientdata);
        }
        else
        {
            printf(".");
            fflush(stdout);
        }

    }
    else {
        if (isDebug & logLevel)
        {
            va_list ap;
            va_start(ap, message);
            vprintf(fmtMsg_port.c_str(), ap);
            va_end(ap);
        }
        else
        {
            printf(".");
            fflush(stdout);
        }
    }
}
void MedfieldUtils::u_error(std::string message, ...)
{
    string fmtMsg = (string)"ERROR: " + message + (string)"\n";
    if (isDebug & DEBUG_ERROR)
    {
        va_list ap;
        va_start(ap, message);
        vprintf(fmtMsg.c_str(), ap);
        va_end(ap);
    }
}

void MedfieldUtils::u_abort(std::string message, ...)
{
    string fmtMsg = (string)"ABORT: " + message + (string)"\n";
    {
        va_list ap;
        va_start(ap, message);
        vprintf(fmtMsg.c_str(), ap);
        va_end(ap);
    }
}

uint32 MedfieldUtils::FileSize(char* filename)
{
    uint32 siz = 0;
    FILE *fp = fopen(filename, "rb");
    if(fp) {
        fseek(fp, 0L, SEEK_END);
        siz = ftell(fp);
        fclose(fp);
        u_log(LOG_UTIL, "%s size:%d bytes", filename, siz);
    }
    return siz;
}

std::string MedfieldUtils::makestring(PUCHAR pbuf, uint32 size)
{
    string tmpStr;
    if(size < 25)
        for(uint32 i=0; i<size; i++)
            tmpStr.insert(i, 1, pbuf[i]);
    else
        tmpStr = "__BINARY__";
    return tmpStr;
}

void MedfieldUtils::SetUsbsn(char* usbsn)
{
    this->u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    if(usbsn && this->usbsn) {
        memset(this->usbsn, 0, 18);
        memcpy(this->usbsn, usbsn, 17);
    }
}
