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
#ifndef XFSTKCOMMON_H
#define XFSTKCOMMON_H
struct IntelSoCGlobalDeviceState {
    bool IsUsb20Initialized;
    bool IsUsb20DebugEnabled;
    bool IsSerialInitialized;
};
static struct IntelSoCGlobalDeviceState IntelSoCGlobalDeviceCurrentState = {
    false, //Is USB Initialized
    false, //Is USB Debug Enabled
    false  //Is Serial Initialized
};

struct dnx_data {
        int size;
        unsigned char* data;
};

struct bati_battery_data {

    unsigned long BATI;
    unsigned short header_size;
    unsigned char   header_revis;
    unsigned char   xor_check;
    unsigned long voltage;
    unsigned long coulomb_count;
};

//Reallocates an array buff while keeping the integrity of the data.
//Check return value for NULL to determine success
template <typename T>
T* realloc_array(T*& ptr,size_t oldsize,size_t size)
{

    if(oldsize == size)
        return ptr;
    T* buff = NULL;
    buff = new T[size];

    if(buff == NULL)
        return buff;

    memset(buff,0,size);
    memcpy(buff,ptr,(oldsize<size)? oldsize:size);


    T* tmp = ptr;
    ptr = buff;
    buff = tmp;
    delete [] buff;
    buff = NULL;


    return ptr;

    //dummy code to remove warnings
    if(IntelSoCGlobalDeviceCurrentState.IsSerialInitialized){};
}

#endif //XFSTKCOMMON
