#ifdef _MSC_VER
#define _SCL_SECURE_NO_WARNINGS
#endif

#define BOOST_TEST_MAIN
#ifndef _MSC_VER
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE ELFIO_Test
#include <boost/test/unit_test.hpp>

#include <elfio/elfio.hpp>

using namespace ELFIO;

////////////////////////////////////////////////////////////////////////////////
void
checkHeader( elfio& reader,
             unsigned char nClass,
             unsigned char encoding,
             unsigned char elfVersion,
             Elf_Half      type,
             Elf_Half      machine,
             Elf_Word      version,
             Elf64_Addr    entry,
             Elf_Word      flags,
             Elf_Half      secNum,
             Elf_Half      segNum,
             unsigned char OSABI,
             unsigned char ABIVersion )
{
    BOOST_CHECK_EQUAL( reader.get_class(),       nClass );
    BOOST_CHECK_EQUAL( reader.get_encoding(),    encoding );
    BOOST_CHECK_EQUAL( reader.get_elf_version(), elfVersion );
    BOOST_CHECK_EQUAL( reader.get_os_abi(),      OSABI );
    BOOST_CHECK_EQUAL( reader.get_abi_version(), ABIVersion );
    BOOST_CHECK_EQUAL( reader.get_type(),        type );
    BOOST_CHECK_EQUAL( reader.get_machine(),     machine );
    BOOST_CHECK_EQUAL( reader.get_version(),     version );
    BOOST_CHECK_EQUAL( reader.get_entry(),       entry );
    BOOST_CHECK_EQUAL( reader.get_flags(),       flags );
    BOOST_CHECK_EQUAL( reader.sections.size(),   secNum );
    BOOST_CHECK_EQUAL( reader.segments.size(),   segNum );
}


////////////////////////////////////////////////////////////////////////////////
void
checkSection( const section* sec,
              Elf_Half    index,
              std::string name,
              Elf_Word    type,
              Elf_Xword   flags,
              Elf64_Addr  address,
              Elf_Xword   size,
              Elf_Word    link,
              Elf_Word    info,
              Elf_Xword   addrAlign,
              Elf_Xword   entrySize )
{
    BOOST_CHECK_EQUAL( sec->get_index(),      index );
    BOOST_CHECK_EQUAL( sec->get_name(),       name );
    BOOST_CHECK_EQUAL( sec->get_type(),       type );
    BOOST_CHECK_EQUAL( sec->get_flags(),      flags );
    BOOST_CHECK_EQUAL( sec->get_address(),    address );
    BOOST_CHECK_EQUAL( sec->get_size(),       size );
    BOOST_CHECK_EQUAL( sec->get_link(),       link );
    BOOST_CHECK_EQUAL( sec->get_info(),       info );
    BOOST_CHECK_EQUAL( sec->get_addr_align(), addrAlign );
    BOOST_CHECK_EQUAL( sec->get_entry_size(), entrySize );
}


////////////////////////////////////////////////////////////////////////////////
void
checkSection( const section* sec,
              std::string name,
              Elf_Word    type,
              Elf_Xword   flags,
              Elf64_Addr  address,
              Elf_Xword   size,
              Elf_Word    link,
              Elf_Word    info,
              Elf_Xword   addrAlign,
              Elf_Xword   entrySize )
{
    checkSection( sec, sec->get_index(), name, type, flags, address, size, link,
                  info, addrAlign, entrySize );
}


////////////////////////////////////////////////////////////////////////////////
void
checkSegment( const segment* seg,
              Elf_Word   type,
              Elf64_Addr vaddr,
              Elf64_Addr paddr,
              Elf_Xword  fsize,
              Elf_Xword  msize,
              Elf_Word   flags,
              Elf_Xword  align )
{
    BOOST_CHECK_EQUAL( seg->get_type(),             type );
    BOOST_CHECK_EQUAL( seg->get_virtual_address(),  vaddr );
    BOOST_CHECK_EQUAL( seg->get_physical_address(), paddr );
    BOOST_CHECK_EQUAL( seg->get_file_size(),        fsize );
    BOOST_CHECK_EQUAL( seg->get_memory_size(),      msize );
    BOOST_CHECK_EQUAL( seg->get_flags(),            flags );
    BOOST_CHECK_EQUAL( seg->get_align(),            align );
}


////////////////////////////////////////////////////////////////////////////////
void
checkSymbol( const symbol_section_accessor& sr, Elf_Xword index,
             std::string name_, Elf64_Addr value_,
             Elf_Xword size_,
             unsigned char bind_, unsigned char type_,
             Elf_Half section_, unsigned char other_ )
{
    std::string   name;
    Elf64_Addr    value;
    Elf_Xword     size;
    unsigned char bind;
    unsigned char type;
    Elf_Half      section;
    unsigned char other;

    BOOST_REQUIRE_EQUAL( sr.get_symbol( index, name, value, size, bind, type, section, other ),
                         true );
    BOOST_CHECK_EQUAL( name,    name_ );
    BOOST_CHECK_EQUAL( value,   value_ );
    BOOST_CHECK_EQUAL( size,    size_ );
    BOOST_CHECK_EQUAL( bind,    bind_ );
    BOOST_CHECK_EQUAL( type,    type_ );
    BOOST_CHECK_EQUAL( section, section_ );
    BOOST_CHECK_EQUAL( other,   other_ );
}


////////////////////////////////////////////////////////////////////////////////
void
checkRelocation( const relocation_section_accessor* pRT, Elf_Xword index,
                 Elf64_Addr offset_, Elf64_Addr symbolValue_,
                 std::string symbolName_,
                 unsigned char type_, Elf_Sxword addend_,
                 Elf_Sxword calcValue_ )
{
    Elf64_Addr  offset;
    Elf64_Addr  symbolValue;
    std::string symbolName;
    Elf_Word    type;
    Elf_Sxword  addend;
    Elf_Sxword  calcValue;

    BOOST_REQUIRE_EQUAL( pRT->get_entry( index, offset, symbolValue, symbolName,
                                        type, addend, calcValue ),
                         true );
    BOOST_CHECK_EQUAL( offset,      offset_ );
    BOOST_CHECK_EQUAL( symbolValue, symbolValue_ );
    BOOST_CHECK_EQUAL( symbolName,  symbolName_ );
    BOOST_CHECK_EQUAL( type,        type_ );
    BOOST_CHECK_EQUAL( addend,      addend_ );
    BOOST_CHECK_EQUAL( calcValue,   calcValue_ );
}


////////////////////////////////////////////////////////////////////////////////
void
    checkNote( const note_section_accessor& notes, Elf_Word index,
                 Elf_Word type_, std::string name_,
                 Elf_Word descSize_ )
{
    Elf_Word    type;
    std::string name;
    void*       desc;
    Elf_Word    descSize;

    BOOST_REQUIRE_EQUAL( notes.get_note( index, type, name, desc, descSize ),
                         true );
    BOOST_CHECK_EQUAL( type,     type_ );
    BOOST_CHECK_EQUAL( name,     name_ );
    BOOST_CHECK_EQUAL( descSize, descSize_ );
}


////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( load32 )
{
    elfio reader;
    BOOST_REQUIRE_EQUAL( reader.load( "../elf_examples/hello_32" ), true );
    checkHeader( reader, ELFCLASS32, ELFDATA2LSB, EV_CURRENT, ET_EXEC,
                          EM_386, 1, 0x80482b0, 0, 28, 7, 0, 0 );

    ////////////////////////////////////////////////////////////////////////////
    // Check sections
    section* sec = reader.sections[0];
    checkSection( sec, 0, "", SHT_NULL, 0, 0, 0, 0, 0, 0, 0 );

    sec = reader.sections[1];
    checkSection( sec, 1, ".interp", SHT_PROGBITS, SHF_ALLOC,
                        0x08048114, 0x13, 0, 0, 1, 0 );

    sec = reader.sections[9];
    checkSection( sec, 9, ".rel.plt", SHT_REL, SHF_ALLOC,
                        0x08048234, 0x18, 4, 11, 4, 8 );

    sec = reader.sections[19];
    checkSection( sec, 19, ".dynamic", SHT_DYNAMIC, SHF_WRITE | SHF_ALLOC,
                        0x080494a0, 0xc8, 5, 0, 4, 8 );

    sec = reader.sections[27];
    checkSection( sec, 27, ".strtab", SHT_STRTAB, 0,
                        0x0, 0x259, 0, 0, 1, 0 );

    const section* sec1 = reader.sections[ ".strtab" ];
    BOOST_CHECK_EQUAL( sec->get_index(), sec1->get_index() );

    ////////////////////////////////////////////////////////////////////////////
    // Check segments
    segment* seg = reader.segments[0];
    checkSegment( seg, PT_PHDR, 0x08048034, 0x08048034,
                       0x000e0, 0x000e0, PF_R + PF_X, 4 );

    seg = reader.segments[4];
    checkSegment( seg, PT_DYNAMIC, 0x080494a0, 0x080494a0,
                        0x000c8, 0x000c8, PF_R + PF_W, 4 );

    seg = reader.segments[6];
    checkSegment( seg, 0x6474E551, 0x0, 0x0,
                       0x0, 0x0, PF_R + PF_W, 4 );

    ////////////////////////////////////////////////////////////////////////////
    // Check symbol table
    sec = reader.sections[ ".symtab" ];
    

    symbol_section_accessor sr( reader, sec );

    BOOST_CHECK_EQUAL( sr.get_symbols_num(), 68 );
    checkSymbol( sr,  0, "", 0x00000000, 0, STB_LOCAL, STT_NOTYPE, STN_UNDEF,
                 ELF_ST_VISIBILITY( STV_DEFAULT ) );
    checkSymbol( sr,  1, "", 0x08048114, 0, STB_LOCAL, STT_SECTION, 1,
                 ELF_ST_VISIBILITY( STV_DEFAULT ) );
    checkSymbol( sr, 39, "hello.c", 0x00000000, 0, STB_LOCAL, STT_FILE, SHN_ABS,
                 ELF_ST_VISIBILITY( STV_DEFAULT ) );
    checkSymbol( sr, 65, "__i686.get_pc_thunk.bx", 0x08048429, 0, STB_GLOBAL, STT_FUNC, 12,
                 ELF_ST_VISIBILITY( STV_HIDDEN ) );
    checkSymbol( sr, 66, "main", 0x08048384, 43, STB_GLOBAL, STT_FUNC, 12,
                 ELF_ST_VISIBILITY( STV_DEFAULT ) );
    checkSymbol( sr, 67, "_init", 0x0804824c, 0, STB_GLOBAL, STT_FUNC, 10,
                 ELF_ST_VISIBILITY( STV_DEFAULT ) );

    ////////////////////////////////////////////////////////////////////////////
    // Check relocation table
    sec = reader.sections[ ".rel.dyn" ];
    

    relocation_section_accessor reloc( reader, sec );
    BOOST_CHECK_EQUAL( reloc.get_entries_num(), 1 );

    checkRelocation( &reloc, 0, 0x08049568, 0x0, "__gmon_start__", R_386_GLOB_DAT, 0, 0 );

    sec = reader.sections[ ".rel.plt" ];
    

    relocation_section_accessor reloc1( reader, sec );
    BOOST_CHECK_EQUAL( reloc1.get_entries_num(), 3 );

    checkRelocation( &reloc1, 0, 0x08049578, 0x0, "__gmon_start__", R_X86_64_JUMP_SLOT, 0, 0 );
    checkRelocation( &reloc1, 1, 0x0804957c, 0x0, "__libc_start_main", R_X86_64_JUMP_SLOT, 0, 0 );
    checkRelocation( &reloc1, 2, 0x08049580, 0x0, "puts", R_X86_64_JUMP_SLOT, 0, 0 );

    ////////////////////////////////////////////////////////////////////////////
    // Check note reader
    sec = reader.sections[ ".note.ABI-tag" ];
    

    note_section_accessor notes( reader, sec );
    BOOST_CHECK_EQUAL( notes.get_notes_num(), 1 );

    checkNote( notes, 0, 1, std::string( "GNU" ), 16 );
}


////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( load64 )
{
    elfio reader;

    BOOST_REQUIRE_EQUAL( reader.load( "../elf_examples/hello_64" ), true );

    ////////////////////////////////////////////////////////////////////////////
    // Check ELF header
    checkHeader( reader, ELFCLASS64, ELFDATA2LSB, EV_CURRENT, ET_EXEC,
                          EM_X86_64, 1, 0x4003c0, 0, 29, 8, 0, 0 );

    ////////////////////////////////////////////////////////////////////////////
    // Check sections
    section* sec = reader.sections[ 0 ];
    
    checkSection( sec, 0, "", SHT_NULL, 0, 0, 0, 0, 0, 0, 0 );

    sec =reader.sections[ 1 ];
    
    checkSection( sec, 1, ".interp", SHT_PROGBITS, SHF_ALLOC,
                        0x0000000000400200, 0x1c, 0, 0, 1, 0 );

    sec =reader.sections[ 9 ];
    
    checkSection( sec, 9, ".rela.plt", SHT_RELA, SHF_ALLOC,
                        0x0000000000400340, 0x30, 4, 11, 8, 0x18 );

    sec =reader.sections[ 20 ];
    
    checkSection( sec, 20, ".dynamic", SHT_DYNAMIC, SHF_WRITE | SHF_ALLOC,
                        0x0000000000600698, 0x190, 5, 0, 8, 0x10 );

    sec =reader.sections[ 28 ];
    
    checkSection( sec, 28, ".strtab", SHT_STRTAB, 0,
                        0x0, 0x23f, 0, 0, 1, 0 );

    const section* sec1 = reader.sections[ ".strtab" ];
    BOOST_CHECK_EQUAL( sec->get_index(), sec1->get_index() );

    ////////////////////////////////////////////////////////////////////////////
    // Check segments
    segment* seg = reader.segments[0];
    checkSegment( seg, PT_PHDR, 0x0000000000400040, 0x0000000000400040,
                        0x00000000000001c0, 0x00000000000001c0, PF_R + PF_X, 8 );

    seg = reader.segments[2];
    checkSegment( seg, PT_LOAD, 0x0000000000400000, 0x0000000000400000,
                        0x000000000000066c, 0x000000000000066c, PF_R + PF_X, 0x200000 );

    seg = reader.segments[7];
    checkSegment( seg, 0x6474E551, 0x0, 0x0,
                        0x0, 0x0, PF_R + PF_W, 8 );

    ////////////////////////////////////////////////////////////////////////////
    // Check symbol table
    sec =reader.sections[ ".symtab" ];
    

    symbol_section_accessor sr( reader, sec );

    BOOST_CHECK_EQUAL( sr.get_symbols_num(), 67 );
    checkSymbol( sr,  0, "", 0x00000000, 0, STB_LOCAL, STT_NOTYPE, STN_UNDEF,
                 ELF_ST_VISIBILITY( STV_DEFAULT ) );
    checkSymbol( sr,  1, "", 0x00400200, 0, STB_LOCAL, STT_SECTION, 1,
                 ELF_ST_VISIBILITY( STV_DEFAULT ) );
    checkSymbol( sr, 40, "hello.c", 0x00000000, 0, STB_LOCAL, STT_FILE, SHN_ABS,
                 ELF_ST_VISIBILITY( STV_DEFAULT ) );
    checkSymbol( sr, 52, "__gmon_start__", 0x00000000, 0, STB_WEAK, STT_NOTYPE, STN_UNDEF,
                 ELF_ST_VISIBILITY( STV_DEFAULT ) );
    checkSymbol( sr, 64, "_edata", 0x0060085c, 0, STB_GLOBAL, STT_NOTYPE, SHN_ABS,
                 ELF_ST_VISIBILITY( STV_DEFAULT ) );
    checkSymbol( sr, 65, "main", 0x00400498, 21, STB_GLOBAL, STT_FUNC, 12,
                 ELF_ST_VISIBILITY( STV_DEFAULT ) );
    checkSymbol( sr, 66, "_init", 0x00400370, 0, STB_GLOBAL, STT_FUNC, 10,
                 ELF_ST_VISIBILITY( STV_DEFAULT ) );

    ////////////////////////////////////////////////////////////////////////////
    // Check relocation table
    sec =reader.sections[ ".rela.dyn" ];
    

    relocation_section_accessor reloc( reader, sec );
    BOOST_CHECK_EQUAL( reloc.get_entries_num(), 1 );

    checkRelocation( &reloc, 0, 0x00600828, 0x0, "__gmon_start__", R_X86_64_GLOB_DAT, 0, 0 );

    sec =reader.sections[ ".rela.plt" ];
    

    relocation_section_accessor reloc1( reader, sec );
    BOOST_CHECK_EQUAL( reloc1.get_entries_num(), 2 );

    checkRelocation( &reloc1, 0, 0x00600848, 0x0, "puts", R_X86_64_JUMP_SLOT, 0, 0 );
    checkRelocation( &reloc1, 1, 0x00600850, 0x0, "__libc_start_main", R_X86_64_JUMP_SLOT, 0, 0 );

    ////////////////////////////////////////////////////////////////////////////
    // Check note reader
    sec =reader.sections[ ".note.ABI-tag" ];
    

    note_section_accessor notes( reader, sec );
    BOOST_CHECK_EQUAL( notes.get_notes_num(), 1 );

    checkNote( notes, 0, 1, std::string( "GNU" ), 16 );
}


////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( hello_64_o )
{
    elfio reader;

    BOOST_REQUIRE_EQUAL( reader.load( "../elf_examples/hello_64.o" ), true );

    ////////////////////////////////////////////////////////////////////////////
    // Check ELF header
    checkHeader( reader, ELFCLASS64, ELFDATA2LSB, EV_CURRENT, ET_REL,
                          EM_X86_64, 1, 0, 0, 13, 0, 0, 0 );

    ////////////////////////////////////////////////////////////////////////////
    // Check sections
    section* sec = reader.sections[ 0 ];
    
    checkSection( sec, 0, "", SHT_NULL, 0, 0, 0, 0, 0, 0, 0 );

    sec =reader.sections[ 1 ];
    
    checkSection( sec, 1, ".text", SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR,
                        0x0, 0x15, 0, 0, 4, 0 );

    section* sec1 = reader.sections[ ".text" ];
    BOOST_CHECK_EQUAL( sec->get_index(), sec1->get_index() );

    sec = reader.sections[12];
    checkSection( sec, 12, ".strtab", SHT_STRTAB, 0,
                        0x0, 0x13, 0, 0, 1, 0 );

    sec1 = reader.sections[ ".strtab" ];
    BOOST_CHECK_EQUAL( sec->get_index(), sec1->get_index() );

    ////////////////////////////////////////////////////////////////////////////
    // Check symbol table
    sec =reader.sections[ ".symtab" ];
    

    symbol_section_accessor sr( reader, sec );

    BOOST_CHECK_EQUAL( sr.get_symbols_num(), 11 );
    checkSymbol( sr,  9, "main", 0x00000000, 21, STB_GLOBAL, STT_FUNC, 1,
                 ELF_ST_VISIBILITY( STV_DEFAULT ) );

    ////////////////////////////////////////////////////////////////////////////
    // Check relocation table
    sec =reader.sections[ ".rela.text" ];
    

    relocation_section_accessor reloc( reader, sec );
    BOOST_CHECK_EQUAL( reloc.get_entries_num(), 2 );

    checkRelocation( &reloc, 0, 0x00000005, 0x0, "", R_X86_64_32, 0, 0 );
    checkRelocation( &reloc, 1, 0x0000000A, 0x0, "puts", R_X86_64_PC32, 0xfffffffffffffffcULL, -14 );

    sec =reader.sections[ ".rela.eh_frame" ];
    

    relocation_section_accessor reloc1( reader, sec );
    BOOST_CHECK_EQUAL( reloc1.get_entries_num(), 1 );

    checkRelocation( &reloc1, 0, 0x00000020, 0x0, "", R_X86_64_32, 0, 0 );
}


////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( hello_32_o )
{
    elfio reader;

    BOOST_REQUIRE_EQUAL( reader.load( "../elf_examples/hello_32.o" ), true );

    ////////////////////////////////////////////////////////////////////////////
    // Check ELF header
    checkHeader( reader, ELFCLASS32, ELFDATA2LSB, EV_CURRENT, ET_REL,
                          EM_386, 1, 0, 0, 11, 0, 0, 0 );

    ////////////////////////////////////////////////////////////////////////////
    // Check sections
    section* sec = reader.sections[ 0 ];
    
    checkSection( sec, 0, "", SHT_NULL, 0, 0, 0, 0, 0, 0, 0 );

    sec =reader.sections[ 1 ];
    
    checkSection( sec, 1, ".text", SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR,
                        0x0, 0x2b, 0, 0, 4, 0 );

    section* sec1 = reader.sections[ ".text" ];
    BOOST_CHECK_EQUAL( sec->get_index(), sec1->get_index() );


    sec = reader.sections[ 10 ];
    
    checkSection( sec, 10, ".strtab", SHT_STRTAB, 0,
                        0x0, 0x13, 0, 0, 1, 0 );

    sec1 = reader.sections[ ".strtab" ];
    BOOST_CHECK_EQUAL( sec->get_index(), sec1->get_index() );

    ////////////////////////////////////////////////////////////////////////////
    // Check symbol table
    sec =reader.sections[ ".symtab" ];
    

    symbol_section_accessor sr( reader, sec );

    BOOST_CHECK_EQUAL( sr.get_symbols_num(), 10 );
    checkSymbol( sr,  8, "main", 0x00000000, 43, STB_GLOBAL, STT_FUNC, 1,
                 ELF_ST_VISIBILITY( STV_DEFAULT ) );

    ////////////////////////////////////////////////////////////////////////////
    // Check relocation table
    sec =reader.sections[ ".rel.text" ];
    

    relocation_section_accessor reloc( reader, sec );
    BOOST_CHECK_EQUAL( reloc.get_entries_num(), 2 );

    checkRelocation( &reloc, 0, 0x00000014, 0x0, "", R_386_32, 0, 0 );
    checkRelocation( &reloc, 1, 0x00000019, 0x0, "puts", R_386_PC32, 0x0, -25 );
}


////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( test_ppc_o )
{
    elfio reader;

    BOOST_REQUIRE_EQUAL( reader.load( "../elf_examples/test_ppc.o" ), true );

    ////////////////////////////////////////////////////////////////////////////
    // Check ELF header
    checkHeader( reader, ELFCLASS32, ELFDATA2MSB, EV_CURRENT, ET_REL,
                          EM_PPC, 1, 0, 0, 16, 0, 0, 0 );

    ////////////////////////////////////////////////////////////////////////////
    // Check sections
    section* sec = reader.sections[ 0 ];
    
    checkSection( sec, 0, "", SHT_NULL, 0, 0, 0, 0, 0, 0, 0 );

    sec =reader.sections[ 1 ];
    
    checkSection( sec, 1, ".text", SHT_PROGBITS, SHF_ALLOC | SHF_EXECINSTR,
                        0x0, 0x118, 0, 0, 4, 0 );

    section* sec1 = reader.sections[ ".text" ];
    BOOST_CHECK_EQUAL( sec->get_index(), sec1->get_index() );

    sec =reader.sections[ 15 ];
    
    checkSection( sec, 15, ".strtab", SHT_STRTAB, 0,
                        0x0, 0x14f, 0, 0, 1, 0 );

    sec1 = reader.sections[ ".strtab" ];
    BOOST_CHECK_EQUAL( sec->get_index(), sec1->get_index() );

    ////////////////////////////////////////////////////////////////////////////
    // Check symbol table
    sec =reader.sections[ ".symtab" ];
    

    symbol_section_accessor sr( reader, sec );

    BOOST_CHECK_EQUAL( sr.get_symbols_num(), 24 );
    checkSymbol( sr,  14, "main", 0x00000000, 92, STB_GLOBAL, STT_FUNC, 1,
                 ELF_ST_VISIBILITY( STV_DEFAULT ) );
    checkSymbol( sr,  8, "_GLOBAL__I_main", 0x000000DC, 60, STB_LOCAL, STT_FUNC, 1,
                 ELF_ST_VISIBILITY( STV_DEFAULT ) );

    ////////////////////////////////////////////////////////////////////////////
    // Check relocation table
    sec =reader.sections[ ".rela.text" ];
    

    relocation_section_accessor reloc( reader, sec );
    BOOST_CHECK_EQUAL( reloc.get_entries_num(), 18 );

    checkRelocation( &reloc,  0, 0x00000016, 0x0, "_ZSt4cout", 6, 0, 0 );
    checkRelocation( &reloc,  1, 0x0000001a, 0x0, "_ZSt4cout", 4, 0x0, 0 );
    checkRelocation( &reloc, 17, 0x000000c0, 0x0, "__cxa_atexit", 10, 0x0, 0 );

    sec =reader.sections[ ".rela.ctors" ];
    

    relocation_section_accessor reloc1( reader, sec );
    BOOST_CHECK_EQUAL( reloc1.get_entries_num(), 1 );

    checkRelocation( &reloc1,  0, 0x00000000, 0x0, "", 1, 0xDC, 0xDC );

    sec =reader.sections[ ".rela.eh_frame" ];
    

    relocation_section_accessor reloc2( reader, sec );
    BOOST_CHECK_EQUAL( reloc2.get_entries_num(), 3 );

    checkRelocation( &reloc2,  1, 0x00000020, 0x0, "", 1, 0x0, 0x0 );
}


////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( test_ppc )
{
    elfio reader;

    BOOST_REQUIRE_EQUAL( reader.load( "../elf_examples/test_ppc" ), true );

    ////////////////////////////////////////////////////////////////////////////
    // Check ELF header
    checkHeader( reader, ELFCLASS32, ELFDATA2MSB, EV_CURRENT, ET_EXEC,
                          EM_PPC, 1, 0x10000550, 0, 31, 8, 0, 0 );

    ////////////////////////////////////////////////////////////////////////////
    // Check sections
    section* sec = reader.sections[ 0 ];
    
    checkSection( sec, 0, "", SHT_NULL, 0, 0, 0, 0, 0, 0, 0 );

    sec =reader.sections[ 1 ];
    
    checkSection( sec, 1, ".interp", SHT_PROGBITS, SHF_ALLOC,
                        0x0000000010000134, 0xd, 0, 0, 1, 0 );

    sec =reader.sections[ 9 ];
    
    checkSection( sec, 9, ".rela.plt", SHT_RELA, SHF_ALLOC,
                        0x00000000010000494, 0x6c, 4, 22, 4, 0xc );

    sec =reader.sections[ 20 ];
    
    checkSection( sec, 20, ".dynamic", SHT_DYNAMIC, SHF_WRITE | SHF_ALLOC,
                        0x0000000010010aec, 0xe8, 5, 0, 4, 0x8 );

    sec =reader.sections[ 28 ];
    
    checkSection( sec, 28, ".shstrtab", SHT_STRTAB, 0,
                        0x0, 0x101, 0, 0, 1, 0 );

    const section* sec1 = reader.sections[ ".shstrtab" ];
    BOOST_CHECK_EQUAL( sec->get_index(), sec1->get_index() );


    ////////////////////////////////////////////////////////////////////////////
    // Check segments
    segment* seg = reader.segments[0];
    checkSegment( seg, PT_PHDR, 0x10000034, 0x10000034,
                        0x00100, 0x00100, PF_R + PF_X, 4 );

    seg = reader.segments[2];
    checkSegment( seg, PT_LOAD, 0x10000000, 0x10000000,
                        0x00acc, 0x00acc, PF_R + PF_X, 0x10000 );

    seg = reader.segments[7];
    checkSegment( seg, 0x6474E551, 0x0, 0x0,
                        0x0, 0x0, PF_R + PF_W, 0x4 );

    ////////////////////////////////////////////////////////////////////////////
    // Check symbol table
    sec =reader.sections[ ".symtab" ];
    

    symbol_section_accessor sr( reader, sec );

    BOOST_CHECK_EQUAL( sr.get_symbols_num(), 80 );
    checkSymbol( sr,  0, "", 0x00000000, 0, STB_LOCAL, STT_NOTYPE, STN_UNDEF,
                 ELF_ST_VISIBILITY( STV_DEFAULT ) );
    checkSymbol( sr,  1, "", 0x10000134, 0, STB_LOCAL, STT_SECTION, 1,
                 ELF_ST_VISIBILITY( STV_DEFAULT ) );
    checkSymbol( sr, 40, "__CTOR_END__", 0x10010AD4, 0, STB_LOCAL, STT_OBJECT, 16,
                 ELF_ST_VISIBILITY( STV_DEFAULT ) );
    checkSymbol( sr, 52, "__init_array_start", 0x10010acc, 0, STB_LOCAL, STT_NOTYPE, 16,
                 ELF_ST_VISIBILITY( STV_HIDDEN ) );
    checkSymbol( sr, 64, "_ZNSt8ios_base4InitD1Ev@@GLIBCXX_3.4", 0x10000920, 204, STB_GLOBAL, STT_FUNC, SHN_UNDEF,
                 ELF_ST_VISIBILITY( STV_DEFAULT ) );
    checkSymbol( sr, 78, "main", 0x1000069c, 92, STB_GLOBAL, STT_FUNC, 11,
                 ELF_ST_VISIBILITY( STV_DEFAULT ) );
    checkSymbol( sr, 79, "_init", 0x10000500, 0, STB_GLOBAL, STT_FUNC, 10,
                 ELF_ST_VISIBILITY( STV_DEFAULT ) );

    ////////////////////////////////////////////////////////////////////////////
    // Check relocation table
    sec =reader.sections[ ".rela.dyn" ];
    

    relocation_section_accessor reloc( reader, sec );
    BOOST_CHECK_EQUAL( reloc.get_entries_num(), 2 );

    checkRelocation( &reloc, 1, 0x10010c0c, 0x10010c0c, "_ZSt4cout", 19, 0, 0 );

    sec =reader.sections[ ".rela.plt" ];
    

    relocation_section_accessor reloc1( reader, sec );
    BOOST_CHECK_EQUAL( reloc1.get_entries_num(), 9 );

    checkRelocation( &reloc1, 0, 0x10010be4, 0x100008e0, "__cxa_atexit", 21, 0, 0 );
    checkRelocation( &reloc1, 1, 0x10010be8, 0x0, "__gmon_start__", 21, 0, 0 );

    ////////////////////////////////////////////////////////////////////////////
    // Check note reader
    sec =reader.sections[ ".note.ABI-tag" ];
    

    note_section_accessor notes( reader, sec );
    BOOST_CHECK_EQUAL( notes.get_notes_num(), 1 );

    checkNote( notes, 0, 1, std::string( "GNU" ), 16 );
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( test_dummy_out_i386_32 )
{
    elfio writer;

    writer.create( ELFCLASS32, ELFDATA2LSB );
    
    writer.set_os_abi( 0 );
    writer.set_abi_version( 0 );
    writer.set_type( ET_REL );
    writer.set_machine( EM_386 );
    writer.set_flags( 0 );

    // Set program entry point
    writer.set_entry( 0x80482b0 );

    // Add Note section
    section* note_sec = writer.sections.add( ".note" );
    note_sec->set_type( SHT_NOTE );
    note_sec->set_flags( SHF_ALLOC );
    note_sec->set_addr_align( 4 );
    note_section_accessor note_writer( writer, note_sec );
    char descr[6] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
    note_writer.add_note( 0x77, "Hello", &descr, 6 );
    BOOST_CHECK_EQUAL( note_sec->get_index(), 2 );

    // Create ELF file
    writer.save( "../elf_examples/elf_dummy_header_i386_32.elf" );

    elfio reader;
    BOOST_REQUIRE_EQUAL( reader.load( "../elf_examples/elf_dummy_header_i386_32.elf" ),
                         true );

    ////////////////////////////////////////////////////////////////////////////
    // Check ELF header
    checkHeader( reader, ELFCLASS32, ELFDATA2LSB, EV_CURRENT, ET_REL,
                             EM_386, EV_CURRENT, 0x80482b0, 0, 3, 0, 0, 0 );
    ////////////////////////////////////////////////////////////////////////////
    // Check sections
    section* sec = reader.sections[ "" ];
    
    checkSection( sec, 0, "", SHT_NULL, 0, 0, 0, 0, 0, 0, 0 );

    sec = reader.sections[ ".shstrtab" ];
    
    checkSection( sec, 1, ".shstrtab", SHT_STRTAB, 0,
                        0, 17, 0, 0, 1, 0 );

    sec =reader.sections[ ".note" ];
    
    BOOST_CHECK_EQUAL( sec->get_index(), 2 );
    checkSection( sec, 2, ".note", SHT_NOTE, SHF_ALLOC,
                        0, 28, 0, 0, 4, 0 );
}


////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( test_dummy_out_ppc_32 )
{
    elfio writer;

    writer.create( ELFCLASS32, ELFDATA2MSB );

    writer.set_os_abi( 0 );
    writer.set_abi_version( 0 );
    writer.set_type( ET_REL );
    writer.set_machine( EM_PPC );
    writer.set_flags( 0 );

    // Set program entry point
    writer.set_entry( 0x80482b0 );

    // Add Note section
    section* note_sec = writer.sections.add( ".note" );
    note_sec->set_type( SHT_NOTE );
    note_sec->set_flags( SHF_ALLOC );
    note_sec->set_addr_align( 4 );
    note_section_accessor note_writer( writer, note_sec );
    char descr[6] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
    note_writer.add_note( 0x77, "Hello", &descr, 6 );
    BOOST_CHECK_EQUAL( note_sec->get_index(), 2 );

    // Create ELF file
    writer.save( "../elf_examples/elf_dummy_header_ppc_32.elf" );

    elfio reader;
    BOOST_REQUIRE_EQUAL( reader.load( "../elf_examples/elf_dummy_header_ppc_32.elf" ),
                         true );

    ////////////////////////////////////////////////////////////////////////////
    // Check ELF header
    checkHeader( reader, ELFCLASS32, ELFDATA2MSB, EV_CURRENT, ET_REL,
                             EM_PPC, EV_CURRENT, 0x80482b0, 0, 3, 0, 0, 0 );
    ////////////////////////////////////////////////////////////////////////////
    // Check sections
    section* sec = reader.sections[ "" ];
    
    checkSection( sec, 0, "", SHT_NULL, 0, 0, 0, 0, 0, 0, 0 );

    sec =reader.sections[ ".note" ];
    
    BOOST_CHECK_EQUAL( sec->get_index(), 2 );
    checkSection( sec, 2, ".note", SHT_NOTE, SHF_ALLOC,
                        0, 28, 0, 0, 4, 0 );

    sec =reader.sections[ ".shstrtab" ];
    
    checkSection( sec, 1, ".shstrtab", SHT_STRTAB, 0,
                        0, 17, 0, 0, 1, 0 );
}


////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( test_dummy_out_i386_64 )
{
    elfio writer;

    writer.create( ELFCLASS64, ELFDATA2LSB );

    writer.set_os_abi( 0 );
    writer.set_abi_version( 0 );
    writer.set_type( ET_REL );
    writer.set_machine( EM_X86_64 );
    writer.set_flags( 0 );

    // Set program entry point
    writer.set_entry( 0x120380482b0ull );

    // Add Note section
    section* note_sec = writer.sections.add( ".note" );
    note_sec->set_type( SHT_NOTE );
    note_sec->set_flags( SHF_ALLOC );
    note_sec->set_addr_align( 4 );
    note_section_accessor note_writer( writer, note_sec );
    char descr[6] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
    note_writer.add_note( 0x77, "Hello", &descr, 6 );
    BOOST_CHECK_EQUAL( note_sec->get_index(), 2 );

    // Create ELF file
    writer.save( "../elf_examples/elf_dummy_header_i386_64.elf" );

    elfio reader;
    BOOST_REQUIRE_EQUAL( reader.load( "../elf_examples/elf_dummy_header_i386_64.elf" ),
                         true );

    ////////////////////////////////////////////////////////////////////////////
    // Check ELF header
    checkHeader( reader, ELFCLASS64, ELFDATA2LSB, EV_CURRENT, ET_REL,
                         EM_X86_64, EV_CURRENT, 0x120380482b0ull, 0, 3, 0, 0, 0 );
    ////////////////////////////////////////////////////////////////////////////
    // Check sections
    section* sec = reader.sections[ "" ];
    
    checkSection( sec, 0, "", SHT_NULL, 0, 0, 0, 0, 0, 0, 0 );

    sec =reader.sections[ ".note" ];
    
    BOOST_CHECK_EQUAL( sec->get_index(), 2 );
    checkSection( sec, 2, ".note", SHT_NOTE, SHF_ALLOC,
                        0, 28, 0, 0, 4, 0 );

    sec =reader.sections[ ".shstrtab" ];
    
    checkSection( sec, 1, ".shstrtab", SHT_STRTAB, 0,
                        0, 17, 0, 0, 1, 0 );
}


////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( test_dummy_out_ppc_64 )
{
    elfio writer;

    writer.create( ELFCLASS64, ELFDATA2MSB );

    writer.set_os_abi( 0 );
    writer.set_abi_version( 0 );
    writer.set_type( ET_REL );
    writer.set_machine( EM_PPC64 );
    writer.set_flags( 0 );

    // Set program entry point
    writer.set_entry( 0x120380482b0ull );

    // Add Note section
    section* note_sec = writer.sections.add( ".note" );
    note_sec->set_type( SHT_NOTE );
    note_sec->set_flags( SHF_ALLOC );
    note_sec->set_addr_align( 4 );
    note_section_accessor note_writer( writer, note_sec );
    char descr[6] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16};
    note_writer.add_note( 0x77, "Hello", &descr, 6 );
    BOOST_CHECK_EQUAL( note_sec->get_index(), 2 );

    // Create ELF file
    writer.save( "../elf_examples/elf_dummy_header_ppc_64.elf" );

    elfio reader;
    BOOST_REQUIRE_EQUAL( reader.load( "../elf_examples/elf_dummy_header_ppc_64.elf" ),
                         true );

    ////////////////////////////////////////////////////////////////////////////
    // Check ELF header
    checkHeader( reader, ELFCLASS64, ELFDATA2MSB, EV_CURRENT, ET_REL,
                         EM_PPC64, EV_CURRENT, 0x120380482b0ull, 0, 3, 0, 0, 0 );
    ////////////////////////////////////////////////////////////////////////////
    // Check sections
    section* sec = reader.sections[ "" ];
    
    checkSection( sec, 0, "", SHT_NULL, 0, 0, 0, 0, 0, 0, 0 );

    sec =reader.sections[ ".shstrtab" ];
    
    checkSection( sec, 1, ".shstrtab", SHT_STRTAB, 0,
                        0, 17, 0, 0, 1, 0 );

    sec = reader.sections[ ".note" ];
    
    BOOST_CHECK_EQUAL( sec->get_index(), 2 );
    checkSection( sec, 2, ".note", SHT_NOTE, SHF_ALLOC,
                        0, 28, 0, 0, 4, 0 );
}


////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( test_dynamic_64_1 )
{
    elfio reader;

    reader.load( "../elf_examples/main" );

    section* dynsec = reader.sections[".dynamic"];

    dynamic_section_accessor da( reader, dynsec );

    BOOST_CHECK_EQUAL( da.get_entries_num(), 26 );

    Elf_Xword   tag;
    Elf_Xword   value;
    std::string str;
    da.get_entry( 0, tag, value, str );
    BOOST_CHECK_EQUAL( tag, DT_NEEDED );
    BOOST_CHECK_EQUAL( str, "libfunc.so" );
    da.get_entry( 1, tag, value, str );
    BOOST_CHECK_EQUAL( tag, DT_NEEDED );
    BOOST_CHECK_EQUAL( str, "libc.so.6" );
    da.get_entry( 2, tag, value, str );
    BOOST_CHECK_EQUAL( tag, DT_INIT );
    BOOST_CHECK_EQUAL( value, 0x400530 );
    da.get_entry( 19, tag, value, str );
    BOOST_CHECK_EQUAL( tag, 0x6ffffff0 );
    BOOST_CHECK_EQUAL( value, 0x40047e );
    da.get_entry( 20, tag, value, str );
    BOOST_CHECK_EQUAL( tag, DT_NULL );
    BOOST_CHECK_EQUAL( value, 0 );
}


////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( test_dynamic_64_2 )
{
    elfio reader;

    reader.load( "../elf_examples/libfunc.so" );

    section* dynsec = reader.sections[".dynamic"];

    dynamic_section_accessor da( reader, dynsec );

    BOOST_CHECK_EQUAL( da.get_entries_num(), 24 );

    Elf_Xword   tag;
    Elf_Xword   value;
    std::string str;
    da.get_entry( 0, tag, value, str );
    BOOST_CHECK_EQUAL( tag, DT_NEEDED );
    BOOST_CHECK_EQUAL( str, "libc.so.6" );
    da.get_entry( 1, tag, value, str );
    BOOST_CHECK_EQUAL( tag, DT_INIT );
    BOOST_CHECK_EQUAL( value, 0x480 );
    da.get_entry( 18, tag, value, str );
    BOOST_CHECK_EQUAL( tag, 0x6ffffff9 );
    BOOST_CHECK_EQUAL( value, 1 );
    da.get_entry( 19, tag, value, str );
    BOOST_CHECK_EQUAL( tag, DT_NULL );
    BOOST_CHECK_EQUAL( value, 0 );
}


////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( test_dynamic_64_3 )
{
    elfio reader;

    reader.load( "../elf_examples/main" );

    section* dynsec = reader.sections[".dynamic"];
    dynamic_section_accessor da( reader, dynsec );
    BOOST_CHECK_EQUAL( da.get_entries_num(), 26 );

    section* strsec1 = reader.sections.add( ".dynstr" );
    strsec1->set_type( SHT_STRTAB );
    strsec1->set_entry_size( reader.get_default_entry_size( SHT_STRTAB ) );

    section* dynsec1 = reader.sections.add( ".dynamic1" );
    dynsec1->set_type( SHT_DYNAMIC );
    dynsec1->set_entry_size( reader.get_default_entry_size( SHT_DYNAMIC ) );
    dynsec1->set_link( strsec1->get_index() );
    dynamic_section_accessor da1( reader, dynsec1 );

    Elf_Xword   tag,   tag1;
    Elf_Xword   value, value1;
    std::string str,   str1;
    
    for ( unsigned int i = 0; i < da.get_entries_num(); ++i ) {
        da.get_entry( i, tag, value, str );
        if ( tag == DT_NEEDED ||
             tag == DT_SONAME ||
             tag == DT_RPATH  ||
             tag == DT_RUNPATH ) {
            da1.add_entry( tag, str );
        }
        else {
            da1.add_entry( tag, value );
        }
    }
    
    for ( unsigned int i = 0; i < da.get_entries_num(); ++i ) {
        da.get_entry( i, tag, value, str );
        da1.get_entry( i, tag1, value1, str1 );

        BOOST_CHECK_EQUAL( tag, tag1 );
        if ( tag == DT_NEEDED ||
             tag == DT_SONAME ||
             tag == DT_RPATH  ||
             tag == DT_RUNPATH ) {
            BOOST_CHECK_EQUAL( str, str1 );
        }
        else {
            BOOST_CHECK_EQUAL( value, value1 );
        }
    }
}
