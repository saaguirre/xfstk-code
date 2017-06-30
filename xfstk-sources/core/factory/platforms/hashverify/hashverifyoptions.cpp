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
#include "hashverifyoptions.h"
#include <iostream>
#include <fstream>

using namespace std;
namespace po = boost::program_options;

HashVerifyOptions::HashVerifyOptions()
{
    this->Clear();
}

void HashVerifyOptions::SetDefaults()
{
    return;
}

void HashVerifyOptions::Parse(int argc, char* argv[])
{
    string debuglevel = "";

    variables_map vm;
    options_description primaryOptions("Command Line Options");
    primaryOptions.add_options()
    // First parameter describes option's long and short name
    // The second is parameter the options type
    // The third is description
    ("help,h", "Print usage message")
    ("hashfile", po::value<string>()->default_value("BLANK.bin"), "Required argument. File Path for hash file.")
    ("write,w",  po::bool_switch()->default_value(0), "Optional argument. Writes hash to file specified in --hashfile option.")
    ;
    options_description visibleOptions;
    visibleOptions.add(primaryOptions);

    primaryOptions.add_options()
    ("debuglevel", po::value<string>()->default_value("0xffffffff"), "Optional argument. 32 Bit Hex Value of the debuglevel, 0x1800 = LOG_STATUS | LOG_PROGRESS")
    ("transfer",   po::value<string>()->default_value("USB"), "Optional argument. Determines how the image will be transferred.")
    ;

    options_description cmdlineOptions;
    cmdlineOptions.add(primaryOptions);

    try
    {
        store(parse_command_line(argc, argv, cmdlineOptions), vm);

        string tmp = vm["hashfile"].as<string>();
        if (argc == 1 || vm.count("help") ||(tmp ==string("BLANK.bin")) ||
                !collect_unrecognized(parse_command_line(argc, argv, cmdlineOptions).options, include_positional).empty()) {
            cout << visibleOptions << endl;
            this->isActionRequired = false;
            return;
        }

        if(vm.count("hashfile"))
        {
            this->hashfile = vm["hashfile"].as<string>();
            if(!validHashFile())
                throw std::runtime_error("Invalid Hash file");
        }

        if(vm.count("write"))
        {
            this->isWrite = vm["write"].as<bool>();
        }
        if(vm.count("debuglevel"))
        {
            debuglevel = vm["debuglevel"].as<string>();
            sscanf(debuglevel.c_str(), "%x", &this->debugLevel);
        }
        if(vm.count("transfer"))
        {
            this->transferType = vm["transfer"].as<string>();
        }

    this->isActionRequired = true;

    }
    catch(exception& e)
    {
        cout << "\nCould not process the supplied options!" << endl;
        cout << "Details: " << e.what() << endl;
        cout << "Help Menu: " << endl;
        cout << visibleOptions << endl;
        this->isActionRequired = false;
    }


}

void HashVerifyOptions::Clear()
{
    this->hashfile = "";
    this->debugLevel = 0xFFFFFFFF;
}

void HashVerifyOptions::PrintAllOptions()
{
    printf("\nHash File: %s\n",this->hashfile.c_str());
}

bool HashVerifyOptions::IsActionRequired()
{
    return this->isActionRequired;
}

DeviceTransportType HashVerifyOptions::GetTransportType()
{
    DeviceTransportType Transport;
    if(this->transferType == "USB") {
        Transport = XFSTK_USB20;
    }
    else if(this->transferType == "SERIAL") {
        Transport = XFSTK_SERIAL;
    }
    else {
        Transport = XFSTK_NOT_SPECIFIED;
    }
    return Transport;
}

bool HashVerifyOptions::IsVerbose()
{
    return this->isVerbose;
}

unsigned int HashVerifyOptions::GetDebugLevel()
{
    return this->debugLevel;
}

bool HashVerifyOptions::IsWrite()
{
    return this->isWrite;
}

const char* HashVerifyOptions::GetHashFile()
{
    return this->hashfile.c_str();
}

bool HashVerifyOptions::validHashFile()
{
    bool retval = true;
    FILE *fp;
    if(this->hashfile.length() > 0)
    {
        fp = fopen(this->hashfile.c_str(), "rb");
        if(fp)
        {
            fclose(fp);
        }
        else
        {
            ofstream outputFile;
            outputFile.open(this->hashfile.c_str());
            if(!outputFile.is_open())
            {
                retval = false;
                cout << "\nCould not open or create the file " << this->hashfile << endl;
            }
            outputFile.close();
        }
    }
    else
        retval = false;
    return retval;
}
