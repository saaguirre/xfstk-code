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
#ifndef XFSTKDLDRFACTORY_H
#define XFSTKDLDRFACTORY_H
#include <QList>
#include "xfstkfactory.h"
#include "xfstkdldrthread.h"
#include <QObject>
#include <QThread>
#include "boost/scoped_array.hpp"

#define XFSTK_ERROR 666
#define XFSTK_PARAM_LENGTH 2048
#define MAX_DOWNLOAD_TIME 40

class xfstksleep : public QThread
{
Q_OBJECT
public:
    explicit xfstksleep(QObject *parent = 0);
    void sleep(unsigned long secs);
    void msleep(unsigned long msecs);
    void usleep(unsigned long usecs);
signals:

public slots:

};

class xfstkdldrfactory
{
public:
    xfstkdldrfactory();
    unsigned long EnumeratedDeviceType;
    int retrycount;
    XfstkStatusPfn StatusCallback;
    void*  ClientData;
    IFactory *XfstkDldrFactory;
    IGenericDevice *XfstkDeviceWorking;
    QList<IGenericDevice *> XfstkDeviceList;
    QList<void *> DeviceHandleList;
    QList<IOptions *> XfstkOptionsList;
    QList<IDownloader *> XfstkDownloaderList;
    QList<xfstkdldrthread *> xfstkmtthreadlist;
    QList<xfstkdldrthreadobj *> xfstkthreadobjlist;


    bool EnumerateDevices();
    int ScanForDevices();
    int ScanForDevices(unsigned long devicetype);
    int GetNumberOfAttachedDevices();
#if defined XFSTK_OS_WIN
    bool SetDeviceCallBackInstances(void* deviceCallBack);
    bool SetDeviceCallBackInstances(void* deviceCallBack, unsigned long ProductId);
#endif
    int GetNumberOfAttachedDevices(SoCDevices * socdevices);
    bool GetLastError(LastError* er);
    bool ClearAllLists();
    bool SetOptions(int argc, char* argv[]);
    bool BindInterfaces();
    bool ExecuteDownloadSerial(const char *usbsn = NULL);
    bool ExecuteDownloadParallelAsync(char *fwdnx, char *fwimage, char *osdnx, char* osimage, char* gpflags, char* numtargets, int retrycount);
    bool ExecuteDownloadParallelSync(char *fwdnx, char *fwimage, char *osdnx, char* osimage, char* gpflags, char* numtargets, int retrycount);
    bool ExecuteDownloadSingleAsync(char *fwdnx, char *fwimage, char *osdnx, char* osimage, char* gpflags, char* usbsn, int retrycount);
    bool ExecuteDownloadSingleAsync(char *fwdnx, char *fwimage, char *osdnx, char* osimage, char* gpflags, char* usbsn, char* softfuse, int retrycount);
    bool SetStatusCallback(XfstkStatusPfn StatusCallback, void* clientdata);
    void SetTransportType(DeviceTransportType transporttype);
    void SetIdrqResponse(unsigned char *buffer, int& maxsize);
    ~xfstkdldrfactory();
    xfstksleep sleeper;
    boost::scoped_array<char> miscdnx;
    bool miscdnxenable;
    char *softfuse;
    bool softfuseinclude;
    bool wipeifwi;
    LastError lasterror;
    unsigned long debuglevel;
    unsigned long usbdelayms;
    DeviceTransportType transporttype;
	unsigned long scudevicetype;
    unsigned long expandedOptions;
    int* idrqBuffSize;
    int maxIdrqBuffSize;
    unsigned char* idrqBuffer;



private:
    bool CreateAllRequiredInterfaces(unsigned long devicetype, int totalnumdevices);
#if defined XFSTK_OS_WIN
    IDeviceFactory* WinDriverDeviceFactory;
#endif
    QList<IGenericDevice *> XfstkWinDriverDeviceWorking;

	xfstkdldrfactory( const xfstkdldrfactory& );
	xfstkdldrfactory& operator=( const xfstkdldrfactory& );
};

#endif // XFSTKDLDRFACTORY_H
