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
#include <cstdlib>
#include <cstdio> 
#include <cstring>
#include "medfieldmessages.h"
#include <sys/stat.h>
#include <exception>
#include "../../common/xfstktypes.h"
#include "../../common/xfstkcommon.h"
#include "medfieldutils.h"
#include "medfieldfw.h"

using namespace std;

extern CPSTR Medfield_error_code_array [MAX_ERROR_CODE];  //JG - This is really used but it ijust a data array that we can just read.


//MedfieldFW Implementation
MedfieldFW::MedfieldFW()
{
    m_fname_dnx_fw = NULL;
    m_fname_fw_image =NULL;
    m_dnx_fw_size_hdr = NULL;
    m_dnx_fw = NULL;
    m_ifwi = NULL;
    m_fw_update_profile_hdr = NULL;
    m_romPatchPadding = NULL;
    m_ved_fw = NULL;
    m_rom_patch = NULL;
    m_padding_between_vedAndRomPatch = NULL;
    m_primary_security_fw_2 = NULL;
    m_secondary_security_fw = NULL;
    m_primary_security_fw_1 = NULL;
    m_first_96kb = NULL;
    m_second_96kb = NULL;
    m_third_96kb = NULL;
    m_lo_128kb = NULL;
    m_hi_128kb = NULL;
    m_mip_hdr = NULL;

    m_fw_image_size = 0;
    m_i_offset = 0;
    m_fw_update_profile_hdr_size = 0;
    m_gpflags = 0;
    m_mip_header_sizes =0;
    m_numChunks = 0;
    m_numResidentChunks = 0;
    m_primary_security_fw_1_size= 0;
    m_primary_security_fw_2_size= 0;
    m_secondary_security_fw_size= 0;
    m_rom_patch_size= 0;
    m_ved_fw_size= 0;
    m_romPatchPaddingSize= 0;
    m_paddingBetweenVedAndRomPatchSize= 0;
    m_ifwi_size= 0;
    m_numChunks= 0;
    m_numResidentChunks= 0;
    m_numRomPatchChunks = 0;
    m_b_IDRQ = 0;
    m_byteCount = 0;

    m_utils = NULL;
}

MedfieldFW::~MedfieldFW()
{
    if(m_dnx_fw_size_hdr)
        delete [] m_dnx_fw_size_hdr;
    if(m_dnx_os_size_hdr)
        delete [] m_dnx_os_size_hdr;
    if(m_dnx_fw)
        delete [] m_dnx_fw;
    if(m_ifwi)
        delete [] m_ifwi;
    if(m_fw_update_profile_hdr)
        delete [] m_fw_update_profile_hdr;
    if(m_romPatchPadding)
        delete [] m_romPatchPadding;
    if(m_ved_fw)
        delete [] m_ved_fw;
    if(m_rom_patch)
        delete [] m_rom_patch;
    if(m_padding_between_vedAndRomPatch)
        delete [] m_padding_between_vedAndRomPatch;
    if(m_primary_security_fw_2)
        delete [] m_primary_security_fw_2;
    if(m_secondary_security_fw)
        delete [] m_secondary_security_fw;
    if(m_primary_security_fw_1)
        delete [] m_primary_security_fw_1;
    if(m_first_96kb)
        delete [] m_first_96kb;
    if(m_second_96kb)
        delete [] m_second_96kb;
    if(m_third_96kb)
        delete [] m_third_96kb;
    if(m_lo_128kb)
        delete [] m_lo_128kb;
    if(m_hi_128kb)
        delete [] m_hi_128kb;
    if(m_mip_hdr)
        delete [] m_mip_hdr;

    for(int i = 0; i < NUM_FW_DATA_MFD; i++) {
       if(fw_data_set[i])
           delete fw_data_set[i];
   }

}

bool MedfieldFW::Init(char* fname_dnx_fw, char* fname_fw_image, 
                      MedfieldUtils* utils, unsigned long gpflags, bool enable_ifwi_wipe)
{
    bool ret = true;
    m_utils = utils;

    m_gpflags = gpflags;

    for(int i = 0; i < NUM_FW_DATA_MFD; i++) {
       fw_data_set[i] = new dnx_data;
       if(!fw_data_set[i])
           return false;
   }

    //This need to be done first since even no fwdnx/image, still need a no sixe header.
    if(InitNoSize()) {
        fw_data_set[FW_DATA_DNX0H_MFD]->size = DNX_FW_SIZE_HDR_SIZE;
        fw_data_set[FW_DATA_DNX0H_MFD]->data = m_dnx_os_size_hdr;
    }

    ret = CheckFile(fname_dnx_fw);
    if(!ret)
        return false;

    ret = CheckFile(fname_fw_image);
    if(!ret)
        return false;

    m_fname_dnx_fw = fname_dnx_fw;
    m_fname_fw_image = fname_fw_image;

    ret = InitDnxHdr();
    if(!ret)
        return false;

    ret = InitDnx();
    if(!ret)
        return false;

    ret = InitFwImage(enable_ifwi_wipe);
    if(!ret)
        return false;

    fw_data_set[FW_DATA_RUPHS]->size = 4;
    fw_data_set[FW_DATA_RUPHS]->data = (unsigned char*)&m_fw_update_profile_hdr_size;
    fw_data_set[FW_DATA_RUPH]->size = m_fw_update_profile_hdr_size;
    fw_data_set[FW_DATA_RUPH]->data = m_fw_update_profile_hdr;
    fw_data_set[FW_DATA_DMIP]->size = m_mip_header_sizes;
    fw_data_set[FW_DATA_DMIP]->data = m_mip_hdr;
    fw_data_set[FW_DATA_LOFW]->size = ONE28_K;
    fw_data_set[FW_DATA_LOFW]->data = m_lo_128kb;
    fw_data_set[FW_DATA_HIFW]->size = ONE28_K;
    fw_data_set[FW_DATA_HIFW]->data = m_hi_128kb;
    fw_data_set[FW_DATA_PSFW1]->size = m_primary_security_fw_1_size;
    fw_data_set[FW_DATA_PSFW1]->data = m_primary_security_fw_1;
    fw_data_set[FW_DATA_PSFW2]->size = m_primary_security_fw_2_size;
    fw_data_set[FW_DATA_PSFW2]->data = m_primary_security_fw_2;
    fw_data_set[FW_DATA_SSFW]->size = m_secondary_security_fw_size;
    fw_data_set[FW_DATA_SSFW]->data = m_secondary_security_fw;
    fw_data_set[FW_DATA_PATCH]->size = m_rom_patch_size;
    fw_data_set[FW_DATA_PATCH]->data = m_rom_patch;
    fw_data_set[FW_DATA_VEDFW]->size = m_ved_fw_size;
    fw_data_set[FW_DATA_VEDFW]->data = m_ved_fw;
    fw_data_set[FW_DATA_IFW1]->size = NINETY_SIX_KB;
    fw_data_set[FW_DATA_IFW1]->data = m_first_96kb;
    fw_data_set[FW_DATA_IFW2]->size = NINETY_SIX_KB;
    fw_data_set[FW_DATA_IFW2]->data = m_second_96kb;
    fw_data_set[FW_DATA_IFW3]->size = NINETY_SIX_KB;
    fw_data_set[FW_DATA_IFW3]->data = m_third_96kb;

	//Set the ifwi content to zero here if user ask to wipe out ifwi on emmc
	if(enable_ifwi_wipe) {
		for(int i = 2; i <= FW_DATA_IFW3; i++) {
		   if(fw_data_set[i]->data)
			   memset(fw_data_set[i]->data, 0, fw_data_set[i]->size);
	   }
	}

    fw_data_set[FW_DATA_DNXH_MEDFIELD]->size = m_dnx_fw_header_size;
    fw_data_set[FW_DATA_DNXH_MEDFIELD]->data = m_dnx_fw_size_hdr;
    fw_data_set[FW_DATA_DNXD_MFD]->size = m_dnx_fw_size;
    fw_data_set[FW_DATA_DNXD_MFD]->data = m_dnx_fw;
    fw_data_set[FW_DATA_DNX0H_MFD]->size = m_dnx_fw_header_size;
    fw_data_set[FW_DATA_DNX0H_MFD]->data = m_dnx_os_size_hdr;
    return ret;
}

bool MedfieldFW::InitDnxHdr()
{
    int ret_code = 1;
    bool ret = true;
    unsigned long reservedWord = 0;
    unsigned long dnx_size_checksum = 0;

    m_dnx_fw_size   = this->m_utils->FileSize(m_fname_dnx_fw);
    m_dnx_fw_header_size = DNX_FW_SIZE_HDR_SIZE;
    m_dnx_fw_size_hdr = new unsigned char[m_dnx_fw_header_size];
    if(!m_dnx_fw_size_hdr)
    {
        // Failed to allocate buffer for m_dnx_fw_size_hdr
        ret_code = 5;
        ret = false;
        LogError(ret_code);
    }
	else
	{
		dnx_size_checksum = m_dnx_fw_size ^ m_gpflags;
		memcpy(m_dnx_fw_size_hdr, (unsigned char *)&m_dnx_fw_size, 4);
		memcpy(m_dnx_fw_size_hdr + 4, (unsigned char *)&m_gpflags, 4);
		memcpy(m_dnx_fw_size_hdr + 8, (unsigned char *)&reservedWord, 4);
		memcpy(m_dnx_fw_size_hdr + 12, (unsigned char *)&reservedWord, 4);
		memcpy(m_dnx_fw_size_hdr + 16, (unsigned char *)&reservedWord, 4);
		memcpy(m_dnx_fw_size_hdr + 20, (unsigned char *)&dnx_size_checksum, 4);	
	}

    return ret;
}

bool MedfieldFW::InitDnx()
{
    int ret_code = 0;
    unsigned long read_cnt;
    FILE *fp_dnx_fw = NULL;

    try {
        m_dnx_fw_size   = this->m_utils->FileSize(m_fname_dnx_fw);

        m_dnx_fw = new unsigned char[m_dnx_fw_size];
        if(!m_dnx_fw)
            throw 5;

        // load buffers in order so file pointer moves along
        fp_dnx_fw = fopen(m_fname_dnx_fw, "rb");
        if(fp_dnx_fw){
            rewind(fp_dnx_fw);
            read_cnt = fread(m_dnx_fw, sizeof(unsigned char), m_dnx_fw_size, fp_dnx_fw);
            if(read_cnt != m_dnx_fw_size)
			{
				if( fp_dnx_fw )
					fclose( fp_dnx_fw );
                throw 6;
			}
        }
    }
    catch( int errorcode ) {
        ret_code = errorcode;
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Exception raised: ErrorCode %d", __FUNCTION__, errorcode);
        LogError(errorcode);
    }
    catch(std::exception & e)
    {
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Exception raised %s", __FUNCTION__, e.what());
        ret_code = 1;
    }
    catch( ... ) {
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Unknown Exception raised", __FUNCTION__);
        ret_code = 1;
    }

    if(fp_dnx_fw)
        fclose(fp_dnx_fw);
    return ret_code ? false:true;
}

void MedfieldFW::InitFuphHeaderData()
{
    m_tempData = m_tempData ^ m_ifwi[m_i_offset + 3];
    m_tempData = m_tempData << 8;
    m_tempData = m_tempData ^ m_ifwi[m_i_offset + 2];
    m_tempData = m_tempData << 8;
    m_tempData = m_tempData ^ m_ifwi[m_i_offset + 1];
    m_tempData = m_tempData << 8;
    m_tempData = m_tempData ^ m_ifwi[m_i_offset];
}

bool MedfieldFW::FindFuphHeaderSignature()
{
    const UINT fuphSignature = 0x55504824;//UPH$
    m_fw_image_size = this->m_utils->FileSize(m_fname_fw_image);
    m_i_offset = m_fw_image_size - 512;  // To check only the Last 512 byte Chunk of the IFWI for FUPH Header
    while(( m_i_offset < m_fw_image_size))
    {
        if(m_tempData == fuphSignature)
            return true;
        m_tempData = m_tempData << 8;
        m_tempData = m_tempData ^ m_ifwi[m_i_offset + 1];
        //if FUPH header not found then increment index
        m_i_offset++;
    }
    return false;
}

unsigned int MedfieldFW::GetFuphHeaderSize()
{
    //How many bytes in the FUPH
    unsigned int fuphheadersize = 0;
    ULONGLONG byteCount = 4;// Byte count is set to 4 since the adding begins after
    uint32 j = m_i_offset + 1;
    while(j < m_fw_image_size)
    {
        byteCount ++;
        j++;
    }
    if(byteCount == C0_FW_UPDATE_PROFILE_HDR_SIZE)
    {
        //new FUPH of 32 bytes
        fuphheadersize = C0_FW_UPDATE_PROFILE_HDR_SIZE;
    }
    else if(byteCount == D0_FW_UPDATE_PROFILE_HDR_SIZE)
    {
        //new FUPH of 36 bytes
        fuphheadersize = D0_FW_UPDATE_PROFILE_HDR_SIZE;
    }
    else
    {
        //Old FUPH of 28 bytes
        fuphheadersize = FW_UPDATE_PROFILE_OLD_HDR_SIZE_MFD;
    }
    return fuphheadersize;
}

unsigned int MedfieldFW::GetDataChunckSize( unsigned int tempData)
{
    tempData = tempData ^ m_ifwi[m_i_offset + 3];
    tempData = tempData << 8;
    tempData = tempData ^ m_ifwi[m_i_offset + 2];
    tempData = tempData << 8;
    tempData = tempData ^ m_ifwi[m_i_offset + 1];
    tempData = tempData << 8;
    tempData = tempData ^ m_ifwi[m_i_offset];
    return tempData *=4;
}

void MedfieldFW::LogError(int errorcode)
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    if(errorcode < MAX_ERROR_CODE)
        this->m_utils->u_abort("Error Code: %d - %s", errorcode, Medfield_error_code_array[errorcode]);
}

bool MedfieldFW::InitFwImage(bool )
{
    int ret_code = 0;
    bool ret = true;
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);


    // fw sections buffers

    ULONG read_cnt = 0;

    uint32 fw_image_size = this->m_utils->FileSize(m_fname_fw_image);

    UINT IsSecondStageBIOS = 0;
    FILE* fp_fw_image = NULL;

    this->m_utils->u_log(LOG_FWUPGRADE, "allocating buffers for FW images...");
    try{
        m_ifwi = new UCHAR[fw_image_size];

        //Open file
        fp_fw_image = fopen(m_fname_fw_image, "rb");
        if(fp_fw_image)
        {
            //read image into 'm_ifwi' buffer
            if(fread(m_ifwi, fw_image_size, 1, fp_fw_image)== 0)
			{
				if( fp_fw_image )
					fclose( fp_fw_image );
                throw 8;
			}
        } else {
            throw 8;
        }


        fseek(fp_fw_image, 0, SEEK_SET); // seek back to beginning of file
        int totalIfwiSize = 0;
        //the 4 bytes of UPH$ is found, therefore it is offset by 4


        InitFuphHeaderData();

        if(FindFuphHeaderSignature()) {

            m_fw_update_profile_hdr_size = GetFuphHeaderSize();

            m_fw_update_profile_hdr = new UCHAR[m_fw_update_profile_hdr_size];
            if(!m_fw_update_profile_hdr)
			{
				if( fp_fw_image )
					fclose( fp_fw_image );
                throw 5;
			}

            this->m_utils->u_log(LOG_FWUPGRADE, "allocating buffers for primary/secondary security FW images...");

            //Read MIP size
            m_i_offset +=1;
            m_mip_header_sizes = GetDataChunckSize(m_mip_header_sizes);

            //***Allocate the MIP Buffers****
            if(m_mip_header_sizes == 0x0200)
            {
                m_mip_hdr = new UCHAR[TWO_K];
                if(!m_mip_hdr)
				{
					if( fp_fw_image )
						fclose( fp_fw_image );
                    throw 5;
				}

                //Keep track of Ifwi Size
                totalIfwiSize += m_mip_header_sizes;
            }
            else
            {
                //Extended SMIP i.e. B0 SMIP
                m_mip_hdr = new UCHAR[ONE28_K];
                if(!m_mip_hdr)
				{
					if( fp_fw_image )
						fclose( fp_fw_image );
                    throw 5;
				}
                //Keep track of Ifwi Size
                totalIfwiSize += m_mip_header_sizes;
            }

            //Read IFWI Size
            m_i_offset+=4; //Next DWORD
            m_ifwi_size = GetDataChunckSize(m_ifwi_size);
            totalIfwiSize += m_ifwi_size;

            //Allocate IFWI Buffers based on the IFWI Size
            //As a change in the IFWI size indicates a different target platform
            if(m_ifwi_size == 0x40000)//256KB (MEDFIELD)
            {
                m_lo_128kb = new UCHAR[ONE28_K];
                if(!m_lo_128kb)
				{
					if( fp_fw_image )
						fclose( fp_fw_image );
                    throw 5;
				}

                m_hi_128kb = new UCHAR[ONE28_K];
                if(!m_hi_128kb)
				{
					if( fp_fw_image )
						fclose( fp_fw_image );
                    throw 5;
				}
            }
            else if(m_ifwi_size == 0x48000) //288KB (CloverTrail)
            {
                m_first_96kb = new UCHAR[NINETY_SIX_KB];
                if(!m_first_96kb)
				{
					if( fp_fw_image )
						fclose( fp_fw_image );
                    throw 5;
				}

                m_second_96kb = new UCHAR[NINETY_SIX_KB];
                if(!m_second_96kb)
				{
					if( fp_fw_image )
						fclose( fp_fw_image );
                    throw 5;
				}

                m_third_96kb = new UCHAR[NINETY_SIX_KB];
                if(!m_third_96kb)
				{
					if( fp_fw_image )
						fclose( fp_fw_image );
                    throw 5;
				}
            }

            //
            //Read Security FW 1 Size
            m_i_offset+=4;
            m_primary_security_fw_1_size = GetDataChunckSize(m_primary_security_fw_1_size);
            totalIfwiSize += m_primary_security_fw_1_size;
            //
            //Read Security FW 2 Size
            m_i_offset+=4;
            m_primary_security_fw_2_size = GetDataChunckSize(m_primary_security_fw_2_size);
            totalIfwiSize += m_primary_security_fw_2_size;

            //Read secondary security fw size
            m_i_offset+=4;
            m_secondary_security_fw_size = GetDataChunckSize(m_secondary_security_fw_size);
            totalIfwiSize += m_secondary_security_fw_size;


            if(m_fw_update_profile_hdr_size == C0_FW_UPDATE_PROFILE_HDR_SIZE)
            {
                //Read ROM Patch Size
                m_i_offset+=4;
                m_rom_patch_size = GetDataChunckSize(m_rom_patch_size);
            }
            else if(m_fw_update_profile_hdr_size == D0_FW_UPDATE_PROFILE_HDR_SIZE)
            {
                //Read ROM Patch Size
                m_i_offset+=4;
                m_rom_patch_size = GetDataChunckSize(m_rom_patch_size);
                //Read VED FW Size
                m_i_offset+=4;
                m_ved_fw_size = GetDataChunckSize(m_ved_fw_size);
            }
            else
            {
                m_rom_patch_size = 0;
                m_ved_fw_size = 0;
            }

            if ( m_ved_fw_size > TWO_HUNDRED_KB && fw_image_size > TWO_MB && m_ifwi_size == 0x48000 )
            {
                IsSecondStageBIOS = 1;
            }

            if ( IsSecondStageBIOS == 0 )
            {
                //Calculate the ROM Patch Padding when IFWI includes path
                if(m_rom_patch_size > 0)
                {
                    if(0 == m_ved_fw_size)
                    {
                        int endOfPadding = fw_image_size - (m_rom_patch_size + m_fw_update_profile_hdr_size);//endOfPaddin = 0x1D8000
                        m_romPatchPaddingSize = endOfPadding - totalIfwiSize;
                    }
                    else //ROM Patch and VED FW
                    {
                        m_paddingBetweenVedAndRomPatchSize = 0x1D8000 - (0x1A6000 + m_ved_fw_size);
                        int endOfPadding = fw_image_size - (m_ved_fw_size + m_paddingBetweenVedAndRomPatchSize + m_rom_patch_size + m_fw_update_profile_hdr_size);//endOfPaddin = 0x1A6000
                        m_romPatchPaddingSize = endOfPadding - totalIfwiSize;
                    }
                }
                else //No ROM Patch in IFWI
                {
                    if(0 != m_ved_fw_size) //VED FW available
                    {
                        int endOfPadding = fw_image_size - (m_ved_fw_size + m_fw_update_profile_hdr_size);//endOfPaddin = 0x1A6000
                        m_romPatchPaddingSize = endOfPadding - totalIfwiSize;
                    }
                }
            }
            else // If Second Stage BIOS is there
            {
                //Calculate the ROM Patch Padding when IFWI includes path
                if(m_rom_patch_size > 0)
                {
                    int endOfPadding = fw_image_size - (m_rom_patch_size + m_ved_fw_size + m_fw_update_profile_hdr_size);//endOfPaddin = 0x1D8000
                    m_romPatchPaddingSize = endOfPadding - totalIfwiSize;
                }
            }

            printf("%u\n", m_rom_patch_size);

            if(m_primary_security_fw_1_size > 0){
                m_primary_security_fw_1 = new UCHAR[m_primary_security_fw_1_size];
                if(!m_primary_security_fw_1)
				{
					if( fp_fw_image )
						fclose( fp_fw_image );
                    throw 5;
				}
            }
            if(m_primary_security_fw_2_size > 0){
                m_primary_security_fw_2 = new UCHAR[m_primary_security_fw_2_size];
                if(!m_primary_security_fw_2)
				{
					if( fp_fw_image )
						fclose( fp_fw_image );
                    throw 5;
				}
            }
            if(m_secondary_security_fw_size > 0){
                m_secondary_security_fw = new UCHAR[m_secondary_security_fw_size];
                if(!m_secondary_security_fw)
				{
					if( fp_fw_image )
						fclose( fp_fw_image );
                    throw 5;
				}
            }

            if ( IsSecondStageBIOS == 0 )
            {
                //Verify VED FW / ROM Patch Size Calculation and Allocate Buffer
                if(m_ved_fw_size > 0 || m_rom_patch_size > 0)
                {
                    //Allocate for rom patch padding
                    m_romPatchPadding = new UCHAR[m_romPatchPaddingSize];
                    if(!m_romPatchPadding)
					{
						if( fp_fw_image )
							fclose( fp_fw_image );
                        throw 5;
					}
                }
                //Allocate ROM Patch Buffer
                if(m_rom_patch_size > 0)
                {
                    //Allocate for rom patch
                    m_rom_patch = new UCHAR[m_rom_patch_size];
                    if(!m_rom_patch)
					{
						if( fp_fw_image )
							fclose( fp_fw_image );
                        throw 5;
					}
                }
                if(m_ved_fw_size > 0)
                {
                    //Allocate for ved FW
                    m_ved_fw = new UCHAR[m_ved_fw_size];
                    if(!m_ved_fw)
					{
						if( fp_fw_image )
							fclose( fp_fw_image );
                        throw 5;
					}

                    //VED Image is not full 200KB, allocate for padding
                    if(m_ved_fw_size < TWO_HUNDRED_KB)
                    {
                        m_padding_between_vedAndRomPatch = new UCHAR[m_paddingBetweenVedAndRomPatchSize];
                        if(!m_padding_between_vedAndRomPatch)
						{
							if( fp_fw_image )
								fclose( fp_fw_image );
                            throw 5;
						}
                    }

                }
            }
            else // If Second Stage BIOS is there
            {
                //Allocate ROM Patch Buffer
                if(m_rom_patch_size > 0)
                {
                    //Allocate for rom patch padding
                    m_romPatchPadding = new UCHAR[m_romPatchPaddingSize];
                    if(!m_romPatchPadding)
                    {
                        // Failed to allocate buffer
                        ret_code = 5;
                    }

                    //Allocate for rom patch
                    m_rom_patch = new UCHAR[m_rom_patch_size];
                    if(!m_rom_patch)
                    {
                        // Failed to allocate buffer
                        ret_code = 5;
                    }
                }
                if(m_ved_fw_size > 0)
                {
                    //Allocate for ved FW
                    m_ved_fw = new UCHAR[m_ved_fw_size];
                    if(!m_ved_fw)
                    {
                        // Failed to allocate buffer
                        ret_code = 5;
                    }
                }
            }
            this->m_utils->u_log(LOG_FWUPGRADE, "loading buffers for FW images...");
            // load buffers in order so file pointer moves along
            rewind (fp_fw_image);
            //Read in the MIP based on size
            if(m_mip_header_sizes == 0x0200)
            {
                read_cnt = fread(m_mip_hdr, sizeof(UCHAR), TWO_K, fp_fw_image);
                if(read_cnt != TWO_K)
				{
					if( fp_fw_image )
						fclose( fp_fw_image );
                    throw 6;
				}
            }
            else
            {
				if( m_mip_hdr ) delete [] m_mip_hdr;
				m_mip_hdr = new UCHAR[ ONE28_K ];
                read_cnt = fread(m_mip_hdr, sizeof(UCHAR), ONE28_K, fp_fw_image);
                if(read_cnt != ONE28_K)
				{
					if( fp_fw_image )
						fclose( fp_fw_image );
                    throw 6;
				}
            }

            //READ the IFWI from file based on the IFWI Size
            if(m_ifwi_size == 0x40000)//256KB (Medfield)
            {
                read_cnt = fread(m_lo_128kb, sizeof(UCHAR), ONE28_K, fp_fw_image);
                if(read_cnt != ONE28_K)
				{
					if( fp_fw_image )
						fclose( fp_fw_image );
                    throw 6;
				}

                read_cnt = fread(m_hi_128kb, sizeof(UCHAR), ONE28_K, fp_fw_image);
                if(read_cnt != ONE28_K)
				{
					if( fp_fw_image )
						fclose( fp_fw_image );
                    throw 6;
				}
            }
            else if(m_ifwi_size == 0x48000)//288KB (CloverTrail)
            {
                read_cnt = fread(m_first_96kb, sizeof(UCHAR), NINETY_SIX_KB, fp_fw_image);
                if(read_cnt != NINETY_SIX_KB)
				{
					if( fp_fw_image )
						fclose( fp_fw_image );
                    throw 6;
				}

                read_cnt = fread(m_second_96kb, sizeof(UCHAR), NINETY_SIX_KB, fp_fw_image);
                if(read_cnt != NINETY_SIX_KB)
				{
					if( fp_fw_image )
						fclose( fp_fw_image );
                    throw 6;
				}

                read_cnt = fread(m_third_96kb, sizeof(UCHAR), NINETY_SIX_KB, fp_fw_image);
                if(read_cnt != NINETY_SIX_KB)
				{
					if( fp_fw_image )
						fclose( fp_fw_image );
                    throw 6;
				}
            }

            this->m_utils->u_log(LOG_FWUPGRADE, "loading buffers for primary/secondary security FW images...");

            //iCache FW
            if(m_primary_security_fw_1_size > 0)
            {
                read_cnt = fread(m_primary_security_fw_1, sizeof(UCHAR), m_primary_security_fw_1_size, fp_fw_image);
                if(read_cnt != m_primary_security_fw_1_size)
				{
					if( fp_fw_image )
						fclose( fp_fw_image );
                    throw 6;
				}
            }

            //Extended FW
            if(m_secondary_security_fw_size > 0)
            {
                read_cnt = fread(m_secondary_security_fw, sizeof(UCHAR), m_secondary_security_fw_size, fp_fw_image);
                if(read_cnt != m_secondary_security_fw_size)
				{
					if( fp_fw_image )
						fclose( fp_fw_image );
                    throw 6;
				}
            }

            //Resident FW
            if(m_primary_security_fw_2_size > 0)
            {
                read_cnt = fread(m_primary_security_fw_2, sizeof(UCHAR), m_primary_security_fw_2_size, fp_fw_image);
                if(read_cnt != m_primary_security_fw_2_size)
				{
					if( fp_fw_image )
						fclose( fp_fw_image );
                    throw 6;
				}
            }

            if ( IsSecondStageBIOS == 0 )
            {
                //Rom Patch
                if(m_rom_patch_size > 0)
                {

                    //Read in the Padding between the Last Chaabi (security) FW image
                    //and ROM Patch or VED FW
                    read_cnt = fread(m_romPatchPadding, sizeof(UCHAR), m_romPatchPaddingSize, fp_fw_image);
                    if(read_cnt != m_romPatchPaddingSize)
					{
						if( fp_fw_image )
							fclose( fp_fw_image );
                        throw 6;
					}

                    if(0 == m_ved_fw_size)
                    {
                        //Read in ROM Patch
                        read_cnt = fread(m_rom_patch, sizeof(UCHAR), m_rom_patch_size, fp_fw_image);
                        if(read_cnt != m_rom_patch_size)
						{
							if( fp_fw_image )
								fclose( fp_fw_image );
                            throw 6;
						}
                    }
                    else //There is VED FW in the IFWI
                    {
                        //Read in VED FW
                        read_cnt = fread(m_ved_fw, sizeof(UCHAR), m_ved_fw_size, fp_fw_image);
                        if(read_cnt != m_ved_fw_size)
						{
							if( fp_fw_image )
								fclose( fp_fw_image );
                            throw 6;
						}

                        //VED FW is not full 200KB so read the padding between
                        //VED FW and ROM Patch
                        if(m_paddingBetweenVedAndRomPatchSize > 0)
                        {
                            //Read in the Padding Between Ved FW and ROM Patch
                            read_cnt = fread(m_padding_between_vedAndRomPatch, sizeof(UCHAR), m_paddingBetweenVedAndRomPatchSize, fp_fw_image);
                            if(read_cnt != m_paddingBetweenVedAndRomPatchSize)
							{
								if( fp_fw_image )
									fclose( fp_fw_image );
                                throw 6;
							}
                        }
                        //Read in ROM Patch
                        read_cnt = fread(m_rom_patch, sizeof(UCHAR), m_rom_patch_size, fp_fw_image);
                        if(read_cnt != m_rom_patch_size)
						{
							if( fp_fw_image )
								fclose( fp_fw_image );
                            throw 6;
						}
                    }
                }
                else //No Patch attached
                {
                    //VED FW only
                    if(m_ved_fw_size > 0)
                    {
                        //Read in the Padding between the Last Chaabi (security) FW image
                        //and VED FW
                        read_cnt = fread(m_romPatchPadding, sizeof(UCHAR), m_romPatchPaddingSize, fp_fw_image);
                        if(read_cnt != m_romPatchPaddingSize)
						{
							if( fp_fw_image )
								fclose( fp_fw_image );
                            throw 6;
						}

                        //Read in VED FW
                        read_cnt = fread(m_ved_fw, sizeof(UCHAR), m_ved_fw_size, fp_fw_image);
                        if(read_cnt != m_ved_fw_size)
						{
							if( fp_fw_image )
								fclose( fp_fw_image );
                            throw 6;
						}
                    }
                }
            }
            else // If Second stage BIOS is there
            {
                //Rom Patch
                if(m_rom_patch_size > 0)
                {

                    //Read in the Padding between the Last Chaabi (security) FW image
                    //and ROM Patch or VED FW

					if( m_romPatchPadding ) delete [] m_romPatchPadding;
					m_romPatchPadding = new UCHAR[ m_romPatchPaddingSize ];
                    read_cnt = fread( m_romPatchPadding, sizeof(UCHAR), m_romPatchPaddingSize, fp_fw_image);
                    if(read_cnt != m_romPatchPaddingSize)
                    {
						if( fp_fw_image )
							fclose( fp_fw_image );
                        // Error reading ROM Patch padding from file
                         throw 6;
                    }
                    //Read in ROM Patch
					if( m_rom_patch ) delete [] m_rom_patch;
					m_rom_patch = new UCHAR[ m_rom_patch_size ];
                    read_cnt = fread(m_rom_patch, sizeof(UCHAR), m_rom_patch_size, fp_fw_image);
                    if(read_cnt != m_rom_patch_size)
                    {
						if( fp_fw_image )
							fclose( fp_fw_image );
                        // Error reading ROM Patch from file
                         throw 6;
                    }
                }
                //VED FW only
                if(m_ved_fw_size > 0)
                {
					if( m_ved_fw ) delete [] m_ved_fw;
					m_ved_fw = new UCHAR[ m_ved_fw_size ];
                    //Read in VED FW
                    read_cnt = fread(m_ved_fw, sizeof(UCHAR), m_ved_fw_size, fp_fw_image);
                    if(read_cnt != m_ved_fw_size)
                    {
						if( fp_fw_image )
							fclose( fp_fw_image );
                        // Error reading VED FW from file
                         throw 6;
                    }
                }
            }
            this->m_utils->u_log(LOG_FWUPGRADE, "loading buffers for FUPH...");
            read_cnt = fread(m_fw_update_profile_hdr, sizeof(UCHAR), m_fw_update_profile_hdr_size, fp_fw_image);
            if(read_cnt != m_fw_update_profile_hdr_size)
			{
				if( fp_fw_image )
					fclose( fp_fw_image );

                throw 6;
			}

        } else {

			if( fp_fw_image )
				fclose( fp_fw_image );
            throw "Ifwi image didn't include a fuph header";
        }

        if(fp_fw_image)
            fclose(fp_fw_image);

        if(ret_code != 0)
            ret = false;

    }
    catch( int errorcode ) {
        ret_code = errorcode;
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Exception raised: ErrorCode %d", __FUNCTION__, errorcode);
        LogError(errorcode);
        if(fp_fw_image)
            fclose(fp_fw_image);

    }
    catch(std::exception & e)
    {
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Exception raised %s", __FUNCTION__, e.what());
        ret_code = 1;
    }
    catch( ... ) {
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Unknown Exception raised", __FUNCTION__);
        ret_code = 1;
        if(fp_fw_image)
            fclose(fp_fw_image);
    }
	
    return ret;
}

bool MedfieldFW::InitNoSize()
{
    bool ret = true;
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    ULONG reservedWord = 0;
    ULONG dnx_size_checksum = 0 ^ m_gpflags;
    ULONG dnx_os_size=0;
    ULONG dnx_header_size = DNX_FW_SIZE_HDR_SIZE;

    m_dnx_os_size_hdr = new UCHAR[dnx_header_size];
    if(m_dnx_os_size_hdr) {
        memcpy(m_dnx_os_size_hdr, (UCHAR *)&dnx_os_size, 4);
        memcpy(m_dnx_os_size_hdr + 4, (UCHAR *)&m_gpflags, 4);
        memcpy(m_dnx_os_size_hdr + 8, (UCHAR *)&reservedWord, 4);
        memcpy(m_dnx_os_size_hdr + 12, (UCHAR *)&reservedWord, 4);
        memcpy(m_dnx_os_size_hdr + 16, (UCHAR *)&reservedWord, 4);
        memcpy(m_dnx_os_size_hdr + 20, (UCHAR *)&dnx_size_checksum, 4);
    } else {
        ret = false;
    }
    return ret;
}

dnx_data* MedfieldFW::GetFwImageData(unsigned long index)
{
    if(index >= NUM_FW_DATA_MFD)
        return NULL;

    if(fw_data_set[index])
        return fw_data_set[index];
    else
        return NULL;
}

bool MedfieldFW::CheckFile(char *filename)
{
    bool ret = true;
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    FILE *fp;

    fp = fopen(filename, "rb" );
    if ( fp == NULL) {
        this->m_utils->u_abort("File %s cannot be opened", filename);
        ret = false;
    }
    struct stat file_info;
    if(stat(filename, &file_info)) {
        this->m_utils->u_abort("Failed to stat file: %s", filename);
        ret = false;
    }

    if(fp)
        fclose(fp);
    return ret;
}

