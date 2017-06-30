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
#ifndef BAYTRAILOS_H
#define BAYTRAILOS_H
#include <memory>
#include "../../common/xfstkcommon.h"


class BayTrailUtils;


class BaytrailOS
{
public:
    BaytrailOS();
    ~BaytrailOS();
    bool Init(char* os_image_name);
    unsigned long long GetOsImageDataSize();

private:
    unsigned long GetFileSize(char* file);
    bool CheckFile(char *filename);
    void LogError(int errorcode);

    BaytrailUtils* m_utils;

    char* m_os_image_name;
    FILE* m_fp_os_image;
    unsigned long long m_os_data_size;
    unsigned char* m_pkt_buffer;

};
#endif //BAYTRAILOS_H


