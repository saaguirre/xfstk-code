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
#ifndef HASHVERIFYOPTIONS_H
#define HASHVERIFYOPTIONS_H

#include <string>
#include "../../interfaces/ioptions.h"
#include <boost/program_options.hpp>



using namespace std;
using namespace boost::program_options;


class HashVerifyOptions : public IOptions
{
public:
    HashVerifyOptions();
    void SetDefaults();
    void Parse(int, char*[]);
    void Clear();
    void PrintAllOptions();
    bool IsActionRequired();
    bool IsWrite();
    bool validHashFile();
    DeviceTransportType GetTransportType();
    bool IsVerbose();
    unsigned int GetDebugLevel();
    const char *GetHashFile();

private:
    string hashfile;
    string transferType;
    bool isVerbose ;
    bool isActionRequired;
    bool isWrite;
    unsigned int debugLevel;
};

#endif // HASHVERIFYOPTIONS_H
