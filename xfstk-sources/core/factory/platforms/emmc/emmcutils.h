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
#ifndef EMMCUTILS_H
#define EMMCUTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <cstdarg>
#include <string>


#define MAX_BUFFER_SIZE                 16384 //16K

#if defined XFSTK_OS_WIN_MSVC
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif

#define LOG_ACK         0x0001
#define LOG_UPDATE      0x0002
#define LOG_OPCODE      0x0004
#define LOG_FWUPGRADE   0x0008
#define LOG_OS          0x0010
#define LOG_USB         0x0020
#define LOG_SOCKET      0x0040
#define LOG_SERIAL      0x0080
#define LOG_UTIL        0x0100
#define LOG_DOWNLOADER  0x0200
#define LOG_ENTRY       0x0400
#define LOG_STATUS      0x0800
#define LOG_PROGRESS    0x1000
#define DEBUG_ERROR     0xfffffff1
#define LOG_ALL         0xffffffff

class EmmcUtils
{
public:
    int StepID;
    int PSFWChunkID;
    unsigned long RIMGChunkSize;
    char szIdrqData[32];
    unsigned int isDebug;
    bool is_second_ROSIP;
    bool is_second_RIMG;
    std::string fmtMsg_port;
    void(*validstatuspfn)(char* status, void* clientdata);
    void* validstatusclientdata;
    char* tmpmsg;
    void SetUsbsn(char* usbsn);

    EmmcUtils();
    ~EmmcUtils();
    void u_log(unsigned int logLevel, std::string message, ...);
    void u_error(std::string message, ...);
    void u_abort(std::string message, ...);
    unsigned long long int endian_swap(unsigned long long int x);
    unsigned long long int scan_string_for_protocol(char szBuff[]);
    int index_of_keyword(char szBuff[], int start_index);
    int strstr_lowercase_strip(char szBuff[], const char *keyword, int keyword_index, bool RIMGSize, bool idrqData=false);
    unsigned int FileSize(char* filename);
    std::string makestring(unsigned char* pbuf, unsigned int size);
    void u_setstatuspfn(void(*physstatuspfn)(char* status, void* clientdata), void* clientdata);
    void u_apistatus(char * message);
    char usbsn[128];

private:

	EmmcUtils( const EmmcUtils& );
	EmmcUtils& operator=( const EmmcUtils& );
};

#endif // EMMCUTILS_H
