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

#include <cvt/gl/progs/GLDrawIconProg.h>
#include <cvt/gfx/Image.h>
#include <cvt/io/Resources.h>


#include <cvt/gl/shader/120/icon_120_vert.h>
#include <cvt/gl/shader/120/icon_120_frag.h>
#include <cvt/gl/shader/150/icon_150_vert.h>
#include <cvt/gl/shader/150/icon_150_frag.h>

namespace cvt {
	GLDrawIconProg::GLDrawIconProg() : _vbo( GL_ARRAY_BUFFER )
	{
		try {
			if( GL::isGLSLVersionSupported( 1, 50 ) ) {
				build( _icon_150_vert_source, _icon_150_frag_source );
			} else {
				build( _icon_120_vert_source, _icon_120_frag_source );
			}
		} catch( GLException e ) {
			std::cout << e.what() << e.log() << std::endl;
		}

		_vbo.alloc( GL_STATIC_DRAW, sizeof( GLint ) * 3 );
		_mvploc = uniformLocation( "MVP" );
		_texloc = uniformLocation( "TexFont" );
		_scaleloc = uniformLocation( "Scale" );

		
		size_t stride;
		cvt::Resources resources;
		Image icons( resources.find( "Icons/Icons.png" ) );

		uint8_t* ptr = icons.map( &stride );
		_tex.alloc( GL_RED, 128, 128, GL_RED, GL_UNSIGNED_BYTE, ptr );
		icons.unmap( ptr );
		bind();
		glUniform1i( _texloc, 0 );
		glUniform1f( _scaleloc, 16.0f / 128.0f );
		unbind();
	}

	GLDrawIconProg::~GLDrawIconProg()
	{

	}

	void GLDrawIconProg::setProjection( const Matrix4f& projection )
	{
		glUniformMatrix4fv( _mvploc, 1, true , ( const GLfloat* ) projection.ptr() );
	}

	void GLDrawIconProg::setColor( const Color& color )
	{
		_vao.setColor( color );
	}

	void GLDrawIconProg::drawIcon( int x, int y, int icon )
	{
		GLint* buf;

		buf = ( GLint* ) _vbo.map( GL_WRITE_ONLY );
		buf[ 0 ] = x + 8;
		buf[ 1 ] = y + 8;
		buf[ 2 ] = icon;
		_vbo.unmap();

		_vao.setVertexData( _vbo, 3, GL_INT );
		_tex.bind();
		glPointSize( 16.0f );
		_vao.draw( GL_POINTS, 0, 1 );
		_tex.unbind();
	}

	void GLDrawIconProg::drawIcons( const Vector2i* pts, size_t npts, int icon )
	{
		GLint* buf;

		_vbo.alloc( GL_STATIC_DRAW, sizeof( GLint ) * 3 * npts );
		buf = ( GLint* ) _vbo.map( GL_WRITE_ONLY );
		for( size_t i = 0; i < npts; i++ ) {
			buf[ i * 3 + 0 ] = pts[ i ].x;
			buf[ i * 3 + 1 ] = pts[ i ].y;
			buf[ i * 3 + 2 ] = icon;
		}
		_vbo.unmap();

		_vao.setVertexData( _vbo, 3, GL_INT );
		_tex.bind();
		glPointSize( 16.0f );
		_vao.draw( GL_POINTS, 0, npts );
		_tex.unbind();
		_vbo.alloc( GL_STATIC_DRAW, sizeof( GLint ) * 3 );
	}

	void GLDrawIconProg::drawIcons( const Vector2f* pts, size_t npts, int icon )
	{
		GLfloat* buf;

		_vbo.alloc( GL_STATIC_DRAW, sizeof( GLfloat ) * 3 * npts );
		buf = ( GLfloat* ) _vbo.map( GL_WRITE_ONLY );
		for( size_t i = 0; i < npts; i++ ) {
			buf[ i * 3 + 0 ] = pts[ i ].x;
			buf[ i * 3 + 1 ] = pts[ i ].y;
			buf[ i * 3 + 2 ] = icon;
		}
		_vbo.unmap();

		_vao.setVertexData( _vbo, 3, GL_FLOAT );
		_tex.bind();
		glPointSize( 16.0f );
		_vao.draw( GL_POINTS, 0, npts );
		_tex.unbind();
		_vbo.alloc( GL_STATIC_DRAW, sizeof( GL_FLOAT ) * 3 );
	}
}
