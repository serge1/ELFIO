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

#include <limits>
#include <sstream>
#include <string>
#include <tuple>

#include <gtest/gtest.h>
#include <elfio/elfio.hpp>

using namespace ELFIO;

namespace {

void append_word( std::string& data, const elfio& file, Elf_Word value )
{
    value = ( *file.get_convertor() )( value );
    data.append( reinterpret_cast<const char*>( &value ), sizeof( value ) );
}

std::string make_note( const elfio&       file,
                       Elf_Word           type,
                       const std::string& name,
                       const std::string& descriptor = {} )
{
    // Names include their declared terminator; an empty name encodes namesz=0.
    std::string data;
    append_word( data, file, static_cast<Elf_Word>( name.size() ) );
    append_word( data, file, static_cast<Elf_Word>( descriptor.size() ) );
    append_word( data, file, type );
    data.append( name );
    data.append( ( 4 - name.size() % 4 ) % 4, '\0' );
    data.append( descriptor );
    data.append( ( 4 - descriptor.size() % 4 ) % 4, '\0' );
    return data;
}

template <class Accessor>
void expect_rejected( const Accessor& reader, Elf_Word index )
{
    Elf_Word    type            = 11;
    std::string name            = "unchanged";
    char        marker          = 0;
    char*       descriptor      = &marker;
    Elf_Word    descriptor_size = 13;
    EXPECT_FALSE(
        reader.get_note( index, type, name, descriptor, descriptor_size ) );
    EXPECT_EQ( type, 11U );
    EXPECT_EQ( name, "unchanged" );
    EXPECT_EQ( descriptor, &marker );
    EXPECT_EQ( descriptor_size, 13U );
}

template <class Accessor>
void expect_note( const Accessor&    reader,
                  Elf_Word           index,
                  Elf_Word           expected_type,
                  const std::string& expected_name,
                  const std::string& expected_descriptor )
{
    Elf_Word    type = 0;
    std::string name;
    char*       descriptor      = nullptr;
    Elf_Word    descriptor_size = 0;
    ASSERT_TRUE(
        reader.get_note( index, type, name, descriptor, descriptor_size ) );
    EXPECT_EQ( type, expected_type );
    EXPECT_EQ( name, expected_name );
    ASSERT_EQ( descriptor_size, expected_descriptor.size() );
    if ( expected_descriptor.empty() ) {
        EXPECT_EQ( descriptor, nullptr );
    }
    else {
        ASSERT_NE( descriptor, nullptr );
        EXPECT_EQ( std::string( descriptor, descriptor_size ),
                   expected_descriptor );
    }
}

class ELFIONoteBoundsTest
    : public ::testing::TestWithParam<std::tuple<unsigned char, unsigned char>>
{
  protected:
    void SetUp() override
    {
        file.create( std::get<0>( GetParam() ), std::get<1>( GetParam() ) );
        notes = file.sections.add( ".note.test" );
        notes->set_type( SHT_NOTE );
        notes->set_addr_align( 4 );
    }

    elfio    file;
    section* notes = nullptr;
};

TEST_P( ELFIONoteBoundsTest, RejectsOutOfRangeIndex )
{
    note_section_accessor reader( file, notes );
    expect_rejected( reader, 0 );
    reader.add_note( 7, "test", "abc", 3 );
    ASSERT_EQ( reader.get_notes_num(), 1U );
    expect_note( reader, 0, 7, "test", "abc" );
    expect_rejected( reader, 1 );
    expect_rejected( reader, std::numeric_limits<Elf_Word>::max() );
}

TEST_P( ELFIONoteBoundsTest, RejectsTruncatedPayloadAndPadding )
{
    const auto data = make_note( file, 7, std::string( "test\0", 5 ), "abc" );
    for ( size_t length = 0; length < data.size(); ++length ) {
        SCOPED_TRACE( length );
        notes->set_data( data.data(), length );
        note_section_accessor reader( file, notes );
        EXPECT_EQ( reader.get_notes_num(), 0U );
        expect_rejected( reader, 0 );
    }
}

TEST_P( ELFIONoteBoundsTest, RejectsOversizedLengths )
{
    for ( bool oversized_name : { false, true } ) {
        std::string data;
        const auto  maximum = std::numeric_limits<Elf_Word>::max();
        append_word( data, file, oversized_name ? maximum : 4 );
        append_word( data, file, oversized_name ? 0 : maximum );
        append_word( data, file, 7 );
        data.append( "GNU\0", 4 );
        notes->set_data( data );
        note_section_accessor reader( file, notes );
        EXPECT_EQ( reader.get_notes_num(), 0U );
        expect_rejected( reader, 0 );
    }
}

TEST_P( ELFIONoteBoundsTest, AcceptsNamelessNoteAndContinuesParsing )
{
    for ( const std::string& payload :
          { std::string{}, std::string{ "abc" } } ) {
        const auto data = make_note( file, 1, {}, payload ) +
                          make_note( file, 2, std::string( "GNU\0", 4 ), "xy" );
        notes->set_data( data );
        note_section_accessor reader( file, notes );
        ASSERT_EQ( reader.get_notes_num(), 2U );
        expect_note( reader, 0, 1, "", payload );
        expect_note( reader, 1, 2, "GNU", "xy" );
    }
}

TEST_P( ELFIONoteBoundsTest, RevalidatesCachedOffsetsWithoutChangingOutputs )
{
    const auto first = make_note( file, 1, std::string( "GNU\0", 4 ) );
    const auto data  = first + make_note( file, 2, {}, "abc" );
    notes->set_data( data );
    note_section_accessor reader( file, notes );
    ASSERT_EQ( reader.get_notes_num(), 2U );
    for ( size_t length : { size_t( 0 ), size_t( 1 ), first.size() + 5 } ) {
        notes->set_data( data.data(), length );
        expect_rejected( reader, 1 );
    }
    std::string oversized = first;
    append_word( oversized, file, 0 );
    append_word( oversized, file, std::numeric_limits<Elf_Word>::max() );
    append_word( oversized, file, 2 );
    notes->set_data( oversized );
    expect_rejected( reader, 1 );
    notes->set_data( data );
    expect_note( reader, 1, 2, "", "abc" );
}

TEST_P( ELFIONoteBoundsTest, KeepsUnterminatedOwnerName )
{
    // Some producers omit the owner's terminator; the full name is returned.
    auto data = make_note( file, 7, std::string( "GNU\0", 4 ), "abc" );
    notes->set_data( data );
    note_section_accessor cached( file, notes );
    data[3 * sizeof( Elf_Word ) + 3] = 'x';
    notes->set_data( data );
    expect_note( cached, 0, 7, "GNUx", "abc" );
    note_section_accessor fresh( file, notes );
    ASSERT_EQ( fresh.get_notes_num(), 1U );
    expect_note( fresh, 0, 7, "GNUx", "abc" );
}

TEST_P( ELFIONoteBoundsTest, ReadsSectionAndSegmentNotes )
{
    notes->set_data( make_note( file, 7, std::string( "GNU\0", 4 ), "abc" ) );
    segment* note_segment = file.segments.add();
    note_segment->set_type( PT_NOTE );
    note_segment->set_align( 4 );
    note_segment->add_section_index( notes->get_index(), 4 );
    std::stringstream stream( std::ios::in | std::ios::out | std::ios::binary );
    ASSERT_TRUE( file.save( stream ) );
    stream.seekg( 0 );
    elfio loaded;
    ASSERT_TRUE( loaded.load( stream ) );
    const_note_section_accessor section_reader( loaded,
                                                loaded.sections[".note.test"] );
    // A note segment's file size, not its memory size, bounds the payload.
    loaded.segments[0]->set_memory_size( loaded.segments[0]->get_file_size() +
                                         512 );
    const_note_segment_accessor segment_reader( loaded, loaded.segments[0] );
    ASSERT_EQ( section_reader.get_notes_num(), 1U );
    ASSERT_EQ( segment_reader.get_notes_num(), 1U );
    expect_note( section_reader, 0, 7, "GNU", "abc" );
    expect_note( segment_reader, 0, 7, "GNU", "abc" );
    expect_rejected( segment_reader, 1 );
}

TEST_P( ELFIONoteBoundsTest, ReadsUnalignedHeaders )
{
    struct note_view
    {
        std::string storage;
        const char* get_data() const { return storage.data() + 1; }
        Elf_Xword   get_size() const { return storage.size() - 1; }
    } view;
    view.storage = "x" + make_note( file, 7, std::string( "GNU\0", 4 ), "abc" );
    note_section_accessor_template<note_view, &note_view::get_size> reader(
        file, &view );
    ASSERT_EQ( reader.get_notes_num(), 1U );
    expect_note( reader, 0, 7, "GNU", "abc" );
}

INSTANTIATE_TEST_SUITE_P(
    Formats,
    ELFIONoteBoundsTest,
    ::testing::Combine( ::testing::Values( ELFCLASS32, ELFCLASS64 ),
                        ::testing::Values( ELFDATA2LSB, ELFDATA2MSB ) ) );

} // namespace
