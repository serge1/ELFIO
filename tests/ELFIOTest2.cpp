#ifdef _MSC_VER
#define _SCL_SECURE_NO_WARNINGS
#define ELFIO_NO_INTTYPES
#endif

#include <boost/test/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>
using boost::test_tools::output_test_stream;

#include <elfio/elfio.hpp>

using namespace ELFIO;

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( modinfo_read )
{
    elfio reader;
    BOOST_REQUIRE_EQUAL( reader.load( "elf_examples/zavl.ko" ), true );

    section* modinfo_sec = reader.sections[".modinfo"];
    BOOST_REQUIRE_NE( modinfo_sec, nullptr );

    const_modinfo_section_accessor modinfo( modinfo_sec );
    BOOST_REQUIRE_EQUAL( modinfo.get_attribute_num(), (Elf_Word)9 );

    struct
    {
        std::string field;
        std::string value;
    } attributes[] = { { "version", "0.8.3-1ubuntu12.1" },
                       { "license", "CDDL" },
                       { "author", "OpenZFS on Linux" },
                       { "description", "Generic AVL tree implementation" },
                       { "srcversion", "98E85778E754CF75DEF9E8E" },
                       { "depends", "spl" },
                       { "retpoline", "Y" },
                       { "name", "zavl" },
                       { "vermagic", "5.4.0-42-generic SMP mod_unload " } };

    for ( auto i = 0; i < sizeof( attributes ) / sizeof( attributes[0] );
          i++ ) {
        std::string field;
        std::string value;
        modinfo.get_attribute( i, field, value );

        BOOST_CHECK_EQUAL( field, attributes[i].field );
        BOOST_CHECK_EQUAL( value, attributes[i].value );
    }

    for ( auto i = 0; i < sizeof( attributes ) / sizeof( attributes[0] );
          i++ ) {
        std::string field = attributes[i].field;
        std::string value;
        modinfo.get_attribute( field, value );

        BOOST_CHECK_EQUAL( value, attributes[i].value );
    }
}

////////////////////////////////////////////////////////////////////////////////
BOOST_AUTO_TEST_CASE( modinfo_write )
{
    elfio writer;
    BOOST_REQUIRE_EQUAL( writer.load( "elf_examples/zavl.ko" ), true );

    section* modinfo_sec = writer.sections[".modinfo"];
    BOOST_REQUIRE_NE( modinfo_sec, nullptr );

    modinfo_section_accessor modinfo( modinfo_sec );
    BOOST_REQUIRE_EQUAL( modinfo.get_attribute_num(), (Elf_Word)9 );

    modinfo.add_attribute( "test1", "value1" );
    modinfo.add_attribute( "test2", "value2" );

    BOOST_REQUIRE_EQUAL( modinfo.get_attribute_num(), (Elf_Word)11 );

    BOOST_REQUIRE_EQUAL( writer.save( "elf_examples/zavl_gen.ko" ), true );

    elfio reader;
    BOOST_REQUIRE_EQUAL( reader.load( "elf_examples/zavl_gen.ko" ), true );

    modinfo_sec = reader.sections[".modinfo"];
    BOOST_REQUIRE_NE( modinfo_sec, nullptr );

    const_modinfo_section_accessor modinfo1( modinfo_sec );
    BOOST_REQUIRE_EQUAL( modinfo1.get_attribute_num(), (Elf_Word)11 );

    struct
    {
        std::string field;
        std::string value;
    } attributes[] = { { "version", "0.8.3-1ubuntu12.1" },
                       { "license", "CDDL" },
                       { "author", "OpenZFS on Linux" },
                       { "description", "Generic AVL tree implementation" },
                       { "srcversion", "98E85778E754CF75DEF9E8E" },
                       { "depends", "spl" },
                       { "retpoline", "Y" },
                       { "name", "zavl" },
                       { "vermagic", "5.4.0-42-generic SMP mod_unload " },
                       { "test1", "value1" },
                       { "test2", "value2" } };

    for ( auto i = 0; i < sizeof( attributes ) / sizeof( attributes[0] );
          i++ ) {
        std::string field;
        std::string value;
        modinfo.get_attribute( i, field, value );

        BOOST_CHECK_EQUAL( field, attributes[i].field );
        BOOST_CHECK_EQUAL( value, attributes[i].value );
    }

    for ( auto i = 0; i < sizeof( attributes ) / sizeof( attributes[0] );
          i++ ) {
        std::string field = attributes[i].field;
        std::string value;
        modinfo.get_attribute( field, value );

        BOOST_CHECK_EQUAL( value, attributes[i].value );
    }
}
