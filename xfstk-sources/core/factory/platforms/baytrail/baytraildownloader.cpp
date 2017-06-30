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
#include <cstddef>
#include "../../common/xfstktypes.h"
#include "baytraildownloader.h"

using namespace std;

extern CPSTR Baytrail_error_code_array [BAYTRAIL_MAX_ERROR_CODE];

BaytrailDownloader::BaytrailDownloader(QObject *parent) :
    QObject(parent)
{
   this->libutils.u_log(LOG_ENTRY, "Creating Qprocess Downloader", __PRETTY_FUNCTION__);
   process = new QProcess(this);
   if(!process)
   {
       this->m_last_error.error_code = 10;
       strcpy(this->m_last_error.error_message, Baytrail_error_code_array[10]);
   }
}

BaytrailDownloader::~BaytrailDownloader()
{
    this->libutils.u_log(LOG_ENTRY, "Deleteing Qprocess Downloader", __PRETTY_FUNCTION__);
    if(process)
    {
        delete process;
        process = NULL;
    }
}

bool BaytrailDownloader::SetOptions(IOptions *options)
{
    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    if(options != NULL)
    {
        this->CurrentDownloaderOptions = options;
        this->DeviceSpecificOptions = (BaytrailOptions *) options;
        if(this->DeviceSpecificOptions->IsVerbose())
        {
            this->SetDebugLevel(this->DeviceSpecificOptions->GetDebugLevel());
        }
        return true;
    }
    else
    {
        this->m_last_error.error_code = 11;
        strcpy(this->m_last_error.error_message, Baytrail_error_code_array[11]);
    }
    return false;
}

bool BaytrailDownloader::SetDevice(IGenericDevice *device)
{
    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    if(device != NULL && this->CurrentDownloaderOptions !=NULL)
    {
        this->CurrentDownloaderDevice = (IDevice *) device;
        return true;
    }
    else
    {
        this->m_last_error.error_code = 12;
        strcpy(this->m_last_error.error_message, Baytrail_error_code_array[12]);
    }
    return false;
}

#if defined XFSTK_OS_WIN
#define MAX_LOOP 200 //used to abort no input loop...abort reading input after xx seconds
#define COUNT_MOD 1
#else
#define MAX_LOOP 1400 //used to abort no input loop...abort reading input after xx seconds
#define COUNT_MOD 10
#endif

bool BaytrailDownloader::UpdateTarget()
{
    const char* fwPath = DeviceSpecificOptions->GetFWImagePath();
    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    this->libutils.u_log(LOG_DOWNLOADER, "FWImagePath -- %s", this->DeviceSpecificOptions->GetFWImagePath());
    this->libutils.u_log(LOG_DOWNLOADER, "DebugLevel -- %x", this->DeviceSpecificOptions->GetDebugLevel());
    this->libutils.u_log(LOG_DOWNLOADER, "TransferType -- %s", this->DeviceSpecificOptions->GetTransferType());
    QString cmdline;
    int percent;

#if defined XFSTK_OS_WIN
    cmdline = "dpcmd.exe -i --vcc 2 -z " + QString::fromLatin1(fwPath);
 #else
    cmdline = "flashrom -p dediprog:1.8V -w " + QString::fromLatin1(fwPath);
 #endif
    // Execute the downloader now
    this->libutils.u_log(LOG_STATUS, "Launching downloader now");
    process->start(cmdline);
    if(!process->waitForStarted(5000)) //wait up to 5 seconds to start
    {
#if defined XFSTK_OS_WIN
        this->libutils.u_log(LOG_STATUS, "dpcmd.exe failed to start. Check DediProg's installation");

#else
        this->libutils.u_log(LOG_STATUS, "flashrom failed to start. Check flashrom's installation");

#endif
        return false; //failed download
    }
    this->libutils.u_log(LOG_STATUS, "Updating SPI flash");
    int loopcount = 0;
    int count = 0;
    bool running = false;
    while((loopcount< MAX_LOOP) && (!running)) // MAX_LOOP seconds without any input to read
    {
        while(process->canReadLine())
        {
            QByteArray outData = process->readLine(128);
            outData = outData.simplified();
            if(!outData.isEmpty())
                this->libutils.u_log(LOG_DOWNLOADER, outData.data());
            loopcount = 0;
        }

        if(process->waitForFinished(1000)) //wait 1 secs, then loop again
            running = true;

        loopcount++;
        count++;

        percent = count/COUNT_MOD;
        if (count%COUNT_MOD== 0) //only log every 4th loop
        {
              if(percent < 98)
                    this->libutils.u_log(LOG_PROGRESS, "%d", percent);
        }
    }
    if(loopcount == MAX_LOOP)
    {
        strcpy(this->m_last_error.error_message, Baytrail_error_code_array[16]);
        this->libutils.u_log(LOG_STATUS, "Download timed out");

        process->kill(); //timed out, so kill the process, or at least try to.
        return false;
    }
    //if available, read the rest of output now that program has finished
    QByteArray readAllOutData = process->readAllStandardOutput();
    if(!readAllOutData.isEmpty())
    {
#if defined XFSTK_OS_WIN
        if(readAllOutData.contains("Error: Automatic program Failed"))
        {
            strcpy(this->m_last_error.error_message, Baytrail_error_code_array[9]);
            this->libutils.u_log(LOG_STATUS, "Download completed with a failure");

            return false;
        }

#endif

//start low level dump
        char outBuffer[120];

        for(int i=0; i<readAllOutData.size()-1; i++)
        {
            //skip line feeds and carraige returns
            while((readAllOutData.at(i) == '\r') || (readAllOutData.at(i) == '\n'))
            {
                i++; //skip all carriage returns and line feeds
                if( i==readAllOutData.size()-1)
                    break;
            }

            //initialize output buffer for new read line
            int j=0;
            memset(outBuffer, '\0', sizeof(outBuffer));
            //capture a read line from array buffer
            while((readAllOutData.at(i) != '\r')&&(readAllOutData.at(i) != '\n')&&(i<readAllOutData.size()))
            {
                outBuffer[j] = readAllOutData.at(i);
                i++; j++;
                if( i==readAllOutData.size()-1)
                    break;
            }
            if(outBuffer[0] != '\0') //do we have something to update?
            {
                this->libutils.u_log(LOG_STATUS, "%s", &outBuffer[0]);
            }
        }
//end low level dump
    }

    int retCode = process->exitCode();

#if defined XFSTK_OS_WIN
    switch(retCode)
    {
    case EXCODE_PASS:
        strcpy(this->m_last_error.error_message, Baytrail_error_code_array[0]);
        this->libutils.u_log(LOG_STATUS, "Download completed successfully");

        break;
    case EXCODE_FAIL_ERASE:
        strcpy(this->m_last_error.error_message, Baytrail_error_code_array[1]);
        this->libutils.u_log(LOG_STATUS, "Download encountered an erase failure");
        break;
    case EXCODE_FAIL_PROG:
        strcpy(this->m_last_error.error_message, Baytrail_error_code_array[2]);
        this->libutils.u_log(LOG_STATUS, "Download encountered a program failure");
        break;
    case EXCODE_FAIL_VERIFY:
        strcpy(this->m_last_error.error_message, Baytrail_error_code_array[3]);
        this->libutils.u_log(LOG_STATUS, "Download encountered a verify failure");
        break;
    case EXCODE_FAIL_READ:
        strcpy(this->m_last_error.error_message, Baytrail_error_code_array[4]);
        this->libutils.u_log(LOG_STATUS, "Download encountered a read failure");
        break;
    case EXCODE_FAIL_BLANK:
        strcpy(this->m_last_error.error_message, Baytrail_error_code_array[5]);
        this->libutils.u_log(LOG_STATUS, "Download encountered a blank failure");
        break;
    case EXCODE_FAIL_BATCH:
        strcpy(this->m_last_error.error_message, Baytrail_error_code_array[6]);
        this->libutils.u_log(LOG_STATUS, "Download encountered a batch failure");
        break;
    case EXCODE_FAIL_CHKSUM:
        strcpy(this->m_last_error.error_message, Baytrail_error_code_array[7]);
        this->libutils.u_log(LOG_STATUS, "Download completed with checksum error");
        break;
    case EXCODE_FAIL_IDENTIFY:
        strcpy(this->m_last_error.error_message, Baytrail_error_code_array[8]);
        this->libutils.u_log(LOG_STATUS, "Failed to identify flash chip");
        break;
    case EXCODE_FAIL_OTHERS:
    default:
        strcpy(this->m_last_error.error_message, Baytrail_error_code_array[9]);
        this->libutils.u_log(LOG_STATUS, "Download completed with a failure");
        break;
    }
#else
    if(retCode == 0)
    {
        strcpy(this->m_last_error.error_message, Baytrail_error_code_array[0]);
        this->libutils.u_log(LOG_STATUS, "Download completed successfully");

    }
    else
    {
        strcpy(this->m_last_error.error_message, Baytrail_error_code_array[9]);
        this->libutils.u_log(LOG_STATUS, "Download completed with an error");

    }

#endif

    if(retCode == 0)
    {
        return true;
    }
    else
    {
        this->m_last_error.error_code = 0xBAADF00D; //to avoid UpdateTarget()retry loop
        return false;
    }
}

bool BaytrailDownloader::GetStatus()
{
    return true;
}

bool BaytrailDownloader::GetLastError(last_error* er)
{
    er->copy(m_last_error);
    return true;
}

bool BaytrailDownloader::Cleanup()
{
    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    return true;
}

bool BaytrailDownloader::SetDebugLevel(unsigned long DebugLevel)
{
    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    if(DebugLevel > 0) {
        this->libutils.isDebug = DebugLevel;
    }
    return true;
}

bool BaytrailDownloader::SetStatusCallback(void(*StatusPfn)(char* Status, void* ClientData), void* ClientData)
{
    this->libutils.u_log(LOG_ENTRY, "%s", __PRETTY_FUNCTION__);
    this->libutils.u_setstatuspfn(StatusPfn,ClientData);
    return true;
}
