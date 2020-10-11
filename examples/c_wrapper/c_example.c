#include <stdio.h>

#include <elfio/elf_types.hpp>
#include "elfio_c_wrapper.h"

int main( int argc, char* argv[] )
{
    pelfio_t pelfio = elfio_new();
    bool     ret    = elfio_load( pelfio, argv[0] );

    if ( !ret ) {
        printf( "Can't load ELF file\n" );
    }

    printf( "Header size   : %d\n", elfio_get_header_size( pelfio ) );
    printf( "Version       : %d\n", elfio_get_version( pelfio ) );
    printf( "Section Entry : %d\n", elfio_get_section_entry_size( pelfio ) );
    printf( "Segment Entry : %d\n", elfio_get_segment_entry_size( pelfio ) );

    /* Uncomment a block of the interest */

    /*
    int secno = elfio_get_sections_num( pelfio );
    printf( "\nSections No   : %d\n", secno );

    for ( int i = 0; i < secno; i++ ) {
        psection_t psection = elfio_get_section_by_index( pelfio, i );
        char       buff[128];
        elfio_section_get_name( psection, buff, 100 );
        printf( "    [%02d] %s\n", i, buff );
        printf( "        %08lx : %08lx\n",
                elfio_section_get_address( psection ),
                elfio_section_get_size( psection ) );
    }
    */

    /*
    int segno = elfio_get_segments_num( pelfio );
    printf( "\nSegments No   : %d\n", segno );

    for ( int i = 0; i < segno; i++ ) {
        psegment_t psegment = elfio_get_segment_by_index( pelfio, i );
        printf( "    [%02d] %08lx : %08lx : %08lx\n", i,
                elfio_segment_get_virtual_address( psegment ),
                elfio_segment_get_memory_size( psegment ),
                elfio_segment_get_file_size( psegment ) );
    }
    */

    /*
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
        elfio_symbol_get_symbol( psymbols, i, name, 128, &value, &size,
                                 &bind, &type, &section_index, &other );
        printf( "[%4d] %10lu, %4lu %s\n", i, value, size, name );
    }
    elfio_symbol_section_accessor_delete( psymbols );
    */

    elfio_delete( pelfio );

    return 0;
}
