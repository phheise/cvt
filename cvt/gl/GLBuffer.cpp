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

#include <cvt/gl/GLBuffer.h>
#include <cvt/util/SIMD.h>

namespace cvt {

#ifdef __APPLE__
#define glFlushMappedBufferRange glFlushMappedBufferRangeAPPLE
#endif

	GLBuffer::GLBuffer( GLenum target ) : _buffer( 0 ), _target( target ), _size( 0 ), _access( 0 )
	{
		glGenBuffers( 1, &_buffer );
	}

	GLBuffer::~GLBuffer()
	{
		glDeleteBuffers( 1, &_buffer );
	}

	void GLBuffer::bind() const
	{
		glBindBuffer( _target, _buffer );
	}

	void GLBuffer::unbind() const
	{
		glBindBuffer( _target, 0 );
	}

	void GLBuffer::alloc( GLenum usage, size_t size, const void* data )
	{
		glBindBuffer( _target, _buffer );
		_size = size;
		glBufferData( _target, ( GLsizeiptr ) size, data, usage );
		glBindBuffer( _target, 0 );
	}

	/*
	void GLBuffer::alloc( GLenum usage, size_t size, const GLBuffer & data )
	{
		if( this == &data )
			throw CVTException( "Target and Source Buffer are the same" );

		glBindBuffer( GL_COPY_WRITE_BUFFER, _buffer );
		_size = size;
		glBufferData( GL_COPY_WRITE_BUFFER, ( GLsizeiptr ) size, NULL, usage );
		
		glBindBuffer( GL_COPY_READ_BUFFER, data._buffer );
		glCopyBufferSubData( GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, Math::min( _size, data._size ) );

		glBindBuffer( GL_COPY_WRITE_BUFFER, 0 );
		glBindBuffer( GL_COPY_READ_BUFFER, 0 );
	}
	*/

	void GLBuffer::setData( size_t size, const void* data, size_t offset )
	{
		glBindBuffer( _target, _buffer );
		if( ( offset + size ) > _size )
			throw CVTException( "Provided size exceeds buffer size!" );
		glBufferSubData( _target, offset, size, data );
		glBindBuffer( _target, 0 );
	}

	void* GLBuffer::map( GLenum access ) const
	{
		void* ptr;

		glBindBuffer( _target, _buffer );
		ptr = glMapBuffer( _target, access );
		glBindBuffer( _target, 0 );
		return ptr;
	}

/*	void GLBuffer::sync( size_t offset, size_t length ) const
	{
		if( ! ( _access & GL_MAP_WRITE_BIT ) )
			return;
		glBindBuffer( _target, _buffer );
		glFlushMappedBufferRange( _target, ( GLintptr ) offset, ( GLsizeiptr ) length );
		glBindBuffer( _target, 0 );
	}*/

	void GLBuffer::unmap() const
	{
		glBindBuffer( _target, _buffer );
		glUnmapBuffer( _target );
		glBindBuffer( _target, 0 );
	}
}
