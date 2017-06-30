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
#include "baytrailmessages.h"
#include <sys/stat.h>
#include "../../common/xfstktypes.h"
#include "baytrailutils.h"
#include "baytrailfw.h"

using namespace std;

extern CPSTR Baytrail_error_code_array [MAX_ERROR_CODE];


//BaytrailFW Implementation
BaytrailFW::BaytrailFW()
{
    m_fname_fw_image =NULL;
    m_ifwi = NULL;
    m_fw_image_size = 0;
    m_i_offset = 0;
    m_utils = NULL;
}

BaytrailFW::~BaytrailFW()
{
   if(m_ifwi)
   {
       delete [] m_ifwi;
       m_ifwi = NULL;
   }
   if(m_fname_fw_image)
   {
       delete [] m_fname_fw_image;
       m_fname_fw_image = NULL;
   }
   if(m_utils)
   {
       delete [] m_utils;
       m_utils = NULL;
   }
}

bool BaytrailFW::Init()
{
    bool ret = true;
    return ret;
}

void BaytrailFW::LogError(int errorcode)
{
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    if(errorcode < MAX_ERROR_CODE)
        this->m_utils->u_abort("Error Code: %d - %s", errorcode, Baytrail_error_code_array[errorcode]);
}

bool BaytrailFW::InitFwImage(bool )
{
    return true;
}

bool BaytrailFW::InitNoSize()
{
    bool ret = true;
    return ret;
}

bool BaytrailFW::CheckFile(char *filename)
{
    bool ret = true;
    this->m_utils->u_log(LOG_ENTRY, "%s", __FUNCTION__);
    FILE *fp;

    fp = fopen(filename, "rb" );
    if ( fp == NULL) {
        this->m_utils->u_abort("File %s cannot be opened", filename);
        this->m_last_error.error_code = 14;
        strcpy(this->m_last_error.error_message, Baytrail_error_code_array[14]);
        ret = false;
    }
    struct stat file_info;
    if(stat(filename, &file_info)) {
        this->m_utils->u_abort("Failed to stat file: %s", filename);
        this->m_last_error.error_code = 15;
        strcpy(this->m_last_error.error_message, Baytrail_error_code_array[15]);
        ret = false;
    }

    if(fp)
        fclose(fp);
    return ret;
}

