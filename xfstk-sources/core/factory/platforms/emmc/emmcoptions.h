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
#ifndef EMMCOPTIONS_H
#define EMMCOPTIONS_H

#include <string>
#include "../../interfaces/ioptions.h"

#include <boost/program_options.hpp>


using namespace std;
using namespace boost::program_options;


class EmmcOptions : public IOptions
{
private:
    string miscDnxPath;
    string file;
    string uFwDnx;
    int partition;
    long blockSize;
    long blockCount;
    long offset;
    string tokenOffset;
    string expirationDuration;
    unsigned int debugLevel;
    unsigned int usbdelayms;
    bool umipdump;
    unsigned int targetIndex;
    bool isVerbose;    
    bool isActionRequired;
    bool m_isRegisterToken;

public:
    EmmcOptions ();
    void SetDefaults();
    void Parse(int, char*[]);
    void Clear();
    void PrintAllOptions();
    DeviceTransportType GetTransportType();
    const char* GetMiscDnxPath();
    string GetFile();
    string GetUnsignedFwDnxFile() const { return uFwDnx; }
    int GetPartition();
    long GetBlockSize();
    long GetBlockCount();
    long GetOffset();
    string GetTokenOffset() const { return tokenOffset; }
    string GetExpirationDuration() const { return expirationDuration; }
    unsigned int GetTarget();
    unsigned int GetDebugLevel();
    unsigned int GetUsbdelayms();
    bool GetUmipdumpValue();
    const char* GetQuery();
    bool IsQuery();
    bool IsActionRequired();
    bool IsVerbose();
    bool IsWipeIfwiEnabled();
    bool IsRegisterToken() const { return m_isRegisterToken; }
};
#endif // EMMCOPTIONS_H
