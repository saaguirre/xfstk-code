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
#ifndef SOFTFUSESFW_H
#define SOFTFUSESFW_H

#include "../cloverviewplus/cloverviewplusutils.h"
#include "../cloverviewplus/clvpdldrstate.h"
#include "../../common/xfstkcommon.h"

#define SFUSE_DATA_RSUPHS   0
#define SFUSE_DATA_RSUPH	1

#define SFUSE_BIN_SUP0H	    2 //Softfuses binary UP header 0 size
#define SFUSE_BIN_KEY_FILE	3 //Softfuses binary data i.e. key file (the response to DSKF)

#define NUM_FW_DATA_SF 4


class softfusesFW
{
public:
        softfusesFW();
        ~softfusesFW();
    bool Init(char* fname_softfuses_bin, CloverviewPlusUtils* utils);
        dnx_data* GetSoftFusesFileData(unsigned long index);
        int softFusesDataSize;
        int softFusesKeyFileSize;

private:
        bool CheckFile(char *filename);
        void LogError(int errorcode);
        void InitSUPHeaderData();
        bool FindSUPHeaderSignature();
        unsigned long GetSUPHeaderSize();
        unsigned long GetDataChunckSize( unsigned long tempData);
        bool InitNoSize();
        bool InitSoftfusesBin();

    char* m_fname_softfuses_bin;//softfuses binary filename
    unsigned long m_softfuses_update_profile_hdr_size;
    unsigned char* m_softfuses_update_profile_hdr;
    unsigned char* m_softfuses_key_file;
    unsigned long m_softfuses_key_file_size;
    unsigned long m_i_offset;
    unsigned long m_tempData;
    unsigned char* m_softfuses_bin;
    unsigned long m_softfuses_bin_size;
    unsigned char* m_sup_hdr_zero_size; //SUP 0 size header data

    CloverviewPlusUtils* m_utils;
    dnx_data* fw_data_set[NUM_FW_DATA_SF];

    softfusesFW( const softfusesFW & );
    softfusesFW& operator=( const softfusesFW& );
};

#endif // SOFTFUSESFW_H
