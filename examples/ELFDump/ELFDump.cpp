/*
ELFDump.cpp - Dump ELF file using ELFIO library.

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

#ifdef _MSC_VER
  #define _SCL_SECURE_NO_WARNINGS
  #define ELFIO_NO_INTTYPES
#endif

#define PRIx64 "llx"

#define __STDC_FORMAT_MACROS

#include <string>
#include <elfio.hpp>

using namespace std;
using namespace ELFIO;

void
PrintHeader( const elfio& reader )
{
    printf( "ELF Header\n" );
    printf( "  Class:      %s (%d)\n",
            ( ELFCLASS32 == reader.get_class() ) ? "ELF32" : "ELF64",
            (int)reader.get_class() );
    if ( ELFDATA2LSB == reader.get_encoding() ) {
        printf( "  Encoding:   Little endian\n" );
    }
    else if ( ELFDATA2MSB == reader.get_encoding() ) {
        printf( "  Encoding:   Big endian\n" );
    }
    else {
        printf( "  Encoding:   Unknown\n" );
    }
    printf( "  ELFVersion: %s (%d)\n",
            ( EV_CURRENT == reader.get_elf_version() ) ? "Current" : "Unknown",
            (int)reader.get_elf_version() );
    printf( "  Type:       0x%04X\n",   reader.get_type() );
    printf( "  Machine:    0x%04X\n",   reader.get_machine() );
    printf( "  Version:    0x%08X\n",   reader.get_version() );
    printf( "  Entry:      0x%08" PRIx64 "\n",   reader.get_entry() );
    printf( "  Flags:      0x%08X\n\n", reader.get_flags() );
}


string
SectionTypes( Elf_Word type )
{
    string sRet = "UNKNOWN";
    switch ( type ) {
    case SHT_NULL :
        sRet = "NULL";
        break;
    case SHT_PROGBITS :
        sRet = "PROGBITS";
        break;
    case SHT_SYMTAB :
        sRet = "SYMTAB";
        break;
    case SHT_STRTAB :
        sRet = "STRTAB";
        break;
    case SHT_RELA :
        sRet = "RELA";
        break;
    case SHT_HASH :
        sRet = "HASH";
        break;
    case SHT_DYNAMIC :
        sRet = "DYNAMIC";
        break;
    case SHT_NOTE :
        sRet = "NOTE";
        break;
    case SHT_NOBITS :
        sRet = "NOBITS";
        break;
    case SHT_REL :
        sRet = "REL";
        break;
    case SHT_SHLIB :
        sRet = "SHLIB";
        break;
    case SHT_DYNSYM :
        sRet = "DYNSYM";
        break;
    case SHT_INIT_ARRAY :
        sRet = "SHT_INIT_ARRAY";
        break;
    case SHT_FINI_ARRAY :
        sRet = "SHT_FINI_ARRAY";
        break;
    case SHT_PREINIT_ARRAY :
        sRet = "SHT_PREINIT_ARRAY";
        break;
    case SHT_GROUP :
        sRet = "SHT_GROUP";
        break;
    case SHT_SYMTAB_SHNDX :
        sRet = "SHT_SYMTAB_SHNDX";
        break;
    }
    
    return sRet;
}


string SectionFlags( Elf_Xword flags )
{
    string sRet = "";
    if ( flags & SHF_WRITE ) {
        sRet += "W";
    }
    if ( flags & SHF_ALLOC ) {
        sRet += "A";
    }
    if ( flags & SHF_EXECINSTR ) {
        sRet += "X";
    }

    return sRet;
}


void
PrintSection( int i, const section* sec )
{
    Elf64_Addr addr = sec->get_address();
    Elf_Xword  size = sec->get_size();
    PRIx64;
    printf( "  [%2x] %-20s %-8.8s %08"PRIx64" %06"PRIx64" %02"PRIx64" %-3.3s %02x %04x %02"PRIx64"\n",
         i,
         string( sec->get_name() ).substr( 0, 20 ).c_str(),
         SectionTypes( sec->get_type() ).c_str(),
         sec->get_address(),
         sec->get_size(),
         sec->get_entry_size(),
         SectionFlags( sec->get_flags() ).c_str(),
         sec->get_link(),
         sec->get_info(),
         sec->get_addr_align() );

    return;
}


string
SegmentTypes( Elf_Word type )
{
    string sRet = "UNKNOWN";
    switch ( type ) {
    case PT_NULL:
        sRet = "NULL";
        break;
    case PT_LOAD:
        sRet = "PT_LOAD";
        break;
    case PT_DYNAMIC:        
        sRet = "PT_DYNAMIC";
        break;
    case PT_INTERP:
        sRet = "PT_INTERP";
        break;
    case PT_NOTE:
        sRet = "PT_NOTE";
        break;
    case PT_SHLIB:
        sRet = "PT_SHLIB";
        break;
    case PT_PHDR:
        sRet = "PT_PHDR";
        break;
    case PT_TLS:
        sRet = "PT_TLS";
        break;
    }
    
    return sRet;
}


void
PrintSegment( int i, const segment* seg )
{
    printf( "  [%2x] %-10.10s %08"PRIx64" %08"PRIx64" %08"PRIx64" %08"PRIx64" %08x %08"PRIx64"\n",
            i,
            SegmentTypes( seg->get_type() ).c_str(),
            seg->get_virtual_address(),
            seg->get_physical_address(),
            seg->get_file_size(),
            seg->get_memory_size(),
            seg->get_flags(),
            seg->get_align() );

    return;
}


void
PrintSymbol( std::string& name, Elf64_Addr value,
             Elf_Xword size,
             unsigned char bind, unsigned char type,
             Elf_Half section )
{
    printf( "%-46.46s %08"PRIx64" %08"PRIx64"    %01x    %01x  %02x\n",
            name.c_str(),
            value,
            size,
            (int)bind,
            (int)type,
            section );
}


int main( int argc, char** argv )
{
    if ( argc != 2 ) {
        printf( "Usage: ELFDump <file_name>\n" );
        return 1;
    }


    // Open ELF reader
    elfio reader;
    
    if ( !reader.load( argv[1] ) ) {
        printf( "File %s is not found or it is not an ELF file\n", argv[1] );
        return 1;
    }

    // Print ELF file header
    PrintHeader( reader );

    // Print ELF file sections
    printf( "Section headers:\n" );
    printf( "  [Nr] Name                 Type     Addr     Size   ES Flg Lk Inf  Al\n" );
    int nSecNo = reader.sections.size();
    int i;
    for ( i = 0; i < nSecNo; ++i ) {    // For all sections
        section* sec = reader.sections[i];
        PrintSection( i, sec );
    }
    printf( "Key to Flags: W (write), A (alloc), X (execute)\n\n" );

    // Print ELF file segments
    int nSegNo = reader.segments.size();
    if ( nSegNo > 0 ) {
        printf( "Segment headers:\n" );
        printf( "  [Nr] Type       VirtAddr PhysAddr FileSize Mem.Size Flags    Align\n" );
    }
    for ( i = 0; i < nSegNo; ++i ) {    // For all sections
        segment* seg = reader.segments[i];
        PrintSegment( i, seg );
    }
    printf( "\n" );

    // Print symbol tables
    nSecNo = reader.sections.size();
    for ( i = 0; i < nSecNo; ++i ) {    // For all sections
        section* sec = reader.sections[i];
        if ( SHT_SYMTAB == sec->get_type() || SHT_DYNSYM == sec->get_type() ) {
            symbol_section_accessor symbols( reader, sec );

            std::string   name;
            Elf64_Addr    value;
            Elf_Xword     size;
            unsigned char bind;
            unsigned char type;
            Elf_Half      section;
            unsigned char other;
            Elf_Xword     nSymNo = symbols.get_symbols_num();
            if ( 0 < nSymNo ) {
                printf( "Symbol table (%s)\n", sec->get_name().c_str() );
                printf( "     Name                                      Value    Size     Bind Type Sect\n" );
                for ( int i = 0; i < nSymNo; ++i ) {
                    symbols.get_symbol( i, name, value, size, bind, type, section, other );
                    PrintSymbol( name, value, size, bind, type, section );
                }
            }

            printf( "\n" );
        }
    }

    for ( i = 0; i < nSecNo; ++i ) {    // For all sections
        section* sec = reader.sections[i];
        if ( SHT_NOTE == sec->get_type() ) {
            note_section_accessor notes( reader, sec );
            int nNotesNo = notes.get_notes_num();
            if ( 0 < nNotesNo ) {
                printf( "Note section (%s)\n", sec->get_name().c_str() );
                printf( "   No     Type    Name\n" );
                for ( int i = 0; i < nNotesNo; ++i ) {    // For all notes
                    Elf_Word  type;
                	std::string name;
                	void* desc;
                    Elf_Word descsz;
                
                    notes.get_note( i, type, name, desc, descsz );
                    printf( "  [%2d] 0x%08x %s\n", i, type, name.c_str() );
                }
            }
            
            printf( "\n" );
        }
    }

    return 0;
}
