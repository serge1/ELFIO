/*
anonymizer.cpp - Overwrites all data from an ELF file with random data.

Copyright (C) 2017 by Martin Bickel

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

#include <string>
#include <iostream>
#include <elfio/elfio_dump.hpp>
#include <fstream>
#include <random>

using namespace ELFIO;


bool preserve_name( const std::string& name ) {
    static std::vector<std::string> names_to_preserve = { ".shstrtab",
                                                           ".rodata",
                                                           ".bss",
                                                           ".data",
                                                           ".text",
                                                           ".text_vle" };

    for ( auto s = names_to_preserve.begin(); s != names_to_preserve.end(); ++s )
        if ( *s == name)
            return true;
    return false;
}

std::default_random_engine generator(0xe1f);
std::uniform_int_distribution<int> distribution(0,255);

void randomize_data( const std::string& filename, long offset, long size ) {
    std::ofstream file (filename, std::ios::in|std::ios::out|std::ios::binary);
    if ( !file )
        throw "error opening file" + filename;
    file.seekp(offset);
    for ( long i = 0; i < size; ++i ) {
        const char value = distribution(generator);
        file.write(&value, 1 );
    }
}

void overwrite_data( const std::string& filename, long offset, const std::string& data ) {
    std::ofstream file (filename, std::ios::in|std::ios::out|std::ios::binary);
    if ( !file )
        throw "error opening file" + filename;
    file.seekp(offset);
    file.write(data.c_str(), data.length()+1 );
}



std::string generate( int length ) {

    static int counters[6] = { 0, 0, 10, 100, 1000, 10000 };

    int counter = counters[length > 5 ? 5 : length ]++;
    
    auto s = std::to_string(counter);
    if ( s.length() > length ) {
        throw "String length error at " + std::to_string( counter ) + "; expecting " + std::to_string( length ) + " bytes ";
    }

    while (s.length() < length)
        s = "s" + s;
    return s;
}


void processStringTable( const section* s, const std::string& filename ) {
    std::cout << "Info: processing string table section" << std::endl;
    int counter = 0;
    int index = 1;
    while ( index < s->get_size() ) {
        auto len = strlen( s->get_data()+index );
        if ( len && !preserve_name( s->get_data()+index ))
            overwrite_data( filename, s->get_offset() + index, generate( len ));
        index += len + 1;
        counter++;
    }
    std::cout << counter << " strings found " << std::endl;
}

int main( int argc, char** argv )
{
    try {
        if ( argc != 2 ) {
            std::cout << "Usage: anonymizer <file_name>\n";
            return 1;
        }

        std::string filename = argv[1];

        elfio reader;

        if ( !reader.load( filename ) ) {
            std::cerr << "File " << filename << " is not found or it is not an ELF file\n";
            return 1;
        }

        for ( auto sect = reader.sections.begin(); sect != reader.sections.end(); ++sect ) {
            section* s = *sect;
            if ( s->get_type() == SHT_STRTAB ) {
                processStringTable(s, filename);
            } else if ( s->get_type() == SHT_SYMTAB  ) {
                std::cout << "Skipping symbol table." << std::endl;
            } else if ( s->get_type() == SHT_PROGBITS ) {
                if ( s->get_size() > 0 ) {
                    randomize_data(filename, s->get_offset(), s->get_size());
                    std::cout << "Sanitized " << s->get_size() << " Bytes in section" << std::endl;
                }
            }
        }
        return 0;
    } catch ( const std::string& s ) {
        std::cerr << s << std::endl;
    } catch ( const char* s ) {
        std::cerr << s << std::endl;
    }
    return 1;
}
