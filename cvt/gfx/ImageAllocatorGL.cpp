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

#include <cvt/gfx/ImageAllocatorGL.h>
#include <cvt/util/SIMD.h>

#include <stdio.h>
#include <execinfo.h>

namespace cvt {

	ImageAllocatorGL::ImageAllocatorGL() : ImageAllocator(), _ptr( NULL ), _ptrcount( 0 ), _dirty( false )
	{
		glGenTextures( 1, &_tex2d );
		glGenBuffers( 1, &_glbuf );
	}

	ImageAllocatorGL::~ImageAllocatorGL()
	{
		glDeleteTextures( 1, &_tex2d );
		glDeleteBuffers( 1, &_glbuf );
	}

	void ImageAllocatorGL::alloc( size_t width, size_t height, const IFormat& format )
	{
		static const GLint GRAY_SWIZZLE[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
		static const GLint GRAYALPHA_SWIZZLE[] = { GL_RED, GL_RED, GL_RED, GL_GREEN };

		GLenum glformat, gltype/*, internalformat*/;

		if( _width == width && _height == height && _format == format )
			return;

		_width = width;
		_height = height;
		_format = format;
		_stride = Math::pad16( _width * _format.bpp );
		_size = _stride * _height;

		glBindBuffer( GL_PIXEL_UNPACK_BUFFER, _glbuf );
		glBufferData( GL_PIXEL_UNPACK_BUFFER, _size, NULL, GL_DYNAMIC_DRAW );
		glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );

		glBindTexture( GL_TEXTURE_2D, _tex2d );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if( format.channels == 1 )
			glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, GRAY_SWIZZLE );
		if( format.channels == 2 )
			glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, GRAYALPHA_SWIZZLE );


		glPixelStorei( GL_UNPACK_ROW_LENGTH, ( GLint ) ( _stride / ( _format.bpp ) ) );
		getGLFormat( _format, glformat, gltype );
/*		if( _format.bpc <= 1 )
		    internalformat = GL_SRGB8_ALPHA8_EXT;
		else*/
//		    internalformat = GL_RGBA;
		/* do not copy non-meaningful PBO content - just allocate space, since current PBO content is undefined */

		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, ( GLsizei ) _width, ( GLsizei ) _height, 0, glformat, gltype, NULL );
	}

	void ImageAllocatorGL::copy( const ImageAllocator* x, const Recti* r = NULL )
	{
		const uint8_t* src;
		const uint8_t* osrc;
		size_t sstride, dstride;
		uint8_t* dst;
		uint8_t* odst;
		size_t i, n;
		Recti rect( 0, 0, ( int ) x->_width, ( int ) x->_height );
		SIMD* simd = SIMD::instance();

		if( r )
			rect.intersect( *r );
		alloc( rect.width, rect.height, x->_format );

		osrc = src = x->map( &sstride );
		src += rect.y * sstride + x->_format.bpp * rect.x;
		odst = dst = map( &dstride );
		n =  x->_format.bpp * rect.width;

		i = rect.height;
		while( i-- ) {
			simd->Memcpy( dst, src, n );
			dst += sstride;
			src += dstride;
		}
		x->unmap( osrc );
		unmap( odst );
	}

	uint8_t* ImageAllocatorGL::map( size_t* stride )
	{
		*stride = _stride;
		if( !_ptrcount ) {
			glBindBuffer( GL_PIXEL_UNPACK_BUFFER, _glbuf );
			_ptr = ( uint8_t* ) glMapBuffer( GL_PIXEL_UNPACK_BUFFER, GL_READ_WRITE );
			glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );
		}
		_ptrcount++;
		_dirty = true;
		return ( uint8_t* ) _ptr;
	}

	const uint8_t* ImageAllocatorGL::map( size_t* stride ) const
	{
		*stride = _stride;
		if( !_ptrcount ) {
			glBindBuffer( GL_PIXEL_UNPACK_BUFFER, _glbuf );
			_ptr = ( uint8_t* ) glMapBuffer( GL_PIXEL_UNPACK_BUFFER, GL_READ_WRITE );
			glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );
		}
		_ptrcount++;
		return ( uint8_t* ) _ptr;
	}

	void ImageAllocatorGL::unmap( const uint8_t* ) const
	{
		_ptrcount--;
		if( !_ptrcount ) {
			glBindBuffer( GL_PIXEL_UNPACK_BUFFER, _glbuf );
			if( _dirty ) {
				GLenum glformat, gltype;

				glUnmapBuffer( GL_PIXEL_UNPACK_BUFFER );

				glBindTexture( GL_TEXTURE_2D, _tex2d );
				glPixelStorei( GL_UNPACK_ROW_LENGTH, ( GLint ) ( _stride / ( _format.bpp ) ) );
				getGLFormat( _format, glformat, gltype );
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ( GLsizei ) _width, ( GLsizei ) _height, glformat, gltype, NULL);
				_dirty = false;
			} else {
				glUnmapBuffer( GL_PIXEL_UNPACK_BUFFER );
			}
			glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );
			_ptr = NULL;
		}
	}

	void ImageAllocatorGL::getGLFormat( const IFormat & format, GLenum& glformat, GLenum& gltype ) const
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
            case IFORMAT_BAYER_GBRG_UINT8:  glformat = GL_RED; gltype = GL_UNSIGNED_BYTE; break;
            case IFORMAT_BAYER_GRBG_UINT8:  glformat = GL_RED; gltype = GL_UNSIGNED_BYTE; break;

			case IFORMAT_YUYV_UINT8:		glformat = GL_RG; gltype = GL_UNSIGNED_BYTE; break;
			case IFORMAT_UYVY_UINT8:		glformat = GL_RG; gltype = GL_UNSIGNED_BYTE; break;
			default:
				std::cout << format << std::endl;
				throw CVTException( "No Equivalent GL Format found" );
				break;
		}
	}
}
