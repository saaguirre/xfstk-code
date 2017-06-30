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
#ifndef EMMCDUMPHANDLER_H
#define EMMCDUMPHANDLER_H
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include <string.h>
#include "emmcutils.h"
#include "../../interfaces/ivisitor.h"

using namespace std;


class StEmmcDumpTransfer: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class EmmcDump$ACK: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class EmmcDumpNACK: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class EmmcDumpECSD: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class EmmcDumpSPR$: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class EmmcDumpREQB: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class EmmcDumpEOIO: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class EmmcDumpFwHandleDFRM: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};

class EmmcDumpStHandleFwNormal: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};
class EmmcDumpFwHandleDxxM: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};
class EmmcDumpStHandleFwMisc: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};
class EmmcDumpFwHandleDXBL: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};
class EmmcDumpRDY$: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};
class EmmcDumpFwHandleHLT$: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};
class EmmcDumpER40: public IBaseVisitable<>
{
public:
    ADD_ACCEPT_FUNC()
};
#endif // EMMCDUMPHANDLER_H
