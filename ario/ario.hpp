//------------------------------------------------------------------------------
//! @file ario.hpp
//! @brief ARIO - Simple ar(1) archive reader/writer interface
//!
//! This file provides the ARIO namespace and the ario class for reading and manipulating UNIX ar archives.
//!
//! Copyright (C) 2025-present by Serge Lamikhov-Center
//!
//! Permission is hereby granted, free of charge, to any person obtaining a copy
//! of this software and associated documentation files (the "Software"), to deal
//! in the Software without restriction, including without limitation the rights
//! to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//! copies of the Software, and to permit persons to whom the Software is
//! furnished to do so, subject to the following conditions:
//!
//! The above copyright notice and this permission notice shall be included in
//! all copies or substantial portions of the Software.
//!
//! THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//! IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//! FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//! AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//! LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//! OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//! THE SOFTWARE.

#ifndef ARIO_HPP
#define ARIO_HPP

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <string_view>
#include <sstream>
#include <vector>
#include <optional>
#include <unordered_map>
#include <algorithm>
#include <memory>

//------------------------------------------------------------------------------
namespace ARIO {

//------------------------------------------------------------------------------
//! @class ario
//! @brief Class for reading and manipulating ar(1) archives
class ario
{
  public:
    //------------------------------------------------------------------------------
    //! @brief Error structure for ARIO operations
    class Result
    {
      public:
        Result() = default;
        Result( const std::string& msg ) : message( msg ) {}
        Result( std::string&& msg ) : message( std::move( msg ) ) {}

        bool        ok() const { return !message.has_value(); }
        std::string what() const { return message.value_or( "No errors" ); }

      private:
        std::optional<std::string> message; ///< Error message, if any
    };

    //------------------------------------------------------------------------------
    //! @struct Member
    //! @brief Represents a single member (file) in the archive
    class Member
    {
        friend class ario;

      public:
        explicit Member( std::istream* pstream = nullptr ) : pstream( pstream )
        {
        }

        std::string    name    = {}; ///< Name of the member
        int            date    = {}; ///< Date of the member
        int            uid     = {}; ///< User ID of the member
        int            gid     = {}; ///< Group ID of the member
        int            mode    = {}; ///< Mode of the member
        int            size    = {}; ///< Size of the member in the archive
        std::streamoff filepos = {}; ///< File position in the archive

        //------------------------------------------------------------------------------
        //! @brief Get the data of the member as a string
        //! @return The data of the member
        std::string data() const
        {
            if ( !pstream ) {
                return { "No input stream available for member data" };
            }

            std::string    data( size, '\0' );
            std::streamoff current_pos = pstream->tellg();
            pstream->seekg( filepos + HEADER_SIZE, std::ios::beg );
            pstream->read( &data[0], size );
            pstream->clear();
            pstream->seekg( current_pos, std::ios::beg );
            if ( pstream->gcount() < size ) {
                return { "Member data read error" };
            }

            return data;
        }

      protected:
        std::string   short_name = {};      ///< Short name of the member
        std::istream* pstream    = nullptr; ///< Pointer to the input stream
    };

    //------------------------------------------------------------------------------
    //! @class Members
    //! @brief Provides access to the members of the archive
    class Members
    {
      public:
        //------------------------------------------------------------------------------
        //! @brief Constructor
        //! @param parent Pointer to the parent ario object
        explicit Members( ario* parent ) : parent( parent ) {}

        //------------------------------------------------------------------------------
        //! @brief Get the number of members
        //! @return The number of members
        size_t size() const { return parent->members_.size(); }

        //------------------------------------------------------------------------------
        //! @brief Get a member by index
        //! @param index The index of the member
        //! @return Reference to the member
        const Member& operator[]( size_t index ) const
        {
            if ( index >= parent->members_.size() ) {
                throw std::out_of_range( "Member index out of range" );
            }
            return parent->members_[index];
        }

        //------------------------------------------------------------------------------
        //! @brief Get a member by name
        //! @param name The name of the member
        //! @return Reference to the member, throws if not found
        const Member& operator[]( std::string_view name ) const
        {
            for ( const auto& m : parent->members_ ) {
                if ( m.name == name ) {
                    return m;
                }
            }
            throw std::out_of_range( std::string( "Member not found: " ) +
                                     std::string( name ) );
        }

        //------------------------------------------------------------------------------
        //! @brief Get an iterator to the beginning of the members
        //! @return Iterator to the beginning of the members
        std::vector<Member>::iterator begin()
        {
            return parent->members_.begin();
        }

        //------------------------------------------------------------------------------
        //! @brief Get an iterator to the end of the members
        //! @return Iterator to the end of the members
        std::vector<Member>::iterator end() { return parent->members_.end(); }

        //------------------------------------------------------------------------------
        //! @brief Get a const iterator to the beginning of the members
        //! @return Const iterator to the beginning of the members
        std::vector<Member>::const_iterator begin() const
        {
            return parent->members_.cbegin();
        }

        //------------------------------------------------------------------------------
        //! @brief Get a const iterator to the end of the members
        //! @return Const iterator to the end of the members
        std::vector<Member>::const_iterator end() const
        {
            return parent->members_.cend();
        }

      private:
        ario* parent; //!< Pointer to the parent ario object
    };

    //------------------------------------------------------------------------------
    //! @brief Constructor
    explicit ario() : members( this ) {};
    ario( const ario& )            = delete;
    ario& operator=( const ario& ) = delete;
    ario( ario&& )                 = delete;
    ario& operator=( ario&& )      = delete;
    ~ario()                        = default;

    //------------------------------------------------------------------------------
    //! @brief Load an archive from a file
    //! @param file_name The name of the archive file
    //! @return Error object indicating success or failure
    Result load( const std::string& file_name )
    {
        auto ifs = std::make_unique<std::ifstream>();
        if ( !ifs ) {
            return { "Failed to create input stream" };
        }

        ifs->open( file_name.c_str(), std::ios::in | std::ios::binary );
        if ( !*ifs ) {
            return { "Failed to open file: " + file_name };
        }

        auto result = load( std::move( ifs ) );

        return result;
    }

    //------------------------------------------------------------------------------
    //! @brief Load an archive from a stream
    //! @param stream The input stream to load from
    //! @return Error object indicating success or failure
    Result load( std::unique_ptr<std::istream> is )
    {
        if ( !is ) {
            return { "Input stream is null" };
        }

        pstream = std::move( is );
        if ( !*pstream ) {
            return { "Failed to set input stream" };
        }

        auto result = load_header();
        if ( !result.ok() ) {
            return result;
        }

        result = load_members();
        if ( !result.ok() ) {
            return result;
        }

        return {};
    }

    //------------------------------------------------------------------------------
    //! @brief Save an archive to a file
    //! @param file_name The name of the archive file
    //! @return Error object indicating success or failure
    Result save( const std::string& file_name )
    {
        std::ofstream ofs;

        ofs.open( file_name.c_str(), std::ios::out | std::ios::binary );
        if ( !ofs ) {
            return { "Failed to open file: " + file_name };
        }

        auto result = save( ofs );

        ofs.close();

        return result;
    }

    //------------------------------------------------------------------------------
    //! @brief Save an archive to a stream
    //! @param stream The output stream to save to
    //! @return Error object indicating success or failure
    Result save( std::ostream& pos )
    {
        if ( !pos ) {
            return { "Output stream is null" };
        }

        auto result = save_header( pos );
        if ( !result.ok() ) {
            return result;
        }

        // Save symbol table if it exists
        if ( !symbol_table.empty() ) {
            result = save_symbol_table( pos );
            if ( !result.ok() ) {
                return result;
            }
        }

        // Save long name directory if it exists
        if ( !string_table.empty() ) {
            result = save_long_name_directory( pos );
            if ( !result.ok() ) {
                return result;
            }
        }

        result = save_members( pos );
        if ( !result.ok() ) {
            return result;
        }

        return {};
    }

    //! @brief Find a symbol in the archive
    //! @param name The name of the symbol to find
    //! @param out_member Pointer to store the found member
    //! @return Error object indicating success or failure
    //! If the symbol is found, out_member will point to the corresponding member
    //! If the symbol is not found, out_member will be set to nullptr
    Result find_symbol( std::string_view name, Member& member ) const
    {
        const auto it = symbol_table.find( std::string( name ) );
        if ( it != symbol_table.end() ) {
            member = members_[it->second];
            return {};
        }
        return { std::string( "Symbol not found: " ) + std::string( name ) };
    }

    //------------------------------------------------------------------------------
    //! @brief Get symbols for a specific member
    //! @param m Pointer to the member
    //! @param symbols Vector to store the found symbols
    //! @return Error object indicating success or failure
    //! If the member is found, symbols will contain the associated symbols
    //! If the member is not found, symbols will be empty
    Result get_symbols_for_member( const ario::Member&       member,
                                   std::vector<std::string>& symbols ) const
    {
        // Use string_view for comparison to avoid unnecessary allocations
        std::string_view member_name = member.name;
        auto             index       = std::distance(
            members.begin(),
            std::find_if(
                members.begin(), members.end(), [&]( const auto& mem ) {
                    return std::string_view( mem.name ) == member_name;
                } ) );
        if ( index >= members.size() ) {
            return { "Member not found in archive" };
        }

        symbols.clear();
        for ( const auto& symbol : symbol_table ) {
            if ( symbol.second == index ) {
                symbols.emplace_back( symbol.first );
            }
        }

        return {};
    }

  protected:
    //------------------------------------------------------------------------------
    //! @brief Load the archive header
    //! @param in Input file stream
    //! @return Error object indicating success or failure
    Result load_header()
    {
        std::string magic( sizeof( ARCH_MAGIC ), ' ' );
        pstream->read( &magic[0], sizeof( ARCH_MAGIC ) );
        if ( magic != ARCH_MAGIC ) {
            return { std::string( "Invalid archive format. Expected magic: " ) +
                     ARCH_MAGIC };
        }

        return {};
    }

    //------------------------------------------------------------------------------
    //! @brief Load all members from the archive
    //! @param in Input file stream
    //! @return Error object indicating success or failure
    Result load_members()
    {
        while ( true ) {
            Member m( &*pstream );
            char   header[HEADER_SIZE];
            auto   filepos = pstream->tellg();

            pstream->read( header, HEADER_SIZE );
            if ( pstream->gcount() < HEADER_SIZE ) {
                break; // End of file or error
            }
            std::streamoff current_pos = pstream->tellg();
            m.short_name               = std::string( header, 16 );
            m.name                     = m.short_name;
            m.filepos                  = filepos;

            std::string date_str( header + 16, 12 );
            std::string uid_str( header + 28, 6 );
            std::string gid_str( header + 34, 6 );
            std::string mode_str( header + 40, 8 );
            std::string size_str( header + 48, 10 );

            try {
                // Get m.size from the header. Do it earlier due to the potential use of m.data()
                // It is the only valid field in special members like symbol table and long name directory
                m.size = std::stoi( size_str );
            }
            catch ( const std::exception& ) {
                return { "Invalid member size" };
            }

            if ( m.short_name ==
                 "/               " ) { // Special case for the symbol table
                m.name      = "/";
                auto result = load_symbol_table();
                if ( !result.ok() ) {
                    return result;
                }
            }
            else if (
                m.short_name ==
                "//              " ) { // Special case for the long name directory
                m.name       = "//";
                string_table = m.data(); // Read the long name directory data
            }
            else {
                try {
                    m.date = std::stoi( date_str );
                    m.uid  = std::stoi( uid_str );
                    m.gid  = std::stoi( gid_str );
                    m.mode = std::stoi( mode_str, nullptr, 8 );
                }
                catch ( const std::exception& ) {
                    return { "Invalid member header's field" };
                }

                if ( m.short_name[0] == '/' ) {
                    auto name_result = convert_name( m.short_name );
                    if ( !name_result.has_value() ) {
                        return { "Failed to convert long name for member " +
                                 m.short_name };
                    }
                    m.name = *name_result;
                }
                else {
                    m.name = m.short_name.substr( 0, m.short_name.find( '/' ) );
                }

                // Add only the regular member to the list
                members_.emplace_back( m );
            }

            // Skip the content of the member
            pstream->clear();
            pstream->seekg( current_pos + m.size + m.size % 2, std::ios::beg );
        }

        pstream->clear();

        // Substitute symbol locations with member indexes
        for ( auto& symbol : symbol_table ) {
            uint32_t   index = 0;
            const auto it    = std::find_if(
                members_.begin(), members_.end(),
                [&]( const Member& m ) { return m.filepos == symbol.second; } );
            if ( it != members_.end() ) {
                index = std::distance( members_.begin(), it );
            }
            symbol.second = index;
        }

        return {};
    }

    Result save_header( std::ostream& os )
    {
        if ( !os ) {
            return { "Output stream is null" };
        }

        // Write the archive magic string
        os << ARCH_MAGIC;

        return {};
    }

    //------------------------------------------------------------------------------
    //! @brief Save the symbol table to the archive
    //! @param os Output stream to save the symbol table
    //! @return Error object indicating success or failure
    Result save_symbol_table( std::ostream& os )
    {
        if ( !os ) {
            return { "Output stream is null" };
        }
        // Write the number of symbols
        uint32_t num_of_symbols = static_cast<uint32_t>( symbol_table.size() );
        os.write( reinterpret_cast<const char*>( &num_of_symbols ),
                  sizeof( num_of_symbols ) );

        for ( const auto& symbol : symbol_table ) {
            os.write( reinterpret_cast<const char*>( &symbol.first ),
                      sizeof( symbol.first ) );
            //os.write( symbol.second.c_str(), symbol.second.size() + 1 );
        }

        return {};
    }

    //------------------------------------------------------------------------------
    //! @brief Save the long name directory to the archive
    //! @param os Output stream to save the long name directory
    //! @return Error object indicating success or failure
    Result save_long_name_directory( std::ostream& os )
    {
        if ( !os ) {
            return { "Output stream is null" };
        }
        // Write the long name directory
        os << "//";
        os << string_table;
        os << HEADER_END_MAGIC; // End of long name directory

        return {};
    }

    //------------------------------------------------------------------------------
    //! @brief Save the member information to the archive
    //! @param os Output stream to save the member information
    //! @return Error object indicating success or failure
    Result save_members( std::ostream& os )
    {
        if ( !os ) {
            return { "Output stream is null" };
        }

        for ( const auto& member : members_ ) {
            // clang-format off
            // Write the member header
            os << std::setw( 16 ) << std::left << member.short_name
                << std::setw( 12 ) << std::left << member.date
                << std::setw( 6 )  << std::left << member.uid
                << std::setw( 6 )  << std::left << member.gid
                << std::setw( 8 )  << std::left << std::oct << member.mode
                << std::setw( 10 ) << std::left << std::dec << member.size
                << HEADER_END_MAGIC;
            // clang-format on

            // Write the content of the member
            os.write( member.data().data(), member.size );
            if ( os.fail() ) {
                return { "Failed to write member data" };
            }
            if ( member.size % 2 != 0 ) {
                // Write a padding byte if the size is odd
                os.put( '\x0A' );
                if ( os.fail() ) {
                    return { "Failed to write padding byte" };
                }
            }
        }

        return {};
    }

    //------------------------------------------------------------------------------
    //! @brief Read the symbol table from the archive symbol table member
    //! @return Error object indicating success or failure
    Result load_symbol_table()
    {
        char buf[4];
        pstream->read( buf, sizeof( buf ) );
        if ( pstream->gcount() < sizeof( buf ) ) {
            return { "Failed to read symbol table" };
        }

        uint32_t num_of_symbols = ( static_cast<uint8_t>( buf[0] ) << 24 ) |
                                  ( static_cast<uint8_t>( buf[1] ) << 16 ) |
                                  ( static_cast<uint8_t>( buf[2] ) << 8 ) |
                                  ( static_cast<uint8_t>( buf[3] ) << 0 );

        std::vector<std::pair<uint32_t, std::string>> v( num_of_symbols );

        // Read symbol locations
        for ( uint32_t i = 0; i < num_of_symbols; ++i ) {
            pstream->read( buf, sizeof( buf ) );
            if ( pstream->gcount() < sizeof( buf ) ) {
                return { "Failed to read symbol table" };
            }
            uint32_t member_location =
                ( static_cast<uint8_t>( buf[0] ) << 24 ) |
                ( static_cast<uint8_t>( buf[1] ) << 16 ) |
                ( static_cast<uint8_t>( buf[2] ) << 8 ) |
                ( static_cast<uint8_t>( buf[3] ) << 0 );
            v[i].first = member_location;
        }

        // Read symbol names
        for ( uint32_t i = 0; i < num_of_symbols; ++i ) {
            std::string sym_name;
            std::getline( *pstream, sym_name, '\0' );
            v[i].second = sym_name;
        }

        // Copy to symbol_table map
        for ( const auto& pair : v ) {
            symbol_table[pair.second] = pair.first;
        }

        return {};
    }

    //------------------------------------------------------------------------------
    //! @brief Convert a short name to a long name using the long name directory
    //! @param short_name The short name to convert
    //! @return The long name
    std::optional<std::string> convert_name( std::string_view short_name )
    {
        size_t pos = short_name.find( '/' );
        if ( pos == 0 ) {
            if ( short_name.size() < 3 ) {
                return std::nullopt;
            }
            size_t offset_in_dir = 0;
            try {
                offset_in_dir = std::stoul( std::string(
                    short_name.substr( 1, short_name.size() - 2 ) ) );
            }
            catch ( const std::exception& ) {
                return std::nullopt;
            }
            if ( offset_in_dir >= string_table.size() ) {
                return std::nullopt;
            }
            size_t end = string_table.find( '/', offset_in_dir );
            if ( end == std::string::npos || end <= offset_in_dir ) {
                return std::nullopt;
            }
            std::string long_name =
                string_table.substr( offset_in_dir, end - offset_in_dir );
            return long_name;
        }
        else if ( pos != std::string::npos && pos != 0 &&
                  pos < short_name.size() ) {
            return std::string( short_name.substr( 0, pos ) );
        }

        return std::string( short_name );
    }

  public:
    Members members; //!< Members object

  protected:
    static constexpr const char* ARCH_MAGIC =
        "!<arch>\x0A"; ///< Archive magic string
    static constexpr const char* HEADER_END_MAGIC =
        "\x60\x0A"; ///< End of header magic
    static constexpr std::streamsize HEADER_SIZE =
        60; ///< Size of archive header

    std::unique_ptr<std::istream> pstream =
        nullptr;                  //!< Pointer to the input stream
    std::vector<Member> members_; //!< Vector of archive members
    //!< Symbol table
    //!< This is a map from symbol names to member indexes
    //!< The member index is the index in the members_ vector
    //!< This allows for quick lookup of symbols by name
    std::unordered_map<std::string, uint32_t> symbol_table;
    std::string string_table; //!< Long names for members
};

} // namespace ARIO

#endif // ARIO_HPP
