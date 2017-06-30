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
#ifndef CLVPDLDRHANDLER_H
#define CLVPDLDRHANDLER_H
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include <string.h>
#include "cloverviewplusutils.h"
#include "../../interfaces/ivisitor.h"
#include "cloverviewplusfw.h"
#include "cloverviewplusos.h"

using namespace std;

class ClvpFwHandleER00: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpFwHandleHLT0: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpFwHandleDFRM: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpFwHandleDxxM: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpFwHandleDXBL: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpFwHandleRUPHS: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpFwHandleRUPH: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpFwHandleDMIP: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpFwHandleLOFW: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpFwHandleHIFW: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpFwHandlePSFW1: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpFwHandlePSFW2: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpFwHandleSSFW: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpFwHandleHLT$: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpFwHandleMFLD: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpFwHandleCLVT: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};
class ClvpFwHandleSuCP: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpFwHandleRTBD: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpFwHandleVEDFW: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpFwHandleSSBIOS: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpFwHandleIFW1: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpFwHandleIFW2: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpFwHandleIFW3: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpFwHandleRESET: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpOsHandleDORM: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpOsHandleOSIPSZ: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpOsHandleROSIP: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpOsHandleDONE: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpOsHandleRIMG: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpOsHandleEOIU: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpStHandleFwNormal: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpStHandleFwMisc: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};
class ClvpStHandleFwWipe: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpStHandleOsNormal: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpStHandleOsMisc: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpErHandleLogError: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpErHandleERRR: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};
//start emmc
class ClvpStEmmcDumpTransfer: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpEmmcDump$ACK: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpEmmcDumpNACK: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpEmmcDumpECSD: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpEmmcDumpSPR$: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpEmmcDumpREQB: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpEmmcDumpEOIO: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpEmmcDumpFwHandleDFRM: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpEmmcDumpStHandleFwNormal: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpEmmcDumpRDY$: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvpEmmcDumpER40: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};
// end emmc

#endif // FWDLDRHANDLER_H
