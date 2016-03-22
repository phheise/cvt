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

#ifndef CVTGLTEXTURE_H
#define CVTGLTEXTURE_H

#include <cvt/gl/OpenGL.h>
#include <cvt/gfx/Image.h>

namespace cvt {
	class GLFBO;

	/**
	  \ingroup GL
	*/
	class GLTexture {
		friend class GLFBO;

		public:
			GLTexture( GLenum target = GL_TEXTURE_2D );
			GLTexture( const Image& img );
			~GLTexture();
			void bind() const;
			void unbind() const;
			GLenum target() const;
			void size( GLsizei& width, GLsizei& height ) const;
			GLsizei width() const;
			GLsizei height() const;
			void alloc( GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* data = NULL, size_t stride = 0 );
			void alloc( const Image& img, bool copy = true );
			void setData( GLint	xoffset, GLint yoffset,	GLsizei	width, GLsizei height, GLenum format, GLenum type, const GLvoid* data, size_t stride = 0 );
//			void setData( const GLBuffer& buffer, GLint	xoffset, GLint yoffset,	GLsizei	width, GLsizei height, GLenum format, GLenum type, const GLvoid* data, size_t stride = 0 );
			void toImage( Image& img, IFormatType itype = IFORMAT_TYPE_UINT8  ) const;

		private:
			GLTexture( const GLTexture& );
			GLTexture& operator=( const GLTexture& );

			void getGLFormat( GLenum& glformat, GLenum& gltype, const IFormat & format ) const;

			GLuint _tex;
			GLenum _target;
			GLint _internalFormat;
			GLsizei _width, _height;
	};

	inline GLenum GLTexture::target() const
	{
		return _target;
	}


	inline void GLTexture::size( GLsizei& width, GLsizei& height ) const
	{
		width = _width;
		height = _height;
	}

	inline GLsizei GLTexture::width() const
	{
		return _width;
	}

	inline GLsizei GLTexture::height() const
	{
		return _height;
	}

}

#endif

