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

#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

#include <elfio/elfio.hpp>
#include <elfio/elfio_dump.hpp>

using namespace ELFIO;

void get_translation_ranges( std::ifstream&                    proc_maps,
                             const std::string&                file_name,
                             std::vector<address_translation>& result )
{
    result.clear();

    const std::regex rexpr(
        "([0-9A-Fa-f]+)-([0-9A-Fa-f]+) ([-r]...) ([0-9A-Fa-f]+) (.....) "
        "([0-9]+)([[:blank:]]*)([[:graph:]]*)" );
    std::smatch match;
    while ( proc_maps ) {
        std::string line;
        std::getline( proc_maps, line );

        if ( std::regex_match( line, match, rexpr ) ) {
            if ( match.size() == 9 && match[8].str() == file_name ) {
                result.emplace_back( address_translation(
                    std::stoul( match[1].str(), 0, 16 ),
                    std::stoul( match[2].str(), 0, 16 ),
                    std::stoul( match[4].str(), 0, 16 ) ) );
            }
        }
    }

    std::sort( result.begin(), result.end(),
               []( address_translation& a, address_translation& b ) -> bool {
                   return a.map_to < b.map_to;
               } );
}

int main( int argc, char** argv )
{
    if ( argc != 3 ) {
        std::cout << "Usage: proc_mem pid full_file_path" << std::endl;
        return 1;
    }

    // Process file translation regions for the ELF file from /proc/pid/maps
    std::ifstream proc_maps( std::string( "/proc/" ) + argv[1] + "/maps" );
    if ( !proc_maps ) {
        std::cout << "Can't open "
                  << std::string( "/proc/" ) + argv[1] + "/maps"
                  << " file" << std::endl;
        return 2;
    }

    std::vector<address_translation> ranges;
    get_translation_ranges( proc_maps, argv[2], ranges );
    // for ( auto& range : ranges ) {
    //     std::cout << std::hex << range.start << " " << range.end << " "
    //               << range.map_to << std::endl;
    // }

    elfio elffile;
    elffile.set_address_translation( ranges );

    if ( elffile.load( std::string( "/proc/" ) + argv[1] + "/mem" ) ) {
        dump::header( std::cout, elffile );
        dump::segment_headers( std::cout, elffile );
        dump::segment_datas( std::cout, elffile );
    }
    else {
        std::cout << "Can't open " << std::string( "/proc/" ) + argv[1] + "/mem"
                  << " file" << std::endl;
    }

    return 0;
}
