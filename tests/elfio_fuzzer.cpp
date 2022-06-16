#include <string>
#include <sstream>

#include <elfio/elfio.hpp>
using namespace ELFIO;

extern "C" int LLVMFuzzerTestOneInput( const uint8_t* Data, size_t Size )
{
    std::string        str( (const char*)Data, Size );
    std::istringstream ss( str );

    elfio elf;
    elf.load( ss );

    return 0;
}
