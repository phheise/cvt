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

#ifndef CVT_GLRBO_H
#define CVT_GLRBO_H

#include <cvt/gl/OpenGL.h>

namespace cvt {
	class GLFBO;

	/**
	  \ingroup GL
	*/
	class GLRBO {
		friend class GLFBO;

		public:
			GLRBO( GLenum format, GLsizei width, GLsizei height );
			~GLRBO();

			GLenum format() const;
			GLsizei width() const;
			GLsizei height() const;
			void size( GLsizei& width, GLsizei& height ) const;

		private:
			GLRBO( const GLRBO& );
			GLRBO& operator=( const GLRBO& );

			GLuint _rbo;
			GLenum _format;
			GLsizei _width, _height;
	};

	inline GLRBO::GLRBO( GLenum format, GLsizei width, GLsizei height ) : _rbo( 0 ), _format( format ), _width( width ), _height( height )
	{
		GL::glGenRenderbuffers( 1, &_rbo );
		GL::glBindRenderbuffer( GL_RENDERBUFFER, _rbo );
		GL::glRenderbufferStorage( GL_RENDERBUFFER, _format, _width, _height );
		GL::glBindRenderbuffer( GL_RENDERBUFFER, 0 );
	}

	inline GLRBO::~GLRBO()
	{
		GL::glDeleteRenderbuffers( 1, &_rbo );
	}

	inline GLenum GLRBO::format() const
	{
		return _format;
	}

	inline GLsizei GLRBO::width() const
	{
		return _width;
	}

	inline GLsizei GLRBO::height() const
	{
		return _height;
	}

	inline void GLRBO::size( GLsizei& width, GLsizei& height ) const
	{
		width = _width;
		height = _height;
	}
}


#endif
