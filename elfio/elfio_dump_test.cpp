#include <iostream>
#include <elfio_dump.hpp>

using namespace ELFIO;

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
    dump::header         ( std::cout, reader );
    dump::section_headers( std::cout, reader );

    return 0;
}
