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
#include <cstdlib>
#include <cstdio>
#include <sys/stat.h>
#include <cstring>
#include <exception>
#include "../../common/xfstktypes.h"
#include "../../common/xfstkcommon.h"
#include "../cloverviewplus/cloverviewplusutils.h"
#include "softfusesfw.h"
#include "softfusesMessages.h"
#include "../cloverviewplus/cloverviewplusmessages.h"

extern CPSTR CloverviewPlus_error_code_array [MAX_ERROR_CODE_CLOVERVIEWPLUS];
using namespace std;

softfusesFW::softfusesFW()
{
    m_fname_softfuses_bin = NULL;
    m_softfuses_update_profile_hdr = NULL;
    m_softfuses_key_file = NULL;
    m_softfuses_bin = NULL;

    m_softfuses_update_profile_hdr_size = 0;
    m_softfuses_key_file_size = 0;
    m_softfuses_bin_size = 0;
    m_i_offset = 0;

    m_utils = NULL;
}

softfusesFW::~softfusesFW()
{

    if(m_softfuses_update_profile_hdr)
        delete m_softfuses_update_profile_hdr;
    if(m_softfuses_key_file)
        delete [] m_softfuses_key_file;
    if(m_softfuses_bin)
        delete [] m_softfuses_bin;
    if(m_sup_hdr_zero_size)
        delete [] m_sup_hdr_zero_size;

    for(int i = 0; i < NUM_FW_DATA_SF; i++) {
       if(fw_data_set[i])
           delete fw_data_set[i];
   }
}

bool softfusesFW::Init(char* fname_softfuses_bin, CloverviewPlusUtils* utils)
{

    bool ret = true;
    m_utils = utils;

    for(int i = 0; i < NUM_FW_DATA_SF; i++) {
       fw_data_set[i] = new dnx_data;
       if(!fw_data_set[i])
           return false;
   }

    if(InitNoSize()) {
        fw_data_set[SFUSE_BIN_SUP0H]->size = m_softfuses_update_profile_hdr_size;
        fw_data_set[SFUSE_BIN_SUP0H]->data = m_sup_hdr_zero_size;
    }

    ret = CheckFile(fname_softfuses_bin);
    if(!ret)
        return false;

    m_fname_softfuses_bin = fname_softfuses_bin;

    ret = InitSoftfusesBin();
    if(!ret)
        return false;

    fw_data_set[SFUSE_DATA_RSUPHS]->size = 4;
    fw_data_set[SFUSE_DATA_RSUPHS]->data = (unsigned char*)&m_softfuses_update_profile_hdr_size;
    fw_data_set[SFUSE_DATA_RSUPH]->size = m_softfuses_update_profile_hdr_size;
    fw_data_set[SFUSE_DATA_RSUPH]->data = m_softfuses_update_profile_hdr;
    fw_data_set[SFUSE_BIN_SUP0H]->size = m_softfuses_update_profile_hdr_size;
    fw_data_set[SFUSE_BIN_SUP0H]->data = m_sup_hdr_zero_size;
    fw_data_set[SFUSE_BIN_KEY_FILE]->size = m_softfuses_key_file_size;
    fw_data_set[SFUSE_BIN_KEY_FILE]->data = m_softfuses_key_file;
    return ret;
}

bool softfusesFW::InitSoftfusesBin()
{
    int ret_code = 0;
    bool ret = true;
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    uint32 softfuses_bin_size = this->m_utils->FileSize(m_fname_softfuses_bin);
    FILE* fp_fw_image = NULL;
    try{
        m_softfuses_bin = new UCHAR[softfuses_bin_size];

        //Open file
        fp_fw_image = fopen(m_fname_softfuses_bin, "rb");
        if(fp_fw_image)
        {
            //read image into 'm_ifwi' buffer
            if(fread(m_softfuses_bin, softfuses_bin_size, 1, fp_fw_image)== 0)
			{
				if( fp_fw_image )
					fclose(fp_fw_image);
                throw 8;
			}
        } else {
            throw 8;
        }

        ULONG read_cnt = 0;

        fseek(fp_fw_image, 0, SEEK_SET); // seek back to beginning of file

        softFusesDataSize = 0;
        softFusesKeyFileSize = 0;
        //the 4 bytes of UPH$ is found, therefore it is offset by 4


        InitSUPHeaderData();

        if(!FindSUPHeaderSignature())
        {
            this->m_utils->u_log(LOG_STATUS, "Unable to find SUP header");
        }

        m_softfuses_update_profile_hdr_size = GetSUPHeaderSize();

        m_softfuses_update_profile_hdr = new UCHAR[m_softfuses_update_profile_hdr_size];
        if(!m_softfuses_update_profile_hdr)
		{
			if( fp_fw_image )
				fclose(fp_fw_image);
            throw 5;
		}

        softFusesDataSize = m_utils->FileSize(m_fname_softfuses_bin) - m_softfuses_update_profile_hdr_size - \
                                      SFUSE_HEADER_SIZE - VRL_HEADER_SIZE_SF;
        softFusesKeyFileSize = softfuses_bin_size- m_softfuses_update_profile_hdr_size;
        m_softfuses_key_file_size = softFusesKeyFileSize;
        m_softfuses_key_file = new UCHAR[m_softfuses_key_file_size];
        if(!m_softfuses_key_file)
		{
			if( fp_fw_image )
				fclose(fp_fw_image);
            throw 5;
		}

        read_cnt = fread(m_softfuses_key_file, sizeof(UCHAR), m_softfuses_key_file_size, fp_fw_image);
        if(read_cnt != m_softfuses_key_file_size)
		{
			if( fp_fw_image )
				fclose(fp_fw_image);
            throw 6;
		}

        read_cnt = fread(m_softfuses_update_profile_hdr, sizeof(UCHAR), m_softfuses_update_profile_hdr_size, fp_fw_image);
        if(read_cnt != m_softfuses_update_profile_hdr_size)
		{
			if( fp_fw_image )
				fclose(fp_fw_image);
            throw 6;
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

    return ret;
}

void softfusesFW::LogError(int errorcode)
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    if(errorcode < MAX_ERROR_CODE_CLOVERVIEWPLUS)
        this->m_utils->u_abort("Error Code: %d - %s", errorcode, CloverviewPlus_error_code_array[errorcode]);
}



bool softfusesFW::InitNoSize()
{
    bool ret = true;
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    ULONG UPH_Signature_size = 0;
    ULONG Num_Keys = 0;
    ULONG Key_Size = 0;
    ULONG SUPH_Checksum = 0;

    m_sup_hdr_zero_size = new UCHAR[GetSUPHeaderSize()];
    if(m_sup_hdr_zero_size) {
        memcpy(m_sup_hdr_zero_size, (UCHAR *)&UPH_Signature_size, 4);
        memcpy(m_sup_hdr_zero_size + 4, (UCHAR *)&Num_Keys, 4);
        memcpy(m_sup_hdr_zero_size + 8, (UCHAR *)&Key_Size, 4);
        memcpy(m_sup_hdr_zero_size + 12, (UCHAR *)&SUPH_Checksum, 4);
    } else {
        ret = false;
    }
    return ret;
}

unsigned long softfusesFW::GetDataChunckSize( unsigned long tempData)
{
    tempData = tempData ^ m_softfuses_bin[m_i_offset + 3];
    tempData = tempData << 8;
    tempData = tempData ^ m_softfuses_bin[m_i_offset + 2];
    tempData = tempData << 8;
    tempData = tempData ^ m_softfuses_bin[m_i_offset + 1];
    tempData = tempData << 8;
    tempData = tempData ^ m_softfuses_bin[m_i_offset];
    return tempData *=4;
}


void softfusesFW::InitSUPHeaderData()
{
    m_tempData = m_tempData ^ m_softfuses_bin[m_i_offset + 3];
    m_tempData = m_tempData << 8;
    m_tempData = m_tempData ^ m_softfuses_bin[m_i_offset + 2];
    m_tempData = m_tempData << 8;
    m_tempData = m_tempData ^ m_softfuses_bin[m_i_offset + 1];
    m_tempData = m_tempData << 8;
    m_tempData = m_tempData ^ m_softfuses_bin[m_i_offset];
}

bool softfusesFW::FindSUPHeaderSignature()
{
    const UINT SUPHSignature = 0x55504824;//UPH$
    m_softfuses_bin_size = this->m_utils->FileSize(m_fname_softfuses_bin);
    m_i_offset = m_softfuses_bin_size/2;  // To check only the Last 512 byte Chunk of the IFWI for FUPH Header
    while(( m_i_offset < m_softfuses_bin_size))
    {
        if(m_tempData == SUPHSignature)
            return true;
        m_tempData = m_tempData << 8;
        m_tempData = m_tempData ^ m_softfuses_bin[m_i_offset + 1];
        //if SUP header not found then increment index
        m_i_offset++;
    }
    return false;
}

unsigned long softfusesFW::GetSUPHeaderSize()
{
    //How many bytes in the FUPH
    unsigned long SUPheadersize = 0;
    ULONGLONG byteCount = 4;// Byte count is set to 4 since the adding begins after
    uint32 j = m_i_offset + 1;
    while(j < m_softfuses_bin_size)
    {
        byteCount ++;
        j++;
    }
    if(byteCount == C0_FW_UPDATE_PROFILE_HDR_SIZE)
    {
        //new FUPH of 32 bytes
        SUPheadersize = C0_FW_UPDATE_PROFILE_HDR_SIZE;
    }
    else if(byteCount == D0_FW_UPDATE_PROFILE_HDR_SIZE)
    {
        //new FUPH of 36 bytes
        SUPheadersize = D0_FW_UPDATE_PROFILE_HDR_SIZE;
    }
    else
    {
        //Old FUPH of 28 bytes
        SUPheadersize = FW_UPDATE_PROFILE_OLD_HDR_SIZE_SF;
    }
    return SUPheadersize;
}

bool softfusesFW::CheckFile(char *filename)
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

dnx_data* softfusesFW::GetSoftFusesFileData(unsigned long index)
{
    if(index >= NUM_FW_DATA_SF)
        return NULL;

    if(fw_data_set[index])
        return fw_data_set[index];
    else
        return NULL;
}

