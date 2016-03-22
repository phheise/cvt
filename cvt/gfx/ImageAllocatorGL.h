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

#ifndef IMAGEALLOCATORGL_H
#define IMAGEALLOCATORGL_H
#include <cvt/gfx/ImageAllocator.h>
#include <cvt/gl/OpenGL.h>

namespace cvt {
	class ImageAllocatorGL : public ImageAllocator {
		friend class GLDrawImageProg;
		friend class GLTexMLSProg;
		public:
			ImageAllocatorGL();
			~ImageAllocatorGL();
			virtual void alloc( size_t width, size_t height, const IFormat & format );
			virtual void copy( const ImageAllocator* x, const Recti* r );
			virtual uint8_t* map( size_t* stride );
			virtual const uint8_t* map( size_t* stride ) const;
			virtual void unmap( const uint8_t* ptr ) const;
			virtual IAllocatorType type() const { return IALLOCATOR_GL; };

		private:
			ImageAllocatorGL( const ImageAllocatorGL& );
			void getGLFormat( const IFormat & format, GLenum& glformat, GLenum& gltype ) const;

		private:
			GLuint _tex2d;
			GLuint _glbuf;
			size_t _stride;
			size_t _size;
			/* PBO can only be mapped once, we need refcounting in our const methods and
			   need to check if the buffer is dirty ( only in case of non-const map )  */
			mutable void* _ptr;
			mutable int _ptrcount;
			mutable bool _dirty;
	};
}

#endif
