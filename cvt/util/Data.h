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

#ifndef CVT_DATA_H
#define CVT_DATA_H

#include <stdint.h>
#include <stdlib.h>

namespace cvt {
	class Data {
		public:
			Data( size_t size = 0 );
			~Data();
			Data( const uint8_t* ptr, size_t size );
			Data( uint8_t* ptr, size_t size, bool copyData );
			Data( const Data& data );
			Data& operator=( const Data& data );

			size_t size() const;
			const uint8_t* ptr() const;
			uint8_t* ptr();

			void assign( const uint8_t* data, size_t size );

			void allocate( size_t size );
			void reallocate( size_t size );

		private:

			uint8_t* _data;
			size_t	 _size;
			bool     _dealloc;
	};

	inline Data::Data( size_t size ) : _data( NULL ), _dealloc( false )
	{
		allocate( size );
	}

	inline Data::~Data()
	{
		if( _dealloc && _data )
		   delete[]	_data;
	}

	inline Data::Data( const uint8_t* ptr, size_t size ) : _data( NULL ), _dealloc( false )
	{
		allocate( size );
		assign( ptr, size );
	}

	inline Data::Data( uint8_t* ptr, size_t size, bool copyData ) : _data( NULL ), _dealloc( copyData )
	{
		if( copyData ){
			allocate( size );
			assign( ptr, size );
		} else {
			_data = ptr;
			_size = size;
		}
	}

	inline Data::Data( const Data& data ) : _data( NULL ), _dealloc( false )
	{
		if( &data == this )
			return;
		allocate( data._size );
		assign( data._data, data._size );
		_dealloc = true;
	}

	inline Data& Data::operator=( const Data& data )
	{
		allocate( data._size );
		assign( data._data, data._size );
		return *this;
	}

	inline size_t Data::size() const
	{
		return _size;
	}

	inline uint8_t* Data::ptr()
	{
		return _data;
	}

	inline const uint8_t* Data::ptr() const
	{
		return _data;
	}


}

#endif
