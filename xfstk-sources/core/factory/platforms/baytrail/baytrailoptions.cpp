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
#include "baytrailoptions.h"
#include <cstdio>
#include <iostream>

namespace po = boost::program_options;

void BaytrailOptions::SetDefaults()
{
    return;
}
/** \brief BaytrailOptions represents the values used to control the
  * behavior of the downloader.
  *
  * The two ways to popluate BaytrailOptions is through a set of
  * key value pairs (such as from a command line) or by setting each
  * individual option.
  */
BaytrailOptions::BaytrailOptions()
{
    this->Clear();
}

/** \brief Processes a list of command line arguments.
  * @param argc The number of tokens to parse.
  * @param argv The tokens to parse.
  */
void BaytrailOptions::Parse(int argc, char* argv[])
{
    variables_map vm;
    string debuglevel = "";

    options_description commandLineOptions("Command Line Options");
    commandLineOptions.add_options()
    ("help,h", "Print usage message")
    ("fwimage",    po::value<string>()->default_value("BLANK.bin"),"File path for the FW Image module")
    ("rev,r",      po::bool_switch()->default_value(0), "Optional argument. Display current version/revision.")
    ("debuglevel", po::value<string>()->default_value("0xffffffff"), "Optional argument. 32 Bit Hex Value of the debuglevel, 0x1800 = LOG_STATUS | LOG_PROGRESS")
    ("verbose,v",  po::bool_switch()->default_value(0), "Optional argument. Display debug information.")
    ;

    try
    {
        store(parse_command_line(argc, argv, commandLineOptions), vm);
        if(vm.count("verbose"))
        {
            this->isVerbose = vm["verbose"].as<bool>();
        }


        if(argc > 1)
        {
            string firstArg = argv[1];

            if(firstArg.find("-") == std::string::npos)
            {
                cout << "\nInvalid argument(s)!" << endl;
                cout << "\nDisplaying HELP menu for list of valid arguments ... " << endl;
                cout << commandLineOptions << endl;
                return;
            }
        }
        if (argc == 1 || vm.count("help")) {
            cout << commandLineOptions << endl;
            this->isActionRequired = false;
            return;
        }
        if(vm.count("fwimage"))
        {
            this->fwImagePath = vm["fwimage"].as<string>();
        }
        if(vm.count("rev"))
        {
            bool rev = vm["rev"].as<bool>();
            this->isActionRequired = !rev & this->isActionRequired;
        }
        if(vm.count("debuglevel"))
        {
            debuglevel = vm["debuglevel"].as<string>();
            sscanf(debuglevel.c_str(), "%x", &this->debugLevel);
        }
    } //try
    catch(exception& e)
    {
        cout << "\nCould not process the supplied options!" << endl;
        cout << "Details: " << e.what() << endl;
        cout << "Help Menu: " << endl;
        cout << commandLineOptions << endl;
        this->isActionRequired = false;
    }

    if(this->fwImagePath != "BLANK.bin") //error only if path was specified
    {
        if(!this->allPathsAreValid())
        {
            cout << "There was a problem with the image paths!!" << endl;
            this->isActionRequired = false;
        }
    }
    else
    {
        this->isActionRequired = false;
    }

}

/** Restores all options to their default values
  */
void BaytrailOptions::Clear()
{
    this->fwImagePath = "";
    this->downloadFW = false;
    this->isActionRequired = false;
    this->isVerbose = false;
    this->debugLevel = 0xffffffff;
    this->transferType = "DEDI-PROG";
}

bool BaytrailOptions::allPathsAreValid()
{
    this->isActionRequired = true;
    bool allPathsAreValid = false;
    FILE *fp;

    fp = fopen(this->fwImagePath.c_str(), "rb");
    if(fp)
    {
        allPathsAreValid = true;
        this->isActionRequired = true;
        fclose(fp);
        return true;
    }

    // Failed to open fw image file...report problem back
    std::cout << "Could not open FW image file: " << this->fwImagePath.c_str() <<  endl;
    allPathsAreValid = false;
    this->isActionRequired = false;
    return false;
}


/** \brief The path for the Firmware Image to download.
  * @return The path for the FW image.
  */
const char* BaytrailOptions::GetFWImagePath()
{
    return this->fwImagePath.c_str();
}

/** \brief Controls how the firmware is transported to the target.
  * @return How the firmware is transported.
  */
const char* BaytrailOptions::GetTransferType()
{
    return this->transferType.c_str();
}

/** \brief The DebugLevel value used to control log meesages
  * @return The value of the DebugLevel.
  */
unsigned int BaytrailOptions::GetDebugLevel()
{
    return this->debugLevel;
}

DeviceTransportType BaytrailOptions::GetTransportType()
{
    DeviceTransportType Transport;
    Transport = XFSTK_DEDI_PROG;
    return Transport;
}


/** \brief Indicates if enough information has been provided to perform a
  * FW Download.
  * @return True when both the firmware DNX and image have been provided.
  * Otherwise returns false.
  */
bool BaytrailOptions::IsFWDownload()
{
    return this->downloadFW;
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
bool BaytrailOptions::IsActionRequired()
{
    return this->isActionRequired;
}

/** \brief Indicates if extra debug information was requested by the user.
  */
bool BaytrailOptions::IsVerbose()
{
    return this->isVerbose;
}

/** \brief Indicates if to zero out ifwi image content.
  */
bool BaytrailOptions::IsWipeIfwiEnabled()
{
    return this->wipeifwi;
}

/** \brief Assembles the current state of all options
  * @return All options, one option and value per line.
  */
void BaytrailOptions::PrintAllOptions()
{
    char targetIndexBuffer [11] = {0};

    string output = "The Downloader Options have the following values:";
    cout << output;

    output = "\nFW Image Path = ";
    output += this->fwImagePath;
    cout << output;

    output = "\nTarget Index = ";
    output += targetIndexBuffer;
    cout << output;

    output = "\nTransfer = ";
    output += this->transferType;
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
    cout << output << endl <<endl;
}
