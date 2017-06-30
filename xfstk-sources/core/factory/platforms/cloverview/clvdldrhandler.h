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
#ifndef CLV_FWDLDRHANDLER_H
#define CLV_FWDLDRHANDLER_H
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include <string.h>
#include "cloverviewutils.h"
#include "../../interfaces/ivisitor.h"
#include "cloverviewfw.h"
#include "cloverviewos.h"

using namespace std;

class ClvFwHandleER00: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvFwHandleHLT0: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvFwHandleDFRM: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvFwHandleDxxM: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvFwHandleDXBL: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvFwHandleRUPHS: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvFwHandleRUPH: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvFwHandleDMIP: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvFwHandleLOFW: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvFwHandleHIFW: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvFwHandlePSFW1: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvFwHandlePSFW2: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvFwHandleSSFW: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvFwHandleHLT$: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvFwHandleMFLD: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvFwHandleCLVT: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};
class ClvFwHandleSuCP: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvFwHandleRTBD: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvFwHandleVEDFW: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvFwHandleSSBIOS: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvFwHandleIFW1: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvFwHandleIFW2: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvFwHandleIFW3: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvFwHandleRESET: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvOsHandleDORM: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvOsHandleOSIPSZ: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvOsHandleROSIP: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvOsHandleDONE: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvOsHandleRIMG: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvOsHandleEOIU: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvStHandleFwNormal: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvStHandleFwMisc: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};
class ClvStHandleFwWipe: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvStHandleOsNormal: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvStHandleOsMisc: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvErHandleLogError: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class ClvErHandleERRR: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};
#endif // CLV_FWDLDRHANDLER_H
