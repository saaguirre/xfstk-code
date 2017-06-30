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
#include "emmcoptions.h"
#include <cstdio>
#include <iostream>

namespace po = boost::program_options;

void EmmcOptions::SetDefaults()
{
    return;
}
/** \brief EmmcOptions represents the values used to control the
  * behavior of the downloader.
  *
  * The two ways to popluate EmmcOptions is through a set of
  * key value pairs (such as from a command line) or by setting each
  * individual option.
  */
EmmcOptions::EmmcOptions()
{
    this->Clear();
}

/** \brief Processes a list of command line arguments.
  * @param argc The number of tokens to parse.
  * @param argv The tokens to parse.
  */
void EmmcOptions::Parse(int argc, char* argv[])
{
    string gpflagsHexFormat = "";
    string targetHexFormat = "";
    string debuglevel = "";
    string usbdelayms = "";

    variables_map vm;
    // First parameter describes option's long and short name
    // The second is parameter the options type
    // The third is description
    options_description emmcOptions("eMMC Dump Options");
    emmcOptions.add_options()
    ("help,h", "Print usage message")
    ("dnx",                  po::value<string>(), "File path for signed eMMC DnX")
    ("file,f",               po::value<string>(), "Output file name")
    ("partition,p",          po::value<int>(), "0 - user partition; 1- boot partition-1; 2 - boot partition-2;3 - E-CSD")
    ("blocksize,b",          po::value<long>(), "size of blocks to read (applicable for partitions 0-2)")
    ("blockcount,c",         po::value<long>(), "number of blocks to read (applicable for partitions 0-2)")
    ("offset,o",             po::value<long>(), "offset from base of partition to begin reading (applicable for partitions 0-2)")
    ("debuglevel",           po::value<string>()->default_value("0xffffffff"), "Optional argument. 32 Bit Hex Value of the debuglevel, 0x1800 = LOG_STATUS | LOG_PROGRESS")
    ("usbdelayms",           po::value<string>()->default_value("0"), "Optional argument. 32 Bit int Value of the usbdelayms, default 0ms")
    ("verbose,v",            po::bool_switch()->default_value(0), "Optional argument. Display debug information.")
    ("register,r",           po::bool_switch()->default_value(0), "Register Token.")
    ("unsigneddnx,u",        po::value<string>(), "Unsigned DnX.")
    ("tokenoffset,t",        po::value<string>(), "Token offset for the unsigned DnX. (Default offset: 0x0108)")
    ("expirationduration,e", po::value<string>(), "Time duration of the token to be valid. Supported format shall be a string which starts with a numeric number followed by h/d/m/y (h for hour, d for day, m for month and y for year)")
    ("umipdump,u",           po::bool_switch()->default_value(0), "UMIP dumping. Default value 0 (disable). Set to 1 to enable.")

    ;

    options_description cmdlineOptions;
    cmdlineOptions.add(emmcOptions);
#if defined UNITTEST
    options_description testingOptions("xFSTK Downloader Options Testing Commands");
    testingOptions.add_options()
    ("unittest", "Processes the options found but does not set the action required flag")
    ("target",  po::value<unsigned int>()->default_value(0), "Optional argument. The target index to use when multiple targets are connected.")
    ;
    cmdlineOptions.add(testingOptions);
#endif

    options_description visibleOptions;
    visibleOptions.add(emmcOptions);

    try
    {
        store(parse_command_line(argc, argv, cmdlineOptions), vm);

        if(vm.count("dnx"))
        {
            this->miscDnxPath = vm["dnx"].as<string>();
        }

        if(vm.count("verbose"))
        {
            this->isVerbose = vm["verbose"].as<bool>();
        }

        if(vm.count("register"))
        {
            m_isRegisterToken = vm["register"].as<bool>();
        }

        if (argc == 1 || vm.count("help") ||
            !collect_unrecognized(parse_command_line(argc, argv, cmdlineOptions).options, include_positional).empty()) {
            cout << visibleOptions << endl;
            this->isActionRequired = false;
            return;
        }

        if(vm.count("file"))
        {
            this->file = vm["file"].as<string>();
        }

        if(vm.count("unsigneddnx"))
        {
            this->uFwDnx = vm["unsigneddnx"].as<string>();
        }

        if(vm.count("partition"))
        {
            this->partition = vm["partition"].as<int>();
        }

        if(vm.count("blocksize"))
        {
            this->blockSize = vm["blocksize"].as<long>();
        }

        if(vm.count("blockcount"))
        {
            this->blockCount = vm["blockcount"].as<long>();
        }

        if(vm.count("offset"))
        {
            this->offset = vm["offset"].as<long>();
        }

        if(vm.count("tokenoffset"))
        {
            this->tokenOffset = vm["tokenoffset"].as<string>();
        }

        if(vm.count("expirationduration"))
        {
            this->expirationDuration = vm["expirationduration"].as<string>();
        }

        if(vm.count("debuglevel"))
        {
            debuglevel = vm["debuglevel"].as<string>();
            sscanf(debuglevel.c_str(), "%x", &this->debugLevel);
        }
        if(vm.count("usbdelayms"))
        {
            usbdelayms = vm["usbdelayms"].as<string>();
            sscanf(usbdelayms.c_str(), "%ud", &this->usbdelayms);
        }
        if(vm.count("umipdump"))
        {
            this->umipdump = vm["umipdump"].as<bool>();
        }

        if(vm.count("target"))
        {
            targetHexFormat = vm["target"].as<string>();
            sscanf(targetHexFormat.c_str(), "%x", &this->targetIndex);
        }

    isActionRequired = true;

#if defined UNITTEST
        if(vm.count("unittest"))
        {
            cout << "\n!!! Unit Test Activated !!!\n" << endl;
            cout << "Previous value of isActionRequired: " << this->isActionRequired << endl << endl;
            this->isActionRequired = false;
            this->PrintAllOptions();
        }
#endif
    }
    catch(exception& e)
    {
        cout << "\nCould not process the supplied options!" << endl;
        cout << "Details: " << e.what() << endl;
        this->isActionRequired = false;
    }

}

/** Restores all options to their default values
  */
void EmmcOptions::Clear()
{
    miscDnxPath = "";
    debugLevel = 0xffffffff;
    usbdelayms = 0x000000FA;
    umipdump = 0;
    targetIndex = 0;
    isVerbose = false;
    isActionRequired = false;
    m_isRegisterToken = false;
    tokenOffset = "0x0108";
}

string EmmcOptions::GetFile()
{
    return this->file;
}

int EmmcOptions::GetPartition()
{
    return this->partition;
}

long EmmcOptions::GetBlockSize()
{
    return this->blockSize;
}

long EmmcOptions::GetBlockCount()
{
    return this->blockCount;
}

long EmmcOptions::GetOffset()
{
    return this->offset;
}
/** \brief The path for the Miscelaneous DNX (Download and Execute) module.
  * @return The path for the Misc DNX.
  */
const char* EmmcOptions::GetMiscDnxPath()
{
    return this->miscDnxPath.c_str();
}

/** \brief The DebugLevel value used to control log meesages
  * @return The value of the DebugLevel.
  */
unsigned int EmmcOptions::GetDebugLevel()
{
    return this->debugLevel;
}

/** \brief The usbdelayms value used to set the usb bulk read/write delay
  * @return The value of the DebugLevel.
  */
unsigned int EmmcOptions::GetUsbdelayms()
{
    return this->usbdelayms;
}

bool EmmcOptions::GetUmipdumpValue()
{
    return this->umipdump;
}

DeviceTransportType EmmcOptions::GetTransportType()
{
    DeviceTransportType Transport;
    Transport = XFSTK_USB20;
    return Transport;
}

/** \brief The target index to download to.
  * @return The value of the target index.
  */
unsigned int EmmcOptions::GetTarget()
{
    return this->targetIndex;
}

/** \brief Indicates if further actions are required after options are parsed
  *
  * DownloadOptions will handle simple information queries such as displaying
  * help text on behalf of the caller. Operations that require further
  * processing (such as an actual download) are flaged using this method.
  * @return True if the caller needs to complete more actions with the parsed
  * options. False if DownloaderOptions was able to provide a response to all
  * parsed options.
  */
bool EmmcOptions::IsActionRequired()
{
    return this->isActionRequired;
}

/** \brief Indicates if extra debug information was requested by the user.
  */
bool EmmcOptions::IsVerbose()
{
    return this->isVerbose;
}

/** \brief Assembles the current state of all options
  * @return All options, one option and value per line.
  */
void EmmcOptions::PrintAllOptions()
{
    char targetIndexBuffer [11];
    sprintf(targetIndexBuffer, "0x%X", this->targetIndex);
    char DebugBuffer [11];
    sprintf(DebugBuffer, "0x%X", this->debugLevel);
    char UsbDelaymsBuffer [11];
    sprintf(UsbDelaymsBuffer, "0x%X", this->usbdelayms);

    string output = "The Downloader Options have the following values:";
    cout << output;

    output = "\nMisc DnX Path = ";
    output += this->miscDnxPath;
    cout << output;

    output = "\nTarget Index = ";
    output += targetIndexBuffer;
    cout << output;

    output = "\nDebug Level = ";
    output += DebugBuffer;
    cout << output;

    output = "\nUsb Delayms = ";
    output += UsbDelaymsBuffer;
    cout << output;

    output = "\nVerbose = ";
    if(this->isVerbose)
    {
        output += "true";
    }
    else
    {
        output += "false";
    }
    cout << output;

    cout << output << endl <<endl;

}
