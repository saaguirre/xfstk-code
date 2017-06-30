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
#include "xfstkdownloader.h"

#ifndef DOWNLOADER_VERSION
#define DOWNLOADER_VERSION "8.8.8"
#endif

#if defined XFSTK_OS_WIN
#include <windows.h>
#endif

void show_version(void)
{
    printf("\nXFSTK Downloader Solo %s \nCopyright (c) 2015 Intel Corporation\n", DOWNLOADER_VERSION);
    printf("Build date and time: %s %s\n\n", __DATE__, __TIME__);
}

void status(char *message, void *)
{
    printf("%s",message);
}
int main(int argc, char* argv[])
{

    int attempteddetect = 0;
    XfstkStatusPfn StatusPfn = status;
    XfstkDownloader usb20downloader;

    show_version();

    if(usb20downloader.ParseCommandline(argc,argv)) {

        while(!usb20downloader.EnumerateDevice()) {
            if(attempteddetect > 20) {
                exit(-1);
            }
#if defined XFSTK_OS_WIN
            Sleep(1000);
#else
            sleep(1);
#endif
            printf("Intel SoC Device Detection Failed: Attempt #%d\n",attempteddetect);
            attempteddetect++;
        }

        //Baytrail cannot detect SoC Device so skip message
        if( usb20downloader.GetEnumDeviceType() != BAYTRAIL)
        {
            printf("Intel SoC Device Detection Found\n");
        }
        printf("Parsing Commandline.... \n");

        if(usb20downloader.ParseCommandline(argc,argv)) {
            printf("Registering Status Callback.... \n");
            if(!usb20downloader.SetStatusCallback(StatusPfn, NULL)) {
                printf("Error Occured During Callback Registration.... \n");
                exit(-1);
            }

            printf("Initiating Download Process.... \n");
            if(usb20downloader.ExecuteDownload()) {
              exit(0);
            }
            else {
              exit(-1);
            }
        }
        else {
           exit(-1);
        }
    }
    exit(0);
}
