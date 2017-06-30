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
#ifndef CLOVERVIEWPLUSOPTIONS_H
#define CLOVERVIEWPLUSOPTIONS_H

#include <string>
#include "../../interfaces/ioptions.h"
#include <boost/program_options.hpp>



using namespace std;
using namespace boost::program_options;


class CloverviewPlusOptions : public IOptions
{
private:
    string fwDnxPath;
    string fwImagePath;
    string SoftfusesPath;
    string osDnxPath;
    string osImagePath;
    string miscDnxPath;
    unsigned int gpFlags;
    unsigned int debugLevel;
    unsigned int usbdelayms;
    unsigned int targetIndex;
    string transferType;
    bool idrqEnabled;
    bool isVerbose;
    bool wipeifwi;
    bool downloadFW;
    bool downloadOS;
    bool isActionRequired;
    void UpdateFlags();
    bool allPathsAreValid();
    void ParseLegacy(int, char*[]);
    //emmc
    string file;
    string uFwDnx;
    int partition;
    long blockSize;
    long blockCount;
    long offset;
    string tokenOffset;
    string expirationDuration;
    bool umipdump;
    bool m_isRegisterToken;
    bool performEmmcDump;
public:
    CloverviewPlusOptions();
    void SetDefaults();
    void Parse(int, char*[]);
    void Clear();
    void PrintAllOptions();
    DeviceTransportType GetTransportType();
    const char* GetFWDnxPath();
    const char* GetFWImagePath();
    const char* GetSoftfusesPath();
    const char* GetOSDnxPath();
    const char* GetOSImagePath();
    const char* GetMiscDnxPath();
    unsigned int GetTarget();
    unsigned int GetGPFlags();
    unsigned int GetDebugLevel();
    unsigned int GetUsbdelayms();
    const char* GetTransferType();
    const char* GetQuery();
    bool IsIdrqEnabled();
    bool IsFWDownload();
    bool IsOSDownload();
    bool IsQuery();
    bool IsActionRequired();
    bool IsVerbose();
    bool IsWipeIfwiEnabled();
    //emmc
    string GetEmmcFile(){return file;}
    string GetEmmcUnsignedFwDNX(){return uFwDnx;}
    string GetEmmcTokenOffset(){return tokenOffset;}
    string GetEmmcExpirationDur() const { return expirationDuration; }
    int GetEmmcPartition(){return partition;}
    long GetEmmcBlockSize(){return blockSize;}
    long GetEmmcBlockCount(){return blockCount;}
    long GetEmmcOffset(){return offset;}
    bool IsEmmcUmipDumpEnabled(){return umipdump;}
    bool IsPerformEmmcDumpEnabled(){return performEmmcDump;}
    bool IsEmmcRegisterTokenEnabled(){return m_isRegisterToken;}
};
#endif // CLOVERVIEWPLUSOPTIONS_H
