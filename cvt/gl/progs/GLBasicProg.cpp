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

#include <cvt/gl/progs/GLBasicProg.h>
#include <cvt/util/SIMD.h>

#include <cvt/gl/shader/120/basic_120_vert.h>
#include <cvt/gl/shader/120/basic_120_frag.h>
#include <cvt/gl/shader/150/basic_150_vert.h>
#include <cvt/gl/shader/150/basic_150_frag.h>

namespace cvt {
	GLBasicProg::GLBasicProg() : _vbo( GL_ARRAY_BUFFER )
	{
		try {
			if( GL::isGLSLVersionSupported( 1, 50 ) ) {
				build( _basic_150_vert_source, _basic_150_frag_source );
			} else {
				build( _basic_120_vert_source, _basic_120_frag_source );
			}
		} catch( GLException e ) {
			std::cout << e.what() << e.log() << std::endl;
		}

		bind();
		_vbo.alloc( GL_STATIC_DRAW, sizeof( GLint ) * 8 );
		_mvploc = uniformLocation( "MVP" );
		unbind();
	}

	GLBasicProg::~GLBasicProg()
	{

	}

	void GLBasicProg::setProjection( const Matrix4f& projection )
	{
		glUniformMatrix4fv( _mvploc, 1, true , ( const GLfloat* ) projection.ptr() );
	}

	void GLBasicProg::setColor( const Color& color )
	{
		_vao.setColor( color );
	}

	void GLBasicProg::fillRect( int x, int y, int w, int h )
	{
		GLint buf[ 8 ];
		buf[ 0 ] = x;
		buf[ 1 ] = y + h;
		buf[ 2 ] = x;
		buf[ 3 ] = y;
		buf[ 4 ] = x + w;
		buf[ 5 ] = y + h;
		buf[ 6 ] = x + w;
		buf[ 7 ] = y;
		_vbo.setData( sizeof( GLint ) * 8, buf );
		_vao.setVertexData( _vbo, 2, GL_INT );
		_vao.draw( GL_TRIANGLE_STRIP, 0, 4 );
	}

	void GLBasicProg::drawLines( const Vector2f* pts, size_t n, float width, GLenum mode )
	{
		GLBuffer buf( GL_ARRAY_BUFFER );

		buf.alloc( GL_STREAM_DRAW, sizeof( GLfloat ) * 2 * n, pts );

		glLineWidth( width );
		_vao.setVertexData( buf, 2, GL_FLOAT );
		_vao.draw( mode, 0, n );
	}

}
