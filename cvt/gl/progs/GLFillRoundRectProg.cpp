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

#include <cvt/gl/progs/GLFillRoundRectProg.h>

#include <cvt/gl/shader/120/basictex_120_vert.h>
#include <cvt/gl/shader/120/basicrr_120_frag.h>
#include <cvt/gl/shader/150/basictex_150_vert.h>
#include <cvt/gl/shader/150/basicrr_150_frag.h>

namespace cvt {
	GLFillRoundRectProg::GLFillRoundRectProg() : _vbo( GL_ARRAY_BUFFER ), _tvbo( GL_ARRAY_BUFFER )
	{
		try {
			if( GL::isGLSLVersionSupported( 1, 50 ) ) {
				build( _basictex_150_vert_source, _basicrr_150_frag_source );
			} else {
				build( _basictex_120_vert_source, _basicrr_120_frag_source );
			}
		} catch( GLException e ) {
			std::cout << e.what() << e.log() << std::endl;
		}

		_vbo.alloc( GL_DYNAMIC_DRAW, sizeof( GLint ) * 8 );

//		GLBuffer _tvbo( GL_ARRAY_BUFFER );
		_tvbo.alloc( GL_STATIC_DRAW, sizeof( float ) * 8 );
		float* tv = ( float* ) _tvbo.map( GL_WRITE_ONLY );
		*tv++ = 0.0f; *tv++ = 1.0f;
		*tv++ = 0.0f; *tv++ = 0.0f;
		*tv++ = 1.0f; *tv++ = 1.0f;
		*tv++ = 1.0f; *tv++ = 0.0f;
		_tvbo.unmap();
		_vao.setTexCoordData( _tvbo, 2, GL_FLOAT );

		bind();
		_mvploc = uniformLocation( "MVP" );
		_radiusloc = uniformLocation( "Radius" );
		_sizeloc = uniformLocation( "Size" );
		unbind();
	}

	GLFillRoundRectProg::~GLFillRoundRectProg()
	{

	}

	void GLFillRoundRectProg::setProjection( const Matrix4f& projection )
	{
		glUniformMatrix4fv( _mvploc, 1, true , ( const GLfloat* ) projection.ptr() );
	}

	void GLFillRoundRectProg::setColor( const Color& color )
	{
		_vao.setColor( color );
	}


	void GLFillRoundRectProg::fillRoundRect( int x, int y, int w, int h, float radius )
	{
		GLint* buf;
		buf = ( GLint* ) _vbo.map( GL_WRITE_ONLY );
		*buf++ = x;
		*buf++ = y + h;
		*buf++ = x;
		*buf++ = y;
		*buf++ = x + w;
		*buf++ = y + h;
		*buf++ = x + w;
		*buf++ = y;
		_vbo.unmap();

		glUniform1f( _radiusloc, radius );
		glUniform2f( _sizeloc, ( float ) w, ( float ) h );

		_vao.setVertexData( _vbo, 2, GL_INT );
		_vao.draw( GL_TRIANGLE_STRIP, 0, 4 );
	}

}
