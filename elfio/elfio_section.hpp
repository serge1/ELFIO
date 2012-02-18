/*
Copyright (C) 2001-2011 by Serge Lamikhov-Center

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef ELFI_SECTION_HPP
#define ELFI_SECTION_HPP

#include <string>
#include <fstream>
#include "elf_types.hpp"
#include "elfio_utils.hpp"

namespace ELFIO {

class section
{
  public:
    virtual ~section() {};

    virtual Elf_Half    get_index()              const = 0;
    virtual std::string get_name()               const = 0;
    virtual Elf_Word    get_type()               const = 0;
    virtual Elf_Xword   get_flags()              const = 0;
    virtual Elf_Word    get_info()               const = 0;
    virtual Elf_Word    get_link()               const = 0;
    virtual Elf_Xword   get_addr_align()         const = 0;
    virtual Elf_Xword   get_entry_size()         const = 0;
    virtual Elf64_Addr  get_address()            const = 0;
    virtual Elf_Xword   get_size()               const = 0;
    virtual Elf_Word    get_name_string_offset() const = 0;

    virtual void set_index( Elf_Half )              = 0;
    virtual void set_name( std::string )            = 0;
    virtual void set_type( Elf_Word value )         = 0;
    virtual void set_flags( Elf_Xword )             = 0;
    virtual void set_info( Elf_Word )               = 0;
    virtual void set_link( Elf_Word )               = 0;
    virtual void set_addr_align( Elf_Xword )        = 0;
    virtual void set_entry_size( Elf_Xword )        = 0;
    virtual void set_address( Elf64_Addr )          = 0;
    virtual void set_size( Elf_Xword )              = 0;
    virtual void set_name_string_offset( Elf_Word ) = 0;

    virtual const char* get_data() const                                = 0;
    virtual void        set_data( const char* pData, Elf_Word size )    = 0;
    virtual void        set_data( const std::string& data )             = 0;
    virtual void        append_data( const char* pData, Elf_Word size ) = 0;
    virtual void        append_data( const std::string& data )          = 0;

    virtual void load( std::ifstream& f,
                       std::streampos header_offset ) const = 0;
    virtual void save( std::ofstream& f,
                       std::streampos header_offset,
                       std::streampos data_offset )         = 0;
};


template< class T >
class section_impl : public section
{
  public:
//------------------------------------------------------------------------------
    section_impl( const endianess_convertor* convertor_ ) : convertor( convertor_ )
    {
        std::fill_n( reinterpret_cast<char*>( &header ), sizeof( header ), '\0' );
        data      = 0;
        data_size = 0;
    }

//------------------------------------------------------------------------------
    ~section_impl()
    {
        delete [] data;
    }
    
//------------------------------------------------------------------------------
    // Section info functions
    ELFIO_GET_SET_ACCESS( Elf_Word,   type,               header.sh_type      )
    ELFIO_GET_SET_ACCESS( Elf_Xword,  flags,              header.sh_flags     )
    ELFIO_GET_SET_ACCESS( Elf64_Addr, address,            header.sh_addr      )
    ELFIO_GET_SET_ACCESS( Elf_Xword,  size,               header.sh_size      )
    ELFIO_GET_SET_ACCESS( Elf_Word,   link,               header.sh_link      )
    ELFIO_GET_SET_ACCESS( Elf_Word,   info,               header.sh_info      )
    ELFIO_GET_SET_ACCESS( Elf_Xword,  addr_align,         header.sh_addralign )
    ELFIO_GET_SET_ACCESS( Elf_Xword,  entry_size,         header.sh_entsize   )
    ELFIO_GET_SET_ACCESS( Elf_Word,   name_string_offset, header.sh_name      )

//------------------------------------------------------------------------------
    Elf_Half
    get_index() const
    {
        return index;
    }

//------------------------------------------------------------------------------
    void
    set_index( Elf_Half value )
    {
        index = value;
    }

//------------------------------------------------------------------------------
    std::string
    get_name() const
    {
        return name;
    }

//------------------------------------------------------------------------------
    void
    set_name( std::string name_ )
    {
        name = name_;
    }

//------------------------------------------------------------------------------
    const char*
    get_data() const
    {
        return data;
    }

//------------------------------------------------------------------------------
    void
    set_data( const char* raw_data, Elf_Word size )
    {
        if ( get_type() != SHT_NOBITS ) {
            delete [] data;
            data = new char[size];
            if ( 0 != data && 0 != raw_data ) {
                data_size = size;
                std::copy( raw_data, raw_data + size, data );
            }
        }

        set_size( size );
    }
    
//------------------------------------------------------------------------------
    void
    set_data( const std::string& str_data )
    {
        return set_data( str_data.c_str(), str_data.size() );
    }
    
//------------------------------------------------------------------------------
    void
    append_data( const char* raw_data, Elf_Word size )
    {
        if ( get_type() != SHT_NOBITS ) {
            if ( get_size() + size < data_size ) {
                std::copy( raw_data, raw_data + size, data + get_size() );
            }
            else {
                data_size = 2*( data_size + size);
                char* new_data = new char[data_size];
                if ( 0 != new_data ) {
                    std::copy( data, data + get_size(), new_data );
                    std::copy( raw_data, raw_data + size, new_data + get_size() );
                    delete [] data;
                    data = new_data;
                }
            }
            set_size( get_size() + size );
        }
    }
    
//------------------------------------------------------------------------------
    void
    append_data( const std::string& str_data )
    {
        return append_data( str_data.c_str(), str_data.size() );
    }
    
//------------------------------------------------------------------------------
    void
    load( std::ifstream& stream,
          std::streampos header_offset ) const
    {
        std::fill_n( reinterpret_cast<char*>( &header ), sizeof( header ), '\0' );
        stream.seekg( header_offset );
        stream.read( reinterpret_cast<char*>( &header ), sizeof( header ) );

        Elf_Xword size = get_size();
        if ( 0 == data && SHT_NULL != get_type() && SHT_NOBITS != get_type() &&
             0 != size ) {
            data = new char[size];
            stream.seekg( (*convertor)( header.sh_offset ) );
            stream.read( data, size );
        }
    }
    
//------------------------------------------------------------------------------
    void
    save( std::ofstream& f,
          std::streampos header_offset,
          std::streampos data_offset )
    {
        if ( 0 != get_index() ) {
            header.sh_offset = data_offset;
            header.sh_offset = (*convertor)( header.sh_offset );
        }
        
        save_header( f, header_offset );
        if ( get_type() != SHT_NOBITS && get_type() != SHT_NULL && get_size() != 0 && data != 0 ) {
            save_data( f, data_offset );
        }
    }

//------------------------------------------------------------------------------
  private:
//------------------------------------------------------------------------------
    void
    save_header( std::ofstream& f,
                 std::streampos header_offset )
    {
        f.seekp( header_offset );
        f.write( reinterpret_cast<const char*>( &header ), sizeof( header ) );
    }
  
//------------------------------------------------------------------------------
    void
    save_data( std::ofstream& f,
               std::streampos data_offset )
    {
        f.seekp( data_offset );
        f.write( get_data(), get_size() );
    }

//------------------------------------------------------------------------------
  private:
    mutable T                   header;
    Elf_Half                    index;
    std::string                 name;
    mutable char*               data;
    Elf_Word                    data_size;
    const endianess_convertor*  convertor;
};

} // namespace ELFIO

#endif // ELFI_SECTION_HPP
