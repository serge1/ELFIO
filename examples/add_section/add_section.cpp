#include <iostream>
#include <elfio/elfio.hpp>

using namespace ELFIO;

int main( int argc, char** argv )
{
    if ( argc != 2 ) {
        std::cout << "Usage: add_section <elf_file>" << std::endl;
        return 1;
    }

    // Create an elfio reader
    elfio reader;

    // Load ELF data
    if ( !reader.load( argv[1] ) ) {
        std::cout << "Can't find or process ELF file " << argv[1] << std::endl;
        return 2;
    }

    // Create additional section at the end of the file
    section* note_sec = reader.sections.add( ".note.ELFIO" );
    note_sec->set_type( SHT_NOTE );
    note_section_accessor note_writer( reader, note_sec );
    note_writer.add_note( 0x01, "Created by ELFIO", "My data", 8 );

    reader.save( "./result.elf" );

    return 0;
}
