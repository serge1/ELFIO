#include <stdio.h>
#include <string.h>

#include <elfio/elf_types.hpp>
#include "elfio_c_wrapper.h"

int main( int argc, char* argv[] )
{
    pelfio_t pelfio = elfio_new();
    bool     ret    = elfio_load( pelfio, argv[0] );

    if ( !ret ) {
        printf( "Can't load ELF file\n" );
    }

    //-----------------------------------------------------------------------------
    // elfio
    //-----------------------------------------------------------------------------
    printf( "Header size   : %d\n", elfio_get_header_size( pelfio ) );
    printf( "Version       : %d\n", elfio_get_version( pelfio ) );
    printf( "Section Entry : %d\n", elfio_get_section_entry_size( pelfio ) );
    printf( "Segment Entry : %d\n", elfio_get_segment_entry_size( pelfio ) );

    /* Uncomment a printf block of the interest */

    //-----------------------------------------------------------------------------
    // section
    //-----------------------------------------------------------------------------
    int secno = elfio_get_sections_num( pelfio );
    printf( "Sections No   : %d\n", secno );

    for ( int i = 0; i < secno; i++ ) {
        psection_t psection = elfio_get_section_by_index( pelfio, i );
        char       buff[128];
        elfio_section_get_name( psection, buff, 100 );
        // printf( "    [%02d] %s\n", i, buff );
        // printf( "        %08lx : %08lx\n",
        //         elfio_section_get_address( psection ),
        //         elfio_section_get_size( psection ) );
    }

    //-----------------------------------------------------------------------------
    // segment
    //-----------------------------------------------------------------------------
    int segno = elfio_get_segments_num( pelfio );
    printf( "Segments No   : %d\n", segno );

    for ( int i = 0; i < segno; i++ ) {
        psegment_t psegment = elfio_get_segment_by_index( pelfio, i );
        // printf( "    [%02d] %08lx : %08lx : %08lx\n", i,
        //         elfio_segment_get_virtual_address( psegment ),
        //         elfio_segment_get_memory_size( psegment ),
        //         elfio_segment_get_file_size( psegment ) );
    }

    //-----------------------------------------------------------------------------
    // symbol
    //-----------------------------------------------------------------------------
    psection_t psection = elfio_get_section_by_name( pelfio, ".symtab" );
    psymbol_t  psymbols = elfio_symbol_section_accessor_new( pelfio, psection );
    int        symno    = elfio_symbol_get_symbols_num( psymbols );
    for ( int i = 0; i < symno; i++ ) {
        char          name[128];
        Elf64_Addr    value;
        Elf_Xword     size;
        unsigned char bind;
        unsigned char type;
        Elf_Half      section_index;
        unsigned char other;
        elfio_symbol_get_symbol( psymbols, i, name, 128, &value, &size, &bind,
                                 &type, &section_index, &other );
        // printf( "[%4d] %10lu, %4lu %s\n", i, value, size, name );
    }
    elfio_symbol_section_accessor_delete( psymbols );

    //------------------------NoA-----------------------------------------------------
    // relocation
    //-----------------------------------------------------------------------------
    psection = elfio_get_section_by_name( pelfio, ".rela.dyn" );
    prelocation_t preloc =
        elfio_relocation_section_accessor_new( pelfio, psection );
    int relno = elfio_relocation_get_entries_num( preloc );
    for ( int i = 0; i < relno; i++ ) {
        Elf64_Addr offset;
        Elf_Word   symbol;
        Elf_Word   type;
        Elf_Sxword addend;
        elfio_relocation_get_entry( preloc, i, &offset, &symbol, &type,
                                    &addend );
        // printf( "[%4d] %16lx, %08x %08x %16lx\n", i, offset, symbol, type, addend );
    }
    elfio_relocation_section_accessor_delete( preloc );

    //-----------------------------------------------------------------------------
    // string
    //-----------------------------------------------------------------------------
    psection            = elfio_get_section_by_name( pelfio, ".strtab" );
    pstring_t   pstring = elfio_string_section_accessor_new( psection );
    int         pos     = 0;
    const char* str     = elfio_string_get_string( pstring, pos );
    while ( str ) {
        pos += strlen( str ) + 1;
        str = elfio_string_get_string( pstring, pos );
        // printf( "%s\n", str );
    }
    elfio_string_section_accessor_new( pstring );

    elfio_delete( pelfio );

    return 0;
}
