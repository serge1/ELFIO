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
#include <elfio.hpp>

namespace ELFIO {
//------------------------------------------------------------------------------
class dump
{
  public:
//------------------------------------------------------------------------------
    static void
    header( std::ostream& out, elfio& reader )
    {
        out << "ELF Header\n" << std::endl;
        out << "  Class:      "
            << str_elf_class( reader.get_class() )
            << " (" << (int)reader.get_class() << ")"
            << std::endl;
        out << "  Encoding:   "
            << ( ( ELFDATA2LSB == reader.get_encoding() ) ? "Little endian" : "" )
            << ( ( ELFDATA2MSB == reader.get_encoding() ) ? "Big endian" : "" )
            << ( ( ( ELFDATA2LSB != reader.get_encoding() ) &&
                 ( ELFDATA2MSB != reader.get_encoding() ) ) ? "Unknown" : "" )
            << std::endl;
        out << "  ELFVersion: "
            << ( ( EV_CURRENT == reader.get_elf_version() ) ? "Current" : "Unknown" )
            << "(" << (int)reader.get_elf_version() << ")"
            << std::endl;
        out << "  Type:       " << std::hex << reader.get_type()    << std::endl;
        out << "  Machine:    " << std::hex << reader.get_machine() << std::endl;
        out << "  Version:    " << std::hex << reader.get_version() << std::endl;
        out << "  Entry:      " << std::hex << reader.get_entry()   << std::endl;
        out << "  Flags:      " << std::hex << reader.get_flags()   << std::endl;
    }


struct convert
{
    Elf_Word    type;
    const char* str;
};

convert converts[] =
{
    { ELFCLASS32, "ELF32" },
    { ELFCLASS64, "ELF64" },
};
    
    
//------------------------------------------------------------------------------
    template<class T>  static std::string
    str_section_type( Elf_Word type )
    {
    }
    
    static std::string
    str_elf_class( Elf_Word type )
    {
        std::string res = "UNKNOWN";
        for ( unsigned int i = 0; i < sizeof( converts )/sizeof( convert ); ++i ) {
            if ( converts[i].type == type ) {
                res = converts[i].str;
                break;
            }
        }

        return res;
    }
    
};

} // namespace ELFIO

#endif // ELFIO_DUMP_HPP
