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

#ifndef CVT_IFORMAT_H
#define CVT_IFORMAT_H

#include <stdlib.h>
#include <cvt/util/Exception.h>
#include <fstream>
#include <cvt/gl/OpenGL.h>
#include <cvt/cl/CLImageFormat.h>

namespace cvt
{
	enum IFormatID
	{
		IFORMAT_GRAY_UINT8 = 1,
		IFORMAT_GRAY_UINT16,
		IFORMAT_GRAY_INT16,
		IFORMAT_GRAY_FLOAT,

		IFORMAT_GRAYALPHA_UINT8,
		IFORMAT_GRAYALPHA_UINT16,
		IFORMAT_GRAYALPHA_INT16,
		IFORMAT_GRAYALPHA_FLOAT,

		IFORMAT_RGBA_UINT8,
		IFORMAT_RGBA_UINT16,
		IFORMAT_RGBA_INT16,
		IFORMAT_RGBA_FLOAT,

		IFORMAT_BGRA_UINT8,
		IFORMAT_BGRA_UINT16,
		IFORMAT_BGRA_INT16,
		IFORMAT_BGRA_FLOAT,

		IFORMAT_BAYER_RGGB_UINT8,
        IFORMAT_BAYER_GRBG_UINT8,
        IFORMAT_BAYER_GBRG_UINT8,
		IFORMAT_YUYV_UINT8,
		IFORMAT_UYVY_UINT8
	};

	enum IFormatType
	{
		IFORMAT_TYPE_OTHER = 0,
		IFORMAT_TYPE_UINT8,
		IFORMAT_TYPE_UINT16,
		IFORMAT_TYPE_INT16,
		IFORMAT_TYPE_FLOAT
	};

	struct IFormat
	{
		IFormat( const IFormat & f );
		bool operator==( const IFormat & other ) const;
		bool operator!=( const IFormat & other ) const;

		size_t channels;
		size_t bpc;
		size_t bpp;
		IFormatID formatID;
		IFormatType type;

		static const IFormat GRAY_UINT8;
		static const IFormat GRAY_UINT16;
		static const IFormat GRAY_INT16;
		static const IFormat GRAY_FLOAT;
		static const IFormat GRAYALPHA_UINT8;
		static const IFormat GRAYALPHA_UINT16;
		static const IFormat GRAYALPHA_INT16;
		static const IFormat GRAYALPHA_FLOAT;
		static const IFormat RGBA_UINT8;
		static const IFormat RGBA_UINT16;
		static const IFormat RGBA_INT16;
		static const IFormat RGBA_FLOAT;
		static const IFormat BGRA_UINT8;
		static const IFormat BGRA_UINT16;
		static const IFormat BGRA_INT16;
		static const IFormat BGRA_FLOAT;
		static const IFormat BAYER_RGGB_UINT8;
        static const IFormat BAYER_GRBG_UINT8;
        static const IFormat BAYER_GBRG_UINT8;
		static const IFormat YUYV_UINT8;
		static const IFormat UYVY_UINT8;

		static const IFormat& uint8Equivalent( const IFormat& format );
		static const IFormat& uint16Equivalent( const IFormat& format );
		static const IFormat& int16Equivalent( const IFormat& format );
		static const IFormat& floatEquivalent( const IFormat& format );
        static const IFormat& formatForId( IFormatID formatID );
		static const IFormat& glEquivalent( GLenum format, GLenum type );

		void toGLFormatType( GLenum& format, GLenum& type ) const;
		void toCLImageFormat( CLImageFormat& format ) const;

		private:
			IFormat( size_t c, size_t bpc, size_t bpp, IFormatID formatID, IFormatType type );
	};

	inline bool IFormat::operator==( const IFormat & other ) const
	{
		return ( other.formatID == formatID );
	}

	inline bool IFormat::operator!=( const IFormat & other ) const
	{
		return ( other.formatID != formatID );
	}

	inline const IFormat & IFormat::uint8Equivalent( const IFormat & format )
	{
		switch ( format.formatID ) {
			case IFORMAT_GRAY_UINT8:
			case IFORMAT_GRAY_UINT16:
			case IFORMAT_GRAY_INT16:
			case IFORMAT_GRAY_FLOAT:
				return IFormat::GRAY_UINT8;
			case IFORMAT_GRAYALPHA_UINT8:
			case IFORMAT_GRAYALPHA_UINT16:
			case IFORMAT_GRAYALPHA_INT16:
			case IFORMAT_GRAYALPHA_FLOAT:
				return IFormat::GRAYALPHA_UINT8;
			case IFORMAT_RGBA_UINT8:
			case IFORMAT_RGBA_UINT16:
			case IFORMAT_RGBA_INT16:
			case IFORMAT_RGBA_FLOAT:
				return IFormat::RGBA_UINT8;
			case IFORMAT_BGRA_UINT8:
			case IFORMAT_BGRA_UINT16:
			case IFORMAT_BGRA_INT16:
			case IFORMAT_BGRA_FLOAT:
				return IFormat::BGRA_UINT8;
			case IFORMAT_BAYER_RGGB_UINT8:
				return IFormat::BAYER_RGGB_UINT8;
            case IFORMAT_BAYER_GRBG_UINT8:
				return IFormat::BAYER_GRBG_UINT8;
            case IFORMAT_BAYER_GBRG_UINT8:
                return IFormat::BAYER_GBRG_UINT8;
			case IFORMAT_YUYV_UINT8:
				return IFormat::YUYV_UINT8;
			case IFORMAT_UYVY_UINT8:
				return IFormat::UYVY_UINT8;
			default:
				throw CVTException( "NO UINT8 equivalent for requested FORMAT" );
		}
	}

	inline const IFormat & IFormat::uint16Equivalent( const IFormat & format )
	{
		switch ( format.formatID ) {
			case IFORMAT_GRAY_UINT8:
			case IFORMAT_GRAY_UINT16:
			case IFORMAT_GRAY_INT16:
			case IFORMAT_GRAY_FLOAT:
				return IFormat::GRAY_UINT16;
			case IFORMAT_GRAYALPHA_UINT8:
			case IFORMAT_GRAYALPHA_UINT16:
			case IFORMAT_GRAYALPHA_INT16:
			case IFORMAT_GRAYALPHA_FLOAT:
				return IFormat::GRAYALPHA_UINT16;
			case IFORMAT_RGBA_UINT8:
			case IFORMAT_RGBA_UINT16:
			case IFORMAT_RGBA_INT16:
			case IFORMAT_RGBA_FLOAT:
				return IFormat::RGBA_UINT16;
			case IFORMAT_BGRA_UINT8:
			case IFORMAT_BGRA_UINT16:
			case IFORMAT_BGRA_INT16:
			case IFORMAT_BGRA_FLOAT:
				return IFormat::BGRA_UINT16;
			default:
				throw CVTException( "NO INT16 equivalent for requested FORMAT" );
				break;
		}
	}

	inline const IFormat & IFormat::int16Equivalent( const IFormat & format )
	{
		switch ( format.formatID ) {
			case IFORMAT_GRAY_UINT8:
			case IFORMAT_GRAY_UINT16:
			case IFORMAT_GRAY_INT16:
			case IFORMAT_GRAY_FLOAT:
				return IFormat::GRAY_INT16;
			case IFORMAT_GRAYALPHA_UINT8:
			case IFORMAT_GRAYALPHA_UINT16:
			case IFORMAT_GRAYALPHA_INT16:
			case IFORMAT_GRAYALPHA_FLOAT:
				return IFormat::GRAYALPHA_INT16;
			case IFORMAT_RGBA_UINT8:
			case IFORMAT_RGBA_UINT16:
			case IFORMAT_RGBA_INT16:
			case IFORMAT_RGBA_FLOAT:
				return IFormat::RGBA_INT16;
			case IFORMAT_BGRA_UINT8:
			case IFORMAT_BGRA_UINT16:
			case IFORMAT_BGRA_INT16:
			case IFORMAT_BGRA_FLOAT:
				return IFormat::BGRA_INT16;
			default:
				throw CVTException( "NO INT16 equivalent for requested FORMAT" );
				break;
		}
	}

	inline const IFormat & IFormat::floatEquivalent( const IFormat & format )
	{
		switch ( format.formatID ) {
			case IFORMAT_GRAY_UINT8:
			case IFORMAT_GRAY_UINT16:
			case IFORMAT_GRAY_INT16:
			case IFORMAT_GRAY_FLOAT:
				return IFormat::GRAY_FLOAT;
			case IFORMAT_GRAYALPHA_UINT8:
			case IFORMAT_GRAYALPHA_UINT16:
			case IFORMAT_GRAYALPHA_INT16:
			case IFORMAT_GRAYALPHA_FLOAT:
				return IFormat::GRAYALPHA_FLOAT;
			case IFORMAT_RGBA_UINT8:
			case IFORMAT_RGBA_UINT16:
			case IFORMAT_RGBA_INT16:
			case IFORMAT_RGBA_FLOAT:
				return IFormat::RGBA_FLOAT;
			case IFORMAT_BGRA_UINT8:
			case IFORMAT_BGRA_UINT16:
			case IFORMAT_BGRA_INT16:
			case IFORMAT_BGRA_FLOAT:
				return IFormat::BGRA_FLOAT;
			default:
				throw CVTException( "NO UINT8 equivalent for requested FORMAT" );
				break;
		}
	}


	inline void IFormat::toGLFormatType( GLenum& glformat, GLenum& gltype ) const
	{
		switch ( formatID ) {
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

			case IFORMAT_BAYER_RGGB_UINT8:	glformat = GL_RED; gltype = GL_UNSIGNED_BYTE; break;
            case IFORMAT_BAYER_GRBG_UINT8:	glformat = GL_RED; gltype = GL_UNSIGNED_BYTE; break;
            case IFORMAT_BAYER_GBRG_UINT8:	glformat = GL_RED; gltype = GL_UNSIGNED_BYTE; break;

			case IFORMAT_YUYV_UINT8:		glformat = GL_RG; gltype = GL_UNSIGNED_BYTE; break;
			case IFORMAT_UYVY_UINT8:		glformat = GL_RG; gltype = GL_UNSIGNED_BYTE; break;
			default:
											throw CVTException( "No equivalent GL format found" );
											break;
		}
	}

	inline void IFormat::toCLImageFormat( CLImageFormat& format ) const
	{
		cl_channel_order clorder;
		cl_channel_type  cltype;

		switch ( formatID ) {
			case IFORMAT_GRAY_UINT8:		clorder = CL_INTENSITY; cltype = CL_UNORM_INT8; break;
			case IFORMAT_GRAY_UINT16:		clorder = CL_INTENSITY; cltype = CL_UNORM_INT16; break;
			case IFORMAT_GRAY_INT16:		clorder = CL_INTENSITY; cltype = CL_SNORM_INT16; break;
			case IFORMAT_GRAY_FLOAT:		clorder = CL_INTENSITY;	cltype = CL_FLOAT; break;

			case IFORMAT_GRAYALPHA_UINT8:	clorder = CL_RG; cltype = CL_UNORM_INT8; break;
			case IFORMAT_GRAYALPHA_UINT16:	clorder = CL_RG; cltype = CL_UNORM_INT16; break;
			case IFORMAT_GRAYALPHA_INT16:	clorder = CL_RG; cltype = CL_SNORM_INT16; break;
			case IFORMAT_GRAYALPHA_FLOAT:	clorder = CL_RG; cltype = CL_FLOAT; break;

			case IFORMAT_RGBA_UINT8:		clorder = CL_RGBA; cltype = CL_UNORM_INT8; break;
			case IFORMAT_RGBA_UINT16:		clorder = CL_RGBA; cltype = CL_UNORM_INT16; break;
			case IFORMAT_RGBA_INT16:		clorder = CL_RGBA; cltype = CL_SNORM_INT16; break;
			case IFORMAT_RGBA_FLOAT:		clorder = CL_RGBA; cltype = CL_FLOAT; break;

			case IFORMAT_BGRA_UINT8:		clorder = CL_BGRA; cltype = CL_UNORM_INT8; break;
			case IFORMAT_BGRA_UINT16:		clorder = CL_BGRA; cltype = CL_UNORM_INT16; break;
			case IFORMAT_BGRA_INT16:		clorder = CL_BGRA; cltype = CL_SNORM_INT16; break;
			case IFORMAT_BGRA_FLOAT:		clorder = CL_BGRA; cltype = CL_FLOAT; break;

			case IFORMAT_BAYER_RGGB_UINT8:	clorder = CL_INTENSITY; cltype = CL_UNORM_INT8; break;
            case IFORMAT_BAYER_GRBG_UINT8:	clorder = CL_INTENSITY; cltype = CL_UNORM_INT8; break;
            case IFORMAT_BAYER_GBRG_UINT8:	clorder = CL_INTENSITY; cltype = CL_UNORM_INT8; break;

			case IFORMAT_YUYV_UINT8:		clorder = CL_RA; cltype = CL_UNORM_INT8; break;
			case IFORMAT_UYVY_UINT8:		clorder = CL_RA; cltype = CL_UNORM_INT8; break;
			default:
				throw CVTException( "No equivalent CL format found" );
				break;
		}
		format = CLImageFormat( clorder, cltype );
	}

	inline const IFormat& IFormat::glEquivalent( GLenum format, GLenum type )
	{
		switch( format )
		{
			case GL_RED:
				switch( type ) {
					case GL_UNSIGNED_BYTE: return IFormat::GRAY_UINT8;
					case GL_UNSIGNED_SHORT: return IFormat::GRAY_UINT16;
					case GL_SHORT: return IFormat::GRAY_INT16;
					case GL_FLOAT: return IFormat::GRAY_FLOAT;
					default:
						throw CVTException("GL type unsupported");
						break;
				}
				break;
			case GL_RG:
				switch( type ) {
					case GL_UNSIGNED_BYTE: return IFormat::GRAYALPHA_UINT8;
					case GL_UNSIGNED_SHORT: return IFormat::GRAYALPHA_UINT16;
					case GL_SHORT: return IFormat::GRAYALPHA_INT16;
					case GL_FLOAT: return IFormat::GRAYALPHA_FLOAT;
					default:
						throw CVTException("GL type unsupported");
						break;
				}
				break;
			case GL_RGBA:
				switch( type ) {
					case GL_UNSIGNED_BYTE: return IFormat::RGBA_UINT8;
					case GL_UNSIGNED_SHORT: return IFormat::RGBA_UINT16;
					case GL_SHORT: return IFormat::RGBA_INT16;
					case GL_FLOAT: return IFormat::RGBA_FLOAT;
					default:
						throw CVTException("GL type unsupported");
						break;
				}
				break;
			case GL_BGRA:
				switch( type ) {
					case GL_UNSIGNED_BYTE: return IFormat::BGRA_UINT8;
					case GL_UNSIGNED_SHORT: return IFormat::BGRA_UINT16;
					case GL_SHORT: return IFormat::BGRA_INT16;
					case GL_FLOAT: return IFormat::BGRA_FLOAT;
					default:
						throw CVTException("GL type unsupported");
						break;
				}
				break;
			default:
				throw CVTException( "GL format unsupported" );
				break;
		}
	}

    inline const IFormat& IFormat::formatForId( IFormatID formatID )
    {
        switch ( formatID ) {
			case IFORMAT_GRAY_UINT8:
                return IFormat::GRAY_UINT8;
			case IFORMAT_GRAY_UINT16:
                return IFormat::GRAY_UINT16;
			case IFORMAT_GRAY_INT16:
                return IFormat::GRAY_INT16;
			case IFORMAT_GRAY_FLOAT:
				return IFormat::GRAY_FLOAT;
			case IFORMAT_GRAYALPHA_UINT8:
                return IFormat::GRAYALPHA_UINT8;
			case IFORMAT_GRAYALPHA_UINT16:
                return IFormat::GRAYALPHA_UINT16;
			case IFORMAT_GRAYALPHA_INT16:
                return IFormat::GRAYALPHA_INT16;
			case IFORMAT_GRAYALPHA_FLOAT:
				return IFormat::GRAYALPHA_FLOAT;
			case IFORMAT_RGBA_UINT8:
                return IFormat::RGBA_UINT8;
			case IFORMAT_RGBA_UINT16:
                return IFormat::RGBA_UINT16;
			case IFORMAT_RGBA_INT16:
                return IFormat::RGBA_INT16;
			case IFORMAT_RGBA_FLOAT:
				return IFormat::RGBA_FLOAT;
			case IFORMAT_BGRA_UINT8:
                return IFormat::BGRA_UINT8;
			case IFORMAT_BGRA_UINT16:
                return IFormat::BGRA_UINT16;
			case IFORMAT_BGRA_INT16:
                return IFormat::BGRA_INT16;
			case IFORMAT_BGRA_FLOAT:
				return IFormat::BGRA_FLOAT;
            case IFORMAT_UYVY_UINT8:
                return IFormat::UYVY_UINT8;
            case IFORMAT_YUYV_UINT8:
                return IFormat::YUYV_UINT8;
            case IFORMAT_BAYER_RGGB_UINT8:
                return IFormat::BAYER_RGGB_UINT8;
            case IFORMAT_BAYER_GRBG_UINT8:
                return IFormat::BAYER_GRBG_UINT8;
            case IFORMAT_BAYER_GBRG_UINT8:
                return IFormat::BAYER_GBRG_UINT8;
			default:
				String msg;
				msg.sprintf( "UNKNOWN INPUT FORMAT: %d", (int)formatID );
				throw CVTException( msg.c_str() );
				break;
		}
    }

	std::ostream& operator<<(std::ostream &out, const IFormat &f);

}

#endif
