/*
Copyright (C) 2001-2020 by Serge Lamikhov-Center

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
#include <cstring>

#include <elfio/elfio_range.hpp>

namespace ELFIO {

class segment
{
    friend class elfio;

  public:
    virtual ~segment(){};

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

    virtual Elf_Half add_section_index( Elf_Half  index,
                                        Elf_Xword addr_align )  = 0;
    virtual Elf_Half get_sections_num() const                   = 0;
    virtual Elf_Half get_section_index_at( Elf_Half num ) const = 0;
    virtual bool     is_offset_initialized() const              = 0;
    enum class removal_result
    {
        noncontinuous, // this is an error condition!
        empty,
        unmodified,
        modified
    };

    virtual removal_result
    remove_sections( const std::vector<section*>& sections,
                     const std::vector<section*>& allSections,
                     bool                         dry_run,
                     bool                         ignore_phys ) = 0;

  protected:
    ELFIO_SET_ACCESS_DECL( Elf64_Off, offset );
    ELFIO_SET_ACCESS_DECL( Elf_Half, index );

    virtual const std::vector<Elf_Half>& get_sections() const               = 0;
    virtual void load( std::istream& stream, std::streampos header_offset ) = 0;
    virtual void save( std::ostream&  stream,
                       std::streampos header_offset,
                       std::streampos data_offset )                         = 0;
};

//------------------------------------------------------------------------------
template <class T> class segment_impl : public segment
{
  public:
    //------------------------------------------------------------------------------
    segment_impl( endianess_convertor* convertor_ )
        : stream_size( 0 ), index( 0 ), data( 0 ), convertor( convertor_ )
    {
        is_offset_set = false;
        std::fill_n( reinterpret_cast<char*>( &ph ), sizeof( ph ), '\0' );
    }

    //------------------------------------------------------------------------------
    virtual ~segment_impl() { delete[] data; }

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
    size_t stream_size;

    //------------------------------------------------------------------------------
    size_t get_stream_size() const { return stream_size; }

    //------------------------------------------------------------------------------
    void set_stream_size( size_t value ) { stream_size = value; }

    //------------------------------------------------------------------------------
    Elf_Half get_index() const { return index; }

    //------------------------------------------------------------------------------
    const char* get_data() const { return data; }

    //------------------------------------------------------------------------------
    Elf_Half add_section_index( Elf_Half sec_index, Elf_Xword addr_align )
    {
        sections.push_back( sec_index );
        if ( addr_align > get_align() ) {
            set_align( addr_align );
        }

        return (Elf_Half)sections.size();
    }

    //------------------------------------------------------------------------------
    Elf_Half get_sections_num() const { return (Elf_Half)sections.size(); }

    //------------------------------------------------------------------------------
    Elf_Half get_section_index_at( Elf_Half num ) const
    {
        if ( num < sections.size() ) {
            return sections[num];
        }

        return Elf_Half( -1 );
    }

    //------------------------------------------------------------------------------
    removal_result
    remove_sections( const std::vector<section*>& sectionsForRemoval,
                     const std::vector<section*>& allSections,
                     bool                         dry_run,
                     bool                         ignore_phys )
    {

        range disk{ get_offset(), get_offset() + get_file_size() };
        range phys_mem{ get_physical_address(),
                        get_physical_address() + get_memory_size() };
        range virt_mem{ get_virtual_address(),
                        get_virtual_address() + get_memory_size() };

        std::for_each(
            sectionsForRemoval.begin(), sectionsForRemoval.end(),
            [&]( auto s ) {
                if ( s->get_type() != SHT_NOBITS ) {
                    disk.subtract(
                        { s->get_offset(), s->get_offset() + s->get_size() } );
                }
                if ( s->get_address() != 0 ) {
                    if ( !ignore_phys )
                        phys_mem.subtract(
                            { s->get_address(),
                              s->get_address() + s->get_size() } );

                    virt_mem.subtract( { s->get_address(),
                                         s->get_address() + s->get_size() } );
                }
            } );

        if ( disk.element_count() == 0 &&
             ( phys_mem.element_count() == 0 || ignore_phys ) &&
             virt_mem.element_count() == 0 ) {
            return removal_result::empty;
        }

        if ( disk.element_count() > 1 || phys_mem.element_count() > 1 ||
             virt_mem.element_count() > 1 ) {
            return removal_result::noncontinuous;
        }

        if ( !dry_run ) {
            for ( auto sec = sections.begin(); sec != sections.end(); ) {
                if ( std::find( sectionsForRemoval.begin(),
                                sectionsForRemoval.end(), allSections[*sec] ) !=
                     sectionsForRemoval.end() )
                    sec = sections.erase( sec );
                else {
                    *sec -= std::count_if(
                        allSections.begin(), allSections.begin() + *sec,
                        [&]( auto it ) {
                            return std::find( sectionsForRemoval.begin(),
                                              sectionsForRemoval.end(),
                                              it ) != sectionsForRemoval.end();
                        } );
                    ++sec;
                }
            }
        }

        removal_result result = removal_result::unmodified;

        if ( disk.get_start() != get_offset() ||
             disk.get_length() != get_file_size() ) {
            result = removal_result::modified;
            if ( dry_run )
                return result;

            if ( data != 0 ) {
                auto newdata = new ( std::nothrow ) char[disk.get_length() + 1];
                std::memcpy( newdata, data + disk.get_start() - get_offset(),
                             disk.get_length() );
                newdata[disk.get_length()] = 0;
                delete[] data;
                data = newdata;
            }

            set_offset( disk.get_start() );
            set_file_size( disk.get_length() );
        }

        if ( phys_mem.get_start() != get_physical_address() ) {
            result = removal_result::modified;
            if ( dry_run )
                return result;

            set_physical_address( phys_mem.get_start() );
        }
        if ( phys_mem.get_length() != get_memory_size() ) {
            result = removal_result::modified;
            if ( dry_run )
                return result;

            set_memory_size( phys_mem.get_length() );
        }

        if ( virt_mem.get_start() != get_virtual_address() ) {
            result = removal_result::modified;
            if ( dry_run )
                return result;

            set_virtual_address( virt_mem.get_start() );
        }
        if ( virt_mem.get_length() != get_memory_size() ) {
            result = removal_result::modified;
            if ( dry_run )
                return result;

            set_memory_size( virt_mem.get_length() );
        }

        return result;
    };

    //------------------------------------------------------------------------------
  protected:
    //------------------------------------------------------------------------------

    //------------------------------------------------------------------------------
    void set_offset( Elf64_Off value )
    {
        ph.p_offset   = value;
        ph.p_offset   = ( *convertor )( ph.p_offset );
        is_offset_set = true;
    }

    //------------------------------------------------------------------------------
    bool is_offset_initialized() const { return is_offset_set; }

    //------------------------------------------------------------------------------
    const std::vector<Elf_Half>& get_sections() const { return sections; }

    //------------------------------------------------------------------------------
    void set_index( Elf_Half value ) { index = value; }

    //------------------------------------------------------------------------------
    void load( std::istream& stream, std::streampos header_offset )
    {

        stream.seekg( 0, stream.end );
        set_stream_size( stream.tellg() );

        stream.seekg( header_offset );
        stream.read( reinterpret_cast<char*>( &ph ), sizeof( ph ) );
        is_offset_set = true;

        if ( PT_NULL != get_type() && 0 != get_file_size() ) {
            stream.seekg( ( *convertor )( ph.p_offset ) );
            Elf_Xword size = get_file_size();

            if ( size > get_stream_size() ) {
                data = 0;
            }
            else {
                data = new ( std::nothrow ) char[size + 1];

                if ( 0 != data ) {
                    stream.read( data, size );
                    data[size] = 0;
                }
            }
        }
    }

    //------------------------------------------------------------------------------
    void save( std::ostream&  stream,
               std::streampos header_offset,
               std::streampos data_offset )
    {
        ph.p_offset = data_offset;
        ph.p_offset = ( *convertor )( ph.p_offset );
        stream.seekp( header_offset );
        stream.write( reinterpret_cast<const char*>( &ph ), sizeof( ph ) );
    }

    //------------------------------------------------------------------------------
  private:
    T                     ph;
    Elf_Half              index;
    char*                 data;
    std::vector<Elf_Half> sections;
    endianess_convertor*  convertor;
    bool                  is_offset_set;
};

} // namespace ELFIO

#endif // ELFIO_SEGMENT_HPP
