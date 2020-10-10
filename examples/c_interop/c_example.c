#include <stdio.h>

#include <elfio/elf_types.hpp>
#include "elfio_c_interop.h"

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



    elfio_delete( pelfio );

    return 0;
}
