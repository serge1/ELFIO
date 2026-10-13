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

#ifndef ELFIO_NOTE_HPP
#define ELFIO_NOTE_HPP

#include <cstring>

namespace ELFIO {

//------------------------------------------------------------------------------
// There are discrepancies in documentations. SCO documentation
// (http://www.sco.com/developers/gabi/latest/ch5.pheader.html#note_section)
// requires 8 byte entries alignment for 64-bit ELF file,
// but Oracle's definition uses the same structure
// for 32-bit and 64-bit formats.
// (https://docs.oracle.com/cd/E23824_01/html/819-0690/chapter6-18048.html)
//
// It looks like EM_X86_64 Linux implementation is similar to Oracle's
// definition. Therefore, the same alignment works for both formats
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//! \class note_section_accessor_template
//! \brief Class for accessing note section data
template <class S, Elf_Xword ( S::*F_get_size )() const>
class note_section_accessor_template
{
  public:
    //------------------------------------------------------------------------------
    //! \brief Constructor
    //! \param elf_file Reference to the ELF file
    //! \param section Pointer to the section
    explicit note_section_accessor_template( const elfio& elf_file, S* section )
        : elf_file( elf_file ), notes( section )
    {
        process_section();
    }

    //------------------------------------------------------------------------------
    //! \brief Get the number of notes
    //! \return Number of notes
    Elf_Word get_notes_num() const
    {
        return (Elf_Word)note_start_positions.size();
    }

    //------------------------------------------------------------------------------
    //! \brief Get a note
    //! \param index Index of the note
    //! \param type Type of the note
    //! \param name Name of the note
    //! \param desc Pointer to the descriptor
    //! \param descSize Size of the descriptor
    //! \return True if successful, false otherwise
    bool get_note( Elf_Word     index,
                   Elf_Word&    type,
                   std::string& name,
                   char*&       desc,
                   Elf_Word&    descSize ) const
    {
        if ( index >= note_start_positions.size() ) {
            return false;
        }

        const char*     data      = notes->get_data();
        const Elf_Xword data_size = ( notes->*F_get_size )();
        note_record     record;
        // The section may have changed since its note offsets were collected.
        if ( !read_note( data, data_size, note_start_positions[index],
                         record ) ) {
            return false;
        }

        name.assign( record.name, record.name_size );
        type     = record.type;
        descSize = record.descriptor_size;
        desc     = const_cast<char*>( record.descriptor );
        return true;
    }

    //------------------------------------------------------------------------------
    //! \brief Add a note
    //! \param type Type of the note
    //! \param name Name of the note
    //! \param desc Pointer to the descriptor
    //! \param descSize Size of the descriptor
    void add_note( Elf_Word           type,
                   const std::string& name,
                   const char*        desc,
                   Elf_Word           descSize )
    {
        const auto& convertor = elf_file.get_convertor();

        // A record without descriptor data must not claim a descriptor
        if ( desc == nullptr ) {
            descSize = 0;
        }

        int         align       = sizeof( Elf_Word );
        Elf_Word    nameLen     = (Elf_Word)name.size() + 1;
        Elf_Word    nameLenConv = ( *convertor )( nameLen );
        std::string buffer( reinterpret_cast<char*>( &nameLenConv ), align );
        Elf_Word    descSizeConv = ( *convertor )( descSize );

        buffer.append( reinterpret_cast<char*>( &descSizeConv ), align );
        type = ( *convertor )( type );
        buffer.append( reinterpret_cast<char*>( &type ), align );
        buffer.append( name );
        buffer.append( 1, '\x00' );
        const char pad[] = { '\0', '\0', '\0', '\0' };
        if ( nameLen % align != 0 ) {
            buffer.append( pad, (size_t)align - nameLen % align );
        }
        if ( desc != nullptr && descSize != 0 ) {
            buffer.append( desc, descSize );
            if ( descSize % align != 0 ) {
                buffer.append( pad, (size_t)align - descSize % align );
            }
        }

        note_start_positions.emplace_back( ( notes->*F_get_size )() );
        notes->append_data( buffer );
    }

  private:
    struct note_record
    {
        Elf_Word    type;
        const char* name;
        Elf_Word    name_size;
        const char* descriptor;
        Elf_Word    descriptor_size;
        Elf_Xword   total_size;
    };

    static Elf_Xword padded_size( Elf_Word size )
    {
        constexpr Elf_Xword alignment = sizeof( Elf_Word );
        return ( static_cast<Elf_Xword>( size ) + alignment - 1 ) / alignment *
               alignment;
    }

    // Decode one complete record. Scanning and lookup share these checks.
    bool read_note( const char*  data,
                    Elf_Xword    data_size,
                    Elf_Xword    position,
                    note_record& record ) const
    {
        constexpr Elf_Xword header_size = 3 * sizeof( Elf_Word );
        if ( data == nullptr || position > data_size ||
             header_size > data_size - position ) {
            return false;
        }

        Elf_Word header[3];
        std::memcpy( header, data + position, sizeof( header ) );
        const auto&     convertor   = elf_file.get_convertor();
        const Elf_Word  namesz      = ( *convertor )( header[0] );
        const Elf_Word  descsz      = ( *convertor )( header[1] );
        const Elf_Xword padded_name = padded_size( namesz );
        const Elf_Xword padded_desc = padded_size( descsz );
        const Elf_Xword remaining   = data_size - position - header_size;
        if ( padded_name > remaining ||
             padded_desc > remaining - padded_name ) {
            return false;
        }

        const char* name = data + position + header_size;
        if ( namesz != 0 && name[namesz - 1] != '\0' ) {
            return false;
        }
        record.type            = ( *convertor )( header[2] );
        record.name            = name;
        record.name_size       = namesz == 0 ? 0 : namesz - 1;
        record.descriptor      = descsz == 0 ? nullptr : name + padded_name;
        record.descriptor_size = descsz;
        record.total_size      = header_size + padded_name + padded_desc;
        return true;
    }

    //------------------------------------------------------------------------------
    //! \brief Process the section to extract note start positions
    void process_section()
    {
        note_start_positions.clear();
        if ( notes == nullptr ) {
            return;
        }
        const char*     data      = notes->get_data();
        const Elf_Xword data_size = ( notes->*F_get_size )();
        Elf_Xword       current   = 0;
        note_record     record;
        while ( read_note( data, data_size, current, record ) ) {
            note_start_positions.emplace_back( current );
            current += record.total_size;
        }
    }

    //------------------------------------------------------------------------------
  private:
    const elfio& elf_file; //!< Reference to the ELF file
    S*           notes;    //!< Pointer to the section or segment
    std::vector<Elf_Xword>
        note_start_positions; //!< Vector of note start positions
};

using note_section_accessor =
    note_section_accessor_template<section, &section::get_size>;
using const_note_section_accessor =
    note_section_accessor_template<const section, &section::get_size>;
using note_segment_accessor =
    note_section_accessor_template<segment, &segment::get_file_size>;
using const_note_segment_accessor =
    note_section_accessor_template<const segment, &segment::get_file_size>;

} // namespace ELFIO

#endif // ELFIO_NOTE_HPP
