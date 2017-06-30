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
#ifndef MRFDLDRHANDLER_H
#define MRFDLDRHANDLER_H
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include <string.h>
#include "merrifieldutils.h"
#include "../../interfaces/ivisitor.h"
#include "merrifieldfw.h"
#include "merrifieldos.h"

using namespace std;

class MrfdFwHandleER00: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdFwHandleHLT0: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdFwHandleDFRM: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdFwHandleDxxM: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdFwHandleDXBL: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdFwHandleRUPHS: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdFwHandleRUPH: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdFwHandleDCFI00: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdFwHandleDIFWI: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdFwHandleHLT$: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdFwHandleMFLD: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdFwHandleCLVT: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdFwHandleRTBD: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdFwHandleRESET: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdOsHandleDORM: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdOsHandleOSIPSZ: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdOsHandleROSIP: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdOsHandleDONE: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdOsHandleRIMG: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdOsHandleEOIU: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdStHandleFwNormal: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdStHandleFwMisc: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};
class MrfdStHandleFwWipe: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdStHandleOsNormal: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdStHandleOsMisc: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdErHandleLogError: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdErHandleERRR: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdFwHandleDCSDB: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdFwHandleUCSDB: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

//start emmc
class MrfdStEmmcDumpTransfer: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdEmmcDump$ACK: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdEmmcDumpNACK: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdEmmcDumpECSD: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdEmmcDumpSPR$: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdEmmcDumpREQB: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdEmmcDumpEOIO: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdEmmcDumpFwHandleDFRM: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdEmmcDumpStHandleFwNormal: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdEmmcDumpRDY$: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdEmmcDumpER40: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class MrfdHandleLogDevice: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

// end emmc

#endif // MRFDLDRHANDLER_H
