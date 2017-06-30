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
#ifndef EMMCFW_H
#define EMMCFW_H
#include "emmcutils.h"
#include "../../common/xfstkcommon.h"

#define FW_DATA_RUPHS   0
#define FW_DATA_RUPH	1
#define FW_DATA_DMIP	2
#define FW_DATA_LOFW	3
#define FW_DATA_HIFW	4
#define FW_DATA_PSFW1	5
#define FW_DATA_PSFW2	6
#define FW_DATA_SSFW	7
#define FW_DATA_PATCH	8
#define FW_DATA_VEDFW	9
#define FW_DATA_IFW1	10
#define FW_DATA_IFW2	11
#define FW_DATA_IFW3	12

const int FW_DATA_DNXH_EMMC =  13; //FW dnx header

#define FW_DATA_DNXD_EMMC	14 //FW dnx
#define FW_DATA_DNX0H_EMMC	15 //FW zero size dnx header data

#define NUM_FW_DATA_EMMC 16

class EmmcFW
{
public:
	EmmcFW();
	~EmmcFW();
	bool Init(char* fname_dnx_fw, char* fname_fw_image, 
					EmmcUtils* utils, unsigned long gpflags, bool enable_ifwi_wipe);
        dnx_data* GetFwImageData(unsigned long index);

private:
	bool InitDnx();
	bool InitDnxHdr();
	bool InitFwImage(bool enable_ifwi_wipe);
	bool InitNoSize();
        bool CheckFile(char *filename);
        void LogError(int errorcode);
        void InitFuphHeaderData();
        bool FindFuphHeaderSignature();
        unsigned long GetFuphHeaderSize();
        unsigned long GetDataChunckSize( unsigned long tempData);

	char* m_fname_dnx_fw;
	char* m_fname_fw_image;
        unsigned long m_fw_image_size;
        unsigned long m_fw_image_offset;
        unsigned char* m_dnx_fw_size_hdr;
	unsigned char* m_dnx_os_size_hdr; // dnx 0 size header data 
	unsigned char* m_dnx_fw;
	unsigned char* m_ifwi;
	unsigned char* m_fw_update_profile_hdr;
	unsigned char* m_romPatchPadding;
	unsigned char* m_ved_fw;
	unsigned char* m_rom_patch;
	unsigned char* m_padding_between_vedAndRomPatch;
	unsigned char* m_primary_security_fw_2;
	unsigned char* m_secondary_security_fw;
	unsigned char* m_primary_security_fw_1;
	unsigned char* m_first_96kb;
	unsigned char* m_second_96kb;
	unsigned char* m_third_96kb;
	unsigned char* m_lo_128kb;	
	unsigned char* m_hi_128kb;
	unsigned char* m_mip_hdr;

	unsigned long m_dnx_fw_header_size;
	unsigned long m_dnx_fw_size;
	unsigned long m_fw_update_profile_hdr_size;
	unsigned long m_gpflags;
	unsigned long m_mip_header_sizes;
	unsigned long m_numChunks;
	unsigned long m_numResidentChunks;
    unsigned long m_primary_security_fw_1_size;
    unsigned long m_primary_security_fw_2_size;
    unsigned long m_secondary_security_fw_size;
    unsigned long m_rom_patch_size;
    unsigned long m_ved_fw_size;
    unsigned long m_romPatchPaddingSize;
    unsigned long m_paddingBetweenVedAndRomPatchSize;
    unsigned long m_ifwi_size;
	unsigned long m_numRomPatchChunks;
	unsigned long m_b_IDRQ;
    unsigned long m_byteCount;
    unsigned long m_i_offset;
    unsigned long m_tempData;

	EmmcUtils* m_utils;
        dnx_data* fw_data_set[NUM_FW_DATA_EMMC];

    EmmcFW( const EmmcFW& );
	EmmcFW& operator=( const EmmcFW& );
};
#endif //EMMCFW_H
