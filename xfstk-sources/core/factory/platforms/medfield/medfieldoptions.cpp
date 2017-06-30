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
#include "medfieldoptions.h"
#include <iostream>
#include <fstream>
#include <string>

namespace po = boost::program_options;

void MedfieldOptions::SetDefaults()
{
    return;
}
/** \brief MedfieldOptions represents the values used to control the
  * behavior of the downloader.
  *
  * The two ways to popluate MedfieldOptions is through a set of
  * key value pairs (such as from a command line) or by setting each
  * individual option.
  */
MedfieldOptions::MedfieldOptions()
{
    this->Clear();
}

/** \brief Processes a list of command line arguments.
  * @param argc The number of tokens to parse.
  * @param argv The tokens to parse.
  */
void MedfieldOptions::Parse(int argc, char* argv[])
{
    string gpflagsHexFormat = "";
    string targetHexFormat = "";
    string debuglevel = "";
    string usbdelayms = "";
    bool rev = false;

    variables_map vm;
    options_description primaryOptions("Command Line Options");
    primaryOptions.add_options()
    // First parameter describes option's long and short name
    // The second is parameter the options type
    // The third is description
    ("help,h", "Print usage message")
    ("fwdnx",      po::value<std::string>()->default_value("BLANK.bin"), "File path for the FW DNX module")
    ("fwimage",    po::value<std::string>()->default_value("BLANK.bin"), "File path for the FW Image module")
    ("osdnx",      po::value<std::string>()->default_value("BLANK.bin"), "File path for the OS DNX module")
    ("osimage",    po::value<std::string>()->default_value("BLANK.bin"), "File path for the OS image")
    ("miscdnx",    po::value<std::string>(), "File path for miscellaneous DNX module")
    ("gpflags",    po::value<std::string>()->default_value(""), "Optional argument. 32 Bit Hex Value of the GPFlags. For example, 0x80000000")
    ("debuglevel", po::value<std::string>()->default_value("0xffffffff"), "Optional argument. 32 Bit Hex Value of the debuglevel, 0x1800 = LOG_STATUS | LOG_PROGRESS")
    ("usbdelayms", po::value<std::string>()->default_value("0"), "Optional argument. 32 Bit int Value of the usbdelayms, default 0ms")
    ("wipeifwi",   po::bool_switch()->default_value(0), "Optional argument. Indicate whether to wipe out ifwi image on emmc. Set to false by default")
    ("transfer",   po::value<std::string>()->default_value("USB"), "Optional argument. Determines how the image will be transferred.")
    ("idrq",       po::bool_switch()->default_value(0), "Optional argument. Indicates whether IDRQ is used. 1 means idrq is used, 0 means idrq is not used.")
    ("verbose,v",  po::bool_switch()->default_value(0), "Optional argument. Display debug information.")
    ("rev,r",      po::bool_switch()->default_value(0), "Optional argument. Display current version/revision.")
    ;

    options_description cmdlineOptions;

#if defined UNITTEST
    options_description testingOptions("xFSTK Downloader Options Testing Commands");
    testingOptions.add_options()
    ("unittest", "Processes the options found but does not set the action required flag")
    ("target",  value<unsigned int>()->default_value(0), "Optional argument. The target index to use when multiple targets are connected.")
    ;
    cmdlineOptions.add(testingOptions);
#endif

    options_description visibleOptions;
    visibleOptions.add(primaryOptions);

    primaryOptions.add_options()
            ("csdb",       po::value<std::string>()->default_value(" "), "Optional argument. 'update' or 'challenge' Chaabi specific data block.")
            ("initcsdb",   po::value<std::string>()->default_value("1"), "Send the first CSDB of a sequence")
            ("finalcsdb",  po::value<std::string>()->default_value("1"), "Send the last CSDB of a sequence")
            ("miscbin",    po::value<std::string>()->default_value("BLANK.bin"), "File path for micellaneous binary file")
            ("softfuse",   po::value<std::string>()->default_value("BLANK.bin"), "Softfuse Path");
    cmdlineOptions.add(primaryOptions);


    try
    {
        store(parse_command_line(argc, argv, cmdlineOptions), vm);



        if(vm.count("verbose"))
        {
            this->isVerbose = vm["verbose"].as<bool>();
        }

        if(argc > 1)
        {
            string firstArg = argv[1];

            if(firstArg.find("-") == std::string::npos)
            {
                cout << "\nInvalid argument(s)!" <<endl;
                cout << "\nDisplaying HELP menu for list of valid arguments ... " << endl;
                cout << visibleOptions << endl;
                cout << "  spi                            Print SPI usage message" << endl;

                return;
            }
        }

        if (argc == 1 || vm.count("help") ||
            !collect_unrecognized(parse_command_line(argc, argv, cmdlineOptions).options, include_positional).empty()) {

            cout << visibleOptions << endl;
            cout << "  spi                            Print SPI usage message" << endl;
            this->isActionRequired = false;
            return;
        }

        if(vm.count("fwdnx"))
        {
            this->fwDnxPath = vm["fwdnx"].as<string>();
        }

        if(vm.count("fwimage"))
        {
            this->fwImagePath = vm["fwimage"].as<string>();
        }

        if(vm.count("osdnx"))
        {
            this->osDnxPath = vm["osdnx"].as<string>();
        }

        if(vm.count("osimage"))
        {
            this->osImagePath = vm["osimage"].as<string>();
        }

        if(vm.count("miscdnx"))
        {
            this->miscDnxPath = vm["miscdnx"].as<string>();
        }
        else
        {
            this->miscDnxPath = "";
        }

        if(vm.count("gpflags"))
        {
            if(vm["gpflags"].as<string>().empty())
            {
                this->gpFlags = ((this->osDnxPath != "BLANK.bin") && (this->osImagePath != "BLANK.bin")) ? 0x80000001 : 0x80000000;
            }else
            {
                gpflagsHexFormat = vm["gpflags"].as<string>();
                sscanf(gpflagsHexFormat.c_str(), "%x", &this->gpFlags);
            }
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
        if(vm.count("wipeifwi"))
        {
            this->wipeifwi = vm["wipeifwi"].as<bool>();
        }
        if(vm.count("transfer"))
        {
            string tmp = vm["transfer"].as<string>();
            size_t point;

            point = tmp.find("SERIAL");
            if(point!= string::npos)
            {
                point = tmp.find('_');
                if((point ==std::string::npos) || ((point+1) == tmp.length()))
                {
                    MedfieldOptionsExceptions("Transfer format for SERIAL is incorrect!!!");
                }else
                {
                    this->transferType = tmp.substr(0,point);
                }
            }else
            {
                this->transferType = vm["transfer"].as<string>();
            }
        }

        if(vm.count("miscbin"))
        {
            this->miscBinPath = vm["miscbin"].as<string>();
        }

        if(vm.count("idrq"))
        {
            this->idrqEnabled = vm["idrq"].as<bool>();
        }

        if(vm.count("target"))
        {
            targetHexFormat = vm["target"].as<string>();
            sscanf(targetHexFormat.c_str(), "%x", &this->targetIndex);
        }

        if(vm.count("csdb"))
        {
            this->csdbStatus = vm["csdb"].as<string>();
        }

        if(vm.count("initcsdb"))
        {
            bool initcsdb = vm["initcsdb"].as<string>() != "0";
            if(!initcsdb && this->csdbStatus == " ")
            {
                throw MedfieldOptionsExceptions("Improper usage of initcsdb options!!!");
            }

        }
        this->UpdateFlags();

        if(!this->allPathsAreValid())
        {
            throw MedfieldOptionsExceptions("There was a problem with the binary file paths!!!");
        }
        if(this->wipeifwi && !this->fwDnxPath.compare("BLANK.bin"))
        {
            throw MedfieldOptionsExceptions("Wipe ifwi requires a DnX");
        }

        if(vm.count("rev"))
        {
            rev = vm["rev"].as<bool>();
            this->isActionRequired = !rev & this->isActionRequired;
        }

        if(!this->downloadFW && !this->downloadOS && !this->idrqEnabled && !(csdbStatus != " ") && !rev)
        {
            throw MedfieldOptionsExceptions("Input ingredient mismatch!!!");
        }

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
        cout << "Help Menu: " << endl;
        cout << visibleOptions << endl;
        cout << "  spi                            Print SPI usage message" << endl;
        this->isActionRequired = false;
    }

}

/** Restores all options to their default values
  */
void MedfieldOptions::Clear()
{
    this->fwDnxPath = "";
    this->fwImagePath = "";
    this->osDnxPath = "";
    this->osImagePath = "";
    this->miscDnxPath = "";
    this->gpFlags = 0x80000000;
    this->debugLevel = 0xffffffff;
    this->usbdelayms = 0x000000FA;
    this->wipeifwi = false;
    this->targetIndex = 0;
    this->transferType = "USB";
    this->idrqEnabled = false;
    this->isVerbose = false;
    this->downloadFW = false;
    this->downloadOS = false;
    this->isActionRequired = false;
}

bool MedfieldOptions::allPathsAreValid()
{
    bool allPathsAreValid = true;
    FILE *fp = NULL;
    string blankbinpath;
    blankbinpath = "BLANK.bin";

    if(this->fwDnxPath.length() > 0)
    {
        fp = fopen(this->fwDnxPath.c_str(), "rb");
        if(fp)
        {
            fclose(fp);
        }
        else if(!(this->fwDnxPath.c_str() == blankbinpath))
        {
            allPathsAreValid = false;
            cout << "Could not open the file " << this->fwDnxPath << endl;
        }
    }

    if(this->fwImagePath.length() > 0)
    {
        fp = fopen(this->fwImagePath.c_str(), "rb");
        if(fp)
        {
            fclose(fp);
        }
        else if(!(this->fwImagePath.c_str() == blankbinpath))
        {
            allPathsAreValid = false;
            cout << "Could not open the file " << this->fwImagePath << endl;
        }
    }

    if(this->osDnxPath.length() > 0)
    {
        fp = fopen(this->osDnxPath.c_str(), "rb");
        if(fp)
        {
            fclose(fp);
        }
        else if(!(this->osDnxPath.c_str() == blankbinpath))
        {
            allPathsAreValid = false;
            cout << "Could not open the file " << this->osDnxPath << endl;
        }
    }

    if(this->osImagePath.length() > 0)
    {
        fp = fopen(this->osImagePath.c_str(), "rb");
        if(fp)
        {
            fclose(fp);
        }
        else if(!(this->osImagePath.c_str() == blankbinpath))
        {
            allPathsAreValid = false;
            cout << "Could not open the file " << this->osImagePath << endl;
        }
    }

    if(this->miscBinPath.length() > 0 && (this->miscBinPath != blankbinpath))
    {
        fp = fopen(this->miscBinPath.c_str(), "rb");
        if(fp)
        {
            fclose(fp);
        }
        else
        {
            ofstream outputFile;
            outputFile.open(this->miscBinPath.c_str());
            if(!outputFile.is_open())
            {
                allPathsAreValid = false;
                cout << "Could not open or create the file " << this->miscBinPath << endl;
            }
            outputFile.close();
        }
    }
    if(fp)
    {
        fp = NULL;
    }

    return allPathsAreValid;
}

/** \brief The path for the Firmware DNX (Download and Execute) module.
  * @return The path for the FW DNX.
  */
const char* MedfieldOptions::GetFWDnxPath()
{
    return this->fwDnxPath.c_str();
}

/** \brief The path for the Firmware Image to download.
  * @return The path for the FW image.
  */
const char* MedfieldOptions::GetFWImagePath()
{
    return this->fwImagePath.c_str();
}

/** \brief The path for the Operating System DNX (Download and Execute) module.
  * @return The path for the OS DNX.
  */
const char* MedfieldOptions::GetOSDnxPath()
{
    return this->osDnxPath.c_str();
}

/** \brief The path for the Operating System Image to download.
  * @return The path for the OS image.
  */
const char* MedfieldOptions::GetOSImagePath()
{
    return this->osImagePath.c_str();
}

/** \brief The path for the Miscelaneous DNX (Download and Execute) module.
  * @return The path for the Misc DNX.
  */
const char* MedfieldOptions::GetMiscDnxPath()
{
    return this->miscDnxPath.c_str();
}

/** \brief The GPFlags value used to control download????
  * @return The value of the GPFlags.
  */
unsigned int MedfieldOptions::GetGPFlags()
{
    return this->gpFlags;
}

/** \brief The DebugLevel value used to control log meesages
  * @return The value of the DebugLevel.
  */
unsigned int MedfieldOptions::GetDebugLevel()
{
    return this->debugLevel;
}

/** \brief The usbdelayms value used to set the usb bulk read/write delay
  * @return The value of the DebugLevel.
  */
unsigned int MedfieldOptions::GetUsbdelayms()
{
    return this->usbdelayms;
}
/** \brief Controls how the firmware is transported to the target.
  * @return How the firmware is transported.
  */
const char* MedfieldOptions::GetTransferType()
{
    return this->transferType.c_str();
}

DeviceTransportType MedfieldOptions::GetTransportType()
{
    DeviceTransportType Transport;
    if(this->transferType == "USB") {
        Transport = XFSTK_USB20;
    }
    else if(this->transferType == "USB30") {
        Transport = XFSTK_USB30;
    }
    else if(this->transferType == "SERIAL") {
        Transport = XFSTK_SERIAL;
    }
    else {
        Transport = XFSTK_NOT_SPECIFIED;
    }
    return Transport;
}

/** \brief The target index to download to.
  * @return The value of the target index.
  */
unsigned int MedfieldOptions::GetTarget()
{
    return this->targetIndex;
}

/** \brief Indicates if IDRQ enabled or not.
  * @return True when IDRQ is enabled and False when IDRQ is disabled.
  */
bool MedfieldOptions::IsIdrqEnabled()
{
    return this->idrqEnabled;
}

/** \brief Indicates if enough information has been provided to perform a
  * FW Download.
  * @return True when both the firmware DNX and image have been provided.
  * Otherwise returns false.
  */
bool MedfieldOptions::IsFWDownload()
{
    return this->downloadFW;
}

/** \brief Indicates if enough information has been provided to perform a
  * OS Download.
  * @return True when both the operating system DNX and image have been
  * provided. Otherwise returns false.
  */
bool MedfieldOptions::IsOSDownload()
{
    return this->downloadOS;
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
bool MedfieldOptions::IsActionRequired()
{
    return this->isActionRequired;
}

/** \brief Indicates if extra debug information was requested by the user.
  */
bool MedfieldOptions::IsVerbose()
{
    return this->isVerbose;
}

/** \brief Indicates if to zero out ifwi image content.
  */
bool MedfieldOptions::IsWipeIfwiEnabled()
{
    return this->wipeifwi;
}
/** \brief Determines if firmware and/or an operating system will be downloaded
  *
  * The isQuery flag will be set if the query string is provided. The downloadFW
  * flag will be set when both the firmware DNX and image are provided and a
  * query is not provided. The download OS flag will be set when both the
  * operating system DNX and image are provided and the query is not provided.
  */
inline void MedfieldOptions::UpdateFlags()
{
    this->downloadFW = (this->fwDnxPath.compare("BLANK.bin") && this->fwImagePath.compare("BLANK.bin"));
    this->downloadOS = (this->osDnxPath.compare("BLANK.bin") && this->osImagePath.compare("BLANK.bin"));
    bool idrq = this->idrqEnabled && (this->miscBinPath.compare("BLANK.bin") != 0);
    bool csdb = (this->csdbStatus != " " );
    this->isActionRequired = (this->downloadFW || this->downloadOS || idrq || csdb);
}

/** \brief Assembles the current state of all options
  * @return All options, one option and value per line.
  */
void MedfieldOptions::PrintAllOptions()
{
    char gpFlagsBuffer [11];
    sprintf(gpFlagsBuffer, "0x%X", this->gpFlags);
    char targetIndexBuffer [11];
    sprintf(targetIndexBuffer, "0x%X", this->targetIndex);
    char DebugBuffer [11];
    sprintf(DebugBuffer, "0x%X", this->debugLevel);
    char UsbDelaymsBuffer [11];
    sprintf(UsbDelaymsBuffer, "0x%X", this->usbdelayms);

    string output = "The Downloader Options have the following values:";
    cout << output;

    output = "\nFW DNX Path = ";
    output += this->fwDnxPath.c_str();
    cout << output;

    output = "\nFW Image Path = ";
    output += this->fwImagePath;
    cout << output;

    output = "\nOS DNX Path = ";
    output += this->osDnxPath;
    cout << output;

    output = "\nOS Image Path = ";
    output += this->osImagePath;
    cout << output;

    output = "\nMisc DNX Path = ";
    output += this->miscDnxPath;
    cout << output;

    output = "\nTarget Index = ";
    output += targetIndexBuffer;
    cout << output;

    output = "\nGP Flags = ";
    output += gpFlagsBuffer;
    cout << output;

    output = "\nDebug Level = ";
    output += DebugBuffer;
    cout << output;

    output = "\nUsb Delayms = ";
    output += UsbDelaymsBuffer;
    cout << output;

    output = "\nTransfer = ";
    output += this->transferType;
    cout << output;

    output = "\nIDRQ Enabled = ";
    if(this->idrqEnabled)
    {
        output += "true";
    }
    else
    {
        output += "false";
    }
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

    output = "\nWipeIfwiEnable = ";
    if(this->wipeifwi)
    {
        output += "true";
    }
    else
    {
        output += "false";
    }
    cout << output << endl <<endl;
}

/** \brief Processes the legacy positional command line
  */
void MedfieldOptions::ParseLegacy(int argc, char* argv[])
{
    cout << "\nInvalid OR Deprecated Command Line Format Used!" << endl;
    cout << "Attempting Legacy Arguments Parsing ..." << endl;
    if(argc >= 9) //The legacy command line has 9 or more positional arguments
    {
        this->fwDnxPath = argv[1];
        this->fwImagePath = argv[2];
        this->osDnxPath = argv[3];
        this->osImagePath = argv[4];
        this->miscDnxPath = argv[5];
        sscanf(argv[6], "%x", &gpFlags);
        this->transferType = "USB";
        char idrq;
        sscanf(argv[8], "%c", &idrq);
        if ((idrq == 'I') || (idrq == 'i') || (idrq == 'N') || (idrq == 'n'))
        {
            switch (idrq)
            {
                case 'I':
                case 'i':
                    this->idrqEnabled = true;
                    break;

                case 'N':
                case 'n':
                    this->idrqEnabled = false;
                    break;

                default:
                    this->idrqEnabled = false;
            }
        }
        if(argc == 10) {
            unsigned int debuglevel;
            sscanf(argv[9], "%x", &debuglevel);
            this->isVerbose = (debuglevel & 0xffffffff) > 0;
        }
        else
        {
            this->isVerbose = false;
        }
 #if defined XFSTK_MULTIDEVICE
        if(argc == 11) {
            sscanf(argv[10], "%ud", &targetIndex);
        }
        else
        {
            this->targetIndex = 0;
        }
#endif

        this->isActionRequired = true;
        this->UpdateFlags();
    }
    else
    {
        cout << "\n\nCannot Parse!" << endl;
        cout << "Possible Reasons: " << endl;
        cout << "1. Invalid arguments, OR" << endl;
        cout << "2. Wrong number of legacy arguments" << endl;
        this->isActionRequired = false;
    }
}
