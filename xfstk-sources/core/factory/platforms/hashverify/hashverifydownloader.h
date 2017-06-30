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
#ifndef HASHVERIFYDOWNLOADER_H
#define HASHVERIFYDOWNLOADER_H
#include "../../interfaces/idownloader.h"
#include "hashverifyoptions.h"
#include "HashVerifyMessages.h"
#include "hashverifyutils.h"
#include "../cloverview/cloverviewdevice.h"
#include "../cloverview/cloverviewutils.h"
#include "../cloverviewplus/cloverviewplusutils.h"
#include "../cloverviewplus/cloverviewplusdevice.h"
#include "../medfield/medfieldutils.h"
#include "../medfield/medfielddevice.h"
#include "../merrifield/merrifielddevice.h"
#include "../merrifield/merrifieldutils.h"


#define MEDFIELD        0x2
#define CLOVERVIEW      0x3
#define CLOVERVIEWPLUS  0x4
#define MERRIFIELD      0x5
#define KEYSIZE_HV 8*sizeof(DWORD)
#define MAXKEYS 16

class HashVerifyDownloader: public IDownloader
{
public:
    HashVerifyDownloader(unsigned long Device);
   ~HashVerifyDownloader();
    bool SetOptions(IOptions *options);
    bool SetDevice(IGenericDevice *device);
    bool UpdateTarget();
    bool GetStatus();
    bool GetLastError(last_error* er);
    bool Cleanup();
    bool SetStatusCallback(void(*StatusPfn)(char* Status, void* ClientData), void* ClientData);
    void Init();
    void SetDeviceType(unsigned long deviceType);
    int GetResponse(unsigned char* ,int ){return false;}
private:
    last_error err;
    int b_provisionfailed;
    bool b_provisionhasstarted;
    IOptions *CurrentDownloaderOptions;
    HashVerifyOptions *DeviceSpecificOptions;
    bool SetDebugLevel(unsigned long DebugLevel);
    IDevice *CurrentDownloaderDevice;
    bool abort;
    bool b_usbinitok;
    unsigned long devicetype;
    bool WriteOutPipe(unsigned char* pbuf, uint32 size);
    bool ReadInPipe(unsigned char* pbuf, uint32 size);
    unsigned long m_delay_ms;
    void SleepMs(int delay);
    bool writeFile(vector<unsigned char *> keys);
    bool readFile(std::vector<unsigned char*> &keys, int numberOfKeys, int size=2*KEYSIZE_HV);
    bool checkFile(const char *filename);
    bool compareKeys(vector<unsigned char*> keyfromFile, vector<unsigned char*> keyFromDevice);
    void binToAscii(unsigned char* bin, unsigned char* ascii);
    unsigned char* reverseString(unsigned char* asciiStr, int size=KEYSIZE_HV);
    bool processKeys(std::vector<unsigned char*> &keys, unsigned char* hashKeys, int numOfKeys, bool write = true);

    HashVerifyUtils   libutils;
    CloverviewPlusUtils clputils;
    CloverviewUtils clvutils;
    MedfieldUtils mfdUtils;
    MerrifieldUtils mrfUtils;
    bool SetUtilityInstance();

	HashVerifyDownloader( const HashVerifyDownloader& );
	HashVerifyDownloader& operator=( const HashVerifyDownloader& );
};

class HashVerifyExceptions : public std::exception
{
public:
    HashVerifyExceptions(std::string exceptn): except(exceptn){}
    ~HashVerifyExceptions()throw(){}
    const char* what() const throw() {return except.c_str();}
    std::string except;
};

#endif // HASHVERIFYDOWNLOADER_H
