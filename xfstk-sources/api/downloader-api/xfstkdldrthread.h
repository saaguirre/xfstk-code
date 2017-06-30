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
#ifndef XFSTKDLDRTHREAD_H
#define XFSTKDLDRTHREAD_H

#include <QObject>
#include <QMutex>
#include <QSize>
#include <QThread>
#include <QWaitCondition>
#include "xfstkfactory.h"
#include "xfstkdldrapi.h"
#include <QMutexLocker>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <QStringList>
class xfstkdldrthreadobj : public QObject
{
Q_OBJECT
public:
    unsigned int isDebug;
    unsigned long mtdownloadtype;
    float time_elapsed;
    XfstkStatusPfn physstatuspfn;
    int retrycount;
    int numtargets;
    void *physclientdata;
    char *fwdnx;
    char *fwimage;
    char *osdnx;
    char* osimage;
    char* gpflags;
    char* softfuse;
    void *handle;
    bool fwisdone;
    bool osisdone;
    bool bFwOnly;
    bool bFwOs;
    bool bOsOnly;
    bool downloadfailed;
    unsigned long  devicetype;
    unsigned long  debuglevel;
    unsigned long  usbdelayms;
    char usbsn[128];
    char miscdnx[2048];
    bool miscdnxenable;
	bool wipeifwi;
    LastError lastError;

    explicit xfstkdldrthreadobj(QObject *parent = 0);
    void configuredownloader(char *fwdnx, char *fwimage, char *osdnx, char* osimage, char* gpflags, char* usbsn, int numtargets);
    void configuredownloader(char *fwdnx, char *fwimage, char *osdnx, char* osimage, char* gpflags, char* softfuse, char* usbsn, int numtargets);
    void go();
signals:
public slots:
private:
    bool downloadmtfwosthread();
    last_error tmpError;
};

class xfstkdldrthread : public QThread
{
Q_OBJECT
public:
    explicit xfstkdldrthread(QObject *parent = 0);
    ~xfstkdldrthread();
    xfstkdldrthreadobj *obj;
    void setobj(xfstkdldrthreadobj *obj);
signals:
    void done();

public slots:
protected:
    void run();

private:
    QMutex mutex;
    QWaitCondition condition;
};

#endif // XFSTKDLDRTHREAD_H
