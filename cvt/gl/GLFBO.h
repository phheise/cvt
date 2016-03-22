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

#ifndef CVT_GLFBO_H
#define CVT_GLFBO_H

#include <cvt/gl/OpenGL.h>
#include <cvt/gl/GLTexture.h>
#include <cvt/gl/GLRBO.h>
#include <cvt/gfx/Drawable.h>

namespace cvt {

	/**
	  \ingroup GL
	*/
	class GLFBO : public Drawable {
		public:
			GLFBO( GLsizei width, GLsizei height );
			~GLFBO();

			void bind();
			void unbind();

			GLsizei width() const;
			GLsizei height() const;
			void size( GLsizei& width, GLsizei& height ) const;

			void attach( GLenum target, GLRBO& rbo );
			void attach( GLenum target, GLTexture& tex, GLint level = 0 );

			GFXEngine* gfxEngine();

		private:
			GLFBO( const GLFBO& );
			GLFBO& operator=( const GLFBO& );

			GLuint _fbo;
			GLsizei _width;
			GLsizei _height;
			GLint _viewport[ 4 ];
	};

	inline GLFBO::GLFBO( GLsizei width, GLsizei height ) : _fbo( 0 ), _width( width ), _height( height )
	{
		GL::glGenFramebuffers( 1, &_fbo );
	}

	inline GLFBO::~GLFBO()
	{
		GL::glDeleteFramebuffers( 1, &_fbo );
	}

	inline GLsizei GLFBO::width() const
	{
		return _width;
	}

	inline GLsizei GLFBO::height() const
	{
		return _height;
	}

	inline void GLFBO::size( GLsizei& width, GLsizei& height ) const
	{
		width = _width;
		height = _height;
	}

	inline void GLFBO::attach( GLenum target, GLRBO& rbo )
	{
		// attach the renderbuffer object
		GL::glFramebufferRenderbuffer( GL_FRAMEBUFFER, target, GL_RENDERBUFFER, rbo._rbo );
	}


	inline void GLFBO::attach( GLenum target, GLTexture& tex, GLint level )
	{
		// attach texture object
		GL::glFramebufferTexture2D( GL_FRAMEBUFFER, target, tex.target(), tex._tex, level );
	}

	inline void GLFBO::bind()
	{
		// save current viewport
		glGetIntegerv( GL_VIEWPORT, _viewport );
		GL::glBindFramebuffer( GL_FRAMEBUFFER, _fbo );
		glViewport( 0, 0, _width, _height );
	}

	inline void GLFBO::unbind()
	{
		GL::glBindFramebuffer( GL_FRAMEBUFFER, 0 );
		// restore old viewport
		glViewport( _viewport[ 0 ], _viewport[ 1 ], _viewport[ 2 ], _viewport[ 3 ] );
	}


}
#endif
