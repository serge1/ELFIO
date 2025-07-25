/*
Copyright (C) 2025-present by Serge Lamikhov-Center

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

#include <gtest/gtest.h>
#include <ario/ario.hpp>
#include <elfio/elfio.hpp>

using namespace ELFIO;
using namespace ARIO;

////////////////////////////////////////////////////////////////////////////////
TEST( ARIOTest, wrong_file_name )
{
    ario archive;
    ASSERT_EQ( archive.load( "ario/does_not_exist.a" ).ok(), false );
}

////////////////////////////////////////////////////////////////////////////////
TEST( ARIOTest, wrong_file_magic )
{
    ario archive;
    auto result = archive.load( "ario/invalid_magic.a" );
    ASSERT_EQ( result.ok(), false );
    ASSERT_EQ( result.what(),
               "Invalid archive format. Expected magic: !<arch>\n" );
}

////////////////////////////////////////////////////////////////////////////////
TEST( ARIOTest, simple_text_load )
{
    ario archive;
    ASSERT_EQ( archive.load( "ario/simple_text.a" ).ok(), true );
    ASSERT_EQ( archive.members.size(), 6 );
    EXPECT_EQ( archive.members[0].name, "hello.c" );
    EXPECT_EQ( archive.members[0].size, 45 );
    EXPECT_EQ( archive.members[1].name, "hello2.c" );
    EXPECT_EQ( archive.members[1].size, 7 );
    EXPECT_EQ( archive.members[2].name, "hello3.c" );
    EXPECT_EQ( archive.members[2].size, 8 );
    EXPECT_EQ( archive.members[3].name, "hello4.c" );
    EXPECT_EQ( archive.members[3].size, 10 );
    EXPECT_EQ( archive.members[4].name, "hello41.c" );
    EXPECT_EQ( archive.members[4].size, 11 );
    EXPECT_EQ( archive.members[5].name, "hello5.c" );
    EXPECT_EQ( archive.members[5].size, 8 );
}

////////////////////////////////////////////////////////////////////////////////
TEST( ARIOTest, long_name_load )
{
    ario archive;
    auto result = archive.load( "ario/long_name.a" );
    ASSERT_EQ( result.ok(), true );
    ASSERT_EQ( result.what(), "No errors" );

    ASSERT_EQ( archive.members.size(), 9 );
    EXPECT_EQ( archive.members[0].name, "hello.c" );
    EXPECT_EQ( archive.members[0].size, 45 );
    EXPECT_EQ( archive.members[1].name, "hello2.c" );
    EXPECT_EQ( archive.members[1].size, 7 );
    EXPECT_EQ( archive.members[6].name, "a_file_with_very_long_name.txt" );
    EXPECT_EQ( archive.members[6].size, 6 );
    EXPECT_EQ( archive.members[7].name, "a_file_with_very_long_name2.txt" );
    EXPECT_EQ( archive.members[7].size, 6 );
    EXPECT_EQ( archive.members[8].name, "a_file_with_very_long_name3.txt" );
    EXPECT_EQ( archive.members[8].size, 6 );

    EXPECT_EQ( archive.members["a_file_with_very_long_name.txt"].name,
               archive.members[6].name );
    EXPECT_EQ( archive.members["a_file_with_very_long_name3.txt"].name,
               archive.members[8].name );
    EXPECT_EQ( archive.members["hello2.c"].name, archive.members[1].name );
}

////////////////////////////////////////////////////////////////////////////////
TEST( ARIOTest, load_libgcov )
{
    ario archive;
    ASSERT_EQ( archive.load( "ario/libgcov.a" ).ok(), true );

    ASSERT_EQ( archive.members.size(), 29 );
    EXPECT_EQ( archive.members[0].name, "_gcov_merge_add.o" );
    EXPECT_EQ( archive.members[0].size, 1416 );
    EXPECT_EQ( archive.members[0].mode, 0644 );
    EXPECT_EQ( archive.members[0].data().substr( 0, 4 ), "\x7F"
                                                         "ELF" );
    EXPECT_EQ( archive.members[6].name, "_gcov_interval_profiler_atomic.o" );
    EXPECT_EQ( archive.members[6].size, 1264 );
    EXPECT_EQ( archive.members[6].mode, 0644 );
    EXPECT_EQ( archive.members[6].data().substr( 0, 4 ), "\x7F"
                                                         "ELF" );
    EXPECT_EQ( archive.members[17].name,
               "_gcov_indirect_call_topn_profiler.o" );
    EXPECT_EQ( archive.members[17].size, 2104 );
    EXPECT_EQ( archive.members[17].mode, 0644 );
    EXPECT_EQ( archive.members[17].data().substr( 0, 4 ), "\x7F"
                                                          "ELF" );
    EXPECT_EQ( archive.members[28].name, "_gcov.o" );
    EXPECT_EQ( archive.members[28].size, 16768 );
    EXPECT_EQ( archive.members[28].mode, 0644 );
    EXPECT_EQ( archive.members[28].data().substr( 0, 4 ), "\x7F"
                                                          "ELF" );
}

////////////////////////////////////////////////////////////////////////////////
TEST( ARIOTest, find_symbol_libgcov )
{
    ario archive;
    ASSERT_EQ( archive.load( "ario/libgcov.a" ).ok(), true );

    ario::Member member;
    auto         result = archive.find_symbol( "__gcov_merge_add", member );
    ASSERT_EQ( result.ok(), true );
    ASSERT_EQ( member.name, "_gcov_merge_add.o" );

    result =
        archive.find_symbol( "__gcov_indirect_call_topn_profiler", member );
    ASSERT_EQ( result.ok(), true );
    ASSERT_EQ( member.name, "_gcov_indirect_call_topn_profiler.o" );

    result = archive.find_symbol( "__not_found", member );
    ASSERT_EQ( result.ok(), false );
    ASSERT_EQ( member.name, "_gcov_indirect_call_topn_profiler.o" );

    result = archive.find_symbol( "__gcov_write_counter", member );
    ASSERT_EQ( result.ok(), true );
    ASSERT_EQ( member.name, "_gcov.o" );
}

////////////////////////////////////////////////////////////////////////////////
TEST( ARIOTest, get_symbols_for_member_libgcov )
{
    ario archive;
    ASSERT_EQ( archive.load( "ario/libgcov.a" ).ok(), true );

    std::vector<std::string> symbols;

    auto result = archive.get_symbols_for_member( archive.members[0], symbols );
    ASSERT_EQ( result.ok(), true );
    ASSERT_EQ( symbols.size(), 1 );
    ASSERT_EQ( symbols[0], "__gcov_merge_add" );

    result = archive.get_symbols_for_member( archive.members[6], symbols );
    ASSERT_EQ( result.ok(), true );
    ASSERT_EQ( symbols.size(), 1 );
    ASSERT_EQ( symbols[0], "__gcov_interval_profiler_atomic" );

    result = archive.get_symbols_for_member( archive.members[28], symbols );
    ASSERT_EQ( result.ok(), true );
    ASSERT_EQ( symbols.size(), 20 );
    ASSERT_EQ( symbols[0], "__gcov_exit" );
    ASSERT_EQ( symbols[10], "__gcov_var" );
    ASSERT_EQ( symbols[19], "__gcov_read_counter" );
}

////////////////////////////////////////////////////////////////////////////////
TEST( ARIOTest, get_symbols_for_ELF_files_in_archive )
{
    // Load the archive file containing ELF object files
    ario archive;
    ASSERT_EQ( archive.load( "ario/libgcov.a" ).ok(), true );

    // Iterate over each member (object file) in the archive
    for ( const auto& member : archive.members ) {
        // Extract the raw data of the member (should be an ELF file)
        std::string        content = member.data();
        std::istringstream iss( content );

        // Parse the member's data as an ELF file using ELFIO
        elfio elf_reader;
        ASSERT_EQ( elf_reader.load( iss ), true );

        uint32_t counter = 0;

        // Iterate over all sections in the ELF file
        for ( const auto& sec : elf_reader.sections ) {
            // Look for the symbol table section
            if ( sec->get_type() == SHT_SYMTAB ) {
                // Access the symbols in the symbol table
                symbol_section_accessor symbols( elf_reader, sec.get() );

                std::string   name;
                Elf64_Addr    value;
                Elf_Xword     size;
                unsigned char bind, type;
                Elf_Half      section_index;
                unsigned char other;

                ario::Member found_member;
                // Iterate over all symbols in the symbol table
                for ( Elf_Xword i = 0; i < symbols.get_symbols_num(); ++i ) {
                    // Extract symbol properties
                    ASSERT_EQ( symbols.get_symbol( i, name, value, size, bind,
                                                   type, section_index, other ),
                               true );
                    // For each global function or object symbol, check that the archive symbol table can find it
                    if ( ( type == STT_FUNC || type == STT_OBJECT ||
                           type == STT_TLS || type == STT_COMMON ) &&
                         bind == STB_GLOBAL ) {
                        ++counter;
                        ASSERT_EQ(
                            archive.find_symbol( name, found_member ).ok(),
                            true );
                    }
                }
            }
        }

        // Check that the number of global symbols found matches the expected count
        std::vector<std::string> symbols_from_the_member;
        ASSERT_EQ(
            archive.get_symbols_for_member( member, symbols_from_the_member )
                .ok(),
            true );
        ASSERT_EQ( counter, symbols_from_the_member.size() );
    }
}

////////////////////////////////////////////////////////////////////////////////
TEST( ARIOTest, basic_header_save )
{
    ario               archive;
    std::ostringstream oss;

    auto result = archive.save( oss );
    ASSERT_EQ( result.ok(), true );

    ASSERT_EQ( oss.str(), "!<arch>\n" );
}

////////////////////////////////////////////////////////////////////////////////
TEST( ARIOTest, header_save )
{
    ario archive;
    ASSERT_EQ( archive.load( "ario/simple_text.a" ).ok(), true );

    // Save the archive to a new file
    auto result = archive.save( "ario/simple_text_saved.a" );
    ASSERT_EQ( result.ok(), true );

    // Load the saved archive and check its contents
    ario loaded_archive;
    ASSERT_EQ( loaded_archive.load( "ario/simple_text_saved.a" ).ok(), true );
    ASSERT_EQ( loaded_archive.members.size(), archive.members.size() );
    EXPECT_EQ( loaded_archive.members[0].name, archive.members[0].name );
    EXPECT_EQ( loaded_archive.members[0].size, archive.members[0].size );
    EXPECT_EQ( loaded_archive.members[archive.members.size() - 1].name,
               archive.members[archive.members.size() - 1].name );
    EXPECT_EQ( loaded_archive.members[archive.members.size() - 1].size,
               archive.members[archive.members.size() - 1].size );
}
