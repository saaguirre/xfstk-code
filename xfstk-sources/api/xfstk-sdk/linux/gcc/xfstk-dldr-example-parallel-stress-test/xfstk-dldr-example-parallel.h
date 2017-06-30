#ifndef XFSTKDLDREXAMPLEPARALLEL_H
#define XFSTKDLDREXAMPLEPARALLEL_H
#include <QtCore/QCoreApplication>
#include <QMutex>
#include <QObject>
#include <QThread>
#include "xfstkdldrapi.h"
//#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <fstream>
//#include <conio.h>
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
