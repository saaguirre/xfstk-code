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
#include <sys/stat.h>
#include <boost/smart_ptr/scoped_array.hpp>
#include "merrifieldmessages.h"
#include "../../common/xfstktypes.h"
#include "merrifieldutils.h"
#include "merrifieldfw.h"
#include "../../common/scoped_file.h"

extern CPSTR Merrifield_error_code_array [MAX_ERROR_CODE];  //JG - This is really used but it ijust a data array that we can just read.

MerrifieldFW::MerrifieldFW()
{
    m_fname_dnx_fw = NULL;
    m_fname_fw_image =NULL;
    m_fname_bin_misc = NULL;

    m_dnx_fw_size_hdr = NULL;
    m_dnx_fw = NULL;
    m_ifwi = NULL;
    m_fw_update_profile_hdr = NULL;
    m_dnx_chfi00 = NULL;
    m_csdb = NULL;
    m_ifwi_fw_block = NULL;

    m_fw_image_size = 0;
    m_i_offset = 0;
    m_fw_update_profile_hdr_size = 0;
    m_gpflags = 0;
    m_ifwi_size= 0;
    m_b_IDRQ = 0;
    m_byteCount = 0;
    m_dnx_chfi00_size = 0;
    m_chaabi_token_size = 0;
    m_chaabi_FW_size = 0;
    m_csdb_size = 0;
    m_footer_size = 0;
    m_CSDBnIFWI = false;
    m_B0Plus =false;
    m_utils = NULL;
    m_fuph_location = std::string::npos;
}

MerrifieldFW::~MerrifieldFW()
{
    if(m_dnx_fw_size_hdr)
    {
        delete [] m_dnx_fw_size_hdr;
        m_dnx_fw_size_hdr = NULL;
    }
    if(m_dnx_os_size_hdr)
    {
        delete [] m_dnx_os_size_hdr;
        m_dnx_os_size_hdr = NULL;
    }
    if(m_dnx_fw)
    {
        delete [] m_dnx_fw;
        m_dnx_fw = NULL;
    }
    if(m_ifwi)
    {
        delete [] m_ifwi;
        m_ifwi = NULL;
    }
    if(m_dnx_chfi00)
    {
        delete [] m_dnx_chfi00;
        m_dnx_chfi00 = NULL;
    }
    if(m_fw_update_profile_hdr)
    {
        delete [] m_fw_update_profile_hdr;
        m_fw_update_profile_hdr = NULL;
    }
    if(m_csdb)
    {
        delete [] m_csdb;
        m_csdb = NULL;
    }
    for(int i = 0; i < NUM_FW_DATA_MRFD; i++)
    {
        if(fw_data_set[i])
        {
            delete fw_data_set[i];
            fw_data_set[i] = NULL;
        }
    }
    if(m_ifwi_fw_block)
    {
        delete [] m_ifwi_fw_block;
        m_ifwi_fw_block = NULL;
    }

}

bool MerrifieldFW::Init(char *fname_dnx_fw, char *fname_fw_image, char* fname_miscbin, string csdbStatus, MerrifieldUtils *utils, unsigned long gpflags, bool enable_ifwi_wipe)
{
    bool ret = true;
    m_utils = utils;

    m_gpflags = gpflags;

    for(int i = 0; i < NUM_FW_DATA_MRFD; i++) {
       fw_data_set[i] = new dnx_data;
       if(!fw_data_set[i])
           return false;
   }

    //This need to be done first since even no fwdnx/image, still need a no sixe header.
    if(InitNoSize()) {
        fw_data_set[FW_DATA_DNX0H_MRFD]->size = DNX_FW_SIZE_HDR_SIZE;
        fw_data_set[FW_DATA_DNX0H_MRFD]->data = m_dnx_os_size_hdr;
    }

    ret = CheckFile(fname_dnx_fw);
    if(!ret)
        return false;

    ret = CheckFile(fname_fw_image);
    if(!ret && (csdbStatus == " "))
        return false;


    m_fname_dnx_fw = fname_dnx_fw;
    m_fname_fw_image = fname_fw_image;
    m_fname_bin_misc = fname_miscbin;

    if(std::string(m_fname_bin_misc) != "BLANK.bin")
    {
        if(!CheckFile(fname_miscbin))
            return false;
    }


    ret = InitDnxHdr();
    if(!ret)
        return false;

    ret = InitDnx();
    if(!ret)
        return false;
    if(std::string(fname_fw_image) != "BLANK.bin")
    {
        ret = InitFwImage(enable_ifwi_wipe);
        if(!ret)
            return false;
    }
    //if there is no footer, the FUPH does not get flashed
    if(!m_footer_size)
    {
        fw_data_set[FW_DATA_IFWI]->size = static_cast<int>(m_fw_image_size - m_fw_update_profile_hdr_size);
    }
    //if there is a footer then fuph gets flashed
    else
    {
        fw_data_set[FW_DATA_IFWI]->size = static_cast<int>(m_fw_image_size);
    }
    fw_data_set[FW_DATA_IFWI]->data = m_ifwi_fw_block;

    if(csdbStatus != " " || m_CSDBnIFWI)
    {
        ret = InitCSDB(csdbStatus);
        if(!ret)
            return false;
        else
        {
            fw_data_set[FW_DATA_CSDB]->size = m_csdb_size;
            fw_data_set[FW_DATA_CSDB]->data = m_csdb;
        }

    }

    fw_data_set[FW_DATA_RUPHS]->size = 4;
    fw_data_set[FW_DATA_RUPHS]->data = (unsigned char*)&m_fw_update_profile_hdr_size;
    fw_data_set[FW_DATA_RUPH]->size = m_fw_update_profile_hdr_size;
    fw_data_set[FW_DATA_RUPH]->data = m_fw_update_profile_hdr;

    fw_data_set[FW_DATA_CHFI00]->size = m_dnx_chfi00_size;
    fw_data_set[FW_DATA_CHFI00]->data = m_dnx_chfi00;


    //Set the ifwi content to zero here if user ask to wipe out ifwi on emmc
    if(enable_ifwi_wipe) {
           if(fw_data_set[FW_DATA_IFWI]->data)
               memset(fw_data_set[FW_DATA_IFWI]->data, 0, fw_data_set[FW_DATA_IFWI]->size);
    }

    fw_data_set[FW_DATA_DNXH_MERRIFIELDUTILS]->size = m_dnx_fw_header_size;
    fw_data_set[FW_DATA_DNXH_MERRIFIELDUTILS]->data = m_dnx_fw_size_hdr;
    fw_data_set[FW_DATA_DNXD_MRFD]->size = m_dnx_fw_size;
    fw_data_set[FW_DATA_DNXD_MRFD]->data = m_dnx_fw;
    fw_data_set[FW_DATA_DNX0H_MRFD]->size = m_dnx_fw_header_size;
    fw_data_set[FW_DATA_DNX0H_MRFD]->data = m_dnx_os_size_hdr;
    return ret;
}

bool MerrifieldFW::InitDnxHdr()
{
    int ret_code = 1;
    bool ret = true;
    unsigned long reservedWord = 0;
    unsigned long dnx_size_checksum = 0;

    m_dnx_fw_size_full   = this->m_utils->FileSize(m_fname_dnx_fw);
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
        dnx_size_checksum = m_dnx_fw_size_full ^ m_gpflags;
        memcpy(m_dnx_fw_size_hdr, (unsigned char *)&m_dnx_fw_size_full, 4);
        memcpy(m_dnx_fw_size_hdr + 4, (unsigned char *)&m_gpflags, 4);
        memcpy(m_dnx_fw_size_hdr + 8, (unsigned char *)&reservedWord, 4);
        memcpy(m_dnx_fw_size_hdr + 12, (unsigned char *)&reservedWord, 4);
        memcpy(m_dnx_fw_size_hdr + 16, (unsigned char *)&reservedWord, 4);
        memcpy(m_dnx_fw_size_hdr + 20, (unsigned char *)&dnx_size_checksum, 4);
    }

    return ret;
}

bool MerrifieldFW::initChaabiSize()
{
    size_t ChPr, CH00, CDPH, CHT, DTKN = 0;

    bool tokenFound = false;
    bool fwfoundFound = false;

    //Chaabi token start at 128 bytes before $CHT in TNG A0
    CHT = this->m_utils->StringLocation(this->m_fname_dnx_fw,std::string("$CHT"));
    //Chaabi token start for token container in TNG B0+
    DTKN = this->m_utils->StringLocation(this->m_fname_dnx_fw,std::string("DTKN"));
    //Chaabi FW starts at 128 bytes befor CH00
    CH00 = this->m_utils->StringLocation(this->m_fname_dnx_fw,std::string("CH00"));
    //CDPH signifies the end of chaabi fw
    CDPH = this->m_utils->StringLocation(this->m_fname_dnx_fw,std::string("CDPH"));
    //Chaabi token start at ChPr for TNG B0/ANN
    ChPr = this->m_utils->StringLocation(this->m_fname_dnx_fw,std::string("ChPr"));


    if((CH00 != std::string::npos))

    {
        CH00 -= 0x80;
        if(DTKN != std::string::npos)
        {
            scoped_file file(m_fname_dnx_fw,"rb");
            size_t filesize = file.size();
            if(filesize <= DTKN+SIXTEEN_KB)
            {
                this->m_utils->u_log(LOG_FWUPGRADE,"Warning File size mismatch");
                return false;
            }

            size_t buffersize = filesize - (DTKN+SIXTEEN_KB);

            boost::scoped_array<char> tmpbuffer(new char[buffersize]);

            file.seek(DTKN+SIXTEEN_KB,SEEK_SET);
            file.read(tmpbuffer.get(),1,buffersize);

            CH00 = this->m_utils->StringLocation(tmpbuffer.get(),"CH00",buffersize);

            if(CH00 == string::npos)
            {
                this->m_utils->u_log(LOG_FWUPGRADE,"Warning unable to determine start of Chaabi Firmware (CH00)");
                return false;
            }
            //DnX token container is 16KB
            CH00 += DTKN+SIXTEEN_KB;
            CH00 -= 0x80;

            this->m_chaabi_token_size = CH00 - DTKN;
            tokenFound =  true;
        }
        //TNG A0
        else if(CHT != std::string::npos)
        {
            CHT -=  0x80;
            this->m_chaabi_token_size = CH00 - CHT;
            tokenFound =  true;
        }//TNG B0/ANN
        else if(ChPr != std::string::npos)
        {
            this->m_chaabi_token_size = CH00 - ChPr;
            tokenFound =  true;
        }

        if(CDPH != std::string::npos)
        {
            this->m_chaabi_FW_size = CDPH - CH00;
            fwfoundFound = true;
        }
    }
    return fwfoundFound & tokenFound;

}

bool MerrifieldFW::InitDnx()
{
    int ret_code = 0;
    unsigned long read_cnt;
    FILE *fp_dnx_fw = NULL;

    try {

        if(!this->initChaabiSize())
        {
            if(this->m_chaabi_token_size == 0)
            {
                this->m_utils->u_log(LOG_STATUS,"Unable to determine chaabi token size, setting to 12KB!!");
                this->m_chaabi_token_size = SIXTEEN_KB;
            }
            if(this->m_chaabi_FW_size == 0)
            {
                this->m_utils->u_log(LOG_STATUS,"Unable to determine chaabi fw size, setting to 72KB!!");
                this->m_chaabi_FW_size = SEVENTY_TWO_KB;
            }
        }
        m_dnx_fw_size_full   = this->m_utils->FileSize(m_fname_dnx_fw);

        if(m_dnx_fw_size_full >= (m_chaabi_token_size + m_chaabi_FW_size))
            m_dnx_fw_size = m_dnx_fw_size_full - (m_chaabi_token_size + m_chaabi_FW_size);

        else
            throw 5;

        m_dnx_fw = new unsigned char[m_dnx_fw_size];
        if(!m_dnx_fw)
            throw 5;

        m_dnx_chfi00_size = m_chaabi_token_size + m_chaabi_FW_size + CDPH_HEADER_SIZE;
        m_dnx_chfi00 = new unsigned char[m_dnx_chfi00_size];
        if(!m_dnx_chfi00)
            throw 5;



        // load buffers in order so file pointer moves along
        fp_dnx_fw = fopen(m_fname_dnx_fw, "rb");
        if(fp_dnx_fw)
        {
            rewind(fp_dnx_fw);
            //Move file pointer to CDPH and readin CDPH first
            fseek(fp_dnx_fw, (m_dnx_fw_size_full - CDPH_HEADER_SIZE), SEEK_SET);
            read_cnt = fread(m_dnx_fw, sizeof(unsigned char), CDPH_HEADER_SIZE, fp_dnx_fw);
            if(read_cnt != CDPH_HEADER_SIZE)
            {
                if( fp_dnx_fw )
                    fclose(fp_dnx_fw);
                
                throw 6;
            }

            //then move file pointer to beginning of the file, readin the VRL and SCU microcode
            rewind(fp_dnx_fw);
            read_cnt = fread((unsigned char*)(m_dnx_fw + CDPH_HEADER_SIZE), sizeof(unsigned char), (m_dnx_fw_size -CDPH_HEADER_SIZE), fp_dnx_fw);
            if(read_cnt != (m_dnx_fw_size - CDPH_HEADER_SIZE))
            {
                if( fp_dnx_fw )
                    fclose(fp_dnx_fw);
                throw 6;
            }

            rewind(fp_dnx_fw);
            //Move file pointer to CDPH and readin CDPH first
            fseek(fp_dnx_fw, (m_dnx_fw_size_full - CDPH_HEADER_SIZE), SEEK_SET);
            read_cnt = fread(m_dnx_chfi00, sizeof(unsigned char), CDPH_HEADER_SIZE, fp_dnx_fw);
           
            if(read_cnt != CDPH_HEADER_SIZE)
            {
                if( fp_dnx_fw )
                    fclose(fp_dnx_fw);
                throw 6;
            }
            //then move file pointer to beginning of the file, readin the Chaabi code
            rewind(fp_dnx_fw);
            //Move file pointer to VRL for chaabi

            fseek(fp_dnx_fw, (m_dnx_fw_size_full - m_chaabi_token_size - m_chaabi_FW_size - CDPH_HEADER_SIZE), SEEK_SET);
            read_cnt = fread((unsigned char*)(m_dnx_chfi00 + CDPH_HEADER_SIZE), sizeof(unsigned char), (m_chaabi_token_size + m_chaabi_FW_size), fp_dnx_fw);
 
            if(read_cnt != (m_chaabi_token_size + m_chaabi_FW_size))
            {
                if( fp_dnx_fw )
                    fclose(fp_dnx_fw);
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

void MerrifieldFW::InitFuphHeaderData()
{
    m_tempData = m_tempData ^ m_ifwi[m_i_offset + 3];
    m_tempData = m_tempData << 8;
    m_tempData = m_tempData ^ m_ifwi[m_i_offset + 2];
    m_tempData = m_tempData << 8;
    m_tempData = m_tempData ^ m_ifwi[m_i_offset + 1];
    m_tempData = m_tempData << 8;
    m_tempData = m_tempData ^ m_ifwi[m_i_offset];
}

bool MerrifieldFW::FindFuphHeaderSignature()
{
    const UINT fuphSignature = 0x55504824;//UPH$
    m_fw_image_size = this->m_utils->FileSize(m_fname_fw_image);
    m_i_offset = m_fw_image_size - (TWO_HUNDRED_KB + m_footer_size);  // To check only the Last 200K Chunk of the IFWI for FUPH Header
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

unsigned long MerrifieldFW::GetFuphHeaderSize()
{
    //How many bytes in the FUPH
    unsigned long fuphheadersize = 0;
    ULONGLONG byteCount = 4;// Byte count is set to 4 since the adding begins after
    unsigned long j = m_i_offset + 1;
    while(j < m_fw_image_size)
    {
        byteCount ++;
        j++;
    }
    if(byteCount == MERRIFIELD_B0_FW_UPDATE_PROFILE_HDR_SIZE)
    {
        //Merrifield FUPH of 164 bytes
        fuphheadersize = MERRIFIELD_B0_FW_UPDATE_PROFILE_HDR_SIZE;
        m_B0Plus = true;
    }
    else if(byteCount == MERRIFIELD_FW_UPDATE_PROFILE_HDR_SIZE)
    {
        //Merrifield FUPH of 144 bytes
        fuphheadersize = MERRIFIELD_FW_UPDATE_PROFILE_HDR_SIZE;
    }
    else if(byteCount == C0_FW_UPDATE_PROFILE_HDR_SIZE)
    {
        //new FUPH of 32 bytes
        fuphheadersize = C0_FW_UPDATE_PROFILE_HDR_SIZE;
    }
    else if(byteCount == D0_FW_UPDATE_PROFILE_HDR_SIZE)
    {
        //new FUPH of 36 bytes
        fuphheadersize = D0_FW_UPDATE_PROFILE_HDR_SIZE;
    }
    else if(byteCount > ONE28_K)
    {
        m_CSDBnIFWI = true;
        m_B0Plus = true;
        fuphheadersize = byteCount;
    }
    else
    {
        //Old FUPH of 28 bytes
        fuphheadersize = FW_UPDATE_PROFILE_OLD_HDR_SIZE_MRFD;
    }
    return fuphheadersize;
}

unsigned long MerrifieldFW::GetDataChunckSize(unsigned long tempData)
{
    tempData = tempData ^ m_ifwi[m_i_offset + 3];
    tempData = tempData << 8;
    tempData = tempData ^ m_ifwi[m_i_offset + 2];
    tempData = tempData << 8;
    tempData = tempData ^ m_ifwi[m_i_offset + 1];
    tempData = tempData << 8;
    tempData = tempData ^ m_ifwi[m_i_offset];
    return tempData;
}

void MerrifieldFW::LogError(int errorcode)
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
        if(errorcode < MAX_ERROR_CODE)
            this->m_utils->u_abort("Error Code: %d - %s", errorcode, Merrifield_error_code_array[errorcode]);
}

bool MerrifieldFW::InitNoSize()
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

dnx_data* MerrifieldFW::GetFwImageData(unsigned long index)
{
    if(index >= NUM_FW_DATA_MRFD)
        return NULL;

    if(fw_data_set[index])
        return fw_data_set[index];
    else
        return NULL;
}

bool MerrifieldFW::InitFwImage(bool)
{

    int ret_code = 0;
    bool ret = true;
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);

    // fw sections buffers

    ULONG read_cnt = 0;

    FILE* fp_fw_image = fopen(m_fname_fw_image, "rb");


    this->m_utils->u_log(LOG_FWUPGRADE, "allocating buffers for FW images...");
    try{

        m_fw_image_size  = this->m_utils->FileSize(m_fname_fw_image);

        FooterSizeInit();
        m_ifwi = new unsigned char[m_fw_image_size];

        if(fp_fw_image)
        {
            if(fread(m_ifwi, m_fw_image_size, 1, fp_fw_image)== 0)
            {
                if( fp_fw_image )
                    fclose(fp_fw_image);
                throw 8;
            }
        }
        else
            throw 8;

        fseek(fp_fw_image, 0, SEEK_SET); // seek back to beginning of file



        InitFuphHeaderData();

        if(FindFuphHeaderSignature())
        {

            if(!m_footer_size)
            {
                 m_fw_update_profile_hdr_size =  GetFuphHeaderSize();
            }
            else
            {
                 m_fw_update_profile_hdr_size = MERRIFIELD_B0_FW_UPDATE_PROFILE_HDR_SIZE;
                 m_B0Plus = true;
            }


            m_fw_update_profile_hdr = new unsigned char[m_fw_update_profile_hdr_size];

            if(!m_fw_update_profile_hdr)
            {
                if( fp_fw_image )
                   fclose(fp_fw_image);

                throw 5;
            }

            this->m_utils->u_log(LOG_FWUPGRADE, "allocating buffers for primary/secondary security FW images...");

            if(m_B0Plus)
            {
                m_ifwi_size = 0;
                m_i_offset = m_utils->StringLocation(this->m_fname_fw_image,"IFWI",false);
                if(m_i_offset == string::npos)
                {
                    this->m_utils->u_log(LOG_FWUPGRADE,"IFWI has invalid FUP header");

                    if( fp_fw_image )
                        fclose(fp_fw_image);
                    throw 10;
                }
                m_i_offset += 4;
                memcpy(&m_ifwi_size,m_ifwi+m_i_offset,sizeof(DWORD));
            }
            else
            {
                m_i_offset +=1;
                m_ifwi_size = GetDataChunckSize(m_ifwi_size);
            }
            if(m_ifwi_size == FOUR_MB)//4MB
            {
                m_ifwi_fw_block = new UCHAR[FOUR_MB];
                if(!m_ifwi_fw_block)
                {
                    if( fp_fw_image )
                        fclose(fp_fw_image);
                    throw 5;
                }
            }
            else
            {
                printf("unknown m_ifwi_size size");

                if( fp_fw_image )
                    fclose(fp_fw_image);

                throw 6;
            }
            this->m_utils->u_log(LOG_FWUPGRADE, "loading buffers for FW images...");
            rewind (fp_fw_image);
            read_cnt = fread(m_ifwi_fw_block, sizeof(UCHAR), FOUR_MB, fp_fw_image);
            if(read_cnt != FOUR_MB)
            {
                if( fp_fw_image )
                    fclose(fp_fw_image);

                throw 6;
            }

            this->m_utils->u_log(LOG_FWUPGRADE, "loading buffers for FUPH...");

            //if there is no footer, fuph is not being flashed
            if(!m_footer_size)
            {
                fseek (fp_fw_image, m_ifwi_size, SEEK_SET);
            }
            //if there is a foot, the fuph will be included
            else
            {
                fseek (fp_fw_image, m_fuph_location, SEEK_SET);
            }
            read_cnt = fread(m_fw_update_profile_hdr, sizeof(UCHAR), m_fw_update_profile_hdr_size, fp_fw_image);
            if(read_cnt != m_fw_update_profile_hdr_size)
            {
                if( fp_fw_image )
                    fclose(fp_fw_image);

                throw 6;
            }
            if(m_footer_size>0)
            {
                footerChecksum();
            }
        }else {
                  if( fp_fw_image )
                      fclose(fp_fw_image);
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

bool MerrifieldFW::CheckFile(char *filename)
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

void MerrifieldFW::footerChecksum()
{

    //calculate last 128K checksum
    DWORD checksum = this->m_utils->dwordCheckSum(m_ifwi +(FOUR_MB - ONE28_K),ONE28_K);

    //inject new last 128K checksum
    memcpy(m_fw_update_profile_hdr + FUP_LAST_CHUNK_OFFSET*sizeof(DWORD),&checksum,sizeof(DWORD));

    //recalculate fuph checksum
    memset(m_fw_update_profile_hdr + FUP_CHKSUM_OFFSET*sizeof(DWORD),0,sizeof(DWORD));
    checksum = this->m_utils->dwordCheckSum(m_fw_update_profile_hdr, m_fw_update_profile_hdr_size);

    //inject new fuph checksum
    memcpy(m_fw_update_profile_hdr + FUP_CHKSUM_OFFSET*sizeof(DWORD),&checksum,sizeof(DWORD));
    return;

}

bool MerrifieldFW::restructFUPH()
{
    size_t position = 0;
    int sizeofDWORD = sizeof(DWORD);
    DWORD data = 0;
    WORD data16 = 0;
    DWORD checksum = 0;
    unsigned int newFuphSize = (FUPH_DWORD_SIZE*sizeofDWORD)+ ONE28_K;
    boost::scoped_array<unsigned char> tmpBuf \
            (new unsigned char[newFuphSize]);

    memset(tmpBuf.get(),0,newFuphSize);

    memcpy(tmpBuf.get()+position,"UPH$",sizeofDWORD);
    position += sizeofDWORD;

    data16 = 1;
    memcpy(tmpBuf.get()+position,&data16,sizeofDWORD/2);
    position += sizeofDWORD;

    //DWORDs for TNG B0 + Bytes for TNG A0
    data = newFuphSize/4;
    memcpy(tmpBuf.get()+position,&data,sizeofDWORD);
    position += 2*sizeofDWORD;

    memcpy(tmpBuf.get()+position,"IFWI",sizeofDWORD);
    position += sizeofDWORD;

    if(m_fw_update_profile_hdr)
        memcpy(tmpBuf.get()+position,m_fw_update_profile_hdr+sizeofDWORD,sizeofDWORD);

    position += sizeofDWORD;

    data = (m_csdb_size == 0) ? 0xFFFFFFFF : 39;
    memcpy(tmpBuf.get()+position,&data,sizeofDWORD);
    position += sizeofDWORD;

    if(m_fw_update_profile_hdr)
        memcpy(tmpBuf.get()+position,m_fw_update_profile_hdr+(4*sizeofDWORD),32*sizeofDWORD);

    position += 32*sizeofDWORD;

    memcpy(tmpBuf.get()+position,"CSDB",sizeofDWORD);
    position += sizeofDWORD;

    memcpy(tmpBuf.get()+position,&m_csdb_size,sizeofDWORD);
    position += sizeofDWORD;

    memcpy(tmpBuf.get()+position,m_csdb,m_csdb_size);
    position += ONE28_K;

    memset(tmpBuf.get()+position,0xFF,sizeofDWORD);
    position += sizeofDWORD;

    if(m_fw_update_profile_hdr == NULL)
    {
        m_fw_update_profile_hdr = new unsigned char[newFuphSize];
    }else
    {
        realloc_array(m_fw_update_profile_hdr,\
                      m_fw_update_profile_hdr_size, newFuphSize);
    }
    if(m_fw_update_profile_hdr == NULL)
        return false;

    memcpy(m_fw_update_profile_hdr, tmpBuf.get(),newFuphSize);

    //calculate FUPH checksum
    checksum = this->m_utils->dwordCheckSum(m_fw_update_profile_hdr,newFuphSize);
    if(!checksum)
        return false;

    //setting the checksum
    memcpy(m_fw_update_profile_hdr + FUP_CHKSUM_OFFSET*sizeofDWORD,&checksum,sizeofDWORD);

    m_fw_update_profile_hdr_size = newFuphSize;

    return true;
}

bool MerrifieldFW::InitCSDB(const string& csdbstatus)
{
    try
    {
        BYTE  tmpbyte  = 0;
        DWORD  tmpdword  = 0;
        //extract CSDB from FUPH
        if(m_CSDBnIFWI)
        {
            //IFWI csdb payload includes fup header

            //retrieve CSDB size
            memcpy(&tmpdword,m_fw_update_profile_hdr+(40*sizeof(DWORD)),sizeof(DWORD));
            m_csdb_size = tmpdword;

            m_csdb = new unsigned char[m_csdb_size];

            memcpy(m_csdb,m_fw_update_profile_hdr+(41*sizeof(DWORD)),m_csdb_size);

        }
        else
        {
            tmpbyte = boost::lexical_cast<short>(csdbstatus.c_str());
            if(string(m_fname_bin_misc) != "BLANK.bin")
            {
                //valid check done in options
                scoped_file file(m_fname_bin_misc,"rb");


                //determine csdb size and read in the file
                m_csdb_size = file.size();
                if(m_csdb_size == 0)
                    throw std::runtime_error("Error CSDB payload size equals 0");
                m_csdb = new unsigned char[m_csdb_size];

                if(!m_csdb)
                    throw std::runtime_error("Error allocating CSDB buffer");

                if(file.read(m_csdb,1,m_csdb_size) != m_csdb_size)
                    throw std::runtime_error("Error reading CSDB from file");
            }
            else if((tmpbyte > 2) || (tmpbyte ==0))
            {
                throw std::runtime_error("Error this CSDB command requires a payload");
            }
            //check to see if file is payload or full csdb structure.
            if(m_csdb_size >4 ? (std::string(reinterpret_cast<const char*>(m_csdb),4) != "CSDB") : 1)
            {
                if((m_csdb_size + CSDB_HEADER_SIZE)> (1024*128))
                    throw std::runtime_error("Error CSDB payload too large");


                int offset = 0;
                boost::scoped_array<unsigned char> tmp(new unsigned char[m_csdb_size + CSDB_HEADER_SIZE]);
                //copy magic number
                strcpy(reinterpret_cast<char*>(tmp.get()),"CSDB");
                offset += 4;

                //word version =0x1
                tmpbyte = CSDB_Version;
                memcpy(tmp.get() + offset,&tmpbyte,sizeof(tmpbyte));
                offset += sizeof(tmpbyte);

                //word opcode
                tmpbyte = boost::lexical_cast<short>(csdbstatus.c_str());
                memcpy(tmp.get() + offset,&tmpbyte,sizeof(tmpbyte));
                offset += sizeof(tmpbyte) + sizeof(WORD);

                //word size
                tmpdword = m_csdb_size + CSDB_HEADER_SIZE;
                memcpy(tmp.get() + offset,&tmpdword,sizeof(tmpdword));
                offset += sizeof(tmpdword);

                memcpy(tmp.get() + offset, m_csdb,m_csdb_size);

                m_csdb_size += CSDB_HEADER_SIZE;

                if( m_csdb ) delete [] m_csdb;
                m_csdb = new unsigned char[m_csdb_size];
                if(m_csdb)
                memcpy(m_csdb, tmp.get(),m_csdb_size);
            }

            if(!restructFUPH())
                throw std::runtime_error("Error restructuring FUPH");
        }
        return true;
    }
    catch(std::exception & e)
    {
        this->m_utils->u_log(LOG_FWUPGRADE,"%s Exception raised %s", __FUNCTION__, e.what());
        return false;
    }

}


void MerrifieldFW::FooterSizeInit()
{
    size_t location =0;

    this->m_footer_size = 0;
    m_fuph_location = this->m_utils->StringLocation(this->m_fname_fw_image,string("UPH$"),false);
    location = this->m_utils->StringLocation(this->m_fname_fw_image,string("$CFS"),false);

    if((location != std::string::npos) && (m_fuph_location != std::string::npos))
    {
        if(location > m_fuph_location)
        {
            //xor compensator
            location -= 4;
            this->m_footer_size = this->m_utils->FileSize(this->m_fname_fw_image) - static_cast<uint32>(location);
            this->m_utils->u_log(LOG_FWUPGRADE, "IFWI Footer found with size: 0x%X",this->m_footer_size);
        }
    }
    else if((m_fuph_location != std::string::npos))
    {
        //if the fuph is under the 4MB it is a CFS IFWI without footer attached
        if(m_fuph_location< FOUR_MB)
        {
            this->m_footer_size = 1;
        }

    }

}
