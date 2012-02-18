/*
 * This example shows how to create an ELF object file for Linux on x86
 *
 * Instructions:
 * 1. Compile and link this file with ELFIO library
 * 2. Execute result file WriteObj
 * 3. Link output file test.o:
 *    gcc -s -nostartfiles test.o -o test
 */

#include <ELFIO.h>

int main( void )
{
    IELFO* pELFO;
    ELFIO::GetInstance()->CreateELFO( &pELFO );

    // You can't proceed before this function call!
    pELFO->SetAttr( ELFCLASS32, ELFDATA2LSB, EV_CURRENT,
                    ET_REL, EM_386, EV_CURRENT, 0 );

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
                    '\xB9', '\x00', '\x00', '\x00', '\x00',   // mov ecx, msg		      
                    '\xBA', '\x0E', '\x00', '\x00', '\x00',   // mov edx, 14		      
                    '\xCD', '\x80',                           // int 0x80		      
                    '\xB8', '\x01', '\x00', '\x00', '\x00',   // mov eax, 1		      
                    '\xCD', '\x80',                           // int 0x80		      
                    '\x48', '\x65', '\x6C', '\x6C', '\x6F',   // msg: db   'Hello, World!', 10
                    '\x2C', '\x20', '\x57', '\x6F', '\x72',
                    '\x6C', '\x64', '\x21', '\x0A'
                  };
    pTextSec->SetData( text, sizeof( text ) );

    // Create string table section
    IELFOSection* pStrSec = pELFO->AddSection( ".strtab",
                                               SHT_STRTAB,
                                               0,
                                               0,
                                               1,
                                               0 );
    // Create string table writer
    IELFOStringWriter* pStrWriter = 0;
    pELFO->CreateSectionWriter( IELFO::ELFO_STRING, pStrSec, (void**)&pStrWriter );
    // Add label name
    Elf32_Word nStrIndex = pStrWriter->AddString( "msg" );
    pStrSec->Release();

    // Create symbol table section
    IELFOSection* pSymSec = pELFO->AddSection( ".symtab",
                                               SHT_SYMTAB,
                                               0,
                                               2,
                                               4,
                                               sizeof(Elf32_Sym) );
    pSymSec->SetLink( pStrSec->GetIndex() );
    // Create symbol table writer
    IELFOSymbolTable* pSymWriter = 0;
    pELFO->CreateSectionWriter( IELFO::ELFO_SYMBOL, pSymSec, (void**)&pSymWriter );
    // Add symbol entry (msg has offset == 29)
    Elf32_Word nSymIndex = pSymWriter->AddEntry( nStrIndex, 29, 0,
                                                 STB_GLOBAL, STT_OBJECT, 0,
                                                 pTextSec->GetIndex() );

    // Or another way to add symbol
    pSymWriter->AddEntry( pStrWriter, "_start", 0x00000000, 0,
                          STB_WEAK, STT_FUNC, 0,
                          pTextSec->GetIndex() );

    // Create relocation table section
    IELFOSection* pRelSec = pELFO->AddSection( ".rel.text",
                                               SHT_REL,
                                               0,
                                               pTextSec->GetIndex(),
                                               4,
                                               sizeof(Elf32_Rel) );
    pRelSec->SetLink( pSymSec->GetIndex() );
    // Create relocation table writer
    IELFORelocationTable* pRelWriter = 0;
    pELFO->CreateSectionWriter( IELFO::ELFO_RELOCATION, pRelSec, (void**)&pRelWriter );
    // Add relocation entry (adjust address at offset 11)
    pRelWriter->AddEntry( 11, nSymIndex, (unsigned char)R_386_RELATIVE );

    // Another method to add the same relocation entry
    // pRelWriter->AddEntry( pStrWriter, "msg",
    //                       pSymWriter, 29, 0,
    //                       ELF32_ST_INFO( STB_GLOBAL, STT_OBJECT ), 0,
    //                       pTextSec->GetIndex(),
    //                       11, (unsigned char)R_386_RELATIVE );

    pStrWriter->Release();
    pSymWriter->Release();
    pRelWriter->Release();

    pTextSec->Release();
    pSymSec->Release();
    pRelSec->Release();

    // Create note section
    IELFOSection* pNoteSec = pELFO->AddSection( ".note",
                                                SHT_NOTE,
                                                0,
                                                0,
                                                1,
                                                0 );
    // Create notes writer
    IELFONotesWriter* pNoteWriter = 0;
    pELFO->CreateSectionWriter( IELFO::ELFO_NOTE, pNoteSec, (void**)&pNoteWriter );
    // Add new entry
    pNoteWriter->AddNote( 1, "Created by ELFIO", 0, 0 );
    pNoteWriter->Release();
    pNoteSec->Release();

    // Create ELF file
    pELFO->Save( "test.o" );

    pELFO->Release();

    return 0;
}
