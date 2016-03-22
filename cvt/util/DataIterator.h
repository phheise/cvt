/*
   The MIT License (MIT)

   Copyright (c) 2011 - 2013, Philipp Heise and Sebastian Klose

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

/*
 * File:   DataIterator.h
 * Author: Sebastian Klose, Philipp Heise
 *
 * Created on June 27, 2011
 */

#ifndef CVT_DATAITERATOR_H
#define	CVT_DATAITERATOR_H

#include <cvt/util/Data.h>
#include <cvt/util/String.h>
#include <vector>

namespace cvt {

    class DataIterator {
      public:

        DataIterator( const Data & d );

        DataIterator( const String & s );

        bool hasNext( );

        bool nextToken( String & token, const String & deliminators );

        bool peekNextToken( String & token, const String & deliminators );

        bool nextLine( String & line );

        void tokenizeNextLine( std::vector<String> & tokens, const String & deliminators );

		const DataIterator& operator+=( size_t n );
		const DataIterator& operator-=( size_t n );

        const uint8_t * pos() const { return _pos; }
        const uint8_t * end() const { return _end; }

        size_t remainingBytes() const { return _end - _pos; }

        long nextLong( int base = 0 );
        double nextDouble( );

        void skip( const String& characters );
		void skipInverse( const String& characters );

		void skip( size_t size );

      private:
        const uint8_t*	_pos;
        const uint8_t*	_end;

        void nextDelim( const String & delim );

        bool isDeliminator( uint8_t c, const String & delims );
    } ;

    inline DataIterator::DataIterator( const Data& d ) :
    	_pos( d.ptr() ), _end( _pos + d.size() )
	{
	}

    inline DataIterator::DataIterator( const String & s ) :
        _pos( ( const uint8_t* )s.c_str() ), _end( _pos + s.length() )
    {
    }

    inline bool DataIterator::hasNext( )
    {
        return _pos != _end;
    }

    inline bool DataIterator::nextToken( String & token, const String & deliminators )
    {
        skip( deliminators );

        if ( !hasNext() )
			return false;

		// here we know that _pos points to a token
		const uint8_t * start = _pos;
		nextDelim( deliminators );

		token.assign( ( const char* )start, _pos - start );
		return true;
    }

    inline bool DataIterator::peekNextToken( String & token, const String & deliminators )
	{
		// FIXME
		const uint8_t* cur = _pos;
		bool ret = nextToken( token, deliminators );
		_pos = cur;
		return ret;
	}

    inline bool DataIterator::nextLine( String & line )
    {
        if( !hasNext() )
            return false;
        const uint8_t * start = _pos;

        while( *_pos != '\n' ){
            if( !hasNext() )
                return false;
            _pos++;
        }

        if( _pos == start ){
            // empty line
            _pos++;
            return false;
        }

        // create the line
        line.assign( ( char* )start, _pos - start );
        _pos++;
        return true;
    }

	inline const DataIterator& DataIterator::operator+=( size_t n )
	{
		size_t num = Math::max( n, remainingBytes() );
		_pos += num;
		return *this;
	}

	inline const DataIterator& DataIterator::operator-=( size_t n )
	{
		// FIXME: also keep _start, like end
		_pos -= n;
		return *this;
	}

    inline void DataIterator::tokenizeNextLine( std::vector<String> & tokens, const String & deliminators )
    {
        String line;
        if( !nextLine( line ) )
            return;

        size_t start = 0;
        size_t end = line.length();
        size_t curr = 0;

        while( curr != end){
            if( isDeliminator( line[ curr ], deliminators ) ){
                curr++;
            } else {
                // curr is a non token!
                start = curr;
                while( curr != end && !isDeliminator( line[ curr ], deliminators ) )
                    curr++;
                tokens.push_back( String( &line.c_str()[ start ], curr - start ) );
            }
        }
    }

    inline long DataIterator::nextLong( int base )
    {
        if( !hasNext() ){
            throw CVTException( "No more data available" );
        }

        uint8_t * end;
        long v = strtol( ( const char* )_pos, ( char** )&end, base );

		//FIXME: this is _wrong_ check for _pos  == end -> conversion error, we are maybe not NULL terminated ...
        if( *end != '\0' )
            _pos = end;
        else
            _pos = _end;

        return v;
    }

    inline double DataIterator::nextDouble( )
    {
        if( !hasNext() ){
            throw CVTException( "No more data available" );
        }

        uint8_t * end;
        double v = strtod( ( const char* )_pos, ( char** )&end );


		//FIXME: check for _pos == end -> conversion error

        _pos = end;

        return v;
    }

    inline bool DataIterator::isDeliminator( uint8_t c, const String & delims )
    {
        for ( size_t i = 0; i < delims.length( ); i++ ) {
            if ( delims[ i ] == c )
                return true;
        }
        return false;
    }

    inline void DataIterator::skip( const String & delim )
    {
        while ( hasNext() ) {
            if ( isDeliminator( *_pos, delim ) )
                _pos++;
            else
                return;
        }
    }

	inline void DataIterator::skip( size_t size )
	{
		_pos += size;
		if( _pos > _end )
			_pos = _end;
	}

    inline void DataIterator::skipInverse( const String & delim )
    {
        while ( hasNext() ) {
            if ( !isDeliminator( *_pos, delim ) )
                _pos++;
            else
                return;
        }
    }

    inline void DataIterator::nextDelim( const String & delim )
    {
        while ( hasNext() ) {
            if ( isDeliminator( *_pos, delim )  ) {
                return;
            }
            _pos++;
        }
    }

}


#endif	/* DATAITERATOR_H */

