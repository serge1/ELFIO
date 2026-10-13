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

#ifndef ELFIO_SYMBOLS_HPP
#define ELFIO_SYMBOLS_HPP

#include <cstring>

namespace ELFIO {

//------------------------------------------------------------------------------
// @class symbol_section_accessor_template
// @brief A template class for accessing symbol sections in an ELF file.
//------------------------------------------------------------------------------
template <class S> class symbol_section_accessor_template
{
  public:
    //------------------------------------------------------------------------------
    // @brief Constructor
    // @param elf_file Reference to the ELF file
    // @param symbol_section Pointer to the symbol section
    //------------------------------------------------------------------------------
    explicit symbol_section_accessor_template( const elfio& elf_file,
                                               S*           symbol_section )
        : elf_file( elf_file ), symbol_section( symbol_section )
    {
        find_hash_section();
    }

    //------------------------------------------------------------------------------
    // @brief Get the number of symbols in the section
    // @return Number of symbols
    //------------------------------------------------------------------------------
    Elf_Xword get_symbols_num() const
    {
        Elf_Xword nRet = 0;

        size_t minimum_symbol_size;
        switch ( elf_file.get_class() ) {
        case ELFCLASS32:
            minimum_symbol_size = sizeof( Elf32_Sym );
            break;
        case ELFCLASS64:
            minimum_symbol_size = sizeof( Elf64_Sym );
            break;
        default:
            return nRet;
        }

        if ( symbol_section->get_entry_size() >= minimum_symbol_size &&
             symbol_section->get_size() <= symbol_section->get_stream_size() ) {
            nRet =
                symbol_section->get_size() / symbol_section->get_entry_size();
        }

        return nRet;
    }

    //------------------------------------------------------------------------------
    // @brief Get the symbol at the specified index
    // @param index Index of the symbol
    // @param name Name of the symbol
    // @param value Value of the symbol
    // @param size Size of the symbol
    // @param bind Binding of the symbol
    // @param type Type of the symbol
    // @param section_index Section index of the symbol
    // @param other Other attributes of the symbol
    // @return True if the symbol is found, false otherwise
    //------------------------------------------------------------------------------
    bool get_symbol( Elf_Xword      index,
                     std::string&   name,
                     Elf64_Addr&    value,
                     Elf_Xword&     size,
                     unsigned char& bind,
                     unsigned char& type,
                     Elf_Half&      section_index,
                     unsigned char& other ) const
    {
        bool ret = false;

        if ( elf_file.get_class() == ELFCLASS32 ) {
            ret = generic_get_symbol<Elf32_Sym>( index, name, value, size, bind,
                                                 type, section_index, other );
        }
        else {
            ret = generic_get_symbol<Elf64_Sym>( index, name, value, size, bind,
                                                 type, section_index, other );
        }

        return ret;
    }

    //------------------------------------------------------------------------------
    // @brief Get the symbol with the specified name
    // @param name Name of the symbol
    // @param value Value of the symbol
    // @param size Size of the symbol
    // @param bind Binding of the symbol
    // @param type Type of the symbol
    // @param section_index Section index of the symbol
    // @param other Other attributes of the symbol
    // @return True if the symbol is found, false otherwise. The output
    //         parameters are only modified when the symbol is found.
    //------------------------------------------------------------------------------
    bool get_symbol( const std::string& name,
                     Elf64_Addr&        value,
                     Elf_Xword&         size,
                     unsigned char&     bind,
                     unsigned char&     type,
                     Elf_Half&          section_index,
                     unsigned char&     other ) const
    {
        Elf64_Addr    found_value         = 0;
        Elf_Xword     found_size          = 0;
        unsigned char found_bind          = 0;
        unsigned char found_type          = 0;
        Elf_Half      found_section_index = 0;
        unsigned char found_other         = 0;

        const Elf_Xword symbol_count = get_symbols_num();
        hash_result     result       = hash_result::invalid;
        // Non-local symbols in [covered_begin, covered_end) are ruled out by
        // a miss
        Elf_Xword covered_begin = 0;
        Elf_Xword covered_end   = 0;
        // Unnamed symbols are never reachable through a hash table
        if ( 0 != get_hash_table_index() && !name.empty() ) {
            if ( hash_section->get_type() == SHT_HASH ) {
                result =
                    hash_lookup( name, found_value, found_size, found_bind,
                                 found_type, found_section_index, found_other );
                covered_end = symbol_count;
            }
            if ( hash_section->get_type() == SHT_GNU_HASH ) {
                if ( elf_file.get_class() == ELFCLASS32 ) {
                    result = gnu_hash_lookup<std::uint32_t>(
                        name, found_value, found_size, found_bind, found_type,
                        found_section_index, found_other, covered_begin,
                        covered_end );
                }
                else {
                    result = gnu_hash_lookup<std::uint64_t>(
                        name, found_value, found_size, found_bind, found_type,
                        found_section_index, found_other, covered_begin,
                        covered_end );
                }
            }
        }

        bool found = result == hash_result::found;
        if ( result == hash_result::invalid ) {
            covered_begin = 0;
            covered_end   = 0;
        }
        for ( Elf_Xword i = 0; !found && i < symbol_count; i++ ) {
            // Linkers may place local symbols in a hashed symbol table without
            // adding them to a chain, so a miss does not rule them out
            if ( i >= covered_begin && i < covered_end &&
                 !is_local_symbol( i ) ) {
                continue;
            }
            std::string symbol_name;
            found =
                get_symbol( i, symbol_name, found_value, found_size, found_bind,
                            found_type, found_section_index, found_other ) &&
                symbol_name == name;
        }
        if ( !found ) {
            return false;
        }

        value         = found_value;
        size          = found_size;
        bind          = found_bind;
        type          = found_type;
        section_index = found_section_index;
        other         = found_other;
        return true;
    }

    //------------------------------------------------------------------------------
    // @brief Get the symbol with the specified value
    // @param value Value of the symbol
    // @param name Name of the symbol
    // @param size Size of the symbol
    // @param bind Binding of the symbol
    // @param type Type of the symbol
    // @param section_index Section index of the symbol
    // @param other Other attributes of the symbol
    // @return True if the symbol is found, false otherwise
    //------------------------------------------------------------------------------
    bool get_symbol( const Elf64_Addr& value,
                     std::string&      name,
                     Elf_Xword&        size,
                     unsigned char&    bind,
                     unsigned char&    type,
                     Elf_Half&         section_index,
                     unsigned char&    other ) const
    {

        const auto& convertor = elf_file.get_convertor();

        Elf_Xword  idx   = 0;
        bool       match = false;
        Elf64_Addr v     = 0;

        if ( elf_file.get_class() == ELFCLASS32 ) {
            match = generic_search_symbols<Elf32_Sym>(
                [&]( const Elf32_Sym* sym ) {
                    return ( *convertor )( sym->st_value ) == value;
                },
                idx );
        }
        else {
            match = generic_search_symbols<Elf64_Sym>(
                [&]( const Elf64_Sym* sym ) {
                    return ( *convertor )( sym->st_value ) == value;
                },
                idx );
        }

        if ( match ) {
            return get_symbol( idx, name, v, size, bind, type, section_index,
                               other );
        }

        return false;
    }

    //------------------------------------------------------------------------------
    // @brief Add a symbol to the section
    // @param name Name of the symbol
    // @param value Value of the symbol
    // @param size Size of the symbol
    // @param info Info of the symbol
    // @param other Other attributes of the symbol
    // @param shndx Section index of the symbol
    // @return Index of the added symbol
    //------------------------------------------------------------------------------
    Elf_Word add_symbol( Elf_Word      name,
                         Elf64_Addr    value,
                         Elf_Xword     size,
                         unsigned char info,
                         unsigned char other,
                         Elf_Half      shndx )
    {
        Elf_Word nRet;

        if ( symbol_section->get_size() == 0 ) {
            if ( elf_file.get_class() == ELFCLASS32 ) {
                nRet = generic_add_symbol<Elf32_Sym>( 0, 0, 0, 0, 0, 0 );
            }
            else {
                nRet = generic_add_symbol<Elf64_Sym>( 0, 0, 0, 0, 0, 0 );
            }
        }

        if ( elf_file.get_class() == ELFCLASS32 ) {
            nRet = generic_add_symbol<Elf32_Sym>( name, value, size, info,
                                                  other, shndx );
        }
        else {
            nRet = generic_add_symbol<Elf64_Sym>( name, value, size, info,
                                                  other, shndx );
        }

        return nRet;
    }

    //------------------------------------------------------------------------------
    // @brief Add a symbol to the section
    // @param name Name of the symbol
    // @param value Value of the symbol
    // @param size Size of the symbol
    // @param bind Binding of the symbol
    // @param type Type of the symbol
    // @param other Other attributes of the symbol
    // @param shndx Section index of the symbol
    // @return Index of the added symbol
    //------------------------------------------------------------------------------
    Elf_Word add_symbol( Elf_Word      name,
                         Elf64_Addr    value,
                         Elf_Xword     size,
                         unsigned char bind,
                         unsigned char type,
                         unsigned char other,
                         Elf_Half      shndx )
    {
        return add_symbol( name, value, size, ELF_ST_INFO( bind, type ), other,
                           shndx );
    }

    //------------------------------------------------------------------------------
    // @brief Add a symbol to the section
    // @param pStrWriter String section accessor
    // @param str Name of the symbol
    // @param value Value of the symbol
    // @param size Size of the symbol
    // @param info Info of the symbol
    // @param other Other attributes of the symbol
    // @param shndx Section index of the symbol
    // @return Index of the added symbol
    //------------------------------------------------------------------------------
    Elf_Word add_symbol( string_section_accessor& pStrWriter,
                         const char*              str,
                         Elf64_Addr               value,
                         Elf_Xword                size,
                         unsigned char            info,
                         unsigned char            other,
                         Elf_Half                 shndx )
    {
        Elf_Word index = pStrWriter.add_string( str );
        return add_symbol( index, value, size, info, other, shndx );
    }

    //------------------------------------------------------------------------------
    // @brief Add a symbol to the section
    // @param pStrWriter String section accessor
    // @param str Name of the symbol
    // @param value Value of the symbol
    // @param size Size of the symbol
    // @param bind Binding of the symbol
    // @param type Type of the symbol
    // @param other Other attributes of the symbol
    // @param shndx Section index of the symbol
    // @return Index of the added symbol
    //------------------------------------------------------------------------------
    Elf_Word add_symbol( string_section_accessor& pStrWriter,
                         const char*              str,
                         Elf64_Addr               value,
                         Elf_Xword                size,
                         unsigned char            bind,
                         unsigned char            type,
                         unsigned char            other,
                         Elf_Half                 shndx )
    {
        return add_symbol( pStrWriter, str, value, size,
                           ELF_ST_INFO( bind, type ), other, shndx );
    }

    //------------------------------------------------------------------------------
    // @brief Arrange local symbols in the section
    // @param func Function to be called for each pair of symbols
    // @return Number of local symbols
    //------------------------------------------------------------------------------
    Elf_Xword arrange_local_symbols(
        std::function<void( Elf_Xword first, Elf_Xword second )> func =
            nullptr )
    {
        Elf_Xword nRet = 0;

        if ( elf_file.get_class() == ELFCLASS32 ) {
            nRet = generic_arrange_local_symbols<Elf32_Sym>( func );
        }
        else {
            nRet = generic_arrange_local_symbols<Elf64_Sym>( func );
        }

        return nRet;
    }

    //------------------------------------------------------------------------------
  private:
    //------------------------------------------------------------------------------
    // @brief Find the hash section
    //------------------------------------------------------------------------------
    void find_hash_section()
    {
        Elf_Half nSecNo = elf_file.sections.size();
        for ( Elf_Half i = 0; i < nSecNo; ++i ) {
            const section* sec = elf_file.sections[i];
            if ( sec->get_link() == symbol_section->get_index() &&
                 ( sec->get_type() == SHT_HASH ||
                   sec->get_type() == SHT_GNU_HASH ) ) {
                hash_section       = sec;
                hash_section_index = i;
                break;
            }
        }
    }

    //------------------------------------------------------------------------------
    // @brief Get the index of the string table
    // @return Index of the string table
    //------------------------------------------------------------------------------
    Elf_Word get_string_table_index() const
    {
        return symbol_section->get_link();
    }

    //------------------------------------------------------------------------------
    // @brief Get the index of the hash table
    // @return Index of the hash table
    //------------------------------------------------------------------------------
    Elf_Half get_hash_table_index() const { return hash_section_index; }

    //------------------------------------------------------------------------------
    // Result of a hash table lookup. A miss in a well-formed table rules out
    // the symbols the table covers; an unusable table rules out none.
    //------------------------------------------------------------------------------
    enum class hash_result
    {
        found,
        not_found,
        invalid
    };

    //------------------------------------------------------------------------------
    // @brief Read and convert a value from the hash section
    // @param byte_offset Byte offset of the value in the hash section
    // @param value Converted value
    // @return True if the complete value is inside the section, false otherwise
    //------------------------------------------------------------------------------
    template <class T>
    bool read_hash_value( Elf_Xword byte_offset, T& value ) const
    {
        if ( hash_section == nullptr ) {
            return false;
        }
        const char*     data      = hash_section->get_data();
        const Elf_Xword data_size = hash_section->get_size();
        if ( data == nullptr || byte_offset > data_size ||
             sizeof( T ) > data_size - byte_offset ) {
            return false;
        }

        T raw_value;
        std::memcpy( &raw_value, data + byte_offset, sizeof( raw_value ) );
        value = ( *elf_file.get_convertor() )( raw_value );
        return true;
    }

    //------------------------------------------------------------------------------
    // @brief Lookup a symbol in the SysV hash table
    // @param name Name of the symbol
    // @param value Value of the symbol
    // @param size Size of the symbol
    // @param bind Binding of the symbol
    // @param type Type of the symbol
    // @param section_index Section index of the symbol
    // @param other Other attributes of the symbol
    // @return Whether the symbol was found, is absent, or the table is unusable
    //------------------------------------------------------------------------------
    hash_result hash_lookup( const std::string& name,
                             Elf64_Addr&        value,
                             Elf_Xword&         size,
                             unsigned char&     bind,
                             unsigned char&     type,
                             Elf_Half&          section_index,
                             unsigned char&     other ) const
    {
        Elf_Word nbucket = 0;
        Elf_Word nchain  = 0;
        if ( !read_hash_value( 0, nbucket ) ||
             !read_hash_value( sizeof( Elf_Word ), nchain ) || nbucket == 0 ) {
            return hash_result::invalid;
        }

        // The table has one chain entry per symbol, so a miss only rules
        // symbols out when it covers the whole symbol table
        const Elf_Xword word_count =
            hash_section->get_size() / sizeof( Elf_Word );
        if ( word_count < 2 || nbucket > word_count - 2 ||
             nchain > word_count - 2 - nbucket ||
             nchain != get_symbols_num() ) {
            return hash_result::invalid;
        }

        const Elf_Word hash =
            elf_hash( reinterpret_cast<const unsigned char*>( name.c_str() ) );
        Elf_Word symbol;
        if ( !read_hash_value(
                 ( 2 + static_cast<Elf_Xword>( hash % nbucket ) ) *
                     sizeof( Elf_Word ),
                 symbol ) ) {
            return hash_result::invalid;
        }

        // A well-formed chain ends at STN_UNDEF within nchain steps
        for ( Elf_Word steps = 0; steps < nchain; ++steps ) {
            if ( symbol == STN_UNDEF ) {
                return hash_result::not_found;
            }
            if ( symbol >= nchain ) {
                return hash_result::invalid;
            }

            std::string symbol_name;
            if ( !get_symbol( symbol, symbol_name, value, size, bind, type,
                              section_index, other ) ) {
                return hash_result::invalid;
            }
            if ( symbol_name == name ) {
                return hash_result::found;
            }

            if ( !read_hash_value(
                     ( 2 + static_cast<Elf_Xword>( nbucket ) + symbol ) *
                         sizeof( Elf_Word ),
                     symbol ) ) {
                return hash_result::invalid;
            }
        }

        return symbol == STN_UNDEF ? hash_result::not_found
                                   : hash_result::invalid;
    }

    //------------------------------------------------------------------------------
    // @brief Lookup a symbol in the GNU hash table
    // @param name Name of the symbol
    // @param value Value of the symbol
    // @param size Size of the symbol
    // @param bind Binding of the symbol
    // @param type Type of the symbol
    // @param section_index Section index of the symbol
    // @param other Other attributes of the symbol
    // @param covered_begin First symbol index the hash chains cover
    // @param covered_end One past the last symbol index the hash chains cover
    // @return Whether the symbol was found, is absent from the covered
    //         symbols, or the table is unusable
    //------------------------------------------------------------------------------
    template <class T>
    hash_result gnu_hash_lookup( const std::string& name,
                                 Elf64_Addr&        value,
                                 Elf_Xword&         size,
                                 unsigned char&     bind,
                                 unsigned char&     type,
                                 Elf_Half&          section_index,
                                 unsigned char&     other,
                                 Elf_Xword&         covered_begin,
                                 Elf_Xword&         covered_end ) const
    {
        std::uint32_t nbuckets    = 0;
        std::uint32_t symoffset   = 0;
        std::uint32_t bloom_size  = 0;
        std::uint32_t bloom_shift = 0;
        if ( !read_hash_value( 0, nbuckets ) ||
             !read_hash_value( sizeof( std::uint32_t ), symoffset ) ||
             !read_hash_value( 2 * sizeof( std::uint32_t ), bloom_size ) ||
             !read_hash_value( 3 * sizeof( std::uint32_t ), bloom_shift ) ||
             nbuckets == 0 || bloom_size == 0 ||
             // glibc indexes the bloom filter with a mask, so its size must be
             // a power of two
             ( bloom_size & ( bloom_size - 1 ) ) != 0 ||
             bloom_shift >= 8 * sizeof( std::uint32_t ) ) {
            return hash_result::invalid;
        }

        const Elf_Xword     hash_size   = hash_section->get_size();
        constexpr Elf_Xword header_size = 4 * sizeof( std::uint32_t );
        if ( hash_size < header_size ||
             bloom_size > ( hash_size - header_size ) / sizeof( T ) ) {
            return hash_result::invalid;
        }

        const Elf_Xword buckets_offset =
            header_size + static_cast<Elf_Xword>( bloom_size ) * sizeof( T );
        if ( nbuckets >
             ( hash_size - buckets_offset ) / sizeof( std::uint32_t ) ) {
            return hash_result::invalid;
        }

        const Elf_Xword chains_offset =
            buckets_offset +
            static_cast<Elf_Xword>( nbuckets ) * sizeof( std::uint32_t );
        const Elf_Xword chain_count =
            ( hash_size - chains_offset ) / sizeof( std::uint32_t );
        const Elf_Xword symbol_count = get_symbols_num();
        if ( symoffset > symbol_count ) {
            return hash_result::invalid;
        }
        // Symbols below symoffset are not hashed, and a table may have fewer
        // chain entries than the remaining symbols
        covered_begin = symoffset;
        covered_end =
            symoffset + std::min( chain_count, symbol_count - symoffset );

        const std::uint32_t hash = elf_gnu_hash(
            reinterpret_cast<const unsigned char*>( name.c_str() ) );
        constexpr unsigned  bloom_word_bits = 8 * sizeof( T );
        const std::uint32_t bloom_index =
            ( hash / bloom_word_bits ) % bloom_size;
        const T bloom_bits =
            ( T{ 1 } << ( hash % bloom_word_bits ) ) |
            ( T{ 1 } << ( ( hash >> bloom_shift ) % bloom_word_bits ) );
        T bloom_value = 0;
        if ( !read_hash_value( header_size +
                                   static_cast<Elf_Xword>( bloom_index ) *
                                       sizeof( T ),
                               bloom_value ) ) {
            return hash_result::invalid;
        }
        if ( ( bloom_value & bloom_bits ) != bloom_bits ) {
            return hash_result::not_found;
        }

        const std::uint32_t bucket = hash % nbuckets;
        std::uint32_t       symbol = 0;
        if ( !read_hash_value( buckets_offset +
                                   static_cast<Elf_Xword>( bucket ) *
                                       sizeof( std::uint32_t ),
                               symbol ) ) {
            return hash_result::invalid;
        }
        // Bucket value 0 marks an empty bucket
        if ( symbol == 0 ) {
            return hash_result::not_found;
        }
        if ( symbol < symoffset ) {
            return hash_result::invalid;
        }

        Elf_Xword chain_index = symbol - symoffset;
        while ( chain_index < chain_count ) {
            if ( chain_index >= symbol_count - symoffset ) {
                return hash_result::invalid;
            }

            std::uint32_t chain_hash = 0;
            if ( !read_hash_value( chains_offset +
                                       chain_index * sizeof( std::uint32_t ),
                                   chain_hash ) ) {
                return hash_result::invalid;
            }

            std::string symbol_name;
            if ( ( chain_hash >> 1 ) == ( hash >> 1 ) &&
                 get_symbol( static_cast<Elf_Xword>( symoffset ) + chain_index,
                             symbol_name, value, size, bind, type,
                             section_index, other ) &&
                 name == symbol_name ) {
                return hash_result::found;
            }

            if ( chain_hash & 1 ) {
                return hash_result::not_found;
            }
            ++chain_index;
        }

        return hash_result::invalid;
    }

    //------------------------------------------------------------------------------
    // @brief Check whether the symbol at the specified index is local
    // @param index Index of the symbol
    // @return True if the symbol exists and has STB_LOCAL binding
    //------------------------------------------------------------------------------
    bool is_local_symbol( Elf_Xword index ) const
    {
        if ( elf_file.get_class() == ELFCLASS32 ) {
            return generic_is_local_symbol<Elf32_Sym>( index );
        }
        return generic_is_local_symbol<Elf64_Sym>( index );
    }

    //------------------------------------------------------------------------------
    template <class T> bool generic_is_local_symbol( Elf_Xword index ) const
    {
        const T* record = generic_get_symbol_ptr<T>( index );
        if ( record == nullptr ) {
            return false;
        }
        T symbol;
        std::memcpy( &symbol, record, sizeof( symbol ) );
        return ELF_ST_BIND( symbol.st_info ) == STB_LOCAL;
    }

    //------------------------------------------------------------------------------
    // @brief Get the symbol at the specified index
    // @param index Index of the symbol
    // @return Pointer to the symbol
    //------------------------------------------------------------------------------
    template <class T> const T* generic_get_symbol_ptr( Elf_Xword index ) const
    {
        if ( 0 != symbol_section->get_data() && index < get_symbols_num() ) {
            if ( symbol_section->get_entry_size() < sizeof( T ) ) {
                return nullptr;
            }
            const auto* pSym = reinterpret_cast<const T*>(
                symbol_section->get_data() +
                index * symbol_section->get_entry_size() );

            return pSym;
        }

        return nullptr;
    }

    //------------------------------------------------------------------------------
    // @brief Search for a symbol in the section
    // @param match Function to be called for each symbol
    // @param idx Index of the found symbol
    // @return True if the symbol is found, false otherwise
    //------------------------------------------------------------------------------
    template <class T>
    bool generic_search_symbols( std::function<bool( const T* )> match,
                                 Elf_Xword&                      idx ) const
    {
        for ( Elf_Xword i = 0; i < get_symbols_num(); i++ ) {
            const T* symPtr = generic_get_symbol_ptr<T>( i );

            if ( symPtr == nullptr )
                return false;

            // Symbol records may be unaligned in the section data
            T symbol;
            std::memcpy( &symbol, symPtr, sizeof( symbol ) );
            if ( match( &symbol ) ) {
                idx = i;
                return true;
            }
        }

        return false;
    }

    //------------------------------------------------------------------------------
    // @brief Get the symbol at the specified index
    // @param index Index of the symbol
    // @param name Name of the symbol
    // @param value Value of the symbol
    // @param size Size of the symbol
    // @param bind Binding of the symbol
    // @param type Type of the symbol
    // @param section_index Section index of the symbol
    // @param other Other attributes of the symbol
    // @return True if the symbol is found, false otherwise
    //------------------------------------------------------------------------------
    template <class T>
    bool generic_get_symbol( Elf_Xword      index,
                             std::string&   name,
                             Elf64_Addr&    value,
                             Elf_Xword&     size,
                             unsigned char& bind,
                             unsigned char& type,
                             Elf_Half&      section_index,
                             unsigned char& other ) const
    {
        bool ret = false;

        if ( nullptr != symbol_section->get_data() &&
             index < get_symbols_num() ) {
            // Symbol records may be unaligned in the section data
            T symbol;
            std::memcpy( &symbol,
                         symbol_section->get_data() +
                             index * symbol_section->get_entry_size(),
                         sizeof( symbol ) );

            const auto& convertor = elf_file.get_convertor();

            // An unresolvable name yields an empty string rather than
            // leaving the caller's previous value in place
            const section* string_section =
                elf_file.sections[get_string_table_index()];
            const char* pStr = nullptr;
            if ( string_section != nullptr &&
                 string_section->get_type() == SHT_STRTAB ) {
                const_string_section_accessor str_reader( string_section );
                pStr =
                    str_reader.get_string( ( *convertor )( symbol.st_name ) );
            }
            name          = pStr != nullptr ? pStr : "";
            value         = ( *convertor )( symbol.st_value );
            size          = ( *convertor )( symbol.st_size );
            bind          = ELF_ST_BIND( symbol.st_info );
            type          = ELF_ST_TYPE( symbol.st_info );
            section_index = ( *convertor )( symbol.st_shndx );
            other         = symbol.st_other;

            ret = true;
        }

        return ret;
    }

    //------------------------------------------------------------------------------
    // @brief Add a symbol to the section
    // @param name Name of the symbol
    // @param value Value of the symbol
    // @param size Size of the symbol
    // @param info Info of the symbol
    // @param other Other attributes of the symbol
    // @param shndx Section index of the symbol
    // @return Index of the added symbol
    //------------------------------------------------------------------------------
    template <class T>
    Elf_Word generic_add_symbol( Elf_Word      name,
                                 Elf64_Addr    value,
                                 Elf_Xword     size,
                                 unsigned char info,
                                 unsigned char other,
                                 Elf_Half      shndx )
    {
        const auto& convertor = elf_file.get_convertor();

        T entry;
        entry.st_name  = ( *convertor )( name );
        entry.st_value = decltype( entry.st_value )( value );
        entry.st_value = ( *convertor )( entry.st_value );
        entry.st_size  = decltype( entry.st_size )( size );
        entry.st_size  = ( *convertor )( entry.st_size );
        entry.st_info  = ( *convertor )( info );
        entry.st_other = ( *convertor )( other );
        entry.st_shndx = ( *convertor )( shndx );

        symbol_section->append_data( reinterpret_cast<char*>( &entry ),
                                     sizeof( entry ) );

        Elf_Word nRet =
            Elf_Word( symbol_section->get_size() / sizeof( entry ) - 1 );

        return nRet;
    }

    //------------------------------------------------------------------------------
    // @brief Arrange local symbols in the section
    // @param func Function to be called for each pair of symbols
    // @return Number of local symbols
    //------------------------------------------------------------------------------
    template <class T>
    Elf_Xword generic_arrange_local_symbols(
        std::function<void( Elf_Xword first, Elf_Xword second )> func )
    {
        const auto& convertor = elf_file.get_convertor();

        Elf_Xword first_not_local =
            1; // Skip the first entry. It is always NOTYPE
        Elf_Xword current = 0;
        // Without data there are no records to rearrange
        Elf_Xword count =
            symbol_section->get_data() != nullptr ? get_symbols_num() : 0;

        // Symbol records may be unaligned in the section data, so they are
        // copied in and out instead of being accessed in place
        auto record = [this]( Elf_Xword index ) {
            return const_cast<char*>( reinterpret_cast<const char*>(
                generic_get_symbol_ptr<T>( index ) ) );
        };
        auto is_local = [&]( Elf_Xword index ) {
            T symbol;
            std::memcpy( &symbol, record( index ), sizeof( symbol ) );
            return ELF_ST_BIND( ( *convertor )( symbol.st_info ) ) == STB_LOCAL;
        };

        while ( true ) {
            while ( first_not_local < count && is_local( first_not_local ) ) {
                ++first_not_local;
            }

            current = first_not_local + 1;
            while ( current < count && !is_local( current ) ) {
                ++current;
            }

            if ( first_not_local < count && current < count ) {
                if ( func )
                    func( first_not_local, current );

                T first;
                T second;
                std::memcpy( &first, record( first_not_local ),
                             sizeof( first ) );
                std::memcpy( &second, record( current ), sizeof( second ) );
                std::memcpy( record( first_not_local ), &second,
                             sizeof( second ) );
                std::memcpy( record( current ), &first, sizeof( first ) );
            }
            else {
                // Update 'info' field of the section
                symbol_section->set_info(
                    static_cast<Elf_Word>( first_not_local ) );
                break;
            }
        }

        return first_not_local;
    }

    //------------------------------------------------------------------------------
  private:
    const elfio&   elf_file;                ///< Reference to the ELF file
    S*             symbol_section;          ///< Pointer to the symbol section
    Elf_Half       hash_section_index{ 0 }; ///< Index of the hash section
    const section* hash_section{ nullptr }; ///< Pointer to the hash section
};

using symbol_section_accessor = symbol_section_accessor_template<section>;
using const_symbol_section_accessor =
    symbol_section_accessor_template<const section>;

} // namespace ELFIO

#endif // ELFIO_SYMBOLS_HPP
