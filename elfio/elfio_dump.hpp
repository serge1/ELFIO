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

#ifndef ELFIO_DUMP_HPP
#define ELFIO_DUMP_HPP

#include <string>
#include <ostream>
#include <sstream>
#include <elfio.hpp>

namespace ELFIO {


static struct class_table_t {
    const char  key;
    const char* str;
} class_table [] = 
{
    { ELFCLASS32, "ELF32" },
    { ELFCLASS64, "ELF64" },
};


static struct endian_table_t {
    const char  key;
    const char* str;
} endian_table [] = 
{
    { ELFDATANONE, "None" },
    { ELFDATA2LSB, "Little endian" },
    { ELFDATA2MSB, "Big endian" },
};


static struct version_table_t {
    const char  key;
    const char* str;
} version_table [] = 
{
    { EV_NONE,    "None" },
    { EV_CURRENT, "Current" },
};


//------------------------------------------------------------------------------
class dump
{
  public:
//------------------------------------------------------------------------------
    static void
    header( std::ostream& out, elfio& reader )
    {
        out << "ELF Header"     << std::endl << std::endl
            << "  Class:      " << str_class( reader.get_class() )     << std::endl
            << "  Encoding:   " << str_endian( reader.get_encoding() ) << std::endl
            << "  ELFVersion: " << str_version( reader.get_elf_version() )
                                                                       << std::endl
            << "  Type:       " << std::hex << reader.get_type()       << std::endl
            << "  Machine:    " << std::hex << reader.get_machine()    << std::endl
            << "  Version:    " << std::hex << reader.get_version()    << std::endl
            << "  Entry:      " << std::hex << reader.get_entry()      << std::endl
            << "  Flags:      " << std::hex << reader.get_flags()      << std::endl;
    }

  private:
    template< typename T, typename K >
    std::string
    static
    find_value_in_table( const T& table, const K& key )
    {
        std::string res = "UNKNOWN";
        for ( unsigned int i = 0; i < sizeof( table )/sizeof( table[0] ); ++i ) {
            if ( table[i].key == key ) {
                res = table[i].str;
                break;
            }
        }

        return res;
    }


    template< typename T, typename K >
    static
    std::string
    format_assoc( const T& table, const K& key )
    {
        std::string str = find_value_in_table( table, key );
        std::ostringstream oss;
        oss << str << " (" << key << ")";

        return oss.str();
    }


#define STR_FUNC_TABLE( name )                              \
    static                                                  \
    std::string                                             \
    str_##name( const char key )                         \
    {                                                       \
        return format_assoc( name##_table, (int)key );   \
    }

    STR_FUNC_TABLE( class );
    STR_FUNC_TABLE( endian );
    STR_FUNC_TABLE( version );

#undef STR_FUNC_TABLE
};
    

}; // namespace ELFIO

#endif // ELFIO_DUMP_HPP
