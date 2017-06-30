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
#ifndef XFSTKTYPES_H
#define XFSTKTYPES_H
#include <time.h>


#ifdef XFSTK_OS_WIN
#else
#include <unistd.h> // sleep() - Fedora
#endif

typedef unsigned char*              PUCHAR;
typedef unsigned long               ULONG;
typedef unsigned short              USHORT;
typedef unsigned char               UCHAR;
typedef char                        *PSZ;
#ifdef XFSTK_OS_WIN
typedef unsigned long               DWORD;
#else
typedef unsigned int               DWORD;
#endif
typedef int                         BOOL;
typedef unsigned char               BYTE;
typedef unsigned short              WORD;
typedef float                       FLOAT;
typedef FLOAT                       *PFLOAT;
typedef int                         INT;
typedef unsigned int                UINT;
typedef unsigned int                *PUINT;
typedef char                        CHAR;
typedef char *                      PSTR;
typedef const char *                CPSTR;
typedef unsigned long long int      ULONGLONG;
typedef unsigned char               uint8;
typedef unsigned short int          uint16;
typedef unsigned int                uint32;
typedef unsigned long long int      uint64;
typedef struct timespec             TIME_T;

#endif // XFSTKTYPES_H
