/*
Copyright (C) 2001-present by Serge Lamikhov-Center

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

#include <boost/test/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>
using boost::test_tools::output_test_stream;

#include <elfio/elfio.hpp>

using namespace ELFIO;

enum Tests
{
    SEG_ALIGN = 1
};

////////////////////////////////////////////////////////////////////////////////
bool write_obj_i386( bool is64bit )
{
    elfio writer;

    writer.create( is64bit ? ELFCLASS64 : ELFCLASS32, ELFDATA2LSB );
    writer.set_type( ET_REL );
    writer.set_os_abi( ELFOSABI_LINUX );
    writer.set_machine( is64bit ? EM_X86_64 : EM_386 );

    // Create code section*
    section* text_sec = writer.sections.add( ".text" );
    text_sec->set_type( SHT_PROGBITS );
    text_sec->set_flags( SHF_ALLOC | SHF_EXECINSTR );
    text_sec->set_addr_align( 0x10 );

    // Add data into it
    char text[] = {
        '\xB8', '\x04', '\x00', '\x00', '\x00', // mov eax, 4
        '\xBB', '\x01', '\x00', '\x00', '\x00', // mov ebx, 1
        '\xB9', '\x00', '\x00', '\x00', '\x00', // mov ecx, msg
        '\xBA', '\x0E', '\x00', '\x00', '\x00', // mov edx, 14
        '\xCD', '\x80',                         // int 0x80
        '\xB8', '\x01', '\x00', '\x00', '\x00', // mov eax, 1
        '\xCD', '\x80'                          // int 0x80
    };
    text_sec->set_data( text, sizeof( text ) );

    // Create data section*
    section* data_sec = writer.sections.add( ".data" );
    data_sec->set_type( SHT_PROGBITS );
    data_sec->set_flags( SHF_ALLOC | SHF_WRITE );
    data_sec->set_addr_align( 4 );

    char data[] = {
        '\x48', '\x65', '\x6C', '\x6C', '\x6F', // msg: db   'Hello, World!', 10
        '\x2C', '\x20', '\x57', '\x6F', '\x72',
        '\x6C', '\x64', '\x21', '\x0A' };
    data_sec->set_data( data, sizeof( data ) );

    section* str_sec = writer.sections.add( ".strtab" );
    str_sec->set_type( SHT_STRTAB );
    str_sec->set_addr_align( 0x1 );

    string_section_accessor str_writer( str_sec );
    Elf_Word                nStrIndex = str_writer.add_string( "msg" );

    section* sym_sec = writer.sections.add( ".symtab" );
    sym_sec->set_type( SHT_SYMTAB );
    sym_sec->set_info( 2 );
    sym_sec->set_link( str_sec->get_index() );
    sym_sec->set_addr_align( 4 );
    sym_sec->set_entry_size( writer.get_default_entry_size( SHT_SYMTAB ) );

    symbol_section_accessor symbol_writer( writer, sym_sec );
    Elf_Word                nSymIndex = symbol_writer.add_symbol(
        nStrIndex, 0, 0, STB_LOCAL, STT_NOTYPE, 0, data_sec->get_index() );

    // Another way to add symbol
    symbol_writer.add_symbol( str_writer, "_start", 0x00000000, 0, STB_WEAK,
                              STT_FUNC, 0, text_sec->get_index() );

    // Create relocation table section*
    section* rel_sec = writer.sections.add( ".rel.text" );
    rel_sec->set_type( SHT_REL );
    rel_sec->set_info( text_sec->get_index() );
    rel_sec->set_link( sym_sec->get_index() );
    rel_sec->set_addr_align( 4 );
    rel_sec->set_entry_size( writer.get_default_entry_size( SHT_REL ) );

    relocation_section_accessor rel_writer( writer, rel_sec );
    rel_writer.add_entry( 11, nSymIndex, (unsigned char)R_386_RELATIVE );

    // Another method to add the same relocation entry
    // pRelWriter->AddEntry( pStrWriter, "msg",
    //                       pSymWriter, 29, 0,
    //                       ELF32_ST_INFO( STB_GLOBAL, STT_OBJECT ), 0,
    //                       data_sec->GetIndex(),
    //                       0, (unsigned char)R_386_RELATIVE );

    // Create note section*
    section* note_sec = writer.sections.add( ".note" );
    note_sec->set_type( SHT_NOTE );
    note_sec->set_addr_align( 1 );

    // Create notes writer
    note_section_accessor note_writer( writer, note_sec );
    note_writer.add_note( 0x77, "Created by ELFIO", 0, 0 );

    // Create ELF file
    writer.save( is64bit ? "elf_examples/write_obj_i386_64.o"
                         : "elf_examples/write_obj_i386_32.o" );

    return true;
}

////////////////////////////////////////////////////////////////////////////////
bool write_exe_i386( const std::string& filename,
                     bool               is64bit,
                     bool               set_addr = false,
                     Elf64_Addr         addr     = 0 )
{
    elfio writer;

    writer.create( is64bit ? ELFCLASS64 : ELFCLASS32, ELFDATA2LSB );
    writer.set_os_abi( ELFOSABI_LINUX );
    writer.set_type( ET_EXEC );
    writer.set_machine( is64bit ? EM_X86_64 : EM_386 );

    // Create code section*
    section* text_sec = writer.sections.add( ".text" );
    text_sec->set_type( SHT_PROGBITS );
    text_sec->set_flags( SHF_ALLOC | SHF_EXECINSTR );
    text_sec->set_addr_align( 0x10 );
    if ( set_addr ) {
        text_sec->set_address( addr );
    }

    // Add data into it
    char text[] = {
        '\xB8', '\x04', '\x00', '\x00', '\x00', // mov eax, 4
        '\xBB', '\x01', '\x00', '\x00', '\x00', // mov ebx, 1
        '\xB9', '\x20', '\x80', '\x04', '\x08', // mov ecx, msg
        '\xBA', '\x0E', '\x00', '\x00', '\x00', // mov edx, 14
        '\xCD', '\x80',                         // int 0x80
        '\xB8', '\x01', '\x00', '\x00', '\x00', // mov eax, 1
        '\xCD', '\x80'                          // int 0x80
    };
    text_sec->set_data( text, sizeof( text ) );

    segment* text_seg = writer.segments.add();
    text_seg->set_type( PT_LOAD );
    text_seg->set_virtual_address( 0x08048000 );
    text_seg->set_physical_address( 0x08048000 );
    text_seg->set_flags( PF_X | PF_R );
    text_seg->set_align( 0x1000 );
    text_seg->add_section( text_sec, text_sec->get_addr_align() );

    // Create data section*
    section* data_sec = writer.sections.add( ".data" );
    data_sec->set_type( SHT_PROGBITS );
    data_sec->set_flags( SHF_ALLOC | SHF_WRITE );
    data_sec->set_addr_align( 0x4 );

    char data[] = {
        '\x48', '\x65', '\x6C', '\x6C', '\x6F', // msg: db   'Hello, World!', 10
        '\x2C', '\x20', '\x57', '\x6F', '\x72',
        '\x6C', '\x64', '\x21', '\x0A' };
    data_sec->set_data( data, sizeof( data ) );

    segment* data_seg = writer.segments.add();
    data_seg->set_type( PT_LOAD );
    data_seg->set_virtual_address( 0x08048020 );
    data_seg->set_physical_address( 0x08048020 );
    data_seg->set_flags( PF_W | PF_R );
    data_seg->set_align( 0x10 );
    data_seg->add_section_index( data_sec->get_index(),
                                 data_sec->get_addr_align() );

    section* note_sec = writer.sections.add( ".note" );
    note_sec->set_type( SHT_NOTE );
    note_sec->set_addr_align( 1 );
    note_section_accessor note_writer( writer, note_sec );
    note_writer.add_note( 0x01, "Created by ELFIO", 0, 0 );
    char descr[6] = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36 };
    note_writer.add_note( 0x01, "Never easier!", descr, sizeof( descr ) );

    // Create ELF file
    writer.set_entry( 0x08048000 );

    writer.save( filename );

    return true;
}

////////////////////////////////////////////////////////////////////////////////
void checkObjestsAreEqual( std::string file_name1, std::string file_name2 )
{
    elfio file1;
    elfio file2;
    BOOST_REQUIRE_EQUAL( file1.load( file_name1 ), true );
    BOOST_CHECK_EQUAL( file1.save( file_name2 ), true );
    BOOST_REQUIRE_EQUAL( file1.load( file_name1 ), true );
    BOOST_REQUIRE_EQUAL( file2.load( file_name2 ), true );

    for ( int i = 0; i < file1.sections.size(); ++i ) {
        BOOST_CHECK_EQUAL( file1.sections[i]->get_address(),
                           file2.sections[i]->get_address() );
        BOOST_CHECK_EQUAL( file1.sections[i]->get_addr_align(),
                           file2.sections[i]->get_addr_align() );
        BOOST_CHECK_EQUAL( file1.sections[i]->get_entry_size(),
                           file2.sections[i]->get_entry_size() );
        BOOST_CHECK_EQUAL( file1.sections[i]->get_flags(),
                           file2.sections[i]->get_flags() );
        BOOST_CHECK_EQUAL( file1.sections[i]->get_index(),
                           file2.sections[i]->get_index() );
        BOOST_CHECK_EQUAL( file1.sections[i]->get_info(),
                           file2.sections[i]->get_info() );
        BOOST_CHECK_EQUAL( file1.sections[i]->get_link(),
                           file2.sections[i]->get_link() );
        BOOST_CHECK_EQUAL( file1.sections[i]->get_name(),
                           file2.sections[i]->get_name() );
        BOOST_CHECK_EQUAL( file1.sections[i]->get_name_string_offset(),
                           file2.sections[i]->get_name_string_offset() );
        BOOST_CHECK_EQUAL( file1.sections[i]->get_size(),
                           file2.sections[i]->get_size() );
        BOOST_CHECK_EQUAL( file1.sections[i]->get_type(),
                           file2.sections[i]->get_type() );

        if ( file1.sections[i]->get_type() == SHT_NULL ||
             file1.sections[i]->get_type() == SHT_NOBITS ) {
            continue;
        }
        BOOST_REQUIRE_NE( file1.sections[i]->get_data(), (const char*)0 );
        BOOST_REQUIRE_NE( file2.sections[i]->get_data(), (const char*)0 );
        std::string pdata1( file1.sections[i]->get_data(),
                            file1.sections[i]->get_data() +
                                file1.sections[i]->get_size() );
        std::string pdata2( file2.sections[i]->get_data(),
                            file2.sections[i]->get_data() +
                                file2.sections[i]->get_size() );

        BOOST_CHECK_EQUAL( file1.sections[i]->get_size(),
                           file2.sections[i]->get_size() );
        if ( ( file2.sections[i]->get_type() != SHT_NULL ) &&
             ( file2.sections[i]->get_type() != SHT_NOBITS ) ) {
            BOOST_CHECK_EQUAL_COLLECTIONS( pdata1.begin(), pdata1.end(),
                                           pdata2.begin(), pdata2.end() );
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void checkExeAreEqual( std::string file_name1,
                       std::string file_name2,
                       int         skipTests = 0 )
{
    checkObjestsAreEqual( file_name1, file_name2 );

    elfio file1;
    elfio file2;

    BOOST_REQUIRE_EQUAL( file1.load( file_name1 ), true );
    BOOST_REQUIRE_EQUAL( file2.load( file_name2 ), true );

    for ( int i = 0; i < file1.segments.size(); ++i ) {
        if ( !( skipTests & SEG_ALIGN ) )
            BOOST_CHECK_EQUAL( file1.segments[i]->get_align(),
                               file2.segments[i]->get_align() );
        BOOST_CHECK_EQUAL( file1.segments[i]->get_file_size(),
                           file2.segments[i]->get_file_size() );
        BOOST_CHECK_EQUAL( file1.segments[i]->get_memory_size(),
                           file2.segments[i]->get_memory_size() );
        BOOST_CHECK_EQUAL( file1.segments[i]->get_type(),
                           file2.segments[i]->get_type() );

        // skip data comparisons of the program header and of empty segments
        if ( file1.segments[i]->get_type() == PT_PHDR ||
             !file1.segments[i]->get_file_size() )
            continue;

        BOOST_REQUIRE_NE( file1.segments[i]->get_data(), (const char*)0 );
        BOOST_REQUIRE_NE( file2.segments[i]->get_data(), (const char*)0 );

        std::string pdata1( file1.segments[i]->get_data(),
                            file1.segments[i]->get_data() +
                                file1.segments[i]->get_file_size() );
        std::string pdata2( file2.segments[i]->get_data(),
                            file2.segments[i]->get_data() +
                                file2.segments[i]->get_file_size() );

        // truncate the data if the header and the segment table is
        // part of the segment
        Elf64_Off afterPHDR =
            file1.get_segments_offset() +
            file1.get_segment_entry_size() * (Elf64_Off)file1.segments.size();
        if ( file1.segments[i]->get_offset() < afterPHDR ) {
            pdata1 = pdata1.substr( (unsigned int)afterPHDR );
            pdata2 = pdata2.substr( (unsigned int)afterPHDR );
        }

        BOOST_CHECK_EQUAL_COLLECTIONS( pdata1.begin(), pdata1.end(),
                                       pdata2.begin(), pdata2.end() );
    }
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( write_obj_i386_32 )
{
    BOOST_CHECK_EQUAL( true, write_obj_i386( false ) );
    output_test_stream output( "elf_examples/write_obj_i386_32_match.o", true,
                               false );
    std::ifstream input( "elf_examples/write_obj_i386_32.o", std::ios::binary );
    output << input.rdbuf();
    BOOST_CHECK( output.match_pattern() );
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( write_obj_i386_64 )
{
    BOOST_CHECK_EQUAL( true, write_obj_i386( true ) );
    output_test_stream output( "elf_examples/write_obj_i386_64_match.o", true,
                               false );
    std::ifstream input( "elf_examples/write_obj_i386_64.o", std::ios::binary );
    output << input.rdbuf();
    BOOST_CHECK( output.match_pattern() );
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( write_exe_i386_32 )
{
    const std::string generated_file( "elf_examples/write_exe_i386_32" );
    const std::string reference_file( "elf_examples/write_exe_i386_32_match" );
    BOOST_CHECK_EQUAL( true, write_exe_i386( generated_file, false ) );
    output_test_stream output( reference_file, true, false );
    std::ifstream      input( generated_file, std::ios::binary );
    output << input.rdbuf();
    BOOST_CHECK_MESSAGE( output.match_pattern(), "Comparing " + generated_file +
                                                     " and " + reference_file );
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( elf_object_copy_32 )
{
    checkObjestsAreEqual( "elf_examples/hello_32.o",
                          "elf_examples/hello_32_copy.o" );
    checkObjestsAreEqual( "elf_examples/hello_64.o",
                          "elf_examples/hello_64_copy.o" );
    checkObjestsAreEqual( "elf_examples/test_ppc.o",
                          "elf_examples/test_ppc_copy.o" );
    checkObjestsAreEqual( "elf_examples/write_obj_i386_32.o",
                          "elf_examples/write_obj_i386_32_copy.o" );
    checkObjestsAreEqual( "elf_examples/write_obj_i386_64.o",
                          "elf_examples/write_obj_i386_64_copy.o" );
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( section_header_address_update )
{
    elfio reader;

    const std::string file_w_addr( "elf_examples/write_exe_i386_32_w_addr" );
    write_exe_i386( file_w_addr, false, true, 0x08048100 );
    reader.load( file_w_addr );
    section* sec = reader.sections[".text"];
    BOOST_REQUIRE_NE( sec, (section*)0 );
    BOOST_CHECK_EQUAL( sec->get_address(), 0x08048100 );

    const std::string file_wo_addr( "elf_examples/write_exe_i386_32_wo_addr" );
    write_exe_i386( file_wo_addr, false, false, 0 );
    reader.load( file_wo_addr );
    sec = reader.sections[".text"];
    BOOST_REQUIRE_NE( sec, (section*)0 );
    BOOST_CHECK_EQUAL( sec->get_address(), 0x08048000 );
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( elfio_copy )
{
    elfio e;

    const std::string filename(
        "elf_examples/write_exe_i386_32_section_added" );
    write_exe_i386( filename, false, true, 0x08048100 );

    e.load( filename );
    Elf_Half num = e.sections.size();
    //section* new_sec =
    e.sections.add( "new" );
    e.save( filename );
    BOOST_CHECK_EQUAL( num + 1, e.sections.size() );
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( elf_exe_copy_64 )
{
    checkExeAreEqual( "elf_examples/64bitLOAD.elf",
                      "elf_examples/64bitLOAD_copy.elf" );
    checkExeAreEqual( "elf_examples/asm64", "elf_examples/asm64_copy" );
    checkExeAreEqual( "elf_examples/hello_64", "elf_examples/hello_64_copy" );

    // The last segment (GNU_RELRO) is bigger than necessary.
    // I don't see why but it contains a few bits of the .got.plt section.
    // -> load, store, compare cycle fails
    //    checkExeAreEqual( "elf_examples/main",
    //                      "elf_examples/main_copy" );
    //    checkExeAreEqual( "elf_examples/ls",
    //                      "elf_examples/ls_copy" );
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( elf_exe_copy_32 )
{
    checkExeAreEqual( "elf_examples/asm", "elf_examples/asm_copy" );
    checkExeAreEqual( "elf_examples/arm_v7m_test_debug.elf",
                      "elf_examples/arm_v7m_test_debug_copy.elf" );
    checkExeAreEqual( "elf_examples/arm_v7m_test_release.elf",
                      "elf_examples/arm_v7m_test_release_copy.elf" );
    checkExeAreEqual( "elf_examples/hello_32", "elf_examples/hello_32_copy" );
    checkExeAreEqual( "elf_examples/hello_arm", "elf_examples/hello_arm_copy" );
    checkExeAreEqual( "elf_examples/hello_arm_stripped",
                      "elf_examples/hello_arm_stripped_copy" );
    checkExeAreEqual( "elf_examples/read_write_arm_elf32_input",
                      "elf_examples/read_write_arm_elf32_input_copy" );
    checkExeAreEqual( "elf_examples/test_ppc", "elf_examples/test_ppc_copy" );
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( elf_exe_loadsave_ppc32big3 )
{
    std::string in  = "elf_examples/ppc-32bit-specimen3.elf";
    std::string out = "elf_examples/ppc-32bit-testcopy3.elf";
    elfio       elf;
    BOOST_REQUIRE_EQUAL( elf.load( in ), true );
    BOOST_REQUIRE_EQUAL( elf.save( out ), true );

    checkObjestsAreEqual( in, out );
    checkExeAreEqual( in, out, SEG_ALIGN );
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( get_symbol_32 )
{
    elfio            elf;
    std::string      name;
    ELFIO::Elf_Xword size;
    unsigned char    bind;
    unsigned char    type;
    ELFIO::Elf_Half  section_index;
    unsigned char    other;
    std::string      in = "elf_examples/hello_32";

    BOOST_REQUIRE_EQUAL( elf.load( in ), true );
    section*                      psymsec = elf.sections[".symtab"];
    const symbol_section_accessor symbols( elf, psymsec );

    BOOST_CHECK_EQUAL( true, symbols.get_symbol( 0x08048478, name, size, bind,
                                                 type, section_index, other ) );
    BOOST_CHECK_EQUAL( "_IO_stdin_used", name );
    BOOST_CHECK_EQUAL( 14, section_index );
    BOOST_CHECK_EQUAL( 4, size );
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( get_symbol_64 )
{
    elfio            elf;
    std::string      name;
    ELFIO::Elf_Xword size;
    unsigned char    bind;
    unsigned char    type;
    ELFIO::Elf_Half  section_index;
    unsigned char    other;
    std::string      in = "elf_examples/hello_64";

    BOOST_REQUIRE_EQUAL( elf.load( in ), true );
    section*                      psymsec = elf.sections[".symtab"];
    const symbol_section_accessor symbols( elf, psymsec );

    BOOST_CHECK_EQUAL( true, symbols.get_symbol( 0x00400498, name, size, bind,
                                                 type, section_index, other ) );
    BOOST_CHECK_EQUAL( "main", name );
    BOOST_CHECK_EQUAL( 12, section_index );
    BOOST_CHECK_EQUAL( 21, size );
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( null_section_inside_segment )
{
    // This test case checks the load/save of SHT_NULL sections in a segment
    // See https://github.com/serge1/ELFIO/issues/19
    //
    // Note: The test case checking the load/save of a segment containing no section
    // is covered by elf_object_copy_32: elf_examples/hello_32 has empty segments

    // Create an ELF file with SHT_NULL sections at the beginning/middle/end of a segment
    elfio writer;
    writer.create( ELFCLASS32, ELFDATA2LSB );
    writer.set_os_abi( ELFOSABI_LINUX );
    writer.set_type( ET_EXEC );
    writer.set_machine( EM_386 );
    // Create code section 1
    section* text_sec1 = writer.sections.add( ".text1" );
    text_sec1->set_type( SHT_PROGBITS );
    text_sec1->set_flags( SHF_ALLOC | SHF_EXECINSTR );
    text_sec1->set_addr_align( 0x10 );
    text_sec1->set_address( 0x08048000 );
    char text[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    text_sec1->set_data( text, sizeof( text ) );
    // Create code section 2
    section* text_sec2 = writer.sections.add( ".text2" );
    text_sec2->set_type( SHT_PROGBITS );
    text_sec2->set_flags( SHF_ALLOC | SHF_EXECINSTR );
    text_sec2->set_addr_align( 0x10 );
    text_sec2->set_address( 0x08048010 );
    text_sec2->set_data( text, sizeof( text ) );
    // Create null sections
    section* null_sec1 = writer.sections.add( "null" );
    null_sec1->set_type( SHT_NULL );
    null_sec1->set_flags( SHF_ALLOC | SHF_EXECINSTR );
    null_sec1->set_address( 0x08048000 );
    section* null_sec2 = writer.sections.add( "null" );
    null_sec2->set_type( SHT_NULL );
    null_sec2->set_flags( SHF_ALLOC | SHF_EXECINSTR );
    null_sec2->set_address( 0x08048010 );
    section* null_sec3 = writer.sections.add( "null" );
    null_sec3->set_type( SHT_NULL );
    null_sec3->set_flags( SHF_ALLOC | SHF_EXECINSTR );
    null_sec3->set_address( 0x08048020 );
    // Create a loadable segment
    segment* text_seg = writer.segments.add();
    text_seg->set_type( PT_LOAD );
    text_seg->set_virtual_address( 0x08048000 );
    text_seg->set_physical_address( 0x08048000 );
    text_seg->set_flags( PF_X | PF_R );
    text_seg->set_align( 0x1000 );
    // Add sections into the loadable segment
    text_seg->add_section_index( null_sec1->get_index(),
                                 null_sec1->get_addr_align() );
    text_seg->add_section_index( text_sec1->get_index(),
                                 text_sec1->get_addr_align() );
    text_seg->add_section_index( null_sec2->get_index(),
                                 null_sec2->get_addr_align() );
    text_seg->add_section_index( text_sec2->get_index(),
                                 text_sec2->get_addr_align() );
    text_seg->add_section_index( null_sec3->get_index(),
                                 null_sec3->get_addr_align() );
    // Setup entry point
    writer.set_entry( 0x08048000 );
    // Create ELF file
    std::string f1 = "elf_examples/null_section_inside_segment1";
    std::string f2 = "elf_examples/null_section_inside_segment2";
    BOOST_CHECK_EQUAL( writer.save( f1 ), true );

    // Load and check the ELF file created above
    elfio elf;
    BOOST_CHECK_EQUAL( elf.load( f1 ), true );
    BOOST_CHECK_EQUAL( elf.save( f2 ), true );
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( invalid_file )
{
    elfio             elf;
    std::string       name;
    ELFIO::Elf64_Addr value;
    ELFIO::Elf_Xword  size;
    unsigned char     bind;
    unsigned char     type;
    ELFIO::Elf_Half   section_index;
    unsigned char     other;
    std::string       in = "elf_examples/crash.elf";

    BOOST_REQUIRE_EQUAL( elf.load( in ), true );
    section* psymsec = elf.sections[".symtab"];
    BOOST_REQUIRE_NE( psymsec, (void*)0 );
    const symbol_section_accessor symbols( elf, psymsec );

    BOOST_CHECK_EQUAL( true, symbols.get_symbol( "main", value, size, bind,
                                                 type, section_index, other ) );
    BOOST_CHECK_EQUAL( 0x402560, value );

    BOOST_CHECK_EQUAL( true,
                       symbols.get_symbol( "frame_dummy", value, size, bind,
                                           type, section_index, other ) );
    BOOST_CHECK_EQUAL( 0x402550, value );

    BOOST_CHECK_EQUAL( false,
                       symbols.get_symbol( 0x00400498, name, size, bind, type,
                                           section_index, other ) );
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( rearrange_local_symbols )
{
    std::string       name          = "";
    ELFIO::Elf64_Addr value         = 0;
    ELFIO::Elf_Xword  size          = 0;
    unsigned char     bind          = STB_LOCAL;
    unsigned char     type          = STT_FUNC;
    ELFIO::Elf_Half   section_index = 0;
    unsigned char     other         = 0;
    const std::string file_name     = "elf_examples/test_symbols_order.elf";

    elfio writer;
    writer.create( ELFCLASS64, ELFDATA2LSB );
    writer.set_os_abi( ELFOSABI_LINUX );
    writer.set_type( ET_EXEC );
    writer.set_machine( EM_X86_64 );

    section* str_sec = writer.sections.add( ".strtab" );
    str_sec->set_type( SHT_STRTAB );
    str_sec->set_addr_align( 0x1 );
    string_section_accessor str_writer( str_sec );

    section* sym_sec = writer.sections.add( ".symtab" );
    sym_sec->set_type( SHT_SYMTAB );
    sym_sec->set_info( 0 );
    sym_sec->set_link( str_sec->get_index() );
    sym_sec->set_addr_align( 4 );
    sym_sec->set_entry_size( writer.get_default_entry_size( SHT_SYMTAB ) );
    symbol_section_accessor symbols( writer, sym_sec );

    name  = "Str1";
    bind  = STB_GLOBAL;
    value = 1;
    symbols.add_symbol( str_writer, name.c_str(), value, size, bind, type,
                        other, section_index );
    name  = "Str2";
    bind  = STB_LOCAL;
    value = 2;
    symbols.add_symbol( str_writer, name.c_str(), value, size, bind, type,
                        other, section_index );
    name  = "Str3";
    bind  = STB_WEAK;
    value = 3;
    symbols.add_symbol( str_writer, name.c_str(), value, size, bind, type,
                        other, section_index );
    name  = "Str4";
    bind  = STB_LOCAL;
    value = 4;
    symbols.add_symbol( str_writer, name.c_str(), value, size, bind, type,
                        other, section_index );
    name  = "Str5";
    bind  = STB_LOCAL;
    value = 5;
    symbols.add_symbol( str_writer, name.c_str(), value, size, bind, type,
                        other, section_index );
    name  = "Str6";
    bind  = STB_GLOBAL;
    value = 6;
    symbols.add_symbol( str_writer, name.c_str(), value, size, bind, type,
                        other, section_index );
    name  = "Str7";
    bind  = STB_LOCAL;
    value = 7;
    symbols.add_symbol( str_writer, name.c_str(), value, size, bind, type,
                        other, section_index );
    name  = "Str8";
    bind  = STB_WEAK;
    value = 8;
    symbols.add_symbol( str_writer, name.c_str(), value, size, bind, type,
                        other, section_index );

    symbols.arrange_local_symbols( [&]( Elf_Xword first, Elf_Xword ) -> void {
        static int counter = 0;
        BOOST_CHECK_EQUAL( first, ++counter );
    } );

    BOOST_REQUIRE_EQUAL( writer.save( file_name ), true );

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    elfio reader;
    BOOST_REQUIRE_EQUAL( reader.load( file_name ), true );

    auto psymsec = reader.sections[".symtab"];
    BOOST_REQUIRE_NE( psymsec, nullptr );

    const_symbol_section_accessor rsymbols( reader, psymsec );

    auto bound = psymsec->get_info();
    auto num   = rsymbols.get_symbols_num();

    BOOST_CHECK_LE( (Elf_Xword)bound, num );

    // Check that all symbols are LOCAL until the bound value
    for ( Elf_Word i = 0; i < bound; i++ ) {
        rsymbols.get_symbol( i, name, value, size, bind, type, section_index,
                             other );
        BOOST_CHECK_EQUAL( bind, (unsigned char)STB_LOCAL );
    }
    BOOST_CHECK_EQUAL( name, "Str7" );

    // Check that all symbols are not LOCAL after the bound value
    for ( Elf_Word i = bound; i < num; i++ ) {
        rsymbols.get_symbol( i, name, value, size, bind, type, section_index,
                             other );

        BOOST_CHECK_NE( bind, (unsigned char)STB_LOCAL );
    }
    BOOST_CHECK_EQUAL( name, "Str8" );
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( rearrange_local_symbols_with_reallocation )
{
    std::string       name          = "";
    ELFIO::Elf64_Addr value         = 0;
    ELFIO::Elf_Xword  size          = 0;
    unsigned char     bind          = STB_LOCAL;
    unsigned char     type          = STT_FUNC;
    ELFIO::Elf_Half   section_index = 0;
    unsigned char     other         = 0;
    const std::string file_name     = "elf_examples/test_symbols_order.elf";

    elfio writer;
    writer.create( ELFCLASS64, ELFDATA2LSB );
    writer.set_os_abi( ELFOSABI_LINUX );
    writer.set_type( ET_EXEC );
    writer.set_machine( EM_X86_64 );

    section* text_sec = writer.sections.add( ".text" );
    text_sec->set_type( SHT_PROGBITS );
    text_sec->set_flags( SHF_ALLOC | SHF_EXECINSTR );
    text_sec->set_addr_align( 0x10 );

    section* str_sec = writer.sections.add( ".strtab" );
    str_sec->set_type( SHT_STRTAB );
    str_sec->set_addr_align( 0x1 );
    string_section_accessor str_writer( str_sec );

    section* sym_sec = writer.sections.add( ".symtab" );
    sym_sec->set_type( SHT_SYMTAB );
    sym_sec->set_info( 0 );
    sym_sec->set_link( str_sec->get_index() );
    sym_sec->set_addr_align( 4 );
    sym_sec->set_entry_size( writer.get_default_entry_size( SHT_SYMTAB ) );
    symbol_section_accessor symbols( writer, sym_sec );

    name          = "Str1";
    bind          = STB_GLOBAL;
    value         = 1;
    Elf_Word sym1 = symbols.add_symbol( str_writer, name.c_str(), value, size,
                                        bind, type, other, section_index );
    name          = "Str2";
    bind          = STB_LOCAL;
    value         = 2;
    Elf_Word sym2 = symbols.add_symbol( str_writer, name.c_str(), value, size,
                                        bind, type, other, section_index );
    name          = "Str3";
    bind          = STB_WEAK;
    value         = 3;
    Elf_Word sym3 = symbols.add_symbol( str_writer, name.c_str(), value, size,
                                        bind, type, other, section_index );
    name          = "Str4";
    bind          = STB_LOCAL;
    value         = 4;
    Elf_Word sym4 = symbols.add_symbol( str_writer, name.c_str(), value, size,
                                        bind, type, other, section_index );
    name          = "Str5";
    bind          = STB_LOCAL;
    value         = 5;
    Elf_Word sym5 = symbols.add_symbol( str_writer, name.c_str(), value, size,
                                        bind, type, other, section_index );
    name          = "Str6";
    bind          = STB_GLOBAL;
    value         = 6;
    Elf_Word sym6 = symbols.add_symbol( str_writer, name.c_str(), value, size,
                                        bind, type, other, section_index );
    name          = "Str7";
    bind          = STB_LOCAL;
    value         = 7;
    Elf_Word sym7 = symbols.add_symbol( str_writer, name.c_str(), value, size,
                                        bind, type, other, section_index );
    name          = "Str8";
    bind          = STB_WEAK;
    value         = 8;
    Elf_Word sym8 = symbols.add_symbol( str_writer, name.c_str(), value, size,
                                        bind, type, other, section_index );

    section* rel_sec = writer.sections.add( ".rel.text" );
    rel_sec->set_type( SHT_REL );
    rel_sec->set_info( text_sec->get_index() );
    rel_sec->set_addr_align( 0x4 );
    rel_sec->set_entry_size( writer.get_default_entry_size( SHT_REL ) );
    rel_sec->set_link( sym_sec->get_index() );

    relocation_section_accessor rela( writer, rel_sec );
    // Add relocation entry (adjust address at offset 11)
    rela.add_entry( 1, sym1, (unsigned char)R_386_RELATIVE );
    rela.add_entry( 8, sym8, (unsigned char)R_386_RELATIVE );
    rela.add_entry( 6, sym6, (unsigned char)R_386_RELATIVE );
    rela.add_entry( 2, sym2, (unsigned char)R_386_RELATIVE );
    rela.add_entry( 3, sym3, (unsigned char)R_386_RELATIVE );
    rela.add_entry( 8, sym8, (unsigned char)R_386_RELATIVE );
    rela.add_entry( 7, sym7, (unsigned char)R_386_RELATIVE );
    rela.add_entry( 2, sym2, (unsigned char)R_386_RELATIVE );
    rela.add_entry( 11, sym1, (unsigned char)R_386_RELATIVE );
    rela.add_entry( 18, sym8, (unsigned char)R_386_RELATIVE );
    rela.add_entry( 16, sym6, (unsigned char)R_386_RELATIVE );
    rela.add_entry( 12, sym2, (unsigned char)R_386_RELATIVE );
    rela.add_entry( 13, sym3, (unsigned char)R_386_RELATIVE );
    rela.add_entry( 17, sym7, (unsigned char)R_386_RELATIVE );
    rela.add_entry( 14, sym4, (unsigned char)R_386_RELATIVE );
    rela.add_entry( 15, sym5, (unsigned char)R_386_RELATIVE );

    std::vector<std::string> before;

    for ( Elf_Word i = 0; i < rela.get_entries_num(); i++ ) {
        Elf64_Addr    offset;
        Elf_Word      symbol;
        unsigned char rtype;
        Elf_Sxword    addend;

        rela.get_entry( i, offset, symbol, rtype, addend );
        symbols.get_symbol( symbol, name, value, size, bind, type,
                            section_index, other );
        before.emplace_back( name );
    }

    symbols.arrange_local_symbols( [&]( Elf_Xword first, Elf_Xword second ) {
        rela.swap_symbols( first, second );
    } );

    BOOST_REQUIRE_EQUAL( writer.save( file_name ), true );

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    elfio reader;
    BOOST_REQUIRE_EQUAL( reader.load( file_name ), true );

    auto prelsec = reader.sections[".rel.text"];
    auto psyms   = reader.sections[".symtab"];

    BOOST_REQUIRE_NE( prelsec, nullptr );
    BOOST_REQUIRE_NE( psyms, nullptr );

    const_relocation_section_accessor rel( reader, prelsec );
    const_symbol_section_accessor     syms( reader, psyms );

    std::vector<std::string> after;

    for ( Elf_Word i = 0; i < rel.get_entries_num(); i++ ) {
        Elf64_Addr    offset;
        Elf_Word      symbol;
        unsigned char rtype;
        Elf_Sxword    addend;

        rel.get_entry( i, offset, symbol, rtype, addend );
        syms.get_symbol( symbol, name, value, size, bind, type, section_index,
                         other );
        after.emplace_back( name );
    }

    BOOST_CHECK_EQUAL_COLLECTIONS( before.begin(), before.end(), after.begin(),
                                   after.end() );
}

BOOST_AUTO_TEST_CASE( detect_mismatched_section_segment_tables )
{
    /*  This file is a hacked copy of hello_32
     *  The error introduced is:
     *     Virtual address of segment 3 (0x804948c) conflicts
     *     with address of section .ctors (0x804948e) at offset 0x48e
     */
    std::string in = "elf_examples/mismatched_segments.elf";
    elfio       elf;
    BOOST_REQUIRE_EQUAL( elf.load( in ), true );
    BOOST_REQUIRE( elf.validate().length() > 0 );
}
