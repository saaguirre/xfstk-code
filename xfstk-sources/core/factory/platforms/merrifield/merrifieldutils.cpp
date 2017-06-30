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
#include <cstring>
#include <string>
#include <fstream>
#include "merrifieldmessages.h"
#include "merrifieldutils.h"
#include "../../common/xfstktypes.h"

using namespace std;

CPSTR Merrifield_error_code_array [44] = {
                        "Success",                                                                //0
                        "Connection error",                                                       //1
                        "Aborted by User",                                                    	  //2
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
                        "ER10 - DX_PUB_KEY_HASH_VALIDATION_FAILURE",                              //22
                        "ER11 - DX_INVALID_PRIMARY_VRL_MAGIC_NUM_VAL",                            //23
                        "ER12 - DX_VRL_PRIMARY_INTEGRITY_FAILURE",                                //24
                        "ER13 - DX_RECORD_INTEGRITY_FAILURE",                                     //25
                        "ER20 - Signature Error",                                                 //26
                        "ER21 - Checksum Mismatch ",                                              //27
                        "ER22 - FW Update Profile Header Sum Mismatch",                           //28
                        "ER25 - Programming  /n ER25 - Storage Programming Error /n ER25 - IFWI checksum mismatch FUPH.IFWIChxxSum", //29
                        "Error sending OSIP Size",                                                //30
                        "Error write out pipe for OSIP header",                                   //31
                        "Error when sending req size cmd",                                        //32
                        "Error when receiving ACK after sending req size cmd",                    //33
                        "Error when sending OS data",                                             //34
                        "Error when getting status from target after OS data sent",               //35
                        "Error when sending complete signal",                                     //36
                        "Error when sending PSFW/SSFW",                                           //37
                        "Error when sending DFN",                                                 //38
                        "Error when reading BATI",                                                //39
                        "ER04 - SecurityInit",                                                    //40
                        "ER06 - Key Index 0",                                                     //41
                        "ER07 - Key Index 0",                                                     //42
                        "ER26 - Storage Programming"                                              //43

};

MerrifieldUtils::MerrifieldUtils()
{
    this->RIMGChunkSize = 0;
    this->PSFWChunkID = 0;
    memset((this->szIdrqData),0,sizeof(this->szIdrqData));
    this->StepID = 0;
    this->validstatusclientdata = NULL;
    this->validstatuspfn = NULL;
    this->tmpmsg = new char[TMP_BUFFER_SIZE];
    this->isDebug = 0x0;
}
MerrifieldUtils::~MerrifieldUtils()
{
    if(this->tmpmsg)
        delete[] this->tmpmsg;
}

ULONGLONG MerrifieldUtils::endian_swap(ULONGLONG x)
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


int MerrifieldUtils::strstr_lowercase_strip(char szBuff[], const char *keyword, int getChunkID,
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
ULONGLONG MerrifieldUtils::scan_string_for_protocol(char szBuff[])
{
    ULONGLONG keywords_as_ulonglong[MAX_ACK_CODE_MERRIFIIELD] = {
        SERIAL_START,             //0

        BULK_ACK_DFRM,            //1
        BULK_ACK_DxxM,            //2
        BULK_ACK_DORM,            //3
        BULK_ACK_MFLD,            //4
        BULK_ACK_DXBL,            //5
        BULK_ACK_READY_UPH_SIZE,  //6
        BULK_ACK_READY_UPH,       //7
        BULK_ACK_DIFWI,           //8
        BULK_ACK_DCFI00,          //9

        //emmc dump
        BULK_ACK_RDY$,            //10
        EMMC_DUMP_ACK,            //11
        EMMC_DUMP_NACK,           //12
        EMMC_DUMP_EOIO,           //13
        BULK_ACK_ER40,            //14

        //OS
        BULK_ACK_OSIPSZ,          //15
        BULK_ACK_ROSIP,           //16
        BULK_ACK_DONE,            //17
        BULK_ACK_RIMG,            //18
        BULK_ACK_EOIU,            //19

        //Error codes
        BULK_ACK_UPDATE_SUCESSFUL,//20
        BULK_ACK_INVALID_PING,    //21
        BULK_ACK_HLT0,            //22
        BULK_ACK_ER01,            //23
        BULK_ACK_ER02,            //24
        BULK_ACK_ER03,            //25
        BULK_ACK_ER04,            //26
        BULK_ACK_ER10,            //27
        BULK_ACK_ER11,            //28
        BULK_ACK_ER12,            //29
        BULK_ACK_ER13,            //30
        BULK_ACK_ER20,            //31
        BULK_ACK_ER21,            //32
        BULK_ACK_ER22,            //33
        BULK_ACK_ER25,            //34
        BULK_ACK_ERRR,            //35
        BULK_ACK_ERB0,            //36
        BULK_ACK_ERB1,            //37
        BULK_ACK_RTBD,            //38
        BULK_ACK_DCSDB,           //39
        BULK_ACK_UCSDB            //40
    };

    int index = index_of_keyword(szBuff, StepID);
    if(index != -1){
        return keywords_as_ulonglong[index];
    }
    else
        return 0;
}

int MerrifieldUtils::index_of_keyword(char szBuff[], int start_index)
{
    int result = -1;
    int getChunkID = 0;
    const char * keywords[MAX_ACK_CODE_MERRIFIIELD] = {
        "SoTx",      //0

        "DFRM",      //1
        "DxxM",      //2
        "DORM",      //3
        "MFLD",      //4
        "DXBL",      //5
        "RUPHS",     //6
        "RUPH",      //7
        "DIFWI",     //8
        "DCFI00",    //9

        //emmc dump
        "RDY$",      //10
        "$ACK",      //11
        "NACK",      //12
        "EOIO",      //13
        "ER40",      //14

        //OS
        "OSIP Sz",   //15
        "ROSIP",     //16
        "DONE",      //17
        "RIMG",      //18
        "EOIU",      //19

        //Error Codes
        "HLT$",      //20
        "ER00",      //21
        "HLT0",      //22
        "ER01",      //23
        "ER02",      //24
        "ER03",      //25
        "ER04",      //26
        "ER10",      //27
        "ER11",      //28
        "ER12",      //29
        "ER13",      //30
        "ER20",      //31
        "ER21",      //32
        "ER22",      //33
        "ER25",      //34
        "ERRR",      //35
        "ERB0",      //36
        "ERB1",      //37
        "RTBD",      //38
        "DCSDB",     //39
        "UCSDB"      //40
    };

    for(int i=start_index; i< MAX_ACK_CODE_MERRIFIIELD; i++){
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

void MerrifieldUtils::u_setstatuspfn(void(*physstatuspfn)(char* status, void* clientdata), void* clientdata)
{
    validstatuspfn = physstatuspfn;
    validstatusclientdata = clientdata;
}
void MerrifieldUtils::u_apistatus(char * message)
{
    if(validstatuspfn != NULL) {
        validstatuspfn(message, validstatusclientdata);
    }
    return;
}
void MerrifieldUtils::u_log(uint32 logLevel, string message, ...)
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

            memset(tmpmsg,0,TMP_BUFFER_SIZE );
            va_list ap;
            va_start(ap, message);
            vsprintf(tmpmsg,fmtMsg_port.c_str(), ap);
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
void MerrifieldUtils::u_error(std::string message, ...)
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

void MerrifieldUtils::u_abort(std::string , ...)
{

}

uint32 MerrifieldUtils::FileSize(char* filename)
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

unsigned int MerrifieldUtils::dwordCheckSum(unsigned char *buff, int size)
{

    unsigned int Xor = 0;
    unsigned int pos = 0;
    unsigned int buffer = 0;
    if(size%4 == 0)
    {

        for(int i = 0; i<size/4; i++)
        {
            memcpy(&buffer,(buff+pos),4);
            Xor ^= buffer;
            pos += 4;
        }
    }
    return Xor;
}

size_t MerrifieldUtils::StringLocation(const char *file, string str, bool front)
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
        std::string* tmpstr = new std::string();
        tmpstr->reserve(size);


        if(tmpBuff)
        {
            inFile.read(tmpBuff,size);
            tmpstr->assign(tmpBuff,size);
            retval = front  ? tmpstr->find(str) : tmpstr->rfind(str);
            delete [] tmpBuff;
        }
        if(tmpstr)
            delete tmpstr;
    }

    inFile.close();

    return retval;

}



size_t MerrifieldUtils::StringLocation(const char *buffer, string str, size_t sizeofBuffer)
{
    size_t retval = std::string::npos;

    std::string* tmpstr = new std::string();
    if(tmpstr)
    {
        tmpstr->reserve(sizeofBuffer);

        tmpstr->assign(buffer,sizeofBuffer);
        retval = tmpstr->find(str);
        delete tmpstr;
    }

    return retval;

}

std::string MerrifieldUtils::makestring(PUCHAR pbuf, uint32 size)
{
    string tmpStr;
    if(size < 25)
        for(uint32 i=0; i<size; i++)
            tmpStr.insert(i, 1, pbuf[i]);
    else
        tmpStr = "__BINARY__";
    return tmpStr;
}
void MerrifieldUtils::SetUsbsn(char* usbsn)
{
    memcpy(this->usbsn, usbsn, 17);
}
