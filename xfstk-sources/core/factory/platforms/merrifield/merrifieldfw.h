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
#ifndef MERRIFIELDFW_H
#define MERRIFIELDFW_H
#include "merrifieldutils.h"
#include "../../common/xfstkcommon.h"
#include <map>
#include "boost/lexical_cast.hpp"

#define FW_DATA_RUPHS   0
#define FW_DATA_RUPH	1
#define FW_DATA_IFWI	2
#define FW_DATA_CHFI00	3

const int FW_DATA_DNXH_MERRIFIELDUTILS =  4; //FW dnx header

#define FW_DATA_DNXD_MRFD	5 //FW dnx
#define FW_DATA_DNX0H_MRFD	6 //FW zero size dnx header data

#define FW_DATA_CSDB	7 //FW zero size dnx header data
#define NUM_FW_DATA_MRFD     8

#define MAX_CSDB_PAYLOAD 1024*128
#define FUPH_DWORD_SIZE 42
#define FUP_CHKSUM_OFFSET 3
#define FUP_LAST_CHUNK_OFFSET 38
#define CSDB_Version 0x1
using namespace std;

class MerrifieldFW
{
public:
    MerrifieldFW();
    ~MerrifieldFW();
    bool Init(char *fname_dnx_fw, char *fname_fw_image, char* fname_miscbin, string csdbStatus,
              MerrifieldUtils *utils, unsigned long gpflags, bool enable_ifwi_wipe,
              bool perform_emmc_dump);
    dnx_data* GetFwImageData(unsigned long index);
private:
    bool initChaabiSize();
	bool InitDnx();
	bool InitDnxHdr();
    bool InitCSDB(const string &csdbstatus);
	bool InitFwImage(bool enable_ifwi_wipe);
	bool InitNoSize();
	bool CheckFile(char *filename);
    bool restructFUPH();
    void footerChecksum();
	void LogError(int errorcode);
	void InitFuphHeaderData();
	bool FindFuphHeaderSignature();
    void FooterSizeInit();
	unsigned long GetFuphHeaderSize();
	unsigned long GetDataChunckSize( unsigned long tempData);

	char* m_fname_dnx_fw;
	char* m_fname_fw_image;
    char* m_fname_bin_misc;
	unsigned long m_fw_image_size;
	unsigned long m_fw_image_offset;
	unsigned char* m_dnx_fw_size_hdr;
	unsigned char* m_dnx_os_size_hdr; // dnx 0 size header data 
	unsigned char* m_dnx_fw;
	unsigned char* m_ifwi;
    unsigned char* m_ifwi_fw_block;
	unsigned char* m_fw_update_profile_hdr;
    unsigned char* m_dnx_chfi00;
    unsigned char* m_csdb;

	unsigned long m_dnx_fw_header_size;
	unsigned long m_dnx_fw_size;
    unsigned long m_dnx_fw_size_full;
	unsigned long m_fw_update_profile_hdr_size;
	unsigned long m_gpflags;
	unsigned long m_ifwi_size;
	unsigned long m_b_IDRQ;
	unsigned long m_byteCount;
	unsigned long m_i_offset;
	unsigned long m_tempData;
    unsigned long m_dnx_chfi00_size;
    unsigned int m_csdb_size;
    unsigned int m_footer_size;
    size_t m_fuph_location;
    size_t m_chaabi_token_size;
    size_t m_chaabi_FW_size;
    bool m_CSDBnIFWI;
    bool m_B0Plus;

	MerrifieldUtils* m_utils;
    dnx_data* fw_data_set[NUM_FW_DATA_MRFD];

	MerrifieldFW( const MerrifieldFW& );
	MerrifieldFW& operator=( const MerrifieldFW& );
};
#endif // MERRIFIELDFW_H
