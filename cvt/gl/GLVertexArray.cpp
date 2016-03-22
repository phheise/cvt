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

#include <cvt/gl/GLVertexArray.h>

namespace cvt {


	GLVertexArray::GLVertexArray() : _arrays( 0 ), _color( 0.0f, 0.0f, 0.0f, 1.0f )
	{
		GL::glGenVertexArrays( 1, &_vao );
	}

	GLVertexArray::~GLVertexArray()
	{
		GL::glDeleteVertexArrays( 1, &_vao );
	}

	void GLVertexArray::setAttribData( GLuint index, const GLBuffer& buffer, GLint size, GLenum type, GLsizei stride, const GLvoid* offset )
	{
		CVT_ASSERT( buffer.target() == GL_ARRAY_BUFFER, "Buffer is not an array buffer!" );
		CVT_ASSERT( index < 16, "OpenGL attrib is greater than 16!" );

		GL::glBindVertexArray( _vao );
		_arrays |= ( 1 << index );
		buffer.bind();
		glVertexAttribPointer( index, size, type, GL_FALSE, stride, offset );
		glEnableVertexAttribArray( index );
		buffer.unbind();
		GL::glBindVertexArray( 0 );
	}

/*	void GLVertexArray::setAttribData( GLuint index, float v1 )
	{
		CVT_ASSERT( index < 16, "OpenGL attrib is greater than 16!" );
		_arrays &= ~( 1 << index );
		glVertexAttrib1f( index, v1 );
	}

	void GLVertexArray::setAttribData( GLuint index, float v1, float v2 )
	{
		CVT_ASSERT( index < 16, "OpenGL attrib is greater than 16!" );
		_arrays &= ~( 1 << index );
		glVertexAttrib2f( index, v1, v2 );
	}

	void GLVertexArray::setAttribData( GLuint index, float v1, float v2, float v3 )
	{
		CVT_ASSERT( index < 16, "OpenGL attrib is greater than 16!" );
		_arrays &= ~( 1 << index );
		glVertexAttrib3f( index, v1, v2, v3 );
	}

	void GLVertexArray::setAttribData( GLuint index, float v1, float v2, float v3, float v4 )
	{
		CVT_ASSERT( index < 16, "OpenGL attrib is greater than 16!" );
		_arrays &= ~( 1 << index );
		glVertexAttrib4f( index, v1, v2, v3, v4 );
	}*/

	void GLVertexArray::setColor( const Color& color )
	{
		_color = color;
/*		GL::glBindVertexArray( _vao );
		_arrays &= ~( 1 << GLSL_COLOR_IDX );
		glVertexAttrib4fv( GLSL_COLOR_IDX, color.data() );
		GL::glBindVertexArray( 0 );*/
	}


	void GLVertexArray::resetAttrib( GLuint index )
	{
		GL::glBindVertexArray( _vao );
		_arrays &= ~( 1 << GLSL_VERTEX_IDX );
		glDisableVertexAttribArray( index );
		GL::glBindVertexArray( 0 );
	}

	void GLVertexArray::setVertexData( const GLBuffer& buffer, GLint size, GLenum type, GLsizei stride, const GLvoid* offset )
	{
		CVT_ASSERT( buffer.target() == GL_ARRAY_BUFFER, "Buffer is not an array buffer!" );
		GL::glBindVertexArray( _vao );
		_arrays |= ( 1 << GLSL_VERTEX_IDX );
		buffer.bind();

		glVertexAttribPointer( GLSL_VERTEX_IDX, size, type, GL_FALSE, stride, offset );
		glEnableVertexAttribArray( GLSL_VERTEX_IDX );

		buffer.unbind();
		GL::glBindVertexArray( 0 );
	}

	void GLVertexArray::draw( GLenum mode, GLint first, GLsizei count) const
	{
		if( !_arrays ) return;

		GL::glBindVertexArray( _vao );

		/*for( unsigned int i = 0; i < 16; i++ ) {
			if( _arrays & ( 1 << i ) ) {
				glEnableVertexAttribArray( i );
			}
		}*/

		if( !( _arrays & ( 1 << GLSL_COLOR_IDX ) ) )
			glVertexAttrib4fv( GLSL_COLOR_IDX, _color.data() );

		glDrawArrays( mode, first, count );

		/*for( unsigned int i = 0; i < 16; i++ ) {
			if( _arrays & ( 1 << i ) ) {
				glDisableVertexAttribArray( i );
			}
		}*/

		GL::glBindVertexArray( 0 );
	}

	void GLVertexArray::drawIndirect( const GLBuffer& elembuf, GLenum type, GLenum mode, GLsizei count ) const
	{
		CVT_ASSERT( elembuf.target() == GL_ELEMENT_ARRAY_BUFFER, "Buffer is not an element array buffer!" );
		if( !_arrays ) return;

		GL::glBindVertexArray( _vao );

/*		for( unsigned int i = 0; i < 16; i++ ) {
			if( _arrays & ( 1 << i ) ) {
				glEnableVertexAttribArray( i );
			}
		}*/

		if( !( _arrays & ( 1 << GLSL_COLOR_IDX ) ) )
			glVertexAttrib4fv( GLSL_COLOR_IDX, _color.data() );

		elembuf.bind();
		glDrawElements( mode, count, type, 0 );
		elembuf.unbind();

/*		for( unsigned int i = 0; i < 16; i++ ) {
			if( _arrays & ( 1 << i ) ) {
				glDisableVertexAttribArray( i );
			}
		}*/

		GL::glBindVertexArray( 0 );
	}

}
