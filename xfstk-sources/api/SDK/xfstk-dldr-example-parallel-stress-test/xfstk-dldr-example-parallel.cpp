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
#include "xfstk-dldr-example-parallel.h"

#if defined XFSTK_OS_WIN
#include <windows.h>
#endif
#include <stdio.h>

QMutex ghMutex;
QMutex ghListMutex;

void status(char *message, void *clientdata)
{
    ghMutex.lock();
    printf(message);
    ghMutex.unlock();
}

list<string> list_usbsn;
list<string>::iterator it_usbsn;
list<void*> list_running;
list<void*> list_done;
list<void*>::iterator it_running;
list<void*>::iterator it_done;
int totaltargets = 6;
int donetargets = 0;
int failtargets = 0;

void DlThreadObj::DldrRun(void* arg)
{
    bool bResult = false;
    LastError er;
    thread_rectype * trd;
    arg_type * myarg = (arg_type*) arg;
    xfstkdldrapi xfstktest;
    xfstkstatuspfn mystatusfn = myarg->statuscallback;
    xfstktest.registerstatuscallback(mystatusfn,0);
    xfstktest.settargetretrycount(20);
    xfstktest.setusbreadwritedelay(0);
    xfstktest.setwipeifwi(false);

    bResult = xfstktest.downloadmtfwosasync(
                (char*)myarg->fwdnx.c_str(),
                (char*)myarg->fwimage.c_str(),
                (char*)myarg->osdnx.c_str(),
                (char*)myarg->osimage.c_str(),
                (char*)myarg->gpflags.c_str(),
                (char*)myarg->usbsn.c_str()
                );
    xfstktest.getlasterror(&er);
    if(!bResult) {
        printf("USBSN: %s -- Downloader thread failed %d Returned error code:%d -- %s\n", myarg->usbsn.c_str(), ThreadId_, er.error_code, er.error_message);
    } else {
       printf("USBSN: %s -- Downloader thread succeed %d Returned error code:%d -- %s\n", myarg->usbsn.c_str(), ThreadId_, er.error_code, er.error_message);
    }
#if 1
    ghListMutex.lock();
    for (list<void*>::iterator it=list_running.begin(); it!=list_running.end(); it++) {
        trd = (thread_rectype *)*it;
        if(trd->usbsn == myarg->usbsn) {

            list_done.push_back(*it);
            donetargets++;
            if(er.error_code != 0)
                failtargets++;
            break;
        }
    }
    ghListMutex.unlock();
#endif
}
void DlThreadObj::DoWork()
{
    this->DldrRun(this->Arg());
}

Dlthread::Dlthread(QObject *parent) :
    QThread(parent)
{
}
Dlthread::~Dlthread()
{
    wait();
}

void Dlthread::setobj(DlThreadObj *obj)
{
    this->obj = obj;
}
void Dlthread::run()
{
    this->obj->DoWork();
    this->done();
    exit();
}

bool IsContains(list<void*>* list_checking, string* usbsn)
{
    bool ret = false;
    ghListMutex.lock();
    for (list<void*>::iterator it=list_checking->begin(); it!=list_checking->end(); it++) {
        thread_rectype *trd = (thread_rectype *)*it;
        if(trd->usbsn == *usbsn) {
            ret = true;
            break;
        }
    }
    ghListMutex.unlock();
    return ret;
}

void Sleepms(int delay)
{
#if defined XFSTK_OS_WIN
        Sleep(delay);
#else
        usleep(1000*delay);
#endif
}
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    int y = 0;
    int pass = 0;
    int fail = 0;
    xfstkdldrapi xfstktest;

    it_running = list_running.begin();
    it_done = list_done.begin();
    it_usbsn = list_usbsn.begin();

    //Here populate list_usbsn for simulate the SCU device dection
    //Remeber replace the ussn string with the ones from your devices
    list_usbsn.push_back ("175D126E48335F80");
    list_usbsn.push_back ("BDA397C26EA8035B");
    list_usbsn.push_back ("3183CBBBDA014AF1");
    list_usbsn.push_back ("E7C123BC1EA72083");
    list_usbsn.push_back ("B9AD3AC29A0BDF5E");
    list_usbsn.push_back ("9724D645A3D7E902");

    //Wait to see if there is a new device arrived
    while(1) {
        if(xfstktest.getavailabletargets() == totaltargets) {
            printf("\n\nXFSTK INFO: Found SCU %d devices.\n",xfstktest.getavailabletargets());
            break;
        }

        //Sleep one second and scan a again
        printf("\n\nXFSTK INFO: Wait One seconds and scan for SCU device again\n");
        Sleepms(1000);//(1000);
    }

    while(1) {
        //In your App, need to use winddk USB enumeration to detect SCU devices
        //here only use populated list_usbsn to simulate
        for (it_usbsn=list_usbsn.begin(); it_usbsn!=list_usbsn.end(); it_usbsn++) {
            if(!IsContains(&list_running,&*it_usbsn) ){
                thread_rectype * thread_rec = new thread_rectype;
                DlThreadObj* downloader = new DlThreadObj;
                Dlthread *dlthread = new Dlthread;
                thread_rec->threadobj = (void*)downloader;
                thread_rec->usbsn = *it_usbsn;

                ghListMutex.lock();
                list_running.push_back((void*)thread_rec);
                ghListMutex.unlock();

                arg_type * myarg = new arg_type;
                myarg->fwdnx   = "//tmp//fwdnx.bin";
                myarg->fwimage   = "//tmp//fwimage.bin";
                myarg->osdnx   = "//tmp//osdnx.bin";
                myarg->osimage   = "//tmp//osimage.bin";
                myarg->gpflags   = "80000045";
                myarg->usbsn   = *it_usbsn;
                myarg->statuscallback   = status;
                cout << "Starting thread on USBSN: " + *it_usbsn << "\n";
                downloader->Arg(myarg);
                thread_rec->hthread = (void*)dlthread;
                dlthread->setobj(downloader);
                dlthread->start();
                //Wait 200ms to let the thread to start
                Sleepms(200);//(200);
            }
        }


        //If done clear up all resources
        if(donetargets >= totaltargets) {
            ghListMutex.lock();
            while (!list_done.empty()) {
                thread_rectype *trd = (thread_rectype *)list_done.front();
                list_done.pop_front();
                delete (Dlthread*)trd->hthread;
                delete (DlThreadObj*)trd->threadobj;
                delete trd;
            }
            list_done.clear();
            list_running.clear();

            if(failtargets > 0) {
                fail += failtargets;
                pass += donetargets;
                pass -= failtargets;
                printf("\n\nXFSTK ERROR: %d devices failed out of %d\n",fail, donetargets);
            } else {
                pass += donetargets;
                printf("\n\nXFSTK INFO: %d devices soccessful out of %d\n",pass, donetargets);
            }
            //clear the variables for next round of run            
            donetargets = failtargets = 0;
            ghListMutex.unlock();
            //break;
        }

        //wait one second to check if threads are done
        Sleepms(200);

    }

   // cout << "Press any key to terminate this program. " ; getch();

    exit(0);
}

