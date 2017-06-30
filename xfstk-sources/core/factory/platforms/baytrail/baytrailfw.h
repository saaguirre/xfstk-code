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
#ifndef BAYTRAILFW_H
#define BAYTRAILFW_H
#include "baytrailutils.h"
#include "baytraildownloader.h"

class BaytrailFW
{
public:
	BaytrailFW();
	~BaytrailFW();
	bool Init();

private:
	bool InitFwImage(bool enable_ifwi_wipe);
	bool InitNoSize();
	bool CheckFile(char *filename);
	void LogError(int errorcode);
	char* m_fname_fw_image;
	unsigned long m_fw_image_size;
	unsigned long m_fw_image_offset;
	unsigned long m_i_offset;
	unsigned char* m_ifwi;
	BaytrailUtils* m_utils;
	last_error m_last_error;

	BaytrailFW( const BaytrailFW& );
	BaytrailFW& operator=( const BaytrailFW& );
};
#endif //BAYTRAILFW_H
