#include <cstdio>
#include <ELFIO.h>

int main( int, char* argv[] )
{
    // Create a ELFI reader
    IELFI* pReader;
    ELFIO::GetInstance()->CreateELFI( &pReader );

    // Initialize it
    char* filename = argv[1];
    pReader->Load( filename );

    // Get .text relocation entry
    // List all sections of the file
    int i;
    int nSecNo = pReader->GetSectionsNum();
    for ( i = 0; i < nSecNo; ++i ) {    // For all sections
        const IELFISection* pSec = pReader->GetSection( i );
        if ( SHT_REL != pSec->GetType() && SHT_RELA != pSec->GetType() ) {
            pSec->Release();
            continue;
        }
        const IELFIRelocationTable* pRel = 0;
        pReader->CreateSectionReader( IELFI::ELFI_RELOCATION, pSec, (void**)&pRel );

        // Print all entries
        Elf64_Addr    offset;
        Elf64_Addr    symbolValue;
        std::string   symbolName;
        unsigned char type;
        Elf_Sxword    addend;
        Elf_Sxword    calcValue;
        Elf_Xword     nNum = pRel->GetEntriesNum();
        if ( 0 < nNum ) {
            std::printf( "\nSection name: %s\n", pSec->GetName().c_str() );
            std::printf( "  Num Type Offset   Addend    Calc   SymValue   SymName\n" );
            for ( Elf_Xword i = 0; i < nNum; ++i ) {
                pRel->GetEntry( i, offset, symbolValue, symbolName,
                                type, addend, calcValue );
                std::printf( "[%4llx] %02x %08llx %08llx %08llx %08llx %s\n",
                             i, type, offset,
                             addend, calcValue,
                             symbolValue, symbolName.c_str() );
            }
        }

        pSec->Release();
        pRel->Release();
    }

    // Free resources
    pReader->Release();

    return 0;
}
