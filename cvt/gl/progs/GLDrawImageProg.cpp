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

#include <cvt/gl/progs/GLDrawImageProg.h>

#include <cvt/gfx/ImageAllocatorGL.h>

#include <cvt/gl/shader/120/basictex_120_vert.h>
#include <cvt/gl/shader/120/basictex_120_frag.h>
#include <cvt/gl/shader/150/basictex_150_vert.h>
#include <cvt/gl/shader/150/basictex_150_frag.h>

namespace cvt {
	GLDrawImageProg::GLDrawImageProg() : _vbo( GL_ARRAY_BUFFER )
	{
		try {
			if( GL::isGLSLVersionSupported( 1, 50 ) ) {
				build( _basictex_150_vert_source, _basictex_150_frag_source );
			} else {
				build( _basictex_120_vert_source, _basictex_120_frag_source );
			}
		} catch( GLException e ) {
			std::cout << e.what() << e.log() << std::endl;
		}

		_vbo.alloc( GL_STATIC_DRAW, sizeof( GLfloat ) * 8 );

		GLfloat* tv = ( GLfloat* ) _vbo.map( GL_WRITE_ONLY );
		*tv++ = 0.0f; *tv++ = 1.0f;
		*tv++ = 0.0f; *tv++ = 0.0f;
		*tv++ = 1.0f; *tv++ = 1.0f;
		*tv++ = 1.0f; *tv++ = 0.0f;
		_vbo.unmap();
		_vao.setTexCoordData( _vbo, 2, GL_FLOAT );
		_vao.setVertexData( _vbo, 2, GL_FLOAT );

		bind();
		_mvploc = uniformLocation( "MVP" );
		_texloc = uniformLocation( "Tex" );
		glUniform1i( _texloc, 0 );
		unbind();
	}

	GLDrawImageProg::~GLDrawImageProg()
	{

	}

	void GLDrawImageProg::setProjection( const Matrix4f& projection )
	{
		_proj = projection;
	}

	void GLDrawImageProg::setAlpha( float alpha )
	{
		Color c( 1.0f, 1.0f, 1.0f, alpha );
		_vao.setColor( c );
	}

	void GLDrawImageProg::drawImage( int x, int y, const Image& img )
	{
		float w, h;

		if( img.memType() != IALLOCATOR_GL )
			return;

		w = ( float ) img.width();
		h = ( float ) img.height();

		Matrix4f proj = _proj;
		Matrix4f mdl( w, 0, 0, x,
					 0, h, 0, y,
					 0, 0, 1, 0,
					 0, 0, 0, 1 );
		proj *= mdl;
		glUniformMatrix4fv( _mvploc, 1, true , ( const GLfloat* ) proj.ptr() );

		glBindTexture( GL_TEXTURE_2D, ( ( ImageAllocatorGL* ) img._mem  )->_tex2d );
		_vao.draw( GL_TRIANGLE_STRIP, 0, 4 );
		glBindTexture( GL_TEXTURE_2D, 0 );
	}

	void GLDrawImageProg::drawImage( int x, int y, int w, int h, const Image& img )
	{
		if( img.memType() != IALLOCATOR_GL )
			return;

		Matrix4f proj = _proj;
		Matrix4f mdl( ( float ) w, 0, 0, ( float ) x,
				      0, ( float )h, 0, ( float )y,
					  0, 0, 1, 0,
					  0, 0, 0, 1 );
		proj *= mdl;
		glUniformMatrix4fv( _mvploc, 1, true , ( const GLfloat* ) proj.ptr() );

		glBindTexture( GL_TEXTURE_2D, ( ( ImageAllocatorGL* ) img._mem  )->_tex2d );
		_vao.draw( GL_TRIANGLE_STRIP, 0, 4 );
		glBindTexture( GL_TEXTURE_2D, 0 );
	}


	void GLDrawImageProg::drawImage( int x, int y, int width, int height, const GLTexture& tex )
	{
		GLint w, h;

		w = Math::max( width, 1 );
		h = Math::max( height, 1 );

		Matrix4f proj = _proj;
		Matrix4f mdl( ( float ) w, 0, 0, ( float ) x,
					 0, ( float )h, 0, ( float )y,
					 0, 0, 1, 0,
					 0, 0, 0, 1 );
		proj *= mdl;
		glUniformMatrix4fv( _mvploc, 1, true , ( const GLfloat* ) proj.ptr() );
		tex.bind();
		_vao.draw( GL_TRIANGLE_STRIP, 0, 4 );
		tex.unbind();
	}
}
