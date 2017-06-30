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
#include <fstream>
#include <vector>
#include <cstring>
#include <sys/stat.h>
#include <exception>

#if defined XFSTK_OS_WIN
#include <windows.h>
#endif

#include <string>
#include "../../common/xfstktypes.h"
#include "hashverifydownloader.h"
#include <sstream>
#include <iomanip>

#define USB_READ_WRITE_DELAY_MS 0
#define HASHDWORDS 9
#define MAXERRORSTRING 100

using namespace std;
extern CPSTR Hash_Verify_error_code_array [MAX_ERROR_CODE];


HashVerifyDownloader::HashVerifyDownloader(unsigned long Device)
{

    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    this->b_provisionfailed = false;
    this->b_provisionhasstarted = false;
    this->CurrentDownloaderDevice = NULL;
    this->CurrentDownloaderOptions = NULL;
    this->DeviceSpecificOptions = NULL;
    this->abort = false;
    m_delay_ms = USB_READ_WRITE_DELAY_MS;
    this->err.error_code = 0;
    this->err.error_message = new char[MAXERRORSTRING];
    this->devicetype= Device;


}
HashVerifyDownloader::~HashVerifyDownloader()
{
   // this->err resources will be taken care of when the class instance goes out of scope
}

bool HashVerifyDownloader::SetOptions(IOptions *options)
{
    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    bool RetVal = false;
    if(options != NULL) {
        this->CurrentDownloaderOptions = options;
        this->DeviceSpecificOptions = (HashVerifyOptions *) options;
        if(this->DeviceSpecificOptions->IsVerbose()) {
            this->SetDebugLevel(this->DeviceSpecificOptions->GetDebugLevel());
        }
        RetVal = true;
    }
    return RetVal;

}

bool HashVerifyDownloader::SetDevice(IGenericDevice *device)
{

    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    bool RetVal = false;
    if(device != NULL && this->CurrentDownloaderOptions !=NULL) {
        this->CurrentDownloaderDevice = (IDevice *) device;
        this->SetUtilityInstance();
        RetVal = true;
    }
    return RetVal;
}
bool HashVerifyDownloader::SetUtilityInstance()
{
    int retval;

    switch(this->devicetype)
    {
         case MEDFIELD :

            retval = this->CurrentDownloaderDevice->SetUtilityInstance((void*) (&(this->mfdUtils)));

            break;
         case CLOVERVIEW  :
            retval = this->CurrentDownloaderDevice->SetUtilityInstance((void*) (&(this->clvutils)));


            break;
         case CLOVERVIEWPLUS :
            retval = this->CurrentDownloaderDevice->SetUtilityInstance((void*) (&(this->clputils)));


            break;

         case MERRIFIELD      :
            retval = this->CurrentDownloaderDevice->SetUtilityInstance((void*) (&(this->mrfUtils)));

            break;
         default:
            retval = this->CurrentDownloaderDevice->SetUtilityInstance((void*) (&(this->libutils)));
            break;

    }

    return retval;
}

void HashVerifyDownloader::SetDeviceType(unsigned long deviceType)
{
    this->devicetype = deviceType;
}



bool HashVerifyDownloader::UpdateTarget()
{
    this->libutils.u_log(LOG_ENTRY, "%s", __FUNCTION__);
    ULONG preamble_msg = PREAMBLE_IDRQ;
    unsigned char numOfkeys = '0';
    unsigned char errorMessage[sizeof(DWORD)+1];
    unsigned char* hashKeys = new unsigned char[HASHDWORDS*9*sizeof(DWORD)+1];

    //Check to make sure Options and Device are instantiated
    if(this->CurrentDownloaderOptions == NULL ||
            this->CurrentDownloaderDevice == NULL) {

		delete [] hashKeys;
        return false;
    }

    //initialize device
    this->Init();

    try{

        //check to see if USB is open
        if((!this->b_usbinitok) || (this->abort))
		{
			if( hashKeys ) delete [] hashKeys;
            throw HashVerifyExceptions("Error Initializing USB");
		}

        if(!this->checkFile(this->DeviceSpecificOptions->GetHashFile()))
		{
			if( hashKeys ) delete [] hashKeys;
            throw HashVerifyExceptions("Error Opening File");
		}

        //Send IDRQ
        if(!WriteOutPipe((PUCHAR)&preamble_msg,4))
		{
			if( hashKeys ) delete [] hashKeys;
            throw HashVerifyExceptions("Error Sending IDRQ");
		}

        //read in header, keys, and error code
        if(!ReadInPipe(hashKeys,((MAXKEYS*8)+2)*sizeof(DWORD)))
		{
			if( hashKeys ) delete [] hashKeys;
            throw HashVerifyExceptions("Error: Reading Hash Keys");
		}

        //Determine the number of keys  eg MFL5 ...5 will be the number of keys
        //key count can be in ascii (0-9) or binary 0-16.
		
		if( hashKeys )
		{
			if((numOfkeys = *(hashKeys+3)) >=  0x30) // ascii
			{
				numOfkeys -= '0'; //convert to ascii
			}
		}

        if((numOfkeys >MAXKEYS))
            throw HashVerifyExceptions("Error: Determining Number of Keys");

        vector<unsigned char*> hashFromFile(numOfkeys);
        vector<unsigned char*> keysFromDevice(numOfkeys);

        //Determine the error message
		// Klocowork detects buffer size incompatibilities, so we need to resize hashKeys 
		// to prevent data truncation

		unsigned char* chashKeys = 0;
		if( hashKeys ) 
		{
			const size_t s1 = HASHDWORDS*9*sizeof(DWORD)+1;
		    chashKeys = new UCHAR[ HASHDWORDS*9*sizeof(DWORD)+1 ];
			for( size_t i = 0; i < s1; ++i )
				chashKeys[i] = hashKeys[i];

			// Now resize hashKeys, the destination buffer
			delete [] hashKeys; 
			const size_t s2 = HASHDWORDS*9*sizeof(DWORD)+1 + (8*numOfkeys+1)*sizeof(DWORD);
			hashKeys = new UCHAR[ s2 ];

			if( hashKeys )
			{
				for( size_t i = 0; i < s1; ++i )
					hashKeys[i] = chashKeys[i];
			}

			delete [] chashKeys;
		}

        memcpy(&errorMessage,((hashKeys+(8*numOfkeys+1)*sizeof(DWORD))),sizeof(DWORD));
		errorMessage[ sizeof(DWORD) ] = '\0';
		string errMess =  reinterpret_cast<char*>(errorMessage) ; 

        if(!errMess.empty())
		{
            //Need to determine meaning of error messages
            //throw errorMessage
			if( hashKeys ) 
				delete [] hashKeys;
            throw HashVerifyExceptions("Error: Error Code Received From Device");
		}

        if(!this->processKeys(keysFromDevice,hashKeys,numOfkeys))
            throw HashVerifyExceptions("Error: Processing Keys");

        if(this->DeviceSpecificOptions->IsWrite())
        {
            if(!this->writeFile((keysFromDevice)))
			{
				if( hashKeys ) delete [] hashKeys;
                throw HashVerifyExceptions("Error: Writing Keys to File");
			}

        }else
        {
            if(!this->readFile(hashFromFile,numOfkeys))
			{
				if( hashKeys ) delete [] hashKeys;				
                throw HashVerifyExceptions("Error: Reading Keys From File");
			}

            if(this->compareKeys(hashFromFile,keysFromDevice))
            {
                this->libutils.u_log(LOG_STATUS, "PASS");
                this->libutils.u_log(LOG_STATUS, "The hashes from the device matches the hashes from the file.");
                this->err.error_code = 0;
                strcpy(this->err.error_message, "A match was found.");
                this->b_provisionfailed = false;
            }else
            {
                this->libutils.u_log(LOG_STATUS, "FAIL");
                this->libutils.u_log(LOG_STATUS, "The hashes do not match!");
                this->err.error_code = 666;
                strcpy(this->err.error_message, "No match was found.");
                this->b_provisionfailed = true;
            }
        }
    }
    catch(std::exception& e)
    {
        this->libutils.u_log(LOG_UPDATE,"Exception raised %s", e.what());
        this->err.error_code = 666;
        strcpy(this->err.error_message, const_cast<char*>(e.what()));
        delete[] hashKeys;
        return false;
    }
    catch( ... )
    {
        this->libutils.u_log(LOG_FWUPGRADE,"%s Unknown Exception raised", __FUNCTION__);
        delete[] hashKeys;
        return false;
    }

    delete[] hashKeys;
    return true;


}

bool HashVerifyDownloader::GetStatus()
{
    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    if(this->CurrentDownloaderOptions == NULL ||
       this->CurrentDownloaderDevice == NULL) {
        return 0x0;
    }
    return !(this->b_provisionfailed);
}

bool HashVerifyDownloader::GetLastError(last_error* er)
{
	if( er )
		er->copy(err);
    return (er == NULL)? true:false;
}

bool HashVerifyDownloader::Cleanup()
{
    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    return true;
}

bool HashVerifyDownloader::SetStatusCallback(void(*StatusPfn)(char* Status, void* ClientData), void* ClientData)
{
    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    this->libutils.u_setstatuspfn(StatusPfn,ClientData);
    return true;
}


bool HashVerifyDownloader::SetDebugLevel(unsigned long DebugLevel)
{
    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    if(DebugLevel > 0) {
        this->libutils.isDebug = DebugLevel;
    }
    return true;
}

void HashVerifyDownloader::Init()
{
    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    b_usbinitok = false;
    bool result = true;
    result = this->CurrentDownloaderDevice->Open();
    this->b_usbinitok = result;

}

bool HashVerifyDownloader::WriteOutPipe(unsigned char* pbuf, uint32 size)
{
    bool result = true;
    this->libutils.u_log(LOG_USB, "%s %s", __FUNCTION__, this->libutils.makestring(pbuf, size).c_str());
    if(this->b_usbinitok) {
        this->libutils.u_log(LOG_USB, "%s %d", __FUNCTION__, m_delay_ms);

        SleepMs(m_delay_ms);
        if(!this->CurrentDownloaderDevice->Write(pbuf,size)) {
            return false;
        }
    }
    else {
        return false;
    }
    return result;
}

bool HashVerifyDownloader::ReadInPipe(unsigned char* pbuf, uint32 size)
{
    bool result = true;
    this->libutils.u_log(LOG_USB, "%s %s", __FUNCTION__, this->libutils.makestring(pbuf, size).c_str());
    if(this->b_usbinitok) {
        SleepMs(m_delay_ms);
        if(!this->CurrentDownloaderDevice->Read(pbuf,size)) {
            return false;
        }
    }
    else {
        return false;
    }
    return result;
}

void HashVerifyDownloader::SleepMs(int delay)
{
#if defined XFSTK_OS_WIN
        Sleep(delay);
#else
        usleep(1000*delay);
#endif
}
bool HashVerifyDownloader::checkFile(const char *filename)
{

    this->libutils.u_log(LOG_ENTRY, "%s", __FUNCTION__);
    FILE *fp;

    fp = fopen(filename, "rb" );
    if ( fp == NULL) {
        this->libutils.u_abort("File %s cannot be opened", filename);
        return false;
    }
    struct stat file_info;
    if(stat(filename, &file_info)) {
        this->libutils.u_abort("Failed to stat file: %s", filename);
        if(fp)
            fclose(fp);
        return false;
    }

    if(fp)
        fclose(fp);
    return true;

}


bool HashVerifyDownloader::writeFile(vector<unsigned char*> keys)
{

    this->libutils.u_log(LOG_ENTRY, "%s", __FUNCTION__);
    fstream hashfile;
    hashfile.open(this->DeviceSpecificOptions->GetHashFile(),ios::out | ios::binary);

    //write the hash to file
    if(hashfile.is_open())
    {
        for(unsigned int i = 0; i<keys.size();i++)
        {
            hashfile.write(reinterpret_cast<const char*>(keys[i]),2*KEYSIZE_HV);
        }
        hashfile.close();
    }else
    {
        return false;
    }
    return true;

}

bool HashVerifyDownloader::readFile(std::vector<unsigned char*> &keys, int numberOfKeys, int size)
{
    this->libutils.u_log(LOG_ENTRY, "%s", __FUNCTION__);
    ifstream hashfile;
    hashfile.open(this->DeviceSpecificOptions->GetHashFile(),ios::in | ios::binary);

    hashfile.seekg(0,ios::end);

    long fileSize = static_cast<long>(hashfile.tellg());

    if(fileSize != static_cast<long>(numberOfKeys*size))
    {
        this->libutils.u_abort("Error: File size is incorrect for number of keys");
        this->err.error_code = 0xBAADF00D; // set to this value to avoid UpdateTarget() retry loop
        this->err.error_message = const_cast<char*>("Error: File size is incorrect for number of keys");
        return false;
    }

    hashfile.seekg(0,ios::beg);

    if(hashfile.is_open())
    {
        //read in and process the strings
        char* hashFromFile= new char[numberOfKeys*size];
        hashfile.read(hashFromFile,numberOfKeys*size);
        hashfile.close();
        if(!this->processKeys(keys, reinterpret_cast<unsigned char*>(hashFromFile),numberOfKeys,false))
            return false;
        delete[] hashFromFile;
        return true;

    }else
    {
        //error
        return false;
    }


}

bool HashVerifyDownloader::compareKeys(vector<unsigned char*> keyfromFile, vector<unsigned char*> keyFromDevice)
{

    this->libutils.u_log(LOG_ENTRY, "%s", __FUNCTION__);
    bool retval = true;

    if(keyfromFile.size() != keyFromDevice.size())
    {
        return false;
    }
    for(std::size_t i=0;i<keyfromFile.size();i++)
    {
        //compares the strings and displays them is they are not equal
        string tmpF (reinterpret_cast<char*>(keyfromFile.at(i)),2*KEYSIZE_HV);
        string tmpD (reinterpret_cast<char*>(keyFromDevice.at(i)),2*KEYSIZE_HV);
        if(tmpF!=tmpD)
        {
            this->libutils.u_log(LOG_UPDATE,"HASH Key %d is different \nHash from File:   %s \nHash from Device: %s", i,tmpF.c_str(),tmpD.c_str());
            retval = false;
        }
    }

    return retval;

}

void HashVerifyDownloader::binToAscii(unsigned char* bin, unsigned char* ascii)
{
    std::stringstream str;
    //Convert the binary to Ascii and reverse string order
    for(uint32 i = 0;i<KEYSIZE_HV;i++)
    {
        //Convert the hex values into a hex string
        str << std::hex << uppercase <<std::setfill('0')<< std::setw(2) << short((*(bin+(KEYSIZE_HV-i-1))));
    }
    string tmpstr = str.str();
    char* tmpcharptr = const_cast<char*>(tmpstr.c_str());
    memcpy(ascii,reinterpret_cast<unsigned char*>(tmpcharptr),2*KEYSIZE_HV);
}

bool HashVerifyDownloader::processKeys(std::vector<unsigned char*> &keys, unsigned char* hashKeys, int numOfKeys, bool write)
{
    unsigned char* tmp =new unsigned char[2*KEYSIZE_HV];
    for(int i = 0;i<numOfKeys;i++)
     {
        if(write)
        {
            // if writing binary to file, first convert to ascii and reverse the string
            this->binToAscii((hashKeys+(i*KEYSIZE_HV)+sizeof(DWORD)),tmp);

        }else
        {
            //if reading from file, split string into keys
            memcpy(tmp,(hashKeys+(i*KEYSIZE_HV*2)),2*KEYSIZE_HV);
        }
        delete keys[i];
        keys[i] = new unsigned char[2*KEYSIZE_HV];
        memcpy(keys[i],tmp,2*KEYSIZE_HV);
     }
    delete[] tmp;
    return true;
}

