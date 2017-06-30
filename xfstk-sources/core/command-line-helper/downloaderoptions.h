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
#ifndef DOWNLOADEROPTIONS_H
#define DOWNLOADEROPTIONS_H

#include <string>
#include "ioption.h"

using namespace std;

class DownloaderOptions : public IOption
{
private:
    string fwDnxPath;
    string fwImagePath;
    string osDnxPath;
    string osImagePath;
    string miscDnxPath;
    unsigned int gpFlags;
    unsigned int targetIndex;
    string transferType;
    bool idrqEnabled;
    bool isVerbose;
    bool downloadFW;
    bool downloadOS;
    bool isActionRequired;
    void UpdateFlags();
    bool allPathsAreValid();
    void ParseLegacy(int, char*[]);
public:
    DownloaderOptions();
    void Parse(int, char*[]);
    void Clear();
    void PrintAllOptions();
    const char* GetFWDnxPath();
    const char* GetFWImagePath();
    const char* GetOSDnxPath();
    const char* GetOSImagePath();
    const char* GetMiscDnxPath();
    unsigned int GetTarget();
    unsigned int GetGPFlags();
    const char* GetTransferType();
    const char* GetQuery();
    bool IsIdrqEnabled();
    bool IsFWDownload();
    bool IsOSDownload();
    bool IsQuery();
    bool IsActionRequired();
    bool IsVerbose();
};

#endif // DOWNLOADEROPTIONS_H
