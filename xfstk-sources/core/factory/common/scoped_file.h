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
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>

class scoped_file
{
public:
    scoped_file( const char * filename, const char * mode )
        : m_file(fopen(filename, mode))
    {
        if( m_file == NULL )
        {
            std::string err("Cannot Open File: ") ;
            err+= filename;
            throw std::runtime_error(err);
        }
    }

    ~scoped_file()
    {
        fclose(m_file);
    }

    size_t read( void * ptr, size_t size, size_t count )
    {
        return fread( ptr, size, count, m_file );
    }


    int seek( long int offset, int origin )
    {
        return fseek( m_file, offset, origin );
    }

    size_t write( const void * ptr, size_t size, size_t count )
    {
        return fwrite( ptr, size, count, m_file );
    }

    bool valid()
    {
        return (m_file == NULL) ? false : true;
    }
    size_t size()
    {
        size_t fileSize = 0;
        if(valid())
        {
            rewind(m_file);
            fseek(m_file, 0, SEEK_END);
            fileSize = ftell(m_file);
            rewind(m_file);
        }
        return fileSize;
    }


private:
    FILE* m_file;

    scoped_file( const scoped_file & );
    scoped_file & operator=( const scoped_file & );
};
