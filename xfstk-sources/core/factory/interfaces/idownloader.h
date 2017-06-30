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
#ifndef IDOWNLOADER_H
#define IDOWNLOADER_H
#include "ioptions.h"
#include <iostream>
#include <string.h>

using namespace std;

typedef void(*XfstkStatusPfn)(char* status, void *);

struct last_error {
    last_error():
		m_buffSize(1024),
        error_message( new char[m_buffSize] ){}
    ~last_error(){if(error_message) delete[] error_message;}
    void copy(const last_error& le)
    {
      error_code = le.error_code;
      strcpy(error_message,le.error_message);
    }

	last_error& operator=( const last_error& rhs )
	{
		if( this != &rhs )
		{
			delete [] error_message;
			error_message = new char[1024];
			for( int i = 0; i < m_buffSize; ++i)
				error_message[i] = rhs.error_message[i];
		}

		return *this;
	}

    const int m_buffSize;
    unsigned long error_code;
	//Pointing the error message
	char* error_message;

private:
	last_error( const last_error& );
};

class IDownloader
{
public:
    virtual ~IDownloader() {};
    virtual bool SetOptions(IOptions *options) = 0;
    virtual bool SetDevice(IGenericDevice *device) = 0;
    virtual bool UpdateTarget() = 0;
    virtual bool GetStatus() = 0;
    virtual bool GetLastError(last_error* er) = 0;
    virtual int GetResponse(unsigned char* buffer,int maxsize) = 0;
    virtual bool Cleanup() = 0;
    virtual bool SetStatusCallback(void(*StatusPfn)(char* Status, void* ClientData), void* ClientData) = 0;
};


#endif // IDOWNLOADER_H
