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

#include <cvt/gfx/ImageAllocatorMem.h>
#include <cvt/util/SIMD.h>
#include <cvt/util/Util.h>

namespace cvt {

	ImageAllocatorMem::ImageAllocatorMem() : ImageAllocator(), _data( 0 ), _mem( 0 ), _refcnt( 0 )
	{
	}

	ImageAllocatorMem::~ImageAllocatorMem()
	{
		release();
	}

	void ImageAllocatorMem::alloc( size_t width, size_t height, const IFormat & format, uint8_t* data, size_t stride )
	{
		release();
		_width = width;
		_height = height;
		_format = format;

		if( stride == 0 ){
			_stride = _width * _format.bpp;
		} else {
			_stride = stride;
		}

		_mem = NULL;
		_data = data;
		_refcnt = new size_t;
		*_refcnt = 0;
		retain();
	}

	void ImageAllocatorMem::alloc( size_t width, size_t height, const IFormat & format )
	{
		if( _width == width && _height == height && _format == format )
			return;

		release();
		_width = width;
		_height = height;
		_format = format;
		_stride = Math::pad16( _width * _format.bpp );
		_mem = new uint8_t[ _stride * _height + 16 ];
		_data = Util::alignPtr( _mem, 16 );
		_refcnt = new size_t;
		*_refcnt = 0;
		retain();
	}

	void ImageAllocatorMem::copy( const ImageAllocator* x, const Recti* r = NULL )
	{
		const uint8_t* src;
		const uint8_t* osrc;
		uint8_t* dst;
		size_t sstride;
		size_t i, n;
		Recti rect( 0, 0, ( int ) x->_width, ( int ) x->_height );
		SIMD* simd = SIMD::instance();

		if( r )
			rect.intersect( *r );

		alloc( rect.width, rect.height, x->_format );

		osrc = src = x->map( &sstride );
		src += rect.y * sstride + x->_format.bpp * rect.x;
		dst = _data;
		n =  _format.bpp * rect.width;

		i = rect.height;
		while( i-- ) {
			simd->Memcpy( dst, src, n );
			dst += _stride;
			src += sstride;
		}
		x->unmap( osrc );
	}

	void ImageAllocatorMem::release()
	{
		if( _refcnt ) {
			*_refcnt -= 1;
			if( *_refcnt <= 0 ) {
				if( _mem )
					delete[] _mem;
				delete _refcnt;
			}
		}
	}

	void ImageAllocatorMem::retain()
	{
		if( _refcnt ) {
			*_refcnt += 1;
		}
	}

}
