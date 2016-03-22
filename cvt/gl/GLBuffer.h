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

#ifndef CVTGLBUFFER_H
#define CVTGLBUFFER_H

#include <cvt/gl/OpenGL.h>

namespace cvt {

	/**
	  \ingroup GL
	*/
	class GLBuffer {
		public:
			GLBuffer( GLenum target = GL_ARRAY_BUFFER );
			~GLBuffer();
			void alloc( GLenum usage, size_t size, const void *data = NULL );
			// void alloc( GLenum usage, size_t size, const GLBuffer & data );
			void bind() const;
			void unbind() const;
			size_t size() const;
			GLenum target() const;
			void setData( size_t size, const void* data, size_t offset = 0 );

			void* map( GLenum access = GL_READ_WRITE ) const;
			void unmap() const;

			GLuint glObject() const;
			/*
			void sync( size_t offset, size_t length ) const;
			 */

		private:
			GLuint _buffer;
			GLenum _target;
			size_t _size;
			mutable GLbitfield _access;
	};



	inline size_t GLBuffer::size() const
	{
		return _size;
	}

	inline GLenum GLBuffer::target() const
	{
		return _target;
	}

	inline GLuint GLBuffer::glObject() const
	{
		return _buffer;
	}
}

#endif
