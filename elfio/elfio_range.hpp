/*
Copyright (C) 2001-2020 by Serge Lamikhov-Center

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

#ifndef ELFIO_RANGE_HPP
#define ELFIO_RANGE_HPP

namespace ELFIO {

class range
{
    class single_range
    {
      private:
        uint64_t start;
        uint64_t end;

      public:
        single_range( uint64_t start, uint64_t end )
            : start( start ), end( end )
        {
        }

        bool operator<( const single_range& other ) const
        {
            return start < other.start ||
                   ( start == other.start && end < other.end );
        }

        bool operator==( const single_range& other ) const
        {
            return start == other.start && end == other.end;
        }

        uint64_t get_last() const { return end - 1; }
        uint64_t get_start() const { return start; }
        uint64_t get_length() const { return end - start; }
        uint64_t get_end() const { return end; }
    };

    std::vector<single_range> ranges;

  public:
    range() {}

    range( uint64_t start, uint64_t end )
    {
        ranges.push_back( single_range{ start, end } );
    }

    void subtract( const single_range& s )
    {
        for ( auto i = ranges.begin(); i != ranges.end(); ) {
            i = subtract( s, i );
        }
    }

    int element_count() const { return ranges.size(); }

    uint64_t get_start() const { return ranges[0].get_start(); }

    uint64_t get_length() const { return ranges[0].get_length(); }

    range operator[]( int i ) const
    {
        return range( ranges.at( i ).get_start(), ranges.at( i ).get_end() );
    };

  private:
    std::vector<single_range>::iterator
    subtract( const single_range& s, std::vector<single_range>::iterator base )
    {
        if ( s.get_length() <= 0 ) // nothing to remove
            return base + 1;

        if ( s.get_start() <= base->get_start() ) {
            if ( s.get_end() <= base->get_start() ) {
                // nothing changes
                return base + 1;
            }
            else if ( s.get_end() < base->get_end() ) {
                // cut from beginning
                *base = { s.get_end(), base->get_end() };
                return base + 1;
            }
            else {
                // nothing left, remove element
                return ranges.erase( base );
            }
        }
        else if ( s.get_start() < base->get_end() ) {
            if ( s.get_end() < base->get_end() ) {
                // cut out in the middle
                auto old_end = base->get_end();
                *base        = { base->get_start(), s.get_start() };
                return ranges.insert( base + 1,
                                      single_range( s.get_end(), old_end ) ) +
                       1;
            }
            else {
                // cut end
                *base = { base->get_start(), s.get_start() };
                return base + 1;
            }
        }
        else {
            // nothing changes, s starts after base ends
            return base + 1;
        }
    }

    void splice()
    {
        std::sort( ranges.begin(), ranges.end() );
        for ( auto i = ranges.begin(); i != ranges.end(); ) {
            auto next = i + 1;
            if ( next != ranges.end() && i->get_end() == next->get_start() ) {
                *i = { i->get_start(), next->get_end() };
                i  = ranges.erase( next );
            }
            else {
                i = next;
            }
        }
    }

  public:
    bool operator==( const range& other ) const
    {
        return ranges == other.ranges;
    }

    friend std::ostream& operator<<( std::ostream& stream, const range& range );
};

} // namespace ELFIO

#endif