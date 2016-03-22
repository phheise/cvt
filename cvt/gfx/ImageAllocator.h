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

#ifndef IMAGEALLOCATOR_H
#define IMAGEALLOCATOR_H

#include <cvt/gfx/IFormat.h>
#include <cvt/geom/Rect.h>

namespace cvt {

	enum IAllocatorType {
		IALLOCATOR_MEM = ( 0 ),
		IALLOCATOR_CL = ( 1 << 0 ),
		IALLOCATOR_GL = ( 1 << 1 )
	};

	class ImageAllocator {
		friend class Image;
		friend class ImageAllocatorMem;
		friend class ImageAllocatorCL;
		friend class ImageAllocatorGL;

		public:
			virtual ~ImageAllocator() {}
			virtual void alloc( size_t width, size_t height, const IFormat & format ) = 0;
			virtual void copy( const ImageAllocator* x, const Recti* r = NULL ) = 0;
			virtual uint8_t* map( size_t* stride ) = 0;
			virtual const uint8_t* map( size_t* stride ) const = 0;
			virtual void unmap( const uint8_t* ptr ) const = 0;
			virtual IAllocatorType type() const = 0;

		protected:
			ImageAllocator() : _width( 0 ), _height( 0 ), _format( IFormat::RGBA_UINT8 ) {}
			ImageAllocator( const ImageAllocator& );

		protected:
			size_t _width;
			size_t _height;
			IFormat _format;
	};
}

#endif
