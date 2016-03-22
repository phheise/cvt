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

#include <cvt/util/Data.h>
#include <cvt/util/SIMD.h>
#include <cvt/math/Math.h>

namespace cvt {

	void Data::allocate( size_t size )
	{
		if( _data && _dealloc )
			delete[] _data;
		_size = size;
		_dealloc = true;

		if( size )
			_data = new uint8_t[ _size ];
		else
			_data = NULL;
	}

	void Data::reallocate( size_t size )
	{
		uint8_t* newdata = new uint8_t[ size ];

		if( _data ) {
			SIMD::instance()->Memcpy( newdata, _data, Math::min( _size, size ) );
			if( _dealloc )
				delete[] _data;
		}
		_data = newdata;
		_size = size;
	}


	void Data::assign( const uint8_t* data, size_t size )
	{
		SIMD::instance()->Memcpy( _data, data , Math::min( size, _size ) );
	}
}
