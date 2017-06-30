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
#ifndef FWDLDRHANDLER_H
#define FWDLDRHANDLER_H
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include <string.h>
#include "medfieldutils.h"
#include "../../interfaces/ivisitor.h"
#include "medfieldfw.h"
#include "medfieldos.h"

using namespace std;

class FwHandleER00: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class FwHandleHLT0: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class FwHandleDFRM: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class FwHandleDxxM: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class FwHandleDXBL: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class FwHandleRUPHS: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class FwHandleRUPH: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class FwHandleDMIP: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class FwHandleLOFW: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class FwHandleHIFW: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class FwHandlePSFW1: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class FwHandlePSFW2: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class FwHandleSSFW: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class FwHandleHLT$: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class FwHandleMFLD: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class FwHandleCLVT: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};
class FwHandleSuCP: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class FwHandleRTBD: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class FwHandleVEDFW: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class FwHandleSSBIOS: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class FwHandleIFW1: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class FwHandleIFW2: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class FwHandleIFW3: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class FwHandleRESET: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class OsHandleDORM: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class OsHandleOSIPSZ: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class OsHandleROSIP: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class OsHandleDONE: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class OsHandleRIMG: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class OsHandleEOIU: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class StHandleFwNormal: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class StHandleFwMisc: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};
class StHandleFwWipe: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class StHandleOsNormal: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class StHandleOsMisc: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ErHandleLogError: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ErHandleERRR: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};
#endif // FWDLDRHANDLER_H
