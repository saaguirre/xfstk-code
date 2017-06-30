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
#include <cstdio>
#include <cstring>
#include <cstddef>
#include "cloverviewmessages.h"
#include <sys/stat.h>


#include <exception>
#include "../../common/xfstktypes.h"
#include "../../common/xfstkcommon.h"
#include "cloverviewutils.h"
#include "cloverviewos.h"

extern CPSTR Cloverview_error_code_array [MAX_ERROR_CODE]; 

//CloverviewOS class Implementation
CloverviewOS::CloverviewOS()
{
    m_dnx_os_name = NULL;
    m_os_image_name = NULL;
    m_gpflags = 0;
    m_b_DnX_OS = 0;
    m_hdr_size = 0;
    m_dnx_os_size = 0;
    m_dnx_os = NULL; 
    m_osip_hdr = NULL;
    m_fp_os_image = NULL;
    m_os_data_size = 0;
    m_pkt_buffer = NULL;
    m_dnx_header_size = DNX_FW_SIZE_HDR_SIZE;
    m_dnx_os_size_hdr = NULL;
    m_ostotalsize = 0;
    m_os_data_size = 0;
    m_osprogress = 0;

}

CloverviewOS::~CloverviewOS()
{
    if(m_dnx_os)
    {
        delete [] m_dnx_os;
        m_dnx_os = NULL;
    }
    if(m_osip_hdr)
    {
        delete [] m_osip_hdr;
        m_osip_hdr = NULL;
    }
    if(m_fp_os_image)
        fclose(m_fp_os_image);

    if(m_dnx_os_size_hdr)
    {
        delete [] m_dnx_os_size_hdr;
        m_dnx_os_size_hdr = NULL;
    }
}

bool CloverviewOS::Init(char* dnx_os_name, char* os_image_name, 
                      CloverviewUtils* utils, unsigned long gpflags)
{
    bool ret = true;

    m_utils = utils;

    ret = CheckFile(dnx_os_name);
    if(!ret)
        return false;

    ret = CheckFile(os_image_name);
    if(!ret)
        return false;

    m_b_DnX_OS = gpflags & 0x20;

    if(!ret)
        return false;
    

    m_dnx_os_name = dnx_os_name;
    m_os_image_name = os_image_name;
    m_gpflags = gpflags;

    //To keep the file pointer in order
    //Need to follow this order
    ret = InitOsImage();

    if(ret)
        ret = InitOsipHdr();

    return ret;
}

dnx_data* CloverviewOS::GetNoSizeData()
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
        m_osdata.size = dnx_header_size;
        m_osdata.data = m_dnx_os_size_hdr;
    } else {
        ret = false;
    }
    return ret ? &m_osdata:NULL;
}

bool CloverviewOS::CheckFile(char *filename)
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

bool CloverviewOS::InitOsipHdr()
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    this->m_utils->u_log(LOG_OS, "Init OSIP header ...");
    int ret_code = 0;
    size_t read_cnt = 0;

    try {
        // move file pointer back to start of file
        rewind(m_fp_os_image);

        m_hdr_size = OSIP_PARTITIONTABLE_SIZE;
        m_osip_hdr = new UCHAR[m_hdr_size];
        if(!m_osip_hdr)
            throw 5;

        read_cnt = fread(m_osip_hdr, sizeof(UCHAR), m_hdr_size, m_fp_os_image);

        this->m_utils->u_log(LOG_OS, "_After fread osip_hdr");

        // sanity
        if(read_cnt != m_hdr_size)
            throw 6;
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

    return ret_code ? false:true;
}

bool CloverviewOS::InitOsImage()
{
    int ret_code = 0;
    unsigned long num_os_images = 0;
    unsigned long temp_data = 0;
    unsigned long i;
    unsigned long size_of_512 = 512;
    try {
        m_fp_os_image = fopen(m_os_image_name, "rb");
        if(!m_fp_os_image)
            throw 8;

        rewind (m_fp_os_image);

        // move file pointer to pointers offset to get number of os images
        fseek(m_fp_os_image, OSIP_NUM_POINTERS_OFFSET, SEEK_SET);
        if(fread(&num_os_images, sizeof(UCHAR), 1, m_fp_os_image) ==0)
            throw 8;


        this->m_utils->u_log(LOG_OS, "_num OS Images : %d ", num_os_images);

        // now walk this and get all os data sizes
        for(i = 0; i < num_os_images; i++){
            rewind(m_fp_os_image);
            int retval = fseek(m_fp_os_image, GET_OS_N_SIZE_OFFSET(i), SEEK_SET);
            //no error
            if(!retval)
            {
                size_t result;
                result = fread(&temp_data, sizeof(unsigned int), 1, m_fp_os_image);

                this->m_utils->u_log(LOG_OS, "_temp_data : %x", temp_data);
                //Error case
                if(0 == result)
                {
                    throw 12;
                }
                else
                {
                    //Calculate OS Data Size
                    m_os_data_size += (temp_data * size_of_512); // in 512 bytes
                    this->m_utils->u_log(LOG_OS, "OSII: %d Data Size: %d", i, m_os_data_size);
                    this->m_utils->u_log(LOG_OS, "_Temp Data is: %x h ", temp_data);

                    temp_data = 0;
                }
            }
            else
            {
                throw 12;
            }

        }

        m_ostotalsize = m_os_data_size;
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

    return ret_code ? false:true;
}

unsigned long CloverviewOS::GetFileSize(char* file)
{
	size_t lSize = 0;
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    FILE* fp_file = fopen(file, "rb");
    //Get file size
	if( fp_file )
	{
		fseek (fp_file , 0 , SEEK_END);
		lSize = ftell (fp_file);
        fclose(fp_file);
	}

	this->m_utils->u_log(LOG_OS, "fileName: %s FileSize : %d ", file, lSize);

    return lSize;
}

dnx_data* CloverviewOS::GetOsipHdr()
{
    bool ret = true;

    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    this->m_utils->u_log(LOG_OS, "Get OSIP header + Partion Table...");

    if(m_osip_hdr) {
        m_osdata.size = m_hdr_size;
        m_osdata.data = m_osip_hdr;
    } else {
        ret = false;
    }

    return ret ? &m_osdata:NULL;
}

dnx_data* CloverviewOS::GetOsImageDataChunk()
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);

    int ret_code = 0;
    unsigned long size_of_512 = 512;
    size_t read_cnt =0;
    try {
        this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
        this->m_utils->u_log(LOG_OS, "Get OS image data...");

        if(!m_fp_os_image)
            throw 8;

        // Remember, the file should be open during download to keep the file position pointer
        if(m_os_data_size && !this->m_utils->RIMGChunkSize)
            throw 12;

        // sanity 2
        this->m_utils->u_log(LOG_OS, "RIMGChunkSize is : %d", this->m_utils->RIMGChunkSize);
        this->m_utils->u_log(LOG_OS, "OS Data Size is : %d", m_os_data_size);
        if(m_os_data_size < this->m_utils->RIMGChunkSize * size_of_512)
            this->m_utils->u_log(LOG_OS, "OS Data Size:%d < chunk size: %d", m_os_data_size, this->m_utils->RIMGChunkSize * size_of_512);


        // allocate buffer for data  to send
        this->m_utils->u_log(LOG_OS, "allocating buffer for data to send...data size: %d", this->m_utils->RIMGChunkSize * size_of_512);
        m_pkt_buffer = new UCHAR[size_of_512 * this->m_utils->RIMGChunkSize];
        if(!m_pkt_buffer)
            throw 5;


        // read data from file
        this->m_utils->u_log(LOG_OS, "reading data from file");
        read_cnt = fread(m_pkt_buffer, sizeof(UCHAR), (unsigned long)this->m_utils->RIMGChunkSize * size_of_512, m_fp_os_image);
        // sanity
        if(read_cnt != this->m_utils->RIMGChunkSize * size_of_512)
            this->m_utils->u_log(LOG_OS, "OS Data Size left:%d < chunk size: %d", read_cnt, this->m_utils->RIMGChunkSize * size_of_512);


        this->m_utils->u_log(LOG_OS, "data size to read: %d", read_cnt);

        // decrement data size from total os data
        m_os_data_size -= read_cnt;
        float oscompleted = m_ostotalsize - m_os_data_size;
        m_osprogress = (int)((oscompleted/m_ostotalsize)*100);
        this->m_utils->u_log(LOG_PROGRESS, "%d",m_osprogress);
        this->m_utils->u_log(LOG_OS, "OS: Bytes left to send: %d", m_os_data_size);

        m_osdata.size = this->m_utils->RIMGChunkSize * size_of_512;
        m_osdata.data = m_pkt_buffer;
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

    return ret_code ? NULL:&m_osdata;
}

void CloverviewOS::ReleaseOsImageDataChunk()
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    if(m_pkt_buffer)
        delete[] m_pkt_buffer;
    m_pkt_buffer = NULL;
}

dnx_data* CloverviewOS::GetOsDnXHdr()
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    int ret_code = 0;

    unsigned long reservedWord = 0;
    unsigned long dnx_size_checksum = 0;


    // allocate buffers
    m_dnx_header_size = DNX_FW_SIZE_HDR_SIZE;
    m_dnx_os_size_hdr = new UCHAR[m_dnx_header_size];
    if(!m_dnx_os_size_hdr){
        // Failed to allocate buffer for dnx_os_size_hdr
        ret_code = 5;
        LogError(ret_code);
		return NULL;
    }

    m_dnx_os_size = this->m_utils->FileSize(m_dnx_os_name);

    // generate DnX Size Header
    dnx_size_checksum = m_dnx_os_size ^ m_gpflags;
    memcpy(m_dnx_os_size_hdr, (UCHAR *)&m_dnx_os_size, 4);
    memcpy(m_dnx_os_size_hdr + 4, (UCHAR *)&m_gpflags, 4);
    memcpy(m_dnx_os_size_hdr + 8, (UCHAR *)&reservedWord, 4);
    memcpy(m_dnx_os_size_hdr + 12, (UCHAR *)&reservedWord, 4);
    memcpy(m_dnx_os_size_hdr + 16, (UCHAR *)&reservedWord, 4);
    memcpy(m_dnx_os_size_hdr + 20, (UCHAR *)&dnx_size_checksum, 4);

    m_osdata.size = m_dnx_header_size;
    m_osdata.data = m_dnx_os_size_hdr;

    return &m_osdata;
}

dnx_data* CloverviewOS::GetOsDnX()
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);

    int ret_code = 0;
    size_t read_cnt = 0;
    try {
        if(!m_dnx_os_name)
            throw 8;

        m_dnx_os_size = this->m_utils->FileSize(m_dnx_os_name);
        m_dnx_os = new UCHAR[m_dnx_os_size];
        if(!m_dnx_os)
            throw 5;

        FILE* fp_dnx_os = fopen(m_dnx_os_name, "rb");
        // load buffers in order so file pointer moves along
		if( fp_dnx_os )
		{
			rewind(fp_dnx_os);
			read_cnt = fread(m_dnx_os, sizeof(UCHAR), m_dnx_os_size, fp_dnx_os);
			if(read_cnt != m_dnx_os_size) 
			{
				if(fp_dnx_os)
					fclose(fp_dnx_os);

				throw 6;
			} 
			else 
			{	
				fclose(fp_dnx_os);
			}
		}

        m_osdata.size = m_dnx_os_size;
        m_osdata.data = m_dnx_os;
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

    return ret_code ? NULL:&m_osdata;
}

unsigned long long CloverviewOS::GetOsImageDataSize()
{
    return m_os_data_size;
}

void CloverviewOS::LogError(int errorcode)
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    if(errorcode < MAX_ERROR_CODE)
        this->m_utils->u_abort("Error Code: %d - %s", errorcode, Cloverview_error_code_array[errorcode]);
}
