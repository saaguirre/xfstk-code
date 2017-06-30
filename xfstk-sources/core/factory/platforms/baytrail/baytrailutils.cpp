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
#include "../../common/xfstktypes.h"
#include <cstring>
#include <string>
#include <time.h>
#include "baytrailmessages.h"
#include "baytrailutils.h"


using namespace std;

CPSTR Baytrail_error_code_array [BAYTRAIL_MAX_ERROR_CODE] =
{
    "Download completed successfully",         //0
    "Download encountered an erase failure",   //1
    "Download encountered a program failure",  //2
    "Download encountered a verify failure",   //3
    "Download encountered a read failure",     //4
    "Download encountered a blank failure",    //5
    "Download encountered a batch failure",    //6
    "Download encountered a checksum error",   //7
    "Failed to identify flash chip",           //8
    "Download completed with a failure",       //9
    "Could not create downloader process",     //10
    "Could not set Downloader options",        //11
    "Could not set Downloader device",         //12
    "Transport type not supported",            //13
    "File could not be opened",                //14
    "Could not get file information"           //15
    "Download timed out"                       //16
};

BaytrailUtils::BaytrailUtils()
{
    this->RIMGChunkSize = 0;
    this->PSFWChunkID = 0;
    memset((this->szIdrqData),0,sizeof(this->szIdrqData));
    this->StepID = 0;
    this->validstatusclientdata = NULL;
    this->validstatuspfn = NULL;
    this->isDebug = 0;
    //this->isDebug = 0;
    memset((this->usbsn),0,128);
}

ULONGLONG BaytrailUtils::endian_swap(ULONGLONG x)
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


int BaytrailUtils::strstr_lowercase_strip(char szBuff[], const char *keyword, int getChunkID,
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
ULONGLONG BaytrailUtils::scan_string_for_protocol(char szBuff[])
{
    ULONGLONG keywords_as_ulonglong[MAX_ACK_CODE] = {
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

int BaytrailUtils::index_of_keyword(char szBuff[], int start_index)
{
    int result = -1;
    int getChunkID = 0;
    const char * keywords[MAX_ACK_CODE] = {
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

    for(int i=start_index; i< MAX_ACK_CODE; i++){
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

void BaytrailUtils::u_setstatuspfn(void(*physstatuspfn)(char* status, void* clientdata), void* clientdata)
{
    validstatuspfn = physstatuspfn;
    validstatusclientdata = clientdata;
}
void BaytrailUtils::u_apistatus(char * message)
{
    if(validstatuspfn != NULL) {
        validstatuspfn(message, validstatusclientdata);
    }
    return;
}
void BaytrailUtils::u_log(uint32 logLevel, string message, ...)
{
    string fmtMsg;
    fmtMsg = (string)message;
    if(logLevel == LOG_PROGRESS) {
        fmtMsg = (string)"XFSTK-PROGRESS--" + message + (string)"\n";
        fmtMsg_port = (string)"XFSTK-PROGRESS--" + message + (string)"\n";
    }
    else if(logLevel == LOG_STATUS) {
            fmtMsg = (string)"XFSTK-STATUS--" + message + (string)"\n";
            fmtMsg_port = (string)"XFSTK-STATUS--" + message + (string)"\n";
    }
    else {
        fmtMsg = (string)"XFSTK-LOG--" + message + (string)"\n";
        fmtMsg_port = (string)"XFSTK-LOG--" + message + (string)"\n";
    }
    if(validstatuspfn != NULL) {
        if (isDebug & logLevel)
        {

            memset(tmpmsg,0,sizeof(tmpmsg));
            va_list ap;
            va_start(ap, message);
            vsprintf(&tmpmsg[0],fmtMsg_port.c_str(), ap);
            va_end(ap);
            validstatuspfn(&tmpmsg[0],validstatusclientdata);
        }
        else
        {
            printf(".");
            fflush(stdout);
        }

    }
    else
    {
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
void BaytrailUtils::u_error(std::string message, ...)
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

void BaytrailUtils::u_abort(std::string message, ...)
{
    string fmtMsg = (string)"ABORT: " + message + (string)"\n";
    {
        va_list ap;
        va_start(ap, message);
        vprintf(fmtMsg.c_str(), ap);
        va_end(ap);
    }
}

uint32 BaytrailUtils::FileSize(char* filename)
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

std::string BaytrailUtils::makestring(PUCHAR pbuf, uint32 size)
{
    string tmpStr;
    if(size < 25)
        for(uint32 i=0; i<size; i++)
            tmpStr.insert(i, 1, pbuf[i]);
    else
        tmpStr = "__BINARY__";
    return tmpStr;
}

void BaytrailUtils::SetUsbsn(char* usbsn)
{
    this->u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    if(usbsn && this->usbsn) {
        memset(this->usbsn, 0, 18);
        memcpy(this->usbsn, usbsn, 17);
    }
}
