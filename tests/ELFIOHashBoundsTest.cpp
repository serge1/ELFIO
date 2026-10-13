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

#include <cstdint>
#include <initializer_list>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <elfio/elfio.hpp>

using namespace ELFIO;

namespace {

template <class T>
void append_value( std::string& data, const elfio& file, T value )
{
    T converted = ( *file.get_convertor() )( value );
    data.append( reinterpret_cast<const char*>( &converted ),
                 sizeof( converted ) );
}

class symbol_lookup_fixture
{
  public:
    symbol_lookup_fixture( unsigned char file_class,
                           unsigned char encoding,
                           Elf_Word      hash_type,
                           bool          local_first = false )
    {
        file.create( file_class, encoding );

        strings = file.sections.add( ".strtab.test" );
        strings->set_type( SHT_STRTAB );
        const char string_data[] = { '\0', 'f', 'o', 'o', '\0' };
        strings->set_data( string_data, sizeof( string_data ) );

        symbols = file.sections.add( ".symtab.test" );
        symbols->set_type( SHT_SYMTAB );
        symbols->set_entry_size( file.get_default_entry_size( SHT_SYMTAB ) );
        symbols->set_link( strings->get_index() );
        symbol_section_accessor symbol_writer( file, symbols );
        symbol_writer.add_symbol( 1, 0, 0, local_first ? STB_LOCAL : STB_GLOBAL,
                                  STT_NOTYPE, 0, SHN_UNDEF );

        hash = file.sections.add( ".hash.test" );
        hash->set_type( hash_type );
        hash->set_link( symbols->get_index() );
    }

    void set_words( std::initializer_list<std::uint32_t> words )
    {
        std::string data;
        for ( std::uint32_t word : words ) {
            append_value( data, file, word );
        }
        hash->set_data( data );
    }

    void set_valid_gnu_hash( const std::string& name, bool terminated )
    {
        set_gnu_chain( { name.c_str() }, 1, terminated );
    }

    Elf_Word add_symbol( const char* name, Elf64_Addr value )
    {
        string_section_accessor names( strings );
        symbol_section_accessor writer( file, symbols );
        return writer.add_symbol( names, name, value, 0, STB_GLOBAL, STT_NOTYPE,
                                  0, SHN_UNDEF );
    }

    void set_gnu_chain( std::initializer_list<const char*> names,
                        std::uint32_t                      symbol_offset,
                        bool                               terminated = true )
    {
        constexpr std::uint32_t bloom_shift = 5;
        const unsigned word_bits = file.get_class() == ELFCLASS32 ? 32 : 64;
        std::uint64_t  bloom     = 0;
        std::vector<std::uint32_t> hashes;
        for ( const char* name : names ) {
            const std::uint32_t hash =
                elf_gnu_hash( reinterpret_cast<const unsigned char*>( name ) );
            hashes.push_back( hash );
            bloom |= ( std::uint64_t{ 1 } << ( hash % word_bits ) ) |
                     ( std::uint64_t{ 1 }
                       << ( ( hash >> bloom_shift ) % word_bits ) );
        }
        std::string data;
        append_value<std::uint32_t>( data, file, 1 );
        append_value<std::uint32_t>( data, file, symbol_offset );
        append_value<std::uint32_t>( data, file, 1 );
        append_value<std::uint32_t>( data, file, bloom_shift );
        if ( file.get_class() == ELFCLASS32 ) {
            append_value( data, file, static_cast<std::uint32_t>( bloom ) );
        }
        else {
            append_value( data, file, bloom );
        }
        append_value( data, file, symbol_offset );
        for ( size_t i = 0; i < hashes.size(); ++i ) {
            const std::uint32_t chain_hash =
                terminated && i + 1 == hashes.size() ? hashes[i] | 1U
                                                     : hashes[i] & ~1U;
            append_value( data, file, chain_hash );
        }
        hash->set_data( data );
    }

    bool find( const std::string& name,
               Elf64_Addr*        found_value = nullptr ) const
    {
        const_symbol_section_accessor reader( file, symbols );
        Elf64_Addr                    value         = 0;
        Elf_Xword                     size          = 0;
        unsigned char                 bind          = 0;
        unsigned char                 type          = 0;
        Elf_Half                      section_index = 0;
        unsigned char                 other         = 0;
        const bool found = reader.get_symbol( name, value, size, bind, type,
                                              section_index, other );
        if ( found_value != nullptr ) {
            *found_value = value;
        }
        return found;
    }

  private:
    elfio    file;
    section* symbols = nullptr;
    section* strings = nullptr;
    section* hash    = nullptr;
};

} // namespace

TEST( ELFIOHashBoundsTest, RejectsMalformedSysvHashTables )
{
    symbol_lookup_fixture fixture( ELFCLASS64, ELFDATA2LSB, SHT_HASH );

    fixture.set_words( { 0, 0 } );
    EXPECT_FALSE( fixture.find( "missing" ) );
    EXPECT_TRUE( fixture.find( "foo" ) );

    fixture.set_words( { 1, 1 } );
    EXPECT_FALSE( fixture.find( "missing" ) );

    fixture.set_words( { 1 } );
    EXPECT_FALSE( fixture.find( "missing" ) );

    fixture.set_words( { UINT32_MAX, UINT32_MAX } );
    EXPECT_FALSE( fixture.find( "missing" ) );

    fixture.set_words( { 1, 2, 1, 0, 1 } );
    EXPECT_FALSE( fixture.find( "missing" ) );
}

TEST( ELFIOHashBoundsTest, PreservesValidSysvHashLookups )
{
    for ( unsigned char file_class : { ELFCLASS32, ELFCLASS64 } ) {
        for ( unsigned char encoding : { ELFDATA2LSB, ELFDATA2MSB } ) {
            symbol_lookup_fixture fixture( file_class, encoding, SHT_HASH );
            fixture.set_words( { 1, 2, 1, 0, 0 } );
            EXPECT_TRUE( fixture.find( "foo" ) );
            EXPECT_FALSE( fixture.find( "missing" ) );
        }
    }
}

TEST( ELFIOHashBoundsTest, RejectsMalformedGnuHashTables )
{
    for ( unsigned char file_class : { ELFCLASS32, ELFCLASS64 } ) {
        symbol_lookup_fixture fixture( file_class, ELFDATA2LSB, SHT_GNU_HASH );

        fixture.set_words( { 0, 0, 0, 0 } );
        EXPECT_FALSE( fixture.find( "missing" ) );
        EXPECT_TRUE( fixture.find( "foo" ) );

        fixture.set_words( { 1, 1, 1, 0 } );
        EXPECT_FALSE( fixture.find( "missing" ) );

        fixture.set_words( { 1, 1, 1 } );
        EXPECT_FALSE( fixture.find( "missing" ) );

        fixture.set_words( { UINT32_MAX, 1, UINT32_MAX, 0 } );
        EXPECT_FALSE( fixture.find( "missing" ) );

        fixture.set_words( { 1, 1, 1, 32 } );
        EXPECT_FALSE( fixture.find( "missing" ) );

        fixture.set_valid_gnu_hash( "missing", false );
        EXPECT_FALSE( fixture.find( "missing" ) );
    }
}

TEST( ELFIOHashBoundsTest, PreservesValidGnuHashLookups )
{
    for ( unsigned char file_class : { ELFCLASS32, ELFCLASS64 } ) {
        for ( unsigned char encoding : { ELFDATA2LSB, ELFDATA2MSB } ) {
            symbol_lookup_fixture fixture( file_class, encoding, SHT_GNU_HASH );
            fixture.set_valid_gnu_hash( "foo", true );
            EXPECT_TRUE( fixture.find( "foo" ) );
            EXPECT_FALSE( fixture.find( "missing" ) );
        }
    }
}

TEST( ELFIOHashBoundsTest, TraversesHashChainsBeforeLinearFallback )
{
    for ( unsigned char file_class : { ELFCLASS32, ELFCLASS64 } ) {
        for ( unsigned char encoding : { ELFDATA2LSB, ELFDATA2MSB } ) {
            for ( Elf_Word hash_type : { SHT_HASH, SHT_GNU_HASH } ) {
                SCOPED_TRACE( ::testing::Message()
                              << "class=" << int( file_class ) << " encoding="
                              << int( encoding ) << " hash=" << hash_type );
                symbol_lookup_fixture fixture( file_class, encoding, hash_type,
                                               true );
                ASSERT_EQ( fixture.add_symbol( "bar", 17 ), 2U );
                ASSERT_EQ( fixture.add_symbol( "foo", 42 ), 3U );
                if ( hash_type == SHT_HASH ) {
                    fixture.set_words( { 1, 4, 2, 0, 0, 3, 0 } );
                }
                else {
                    fixture.set_gnu_chain( { "bar", "foo" }, 2 );
                }

                // The chain is bar -> global foo. A broken hash lookup would
                // fall back to the earlier local foo, whose value is zero.
                Elf64_Addr value = 0;
                ASSERT_TRUE( fixture.find( "foo", &value ) );
                EXPECT_EQ( value, 42U );
                ASSERT_TRUE( fixture.find( "bar", &value ) );
                EXPECT_EQ( value, 17U );
                EXPECT_FALSE( fixture.find( "missing" ) );
            }
        }
    }
}
