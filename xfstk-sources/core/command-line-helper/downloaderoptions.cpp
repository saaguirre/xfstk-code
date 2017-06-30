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
#include <stdio.h>
#include <iostream>
#include <boost/program_options.hpp>
#include "downloaderoptions.h"

using namespace boost::program_options;

/** \brief DownloaderOptions represents the values used to control the
  * behavior of the downloader.
  *
  * The two ways to popluate DownloaderOptions is through a set of
  * key value pairs (such as from a command line) or by setting each
  * individual option.
  */
DownloaderOptions::DownloaderOptions()
{
    this->Clear();
}

/** \brief Processes a list of command line arguments.
  * @param argc The number of tokens to parse.
  * @param argv The tokens to parse.
  */
void DownloaderOptions::Parse(int argc, char* argv[])
{
    string gpflagsHexFormat = "";
    string targetHexFormat = "";

    variables_map vm;
    options_description primaryOptions("Command Line Options");
    primaryOptions.add_options()
    // First parameter describes option's long and short name
    // The second is parameter the options type
    // The third is description
    ("help,h", "Print usage message")
#if defined XFSTK_OS_WIN
    ("fwdnx", value<string>()->default_value("BLANK.bin"), "File path for the FW DNX module")
    ("fwimage", value<string>()->default_value("BLANK.bin"), "File path for the FW Image module")
    ("osdnx", value<string>()->default_value("BLANK.bin"), "File path for the OS DNX module")
    ("osimage", value<string>()->default_value("BLANK.bin"), "File path for the OS image")
#else
    ("fwdnx", value<string>()->default_value("/usr/bin/BLANK.bin"), "File path for the FW DNX module")
    ("fwimage", value<string>()->default_value("/usr/bin/BLANK.bin"), "File path for the FW Image module")
    ("osdnx", value<string>()->default_value("/usr/bin/BLANK.bin"), "File path for the OS DNX module")
    ("osimage", value<string>()->default_value("/usr/bin/BLANK.bin"), "File path for the OS image")
#endif
    ("miscdnx", value<string>(), "File path for miscellaneous DNX module")
    ("gpflags", value<string>()->default_value("0x80000000"), "Optional argument. 32 Bit Hex Value of the GPFlags.")
    ("transfer", value<string>()->default_value("USB"), "Optional argument. Determines how the image will be transferred.")
    ("idrq", value<bool>()->default_value(false), "Optional argument. Indicates whether IDRQ is used. 1 means idrq is used, 0 means idrq is not used.")
    ("verbose,v", bool_switch()->default_value(0), "Optional argument. Display debug information.")
    ;

    options_description cmdlineOptions;
    cmdlineOptions.add(primaryOptions);
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

        if (argc == 1 || vm.count("help")) {
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
            this->miscDnxPath = this->fwDnxPath;
        }

        if(vm.count("gpflags"))
        {
            gpflagsHexFormat = vm["gpflags"].as<string>();
            sscanf(gpflagsHexFormat.c_str(), "%x", &this->gpFlags);
        }

        if(vm.count("transfer"))
        {
            this->transferType = vm["transfer"].as<string>();
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

        this->UpdateFlags();
        if(!this->allPathsAreValid())
        {
            this->isActionRequired = false;
            cout << "There was a problem with the image paths!!!" << endl;
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
void DownloaderOptions::Clear()
{
    this->fwDnxPath = "";
    this->fwImagePath = "";
    this->osDnxPath = "";
    this->osImagePath = "";
    this->miscDnxPath = "";
    this->gpFlags = 0x80000000;
    this->targetIndex = 0;
    this->transferType = "USB";
    this->idrqEnabled = false;
    this->isVerbose = false;
    this->downloadFW = false;
    this->downloadOS = false;
    this->isActionRequired = false;
}

bool DownloaderOptions::allPathsAreValid()
{
    bool allPathsAreValid = true;
    FILE *fp;
    if(this->fwDnxPath.length() > 0)
    {
        fp = fopen(this->fwDnxPath.c_str(), "rb");
        if(fp)
        {
            fclose(fp);
        }
        else
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
        else
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
        else
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
        else
        {
            allPathsAreValid = false;
            cout << "Could not open the file " << this->osImagePath << endl;
        }
    }

    if(fp)
    {
        //delete fp;
		fp = NULL;
    }

    return allPathsAreValid;
}

/** \brief The path for the Firmware DNX (Download and Execute) module.
  * @return The path for the FW DNX.
  */
const char* DownloaderOptions::GetFWDnxPath()
{
    return this->fwDnxPath.c_str();
}

/** \brief The path for the Firmware Image to download.
  * @return The path for the FW image.
  */
const char* DownloaderOptions::GetFWImagePath()
{
    return this->fwImagePath.c_str();
}

/** \brief The path for the Operating System DNX (Download and Execute) module.
  * @return The path for the OS DNX.
  */
const char* DownloaderOptions::GetOSDnxPath()
{
    return this->osDnxPath.c_str();
}

/** \brief The path for the Operating System Image to download.
  * @return The path for the OS image.
  */
const char* DownloaderOptions::GetOSImagePath()
{
    return this->osImagePath.c_str();
}

/** \brief The path for the Miscelaneous DNX (Download and Execute) module.
  * @return The path for the Misc DNX.
  */
const char* DownloaderOptions::GetMiscDnxPath()
{
    return this->miscDnxPath.c_str();
}

/** \brief The GPFlags value used to control download????
  * @return The value of the GPFlags.
  */
unsigned int DownloaderOptions::GetGPFlags()
{
    return this->gpFlags;
}

/** \brief Controls how the firmware is transported to the target.
  * @return How the firmware is transported.
  */
const char* DownloaderOptions::GetTransferType()
{
    return this->transferType.c_str();
}

/** \brief The target index to download to.
  * @return The value of the target index.
  */
unsigned int DownloaderOptions::GetTarget()
{
    return this->targetIndex;
}

/** \brief Indicates if IDRQ enabled or not.
  * @return True when IDRQ is enabled and False when IDRQ is disabled.
  */
bool DownloaderOptions::IsIdrqEnabled()
{
    return this->idrqEnabled;
}

/** \brief Indicates if enough information has been provided to perform a
  * FW Download.
  * @return True when both the firmware DNX and image have been provided.
  * Otherwise returns false.
  */
bool DownloaderOptions::IsFWDownload()
{
    return this->downloadFW;
}

/** \brief Indicates if enough information has been provided to perform a
  * OS Download.
  * @return True when both the operating system DNX and image have been
  * provided. Otherwise returns false.
  */
bool DownloaderOptions::IsOSDownload()
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
bool DownloaderOptions::IsActionRequired()
{
    return this->isActionRequired;
}

/** \brief Indicates if extra debug information was requested by the user.
  */
bool DownloaderOptions::IsVerbose()
{
    return this->isVerbose;
}

/** \brief Determines if firmware and/or an operating system will be downloaded
  *
  * The isQuery flag will be set if the query string is provided. The downloadFW
  * flag will be set when both the firmware DNX and image are provided and a
  * query is not provided. The download OS flag will be set when both the
  * operating system DNX and image are provided and the query is not provided.
  */
inline void DownloaderOptions::UpdateFlags()
{
    this->downloadFW = (this->fwDnxPath.length() > 0 && this->fwImagePath.length() > 0);
    this->downloadOS = (this->osDnxPath.length() > 0 && this->osImagePath.length() > 0);
    this->isActionRequired = (this->downloadFW || this->downloadOS);
}

/** \brief Assembles the current state of all options
  * @return All options, one option and value per line.
  */
void DownloaderOptions::PrintAllOptions()
{
    char gpFlagsBuffer [11];
    sprintf(gpFlagsBuffer, "0x%X", this->gpFlags);
    char targetIndexBuffer [11];
    sprintf(targetIndexBuffer, "0x%X", this->targetIndex);

    string output = "The Downloader Options have the following values:";
    output += "\nFW DNX Path = ";
    output += this->fwDnxPath.c_str();
    output += "\nFW Image Path = ";
    output += this->fwImagePath;
    output += "\nOS DNX Path = ";
    output += this->osDnxPath;
    output += "\nOS Image Path = ";
    output += this->osImagePath;
    output += "\nMisc DNX Path = ";
    output += this->miscDnxPath;
    output += "\nTarget Index = ";
    output += targetIndexBuffer;
    output += "\nGP Flags = ";
    output += gpFlagsBuffer;
    output += "\nTransfer = ";
    output += this->transferType;
    output += "\nIDRQ Enabled = ";
    if(this->idrqEnabled)
    {
        output += "true";
    }
    else
    {
        output += "false";
    }

    output += "\nVerbose = ";
    if(this->isVerbose)
    {
        output += "true";
    }
    else
    {
        output += "false";
    }

    cout << endl << output << endl <<endl;
}

/** \brief Processes the legacy positional command line
  */
void DownloaderOptions::ParseLegacy(int argc, char* argv[])
{
    cout << "\nDeprecated Command Line Format Used!" << endl;
    cout << "Use the --help command to see the new comand line format" << endl << endl;
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
            this->isVerbose = (debuglevel == 0xffffffff);
        }
        else
        {
            this->isVerbose = false;
        }
 #if defined XFSTK_MULTIDEVICE
        if(argc == 11) {
            sscanf(argv[10], "%d", &targetIndex);
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
        cout << "\nWrong number of legacy arguments used!" << endl;
        this->isActionRequired = false;
    }
}
