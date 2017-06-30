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
#include "baytrailos.h"

#include <cstdio>
#include <utility>
#include <sys/stat.h>
#include "../../common/xfstktypes.h"
#include "baytrailmessages.h"
#include "baytrailutils.h"


extern CPSTR Baytrail_error_code_array [MAX_ERROR_CODE];

//BaytrailOS class Implementation
BaytrailOS::BaytrailOS()
{
    m_os_image_name = NULL;
    m_os_data_size = 0;
    m_pkt_buffer = NULL;
}

BaytrailOS::~BaytrailOS()
{
    if(m_fp_os_image)
    {
        fclose(m_fp_os_image);
    }
}

bool BaytrailOS::Init(char* os_image_name)
{
    bool ret = true;

    ret = CheckFile(os_image_name);
    if(!ret)
    {
        return false;
    }
    m_os_image_name = os_image_name;

    return ret;
}

unsigned long long BaytrailOS::GetOsImageDataSize()
{
    return m_os_data_size;
}

unsigned long BaytrailOS::GetFileSize(char* file)
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

bool BaytrailOS::CheckFile(char *filename)
{
    bool ret = true;
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    FILE *fp;

    fp = fopen(filename, "rb" );
    if (fp == NULL)
    {
        this->m_utils->u_abort("File %s cannot be opened", filename);
        ret = false;
    }
    struct stat file_info;
    if(stat(filename, &file_info))
    {
        this->m_utils->u_abort("Failed to stat file: %s", filename);
        ret = false;
    }

    if(fp)
        fclose(fp);
    return ret;
}

void BaytrailOS::LogError(int errorcode)
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    if(errorcode < MAX_ERROR_CODE)
    {
        this->m_utils->u_abort("Error Code: %d - %s", errorcode, Baytrail_error_code_array[errorcode]);
    }
}
