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
#ifndef MEDFIELDOS_H
#define MEDFIELDOS_H
#include <memory>
#include "../../common/xfstkcommon.h"


class MedfieldUtils;


class MedfieldOS
{
public:
    MedfieldOS();
    ~MedfieldOS();
    bool Init(char* dnx_os_name, char* os_image_name,
              MedfieldUtils* utils, unsigned long gpflags);
    dnx_data* GetOsipHdr();
    dnx_data* GetOsImageDataChunk();
    void ReleaseOsImageDataChunk();
    dnx_data* GetOsDnXHdr();
    dnx_data* GetOsDnX();
    dnx_data* GetNoSizeData();
    unsigned long long GetOsImageDataSize();

private:
    bool InitOsipHdr();
    bool InitOsImage();
    unsigned long GetFileSize(char* file);
    bool CheckFile(char *filename);
    void LogError(int errorcode);


    MedfieldUtils* m_utils;

    char* m_dnx_os_name;
    char* m_os_image_name;
    unsigned long m_gpflags;
    unsigned long m_b_DnX_OS;
    unsigned long m_hdr_size;
    FILE* m_fp_os_image;
    unsigned long m_dnx_os_size;
    unsigned char* m_dnx_os; 
    unsigned char* m_osip_hdr;
    unsigned long long m_os_data_size;
    unsigned char* m_pkt_buffer;
    unsigned int m_dnx_header_size;
    unsigned char* m_dnx_os_size_hdr;
    float m_ostotalsize;
    int m_osprogress;
    dnx_data m_osdata;

	MedfieldOS( const MedfieldOS& );
	MedfieldOS& operator=( const MedfieldOS& );
};
#endif //MFLDPOSDATA_H


