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
#include <QtCore/QCoreApplication>
#include "xfstkdldrapi.h"
#include <stdlib.h>
#include <stdio.h>
void status(char *message, void *clientdata)
{
        printf(message);
}
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    xfstkdldrapi test;
    int numtargets = 0;
    int retrycount = 0;


    if(argc > 1) {
            numtargets = atoi(argv[1]);
            printf("BEGIN PROVISION - ################# %d TARGETS #################\n",numtargets);
            if(argc > 2) {
                    retrycount = atoi(argv[2]);
                    printf("RETRY COUNT - ################# %d RETRIES #################\n",retrycount);
            }
    }
    else {
            printf("XFSTK: xfstk-api-example-serial performs multi-target provisioning of both fw and os in a serial raound robin fashion.  This approach is useful when dealing with unreliable USB connectivity.\n\n");
            printf("Usage: xfstk-api-example-serial.exe <number of targets> <number of retries>\n");
            printf("\nExample: xfstk-api-example-serial.exe 8 1000\n");
            printf("\nThe above commandline will perform provisoning for 8 targets and retry 1000 times on I/O error.\n");
    }

    bool provisioningok = false;
    xfstkstatuspfn mystatusfn = &status;
    test.registerstatuscallback(status, 0);
    if(retrycount > 0) {
            test.settargetretrycount(retrycount);
    }

    int targetcounter = 0;
    int provpass = 0;
    int provfail = 0;
    int sanity = 0;
    while(targetcounter < numtargets) {
            provisioningok = test.downloadfwos("//tmp//fwdnx.bin", "//tmp//fwimage.bin", "//tmp//osdnx.bin", "//tmp//osimage.bin", "0x80000001");
            if(provisioningok) {
                    printf("TARGET: %d -  ################# SUCCESS!!! ###############\n",targetcounter);
                    provpass++;
            }
            else {
                    printf("TARGET: %d -  !!!!!!!!!!!!!!!!! FAILURE... !!!!!!!!!!!!!!!\n",targetcounter);
                    printf("\nXFSTK: SUMMARY - TOTAL PASS = 0 - TOTAL FAIL = %d\n", numtargets);
                    printf("\nXFSTK: Programming NOT completed for all %d targets - FAIL\n",numtargets);
                    exit(0);
                    provfail++;
            }
            sanity++;
            provisioningok = false;
            targetcounter++;
            if(sanity > 2*numtargets) {
                    printf("\nXFSTK: SUMMARY - TOTAL PASS = 0 - TOTAL FAIL = %d\n", numtargets);
                    printf("\nXFSTK: Programming NOT completed for all %d targets - FAIL\n",numtargets);
                    exit(0);
            }

    }
    printf("\nXFSTK: SUMMARY - TOTAL PASS = %d - TOTAL FAIL = %d\n", provpass, provfail);
    if(provfail > 0) {
            printf("\nXFSTK: Programming NOT completed for all %d targets - FAIL\n",numtargets);
    }
    else {
            printf("\nXFSTK: Programming completed for all %d targets - PASS!\n",numtargets);
    }
    exit(0);
}
