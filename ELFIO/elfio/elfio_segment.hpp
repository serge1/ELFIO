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

#ifndef ELFI_SEGMENT_HPP
#define ELFI_SEGMENT_HPP

#include <fstream>
#include <vector>
#include "elf_types.hpp"
#include "elfio_utils.hpp"

namespace ELFIO {

class segment
{
  public:
    virtual ~segment() {};

    virtual Elf_Half    get_index()            const = 0;
    virtual Elf_Word    get_type()             const = 0;
    virtual Elf_Word    get_flags()            const = 0;
    virtual Elf_Xword   get_align()            const = 0;
    virtual Elf64_Addr  get_virtual_address()  const = 0;
    virtual Elf64_Addr  get_physical_address() const = 0;
    virtual Elf_Xword   get_file_size()        const = 0;
    virtual Elf_Xword   get_memory_size()      const = 0;

    virtual void set_index( Elf_Half )              = 0;
    virtual void set_type( Elf_Word )               = 0;
    virtual void set_flags( Elf_Word )              = 0;
    virtual void set_align( Elf_Xword )             = 0;
    virtual void set_virtual_address( Elf64_Addr )  = 0;
    virtual void set_physical_address( Elf64_Addr ) = 0;
    virtual void set_file_size(  Elf_Xword  )       = 0;
    virtual void set_memory_size( Elf_Xword )       = 0;

    virtual const char* get_data() const = 0;

    virtual Elf_Half add_section_index( Elf_Half index, Elf_Xword addr_align ) = 0;
    virtual Elf_Half get_sections_num()                                  const = 0;
    virtual Elf_Half get_section_index_at( Elf_Half num )                const = 0;

    virtual void load( std::ifstream& stream, std::streampos header_offset ) const = 0;
    virtual void save( std::ofstream& f, std::streampos header_offset,
                       std::streampos data_offset )                                = 0;
};


//------------------------------------------------------------------------------
template< class T >
class segment_impl : public segment
{
  public:
//------------------------------------------------------------------------------
    segment_impl( endianess_convertor* convertor_ ) :
        convertor( convertor_ )
    {
        std::fill_n( reinterpret_cast<char*>( &ph ), sizeof( ph ), '\0' );
        data = 0;
    }

//------------------------------------------------------------------------------
    virtual ~segment_impl()
    {
        delete [] data;
    }

//------------------------------------------------------------------------------
    // Section info functions
    ELFIO_GET_SET_ACCESS( Elf_Word,   type,             ph.p_type )
    ELFIO_GET_SET_ACCESS( Elf_Word,   flags,            ph.p_flags )
    ELFIO_GET_SET_ACCESS( Elf_Xword,  align,            ph.p_align )
    ELFIO_GET_SET_ACCESS( Elf64_Addr, virtual_address,  ph.p_vaddr )
    ELFIO_GET_SET_ACCESS( Elf64_Addr, physical_address, ph.p_paddr )
    ELFIO_GET_SET_ACCESS( Elf_Xword,  file_size,        ph.p_filesz )
    ELFIO_GET_SET_ACCESS( Elf_Xword,  memory_size,      ph.p_memsz )

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
    const char*
    get_data() const
    {
        return data;
    }

//------------------------------------------------------------------------------
    Elf_Half
    add_section_index( Elf_Half index, Elf_Xword addr_align )
    {
        sections.push_back( index );
        if ( addr_align > get_align() ) {
            set_align( addr_align );
        }

        return sections.size();
    }
    
//------------------------------------------------------------------------------
    Elf_Half
    get_sections_num() const
    {
        return sections.size();
    }
    
//------------------------------------------------------------------------------
    Elf_Half
    get_section_index_at( Elf_Half num ) const
    {
        if ( num < sections.size() ) {
            return sections[num];
        }
        
        return -1;
    }
    
//------------------------------------------------------------------------------
    void
    load( std::ifstream& stream,
          std::streampos header_offset ) const
    {
        stream.seekg( header_offset );
        stream.read( reinterpret_cast<char*>( &ph ), sizeof( ph ) );
        
        if ( PT_NULL != get_type() && 0 != get_file_size() ) {
            stream.seekg( (*convertor)( ph.p_offset ) );
            Elf_Xword size = get_file_size();
            data = new char[size];
            if ( 0 != data ) {
                stream.read( data, size );
            }
        }
    }

//------------------------------------------------------------------------------
    void save( std::ofstream& f,
               std::streampos header_offset,
               std::streampos data_offset )
    {
        ph.p_offset = data_offset;
        f.seekp( header_offset );
        f.write( reinterpret_cast<const char*>( &ph ), sizeof( ph ) );
    }

//------------------------------------------------------------------------------
  private:
    mutable T             ph;
    Elf_Half              index;
    mutable char*         data;
    std::vector<Elf_Half> sections;
    endianess_convertor*  convertor;
};

} // namespace ELFIO

#endif // ELFI_SEGMENT_HPP
