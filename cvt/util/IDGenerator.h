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

#ifndef CVT_IDGENERATOR_H
#define CVT_IDGENERATOR_H

#include <stdlib.h>
#include <stdint.h>

#include <cvt/util/Exception.h>

#include <iostream>

namespace cvt {

	class IDGenerator {
		public:
			IDGenerator();
			~IDGenerator();
			uint32_t allocateID();
			void releaseID( uint32_t id );

		private:
			void allocBucket( size_t index );
			void idToBucket( uint32_t id, uint32_t& index, uint32_t& offset );

			uint32_t* buckets[ 8 ];
			uint32_t nid;
	};

	/*never use 0 as id */
	inline IDGenerator::IDGenerator() : nid( 1 ) {
		for( int i = 0; i < 8; i++ )
			buckets[ i ] = NULL;
	}

	inline IDGenerator::~IDGenerator()
	{
		for( int i = 0; i < 8 && buckets[ i ]; i++ )
			delete[] buckets[ i ];
	}

	inline uint32_t IDGenerator::allocateID()
	{
		uint32_t ret;
		uint32_t idx, off;

		ret = nid;
		idToBucket( nid, idx, off );

		if( idx > 7 )
			throw CVTException( " Out of unique IDs" );

		if( !buckets[ idx ] )
			allocBucket( idx );

		nid = buckets[ idx ][ off ];
		return ret;
	}

	inline void IDGenerator::releaseID( uint32_t id )
	{
		uint32_t idx, off;

		idToBucket( id, idx, off );
		buckets[ idx ][ off ] = nid;
		nid = id;
	}

	inline void IDGenerator::idToBucket( uint32_t id, uint32_t& index, uint32_t& offset )
	{
		uint32_t tid = id;

		index = 0;
		while( tid >= ( ( ( uint32_t ) 8 ) << ( 3 * index ) ) ) {
			tid -= ( ( ( uint32_t ) 8 ) << ( 3 * index ) );
			index++;
		}
		offset = tid;
	}

	inline void IDGenerator::allocBucket( size_t index )
	{
		uint32_t* ptr;
		uint32_t val;
		uint32_t size = 8 << ( 3 * index );

		if( buckets[ index ] )
			return;

		buckets[ index ] = new uint32_t[ size ];

		ptr = buckets[ index ];
		val = 0;
		for( uint32_t i = 0; i <= index; i++ )
			val +=  ( 1 << ( 3 * i ) );

		for( uint32_t i = 0; i < size; i++ ) {
			*ptr++ = ( i + val );
		}
	}


}

#endif
