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

#ifndef ELFIO_STRINGS_HPP
#define ELFIO_STRINGS_HPP

#include <cstdlib>
#include <cstring>
#include <string>

namespace ELFIO {

//------------------------------------------------------------------------------
//! \class string_section_accessor_template
//! \brief Class for accessing string section data
template <class S> class string_section_accessor_template
{
  public:
    //------------------------------------------------------------------------------
    //! \brief Constructor
    //! \param section Pointer to the section
    explicit string_section_accessor_template( S* section )
        : string_section( section )
    {
    }

    //------------------------------------------------------------------------------
    //! \brief Get a string from the section
    //! \param index Index of the string
    //! \return Pointer to the string, or nullptr if not found
    const char* get_string( Elf_Word index ) const
    {
        if ( string_section ) {
            const char* data = string_section->get_data();
            if ( index < string_section->get_size() && nullptr != data ) {
                size_t string_length = strnlength(
                    data + index,
                    static_cast<size_t>( string_section->get_size() ) - index );
                if ( string_length < ( string_section->get_size() - index ) )
                    return data + index;
            }
        }

        return nullptr;
    }

    //------------------------------------------------------------------------------
    //! \brief Add a string to the section
    //! \param str Pointer to the string
    //! \return Index of the added string
    Elf_Word add_string( const char* str )
    {
        Elf_Word current_position = 0;

        if ( string_section ) {
            // Strings are added to the end of the current section data
            current_position =
                static_cast<Elf_Word>( string_section->get_size() );

            if ( current_position == 0 ) {
                char empty_string = '\0';
                string_section->append_data( &empty_string, 1 );
                current_position++;
            }
            string_section->append_data(
                str, static_cast<Elf_Word>( std::strlen( str ) + 1 ) );
        }

        return current_position;
    }

    //------------------------------------------------------------------------------
    //! \brief Add a string to the section
    //! \param str The string to add
    //! \return Index of the added string
    Elf_Word add_string( const std::string& str )
    {
        return add_string( str.c_str() );
    }

    //------------------------------------------------------------------------------
  private:
    S* string_section; //!< Pointer to the section
};

using string_section_accessor = string_section_accessor_template<section>;
using const_string_section_accessor =
    string_section_accessor_template<const section>;

} // namespace ELFIO

#endif // ELFIO_STRINGS_HPP
