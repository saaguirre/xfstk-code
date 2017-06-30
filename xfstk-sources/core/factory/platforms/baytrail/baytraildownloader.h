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
#ifndef BAYTRAILDOWNLOADER_H
#define BAYTRAILDOWNLOADER_H
#include "../../interfaces/idownloader.h"
#include "baytrailoptions.h"
#include "baytrailutils.h"
#include <QProcess>
#include <QObject>

class BaytrailDownloader : public IDownloader, public QObject
{
public:
    BaytrailDownloader(QObject *parent = 0);
    ~BaytrailDownloader();
    bool SetOptions(IOptions *options);
    bool SetDevice(IGenericDevice *device);
    bool UpdateTarget();
    bool GetStatus();
    bool GetLastError(last_error* er);
    bool Cleanup();
    bool SetDebugLevel(unsigned long DebugLevel);
    bool SetStatusCallback(void(*StatusPfn)(char* Status, void* ClientData), void* ClientData);
    int GetResponse(unsigned char* ,int ){return false;}

private:
    IDevice *CurrentDownloaderDevice;
    IOptions *CurrentDownloaderOptions;
    BaytrailOptions *DeviceSpecificOptions;
    BaytrailUtils   libutils;
    QProcess *process;
    last_error m_last_error;

};

enum ErrorCode
{
EXCODE_PASS,
EXCODE_FAIL_ERASE,
EXCODE_FAIL_PROG,
EXCODE_FAIL_VERIFY,
EXCODE_FAIL_READ,
EXCODE_FAIL_BLANK, // 5
EXCODE_FAIL_BATCH,
EXCODE_FAIL_CHKSUM,
EXCODE_FAIL_IDENTIFY,
EXCODE_FAIL_OTHERS=99
};

#endif // BAYTRAILDOWNLOADER_H
