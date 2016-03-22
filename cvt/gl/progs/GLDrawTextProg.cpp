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

#include <cvt/gl/progs/GLDrawTextProg.h>

#include <cvt/gl/shader/120/text_120_vert.h>
#include <cvt/gl/shader/120/text_120_frag.h>
#include <cvt/gl/shader/150/text_150_vert.h>
#include <cvt/gl/shader/150/text_150_frag.h>

namespace cvt {
	GLDrawTextProg::GLDrawTextProg() : _vbo( GL_ARRAY_BUFFER )
	{
		try {
			if( GL::isGLSLVersionSupported( 1, 50 ) ) {
				build( _text_150_vert_source, _text_150_frag_source );
			} else {
				build( _text_120_vert_source, _text_120_frag_source );
			}
		} catch( GLException e ) {
			std::cout << e.what() << e.log() << std::endl;
		}

		_vbo.alloc( GL_STATIC_DRAW, sizeof( GLint ) * 8 );

		bind();
		_mvploc = uniformLocation( "MVP" );
		_texloc = uniformLocation( "TexFont" );
		_scaleloc = uniformLocation( "Scale" );
		glUniform1i( _texloc, 0 );
		unbind();
	}

	GLDrawTextProg::~GLDrawTextProg()
	{

	}

	void GLDrawTextProg::setProjection( const Matrix4f& projection )
	{
		glUniformMatrix4fv( _mvploc, 1, true , ( const GLfloat* ) projection.ptr() );
	}

	void GLDrawTextProg::setColor( const Color& color )
	{
		_vao.setColor( color );
	}


	void GLDrawTextProg::drawText( int x, int y, const char* text, const GLTexFont& glfont )
	{
		GLint* buf;

		int len = ( int ) strlen( text );

		if( _vbo.size() < sizeof( int ) * 3 * len )
			_vbo.alloc( GL_DYNAMIC_DRAW, sizeof( int ) * 3 * len );

		buf = ( GLint* ) _vbo.map( GL_WRITE_ONLY );
		for( int i = 0; i < len; i++ ) {
			buf[ i * 3 + 0 ] = x;
			buf[ i * 3 + 1 ] = y;
			buf[ i * 3 + 2 ] = text[ i ];
			x += glfont.advance( text[ i ] );
		}
		_vbo.unmap();

		glUniform1f( _scaleloc, ( float ) glfont.spriteSize() / ( float ) ( glfont.texture().width() ) );
		_vao.setVertexData( _vbo, 3, GL_INT );
		glfont.texture().bind();
		glPointSize( glfont.spriteSize() );
		_vao.draw( GL_POINTS, 0, len );
		glfont.texture().unbind();
	}

}
