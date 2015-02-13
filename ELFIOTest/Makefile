CXX=g++
CPPFLAGS=-c -Wall -std=c++0x -I..
LDLIBS=-lboost_unit_test_framework

ELFIODIR=../elfio/
SOURCES=ELFIOTest.cpp ELFIOTest1.cpp
INCLUDES=$(ELFIODIR)elfio.hpp         $(ELFIODIR)elfio_header.hpp  \
         $(ELFIODIR)elfio_note.hpp    $(ELFIODIR)elfio_section.hpp \
         $(ELFIODIR)elfio_segment.hpp $(ELFIODIR)elfio_strings.hpp \
         $(ELFIODIR)elfio_symbols.hpp $(ELFIODIR)elfio_utils.hpp   \
         $(ELFIODIR)elf_types.hpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=ELFIOTest

all: $(SOURCES) $(EXECUTABLE) $(INCLUDES)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@

.cpp.o:
	$(CXX) $(CPPFLAGS) $< -o $@

test: $(EXECUTABLE)
	./$(EXECUTABLE) -r short

clean:
	rm $(OBJECTS) $(EXECUTABLE)
