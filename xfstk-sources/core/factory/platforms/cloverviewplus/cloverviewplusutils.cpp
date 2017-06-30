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
#include "../../common/xfstktypes.h"
#include <cstring>
#include <string>
#include <time.h>
#include "cloverviewplusmessages.h"
#include "cloverviewplusutils.h"


using namespace std;


CPSTR CloverviewPlus_error_code_array [MAX_ERROR_CODE_CLOVERVIEWPLUS] = {
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
                    "HLT0 - DnX FW or IFWI Size = 0 / SF Data size or Num of Keys = 0",       //18
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
                    "Error when reading BATI",                                                //44
                    "ER30 - Softfuses Signature Error",                                       //45
                    "ER32 - Softfuses Sum Mismatch",                                          //46
                    "ER14 - Bulk Unknown Hash Verify Failure"                                 //47
};

CloverviewPlusUtils::CloverviewPlusUtils()
{
    this->RIMGChunkSize = 0;
    this->PSFWChunkID = 0;
    memset((this->szIdrqData),0,sizeof(this->szIdrqData));
    this->StepID = 0;
    this->validstatusclientdata = NULL;
    this->validstatuspfn = NULL;
    this->isDebug = 0x0;
    this->tmpmsg = new char[TMP_BUFFER_SIZE];
    memset((this->usbsn),0,128);
}
CloverviewPlusUtils::~CloverviewPlusUtils()
{
    if(this->tmpmsg)
        delete [] this->tmpmsg;
}

ULONGLONG CloverviewPlusUtils::endian_swap(ULONGLONG x)
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


int CloverviewPlusUtils::strstr_lowercase_strip(char szBuff[], const char *keyword, int getChunkID,
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
ULONGLONG CloverviewPlusUtils::scan_string_for_protocol(char szBuff[])
{
    ULONGLONG keywords_as_ulonglong[MAX_ACK_CODE_CLOVERVIEWPLUS] = {
        SERIAL_START,                         //0

                BULK_ACK_DFRM,                //1
                BULK_ACK_DxxM,                //2
                BULK_ACK_DORM,                //3
                BULK_ACK_MFLD,                //4
                BULK_ACK_CLVT,                //5

                BULK_ACK_DXBL,                //6
                BULK_ACK_READY_UPH_SIZE,      //7
                BULK_ACK_READY_UPH,           //8
                BULK_ACK_DMIP,                //9
                BULK_ACK_LOFW,                //10
                BULK_ACK_HIFW,                //11
                BULK_ACK_PSFW1,               //12
                BULK_ACK_PSFW2,               //13
                BULK_ACK_SSFW,                //14
                BULK_ACK_PATCH,               //15
                BULK_ACK_VEDFW,               //16
                BULK_ACK_SSBS,                //17
                BULK_ACK_IFW1,                //18
                BULK_ACK_IFW2,                //19
                BULK_ACK_IFW3,                //20
                BULK_ACK_GPP_RESET,           //21

                //emmc dump
                BULK_ACK_RDY$,                //22
                EMMC_DUMP_ACK,                //23
                EMMC_DUMP_NACK,               //24
                EMMC_DUMP_EOIO,               //25
                BULK_ACK_ER40,                //26

                //OS
                BULK_ACK_OSIPSZ,              //27
                BULK_ACK_ROSIP,               //28
                BULK_ACK_DONE,                //29
                BULK_ACK_RIMG,                //30
                BULK_ACK_EOIU,                //31

                //Error codes
                BULK_ACK_UPDATE_SUCESSFUL,    //32
                BULK_ACK_INVALID_PING,        //33
                BULK_ACK_HLT0,                //34
                BULK_ACK_ER01,                //35
                BULK_ACK_ER02,                //36
                BULK_ACK_ER03,                //37
                BULK_ACK_ER04,                //38
                BULK_ACK_ER10,                //39
                BULK_ACK_ER11,                //40
                BULK_ACK_ER12,                //41
                BULK_ACK_ER13,                //42
                BULK_ACK_ER15,                //43
                BULK_ACK_ER16,                //44
                BULK_ACK_ER17,                //45
                BULK_ACK_ER18,                //46
                BULK_ACK_ER20,                //47
                BULK_ACK_ER21,                //48
                BULK_ACK_ER22,                //49
                BULK_ACK_ER25,                //50
                BULK_ACK_ERRR,                //51
                BULK_ACK_RTBD,                //52
                BULK_ACK_ER30,                //53
                BULK_ACK_ER32,                //54
                BULK_ACK_ER14,                //55
                BULK_ACK_READY_SFUSE_UPH_SIZE,//56
                BULK_ACK_READY_SFUSE_UPH,     //57
                BULK_ACK_DSKF                 //58
        };

    int index = index_of_keyword(szBuff, StepID);
    if(index != -1){
        return keywords_as_ulonglong[index];
    }
    else
        return 0;
}

int CloverviewPlusUtils::index_of_keyword(char szBuff[], int start_index)
{
    int result = -1;
    int getChunkID = 0;
    const char * keywords[MAX_ACK_CODE_CLOVERVIEWPLUS] = {
        "SoTx",     //0

        "DFRM",     //1
        "DxxM",     //2
        "DORM",     //3
        "MFLD",     //4
        "CLVT",     //5

        "DXBL",     //6
        "RUPHS",    //7
        "RUPH",     //8
        "DMIP",     //9
        "LOFW",     //10
        "HIFW",     //11
        "PSFW1",    //12
        "PSFW2",    //13
        "SSFW",     //14
        "SuCP",     //15
        "VEDFW",    //16
        "SSBS",     //17
        "IFW1",     //18
        "IFW2",     //19
        "IFW3",     //20
        "RESET",    //21

        //emmc dump
        "RDY$",     //22
        "$ACK",     //23
        "NACK",     //24
        "EOIO",     //25
        "ER40",     //26

        //OS
        "OSIP Sz",  //27
        "ROSIP",    //28
        "DONE",     //29
        "RIMG",     //30
        "EOIU",     //31

        //Error Codes
        "HLT$",     //32
        "ER00",     //33
        "HLT0",     //34
        "ER01",     //35
        "ER02",     //36
        "ER03",     //37
        "ER04",     //38
        "ER10",     //39
        "ER11",     //40
        "ER12",     //41
        "ER13",     //42
        "ER15",     //43
        "ER16",     //44
        "ER17",     //45
        "ER18",     //46
        "ER20",     //47
        "ER21",     //48
        "ER22",     //49
        "ER25",     //50
        "ERRR",     //51
        "RTBD",     //52
        "ER30",     //53
        "ER32",     //54
        "ER14",     //55
        "RSUPHS",   //56
        "RSUPH",    //57
        "DSKF"      //58
    };

    for(int i=start_index; i< MAX_ACK_CODE_CLOVERVIEWPLUS; i++){
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

void CloverviewPlusUtils::u_setstatuspfn(void(*physstatuspfn)(char* status, void* clientdata), void* clientdata)
{
    validstatuspfn = physstatuspfn;
    validstatusclientdata = clientdata;
}
void CloverviewPlusUtils::u_apistatus(char * message)
{
    if(validstatuspfn != NULL) {
        validstatuspfn(message, validstatusclientdata);
    }
    return;
}
void CloverviewPlusUtils::u_log(uint32 logLevel, string message, ...)
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

            memset(tmpmsg,0,TMP_BUFFER_SIZE );
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
void CloverviewPlusUtils::u_error(std::string message, ...)
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

void CloverviewPlusUtils::u_abort(std::string message, ...)
{
    string fmtMsg = (string)"ABORT: " + message + (string)"\n";
    {
        va_list ap;
        va_start(ap, message);
        vprintf(fmtMsg.c_str(), ap);
        va_end(ap);
    }
}

uint32 CloverviewPlusUtils::FileSize(char* filename)
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

std::string CloverviewPlusUtils::makestring(PUCHAR pbuf, uint32 size)
{
    string tmpStr;
    if(size < 25)
        for(uint32 i=0; i<size; i++)
            tmpStr.insert(i, 1, pbuf[i]);
    else
        tmpStr = "__BINARY__";
    return tmpStr;
}
void CloverviewPlusUtils::SetUsbsn(char* usbsn)
{
    this->u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    if(usbsn && this->usbsn) {
        memset(this->usbsn, 0, 18);
        memcpy(this->usbsn, usbsn, 17);
    }
}

size_t CloverviewPlusUtils::StringLocation(const char *file, string str, bool front)
{
    size_t retval = std::string::npos;
    size_t size;

    std::fstream inFile;
    inFile.open(file,std::ios_base::binary | std::ios_base::in);


    if(inFile.is_open())
    {
        inFile.seekg(0,inFile.end);
        size = inFile.tellg();
        inFile.seekg(0,inFile.beg);


        char* tmpBuff = new char[size];
        std::string*tmpstr = new std::string();
        tmpstr->reserve(size);


        if(tmpBuff)
        {
            inFile.read(tmpBuff,size);
            tmpstr->assign(tmpBuff,size);
            retval = front  ? tmpstr->find(str) : tmpstr->rfind(str);
            delete [] tmpBuff;
        }
       // tmpstr->clear();
        if(tmpstr)
            delete tmpstr;
    }
    inFile.close();

    return retval;

}
