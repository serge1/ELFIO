/*
Copyright (C) 2001-present by Serge Lamikhov-Center

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

#ifndef ELFIO_SEGMENT_HPP
#define ELFIO_SEGMENT_HPP

#include <iostream>
#include <vector>
#include <new>
#include <limits>

namespace ELFIO {

class segment
{
    friend class elfio;

  public:
    virtual ~segment() = default;

    ELFIO_GET_ACCESS_DECL( Elf_Half, index );
    ELFIO_GET_SET_ACCESS_DECL( Elf_Word, type );
    ELFIO_GET_SET_ACCESS_DECL( Elf_Word, flags );
    ELFIO_GET_SET_ACCESS_DECL( Elf_Xword, align );
    ELFIO_GET_SET_ACCESS_DECL( Elf64_Addr, virtual_address );
    ELFIO_GET_SET_ACCESS_DECL( Elf64_Addr, physical_address );
    ELFIO_GET_SET_ACCESS_DECL( Elf_Xword, file_size );
    ELFIO_GET_SET_ACCESS_DECL( Elf_Xword, memory_size );
    ELFIO_GET_ACCESS_DECL( Elf64_Off, offset );

    virtual const char* get_data() const = 0;

    virtual Elf_Half add_section( section* psec, Elf_Xword addr_align ) = 0;
    virtual Elf_Half add_section_index( Elf_Half  index,
                                        Elf_Xword addr_align )          = 0;
    virtual Elf_Half get_sections_num() const                           = 0;
    virtual Elf_Half get_section_index_at( Elf_Half num ) const         = 0;
    virtual bool     is_offset_initialized() const                      = 0;

  protected:
    ELFIO_SET_ACCESS_DECL( Elf64_Off, offset );
    ELFIO_SET_ACCESS_DECL( Elf_Half, index );

    virtual const std::vector<Elf_Half>& get_sections() const               = 0;
    virtual bool load( std::istream& stream, std::streampos header_offset ) = 0;
    virtual void save( std::ostream&  stream,
                       std::streampos header_offset,
                       std::streampos data_offset )                         = 0;
};

//------------------------------------------------------------------------------
template <class T> class segment_impl : public segment
{
  public:
    //------------------------------------------------------------------------------
    segment_impl( const endianess_convertor* convertor,
                  const address_translator*  translator )
        : convertor( convertor ), translator( translator )
    {
    }

    //------------------------------------------------------------------------------
    // Section info functions
    ELFIO_GET_SET_ACCESS( Elf_Word, type, ph.p_type );
    ELFIO_GET_SET_ACCESS( Elf_Word, flags, ph.p_flags );
    ELFIO_GET_SET_ACCESS( Elf_Xword, align, ph.p_align );
    ELFIO_GET_SET_ACCESS( Elf64_Addr, virtual_address, ph.p_vaddr );
    ELFIO_GET_SET_ACCESS( Elf64_Addr, physical_address, ph.p_paddr );
    ELFIO_GET_SET_ACCESS( Elf_Xword, file_size, ph.p_filesz );
    ELFIO_GET_SET_ACCESS( Elf_Xword, memory_size, ph.p_memsz );
    ELFIO_GET_ACCESS( Elf64_Off, offset, ph.p_offset );

    //------------------------------------------------------------------------------
    Elf_Half get_index() const override { return index; }

    //------------------------------------------------------------------------------
    const char* get_data() const override { return data.get(); }

    //------------------------------------------------------------------------------
    Elf_Half add_section_index( Elf_Half  sec_index,
                                Elf_Xword addr_align ) override
    {
        sections.emplace_back( sec_index );
        if ( addr_align > get_align() ) {
            set_align( addr_align );
        }

        return (Elf_Half)sections.size();
    }

    //------------------------------------------------------------------------------
    Elf_Half add_section( section* psec, Elf_Xword addr_align ) override
    {
        return add_section_index( psec->get_index(), addr_align );
    }

    //------------------------------------------------------------------------------
    Elf_Half get_sections_num() const override
    {
        return (Elf_Half)sections.size();
    }

    //------------------------------------------------------------------------------
    Elf_Half get_section_index_at( Elf_Half num ) const override
    {
        if ( num < sections.size() ) {
            return sections[num];
        }

        return Elf_Half( -1 );
    }

    //------------------------------------------------------------------------------
  protected:
    //------------------------------------------------------------------------------

    //------------------------------------------------------------------------------
    void set_offset( Elf64_Off value ) override
    {
        ph.p_offset   = decltype( ph.p_offset )( value );
        ph.p_offset   = ( *convertor )( ph.p_offset );
        is_offset_set = true;
    }

    //------------------------------------------------------------------------------
    bool is_offset_initialized() const override { return is_offset_set; }

    //------------------------------------------------------------------------------
    const std::vector<Elf_Half>& get_sections() const override
    {
        return sections;
    }

    //------------------------------------------------------------------------------
    void set_index( Elf_Half value ) override { index = value; }

    //------------------------------------------------------------------------------
    bool load( std::istream& stream, std::streampos header_offset ) override
    {
        if ( translator->empty() ) {
            stream.seekg( 0, std::istream::end );
            set_stream_size( size_t( stream.tellg() ) );
        }
        else {
            set_stream_size( std::numeric_limits<size_t>::max() );
        }

        stream.seekg( ( *translator )[header_offset] );
        stream.read( reinterpret_cast<char*>( &ph ), sizeof( ph ) );
        is_offset_set = true;

        if ( PT_NULL == get_type() || 0 == get_file_size() ) {
            return true;
        }

        stream.seekg( ( *translator )[( *convertor )( ph.p_offset )] );
        Elf_Xword size = get_file_size();

        if ( size > get_stream_size() ) {
            data = nullptr;
        }
        else {
            data.reset( new ( std::nothrow ) char[(size_t)size + 1] );

            if ( nullptr != data.get() && stream.read( data.get(), size ) ) {
                data.get()[size] = 0;
            }
            else {
                data = nullptr;
                return false;
            }
        }

        return true;
    }

    //------------------------------------------------------------------------------
    void save( std::ostream&  stream,
               std::streampos header_offset,
               std::streampos data_offset ) override
    {
        ph.p_offset = decltype( ph.p_offset )( data_offset );
        ph.p_offset = ( *convertor )( ph.p_offset );
        adjust_stream_size( stream, header_offset );
        stream.write( reinterpret_cast<const char*>( &ph ), sizeof( ph ) );
    }

    //------------------------------------------------------------------------------
    size_t get_stream_size() const { return stream_size; }

    //------------------------------------------------------------------------------
    void set_stream_size( size_t value ) { stream_size = value; }

    //------------------------------------------------------------------------------
  private:
    T                          ph    = { 0 };
    Elf_Half                   index = 0;
    std::unique_ptr<char[]>    data;
    std::vector<Elf_Half>      sections;
    const endianess_convertor* convertor     = nullptr;
    const address_translator*  translator    = nullptr;
    size_t                     stream_size   = 0;
    bool                       is_offset_set = false;
};

} // namespace ELFIO

#endif // ELFIO_SEGMENT_HPP
