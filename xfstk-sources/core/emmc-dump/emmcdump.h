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
#ifndef EMMCDUMP_H
#define EMMCDUMP_H
#include "xfstkfactory.h"

class EmmcDump
{
public:
    EmmcDump();
    bool ParseCommandline(int argc, char* argv[]);
    bool EnumerateDevice();
    bool ExecuteDownload();
    bool SetStatusCallback(XfstkStatusPfn StatusPfn, void *ClientData);
    ~EmmcDump();
private:
    unsigned long enumerateddevicetype;
    bool CreateAllRequiredInterfaces(unsigned long devicetype);
    IFactory *xfstkfactory;
    IGenericDevice *xfstkdevice;
    IOptions *xfstkoptions;
    IDownloader *xfstkdownloader;
};

#endif // EMMCDUMP_H
