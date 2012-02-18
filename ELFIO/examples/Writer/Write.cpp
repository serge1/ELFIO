#include <ELFIO.h>

int main( void )
{
    IELFO* pELFO;
    ELFIO::GetInstance()->CreateELFO( &pELFO );

    // You can't proceed without this function call!
    pELFO->SetAttr( ELFCLASS32, ELFDATA2LSB, EV_CURRENT,
                    ET_EXEC, EM_386, EV_CURRENT, 0 );

    // Create a loadable segment
    IELFOSegment* pSegment = pELFO->AddSegment( PT_LOAD,
                                                0x08040000,
                                                0x08040000,
                                                PF_X | PF_R,
                                                0x1000 );

    // Create code section
    IELFOSection* pTextSec = pELFO->AddSection( ".text",
                                                SHT_PROGBITS,
                                                SHF_ALLOC | SHF_EXECINSTR,
                                                0,
                                                0x10,
                                                0 );
    // Add data into it
    char text[] = { '\xB8', '\x04', '\x00', '\x00', '\x00',   // mov eax, 4
                    '\xBB', '\x01', '\x00', '\x00', '\x00',   // mov ebx, 1
                    '\xB9', '\xFD', '\x00', '\x04', '\x08',   // mov ecx, msg
                    '\xBA', '\x0E', '\x00', '\x00', '\x00',   // mov edx, 14
                    '\xCD', '\x80',                           // int 0x80
                    '\xB8', '\x01', '\x00', '\x00', '\x00',   // mov eax, 1
                    '\xCD', '\x80',                           // int 0x80
                    '\x48', '\x65', '\x6C', '\x6C', '\x6F',   // msg: db   'Hello, World!', 10
                    '\x2C', '\x20', '\x57', '\x6F', '\x72',
                    '\x6C', '\x64', '\x21', '\x0A'
                  };
    pTextSec->SetData( text, sizeof( text ) );

    // Add code section into program segment
    pSegment->AddSection( pTextSec );
    pTextSec->Release();
    pSegment->Release();

    // Set program entry point
    pELFO->SetEntry( 0x08040000 );
    // Create ELF file
    pELFO->Save( "test.elf" );

    pELFO->Release();

    return 0;
}
