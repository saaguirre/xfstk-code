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
#ifndef XFSTKDLDREXAMPLEPARALLEL_H
#define XFSTKDLDREXAMPLEPARALLEL_H
#include <QtCore/QCoreApplication>
#include <QMutex>
#include <QObject>
#include <QThread>
#include "xfstkdldrapi.h"
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <list>
#include <time.h>
#include <utility>
#include <sys/stat.h>

using namespace std;

typedef struct {
    string fwdnx;
    string fwimage;
    string osdnx;
    string osimage;
    string gpflags;
    string usbsn;
    xfstkstatuspfn statuscallback;
} arg_type;

typedef struct {
    void* threadobj;
    string usbsn;
    void* hthread;
} thread_rectype;

class DlThreadObj : public QObject
{
    Q_OBJECT

public:
    void DldrRun(void*);
    void * Arg() const {return Arg_;}
    void Arg(void* a){Arg_ = a;}
    void DoWork();
private:
    unsigned int ThreadId_;
    void * Arg_;

};

class Dlthread : public QThread
{
Q_OBJECT
public:
    explicit Dlthread(QObject *parent = 0);
    ~Dlthread();
    DlThreadObj *obj;
    void setobj(DlThreadObj *obj);
signals:
    void done();

public slots:
protected:
    void run();

};
#endif // XFSTKDLDREXAMPLEPARALLEL_H
