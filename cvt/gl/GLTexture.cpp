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

#include <cvt/gl/GLTexture.h>
#include <cvt/gl/GLBuffer.h>
#include <cvt/util/SIMD.h>

namespace cvt {

	GLTexture::GLTexture( GLenum target ) : _tex( 0 ), _target( target ), _internalFormat( 0 ), _width( 0 ), _height( 0 )
	{
		glGenTextures( 1, &_tex );
	}

	GLTexture::GLTexture( const Image& img ) : _tex( 0 ), _target( GL_TEXTURE_2D ), _internalFormat( 0 ), _width( 0 ), _height( 0 )
	{
		glGenTextures( 1, &_tex );
		alloc( img );
	}

	GLTexture::~GLTexture()
	{
		glDeleteTextures( 1, &_tex );
	}

	void GLTexture::bind() const
	{
		glBindTexture( _target, _tex );
	}

	void GLTexture::unbind() const
	{
		glBindTexture( _target, 0 );
	}

	void GLTexture::alloc( GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* data, size_t stride )
	{
		_internalFormat = internalFormat;
		_width = width;
		_height = height;
		glBindTexture( _target, _tex );
		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
		glTexParameteri( _target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri( _target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(  _target, 0, internalFormat, width, height, 0, format, type, data );
		glBindTexture( _target, 0 );
	}


	void GLTexture::setData( GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* data, size_t stride )
	{
		glBindTexture( _target, _tex );
		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
		glTexSubImage2D(  _target, 0, xoffset, yoffset, width, height, format, type, data );
		glBindTexture( _target, 0 );
	}

	void GLTexture::alloc( const Image& img, bool copy )
	{
		GLenum glformat, gltype/*, internalformat*/;
		size_t stride;
		const uint8_t* ptr;

		getGLFormat( glformat, gltype, img.format() );

		_width = img.width();
		_height = img.height();
		_internalFormat = glformat;

		glBindTexture( GL_TEXTURE_2D, _tex );
		// FIXME: move outside
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if( copy ) {
			ptr = img.map( &stride );
			// FIXME: we assume that image memory is always aligned - at least to 4 bytes
			//glPixelStorei( GL_UNPACK_ALIGNMENT, )
			glPixelStorei( GL_UNPACK_ROW_LENGTH, ( GLint ) ( stride / ( img.format().bpp ) ) );
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, ( GLsizei ) _width, ( GLsizei ) _height, 0, glformat, gltype, ptr );
			glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );

			img.unmap( ptr );
		} else {
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, ( GLsizei ) _width, ( GLsizei ) _height, 0, glformat, gltype, NULL );
		}
	}


	void GLTexture::toImage( Image& img, IFormatType itype ) const
	{
		GLBuffer pbo( GL_PIXEL_PACK_BUFFER );
		pbo.alloc( GL_STREAM_READ, IFormat::GRAY_FLOAT.bpp * _width * _height  );
		pbo.bind();
		bind();
		//glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
		glGetTexImage( _target, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );
		unbind();
		pbo.unbind();

		img.reallocate( _width, _height, IFormat::GRAY_FLOAT );

		uint8_t* src = ( uint8_t* ) pbo.map( GL_READ_ONLY );
		size_t dstride, sstride;
		uint8_t* dst = img.map( &dstride );
		uint8_t* pdst = dst;
		SIMD* simd = SIMD::instance();

		size_t n = _height;
		sstride = IFormat::GRAY_FLOAT.bpp * _width;
		while( n-- ) {
			simd->Memcpy( pdst, src, sstride );
			src += sstride;
			pdst += dstride;
		}
		pbo.unmap();
		img.unmap( dst );
	}

	void GLTexture::getGLFormat( GLenum& glformat, GLenum& gltype, const IFormat & format ) const
	{
		switch ( format.formatID ) {
			case IFORMAT_GRAY_UINT8:		glformat = GL_RED; gltype = GL_UNSIGNED_BYTE; break;
			case IFORMAT_GRAY_UINT16:		glformat = GL_RED; gltype = GL_UNSIGNED_SHORT; break;
			case IFORMAT_GRAY_INT16:		glformat = GL_RED; gltype = GL_SHORT; break;
			case IFORMAT_GRAY_FLOAT:		glformat = GL_RED; gltype = GL_FLOAT; break;

			case IFORMAT_GRAYALPHA_UINT8:	glformat = GL_RG; gltype = GL_UNSIGNED_BYTE; break;
			case IFORMAT_GRAYALPHA_UINT16:	glformat = GL_RG; gltype = GL_UNSIGNED_SHORT; break;
			case IFORMAT_GRAYALPHA_INT16:	glformat = GL_RG; gltype = GL_SHORT; break;
			case IFORMAT_GRAYALPHA_FLOAT:	glformat = GL_RG; gltype = GL_FLOAT; break;

			case IFORMAT_RGBA_UINT8:		glformat = GL_RGBA; gltype = GL_UNSIGNED_BYTE; break;
			case IFORMAT_RGBA_UINT16:		glformat = GL_RGBA; gltype = GL_UNSIGNED_SHORT; break;
			case IFORMAT_RGBA_INT16:		glformat = GL_RGBA; gltype = GL_SHORT; break;
			case IFORMAT_RGBA_FLOAT:		glformat = GL_RGBA; gltype = GL_FLOAT; break;

			case IFORMAT_BGRA_UINT8:		glformat = GL_BGRA; gltype = GL_UNSIGNED_BYTE; break;
			case IFORMAT_BGRA_UINT16:		glformat = GL_BGRA; gltype = GL_UNSIGNED_SHORT; break;
			case IFORMAT_BGRA_INT16:		glformat = GL_BGRA; gltype = GL_SHORT; break;
			case IFORMAT_BGRA_FLOAT:		glformat = GL_BGRA; gltype = GL_FLOAT; break;

			case IFORMAT_BAYER_RGGB_UINT8:  glformat = GL_RED; gltype = GL_UNSIGNED_BYTE; break;
			case IFORMAT_BAYER_GRBG_UINT8:  glformat = GL_RED; gltype = GL_UNSIGNED_BYTE; break;

			case IFORMAT_YUYV_UINT8:		glformat = GL_RG; gltype = GL_UNSIGNED_BYTE; break;
			case IFORMAT_UYVY_UINT8:		glformat = GL_RG; gltype = GL_UNSIGNED_BYTE; break;
			default:
											throw CVTException( "No equivalent GL format found" );
											break;
		}
	}
}
