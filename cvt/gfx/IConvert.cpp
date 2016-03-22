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

#include <cvt/gfx/IConvert.h>
#include <cvt/gfx/Image.h>
#include <cvt/util/SIMD.h>

namespace cvt {

#define LAST_FORMAT	( IFORMAT_UYVY_UINT8 )

#define TABLE( table, source, dst ) table[ ( ( source ) - 1 ) * LAST_FORMAT + ( dst ) - 1 ]

    IConvert* IConvert::_instance = 0;


    #define CONV( func, dI, dsttype, sI, srctype, width )				\
    {																	\
        sbase = src = sI.map( &sstride );								\
        dbase = dst = dI.map( &dstride );								\
        h = sI.height();												\
        while( h-- ) {													\
            simd->func( ( dsttype ) dst, ( const srctype ) src, width );\
            src += sstride;												\
            dst += dstride;												\
        }																\
        sI.unmap( sbase );												\
        dI.unmap( dbase );												\
        return;															\
    }

    static void Conv_XYZAf_to_ZYXAf( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;
        CONV( Conv_XYZAf_to_ZYXAf, dstImage, float*, sourceImage, float*, sourceImage.width() )
    }

    static void Conv_XYZAu8_to_ZYXAu8( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;
        CONV( Conv_XYZAu8_to_ZYXAu8, dstImage, uint8_t*, sourceImage, uint8_t*, sourceImage.width() )
    }

    static void Conv_u8_to_f( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;
        CONV( Conv_u8_to_f, dstImage, float*, sourceImage, uint8_t*, sourceImage.width() * dstImage.channels() )
    }

    static void Conv_u16_to_u8( Image& dstImage, const Image& sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;
        CONV( Conv_u16_to_u8, dstImage, uint8_t*, sourceImage, uint16_t*, sourceImage.width() * dstImage.channels() )
    }
    static void Conv_u16_to_XXXAu8( Image& dstImage, const Image& sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;
        CONV( Conv_u16_to_XXXAu8, dstImage, uint8_t*, sourceImage, uint16_t*, sourceImage.width() )
    }

    static void Conv_u16_to_f( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;
        CONV( Conv_u16_to_f, dstImage, float*, sourceImage, uint16_t*, sourceImage.width() * dstImage.channels() )
    }

    static void Conv_f_to_u8( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_f_to_u8, dstImage, uint8_t*, sourceImage, float*, sourceImage.width() * dstImage.channels() )
    }

    static void Conv_f_to_u16( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_f_to_u16, dstImage, uint16_t*, sourceImage, float*, sourceImage.width() * dstImage.channels() )
    }

    static void Conv_s16_to_u8( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_s16_to_u8, dstImage, uint8_t*, sourceImage, int16_t*, sourceImage.width() * dstImage.channels() )
    }

    static void Conv_GRAYf_to_GRAYu8( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_GRAYf_to_GRAYu8, dstImage, uint8_t*, sourceImage, float*, sourceImage.width() * dstImage.channels() )
    }

    static void Conv_GRAYALPHAf_to_GRAYf( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_GRAYALPHAf_to_GRAYf, dstImage, float*, sourceImage, float*, sourceImage.width() )
    }


    static void Conv_GRAYf_to_XXXAf( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_GRAYf_to_XXXAf, dstImage, float*, sourceImage, float*, sourceImage.width() )
    }


    static void Conv_RGBAu8_to_GRAYf( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_RGBAu8_to_GRAYf, dstImage, float*, sourceImage, uint8_t*, sourceImage.width() )
    }

    static void Conv_GRAYu8_to_XXXAu8( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_GRAYu8_to_XXXAu8, dstImage, uint8_t*, sourceImage, uint8_t*, sourceImage.width() )
    }

    static void Conv_XXXAu8_to_XXXAf( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_XXXAu8_to_XXXAf, dstImage, float*, sourceImage, uint8_t*, sourceImage.width() )
    }

    static void Conv_XXXAf_to_XXXAu8( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_XXXAf_to_XXXAu8, dstImage, uint8_t*, sourceImage, float*, sourceImage.width() )
    }

    static void Conv_XYZAu8_to_ZYXAf( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_XYZAu8_to_ZYXAf, dstImage, float*, sourceImage, uint8_t*, sourceImage.width() )
    }

    static void Conv_XYZAf_to_ZYXAu8( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_XYZAf_to_ZYXAu8, dstImage, uint8_t*, sourceImage, float*, sourceImage.width() )
    }

    static void Conv_BGRAu8_to_GRAYu8( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_BGRAu8_to_GRAYu8, dstImage, uint8_t*, sourceImage, uint8_t*, sourceImage.width() )
    }

    static void Conv_RGBAu8_to_GRAYu8( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_RGBAu8_to_GRAYu8, dstImage, uint8_t*, sourceImage, uint8_t*, sourceImage.width() )
    }


    static void Conv_BGRAu8_to_GRAYf( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_BGRAu8_to_GRAYf, dstImage, float*, sourceImage, uint8_t*, sourceImage.width() )
    }

    static void Conv_BGRAf_to_GRAYf( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_BGRAf_to_GRAYf, dstImage, float*, sourceImage, float*, sourceImage.width() )
    }

    static void Conv_RGBAf_to_GRAYf( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_RGBAf_to_GRAYf, dstImage, float*, sourceImage, float*, sourceImage.width() )
    }


    static void Conv_YUYVu8_to_RGBAu8( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_YUYVu8_to_RGBAu8, dstImage, uint8_t*, sourceImage, uint8_t*, sourceImage.width() )
    }

    static void Conv_YUYVu8_to_BGRAu8( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_YUYVu8_to_BGRAu8, dstImage, uint8_t*, sourceImage, uint8_t*, sourceImage.width() )
    }

    static void Conv_UYVYu8_to_RGBAu8( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_UYVYu8_to_RGBAu8, dstImage, uint8_t*, sourceImage, uint8_t*, sourceImage.width() )
    }

    static void Conv_UYVYu8_to_BGRAu8( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_UYVYu8_to_BGRAu8, dstImage, uint8_t*, sourceImage, uint8_t*, sourceImage.width() )
    }


    static void Conv_UYVYu8_to_GRAYu8( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_UYVYu8_to_GRAYu8, dstImage, uint8_t*, sourceImage, uint8_t*, sourceImage.width() )
    }

    static void Conv_UYVYu8_to_GRAYf( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_UYVYu8_to_GRAYf, dstImage, float*, sourceImage, uint8_t*, sourceImage.width() )
    }



    static void Conv_UYVYu8_to_GRAYALPHAu8( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_UYVYu8_to_GRAYALPHAu8, dstImage, uint8_t*, sourceImage, uint8_t*, sourceImage.width() )
    }

    static void Conv_YUYVu8_to_GRAYu8( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_YUYVu8_to_GRAYu8, dstImage, uint8_t*, sourceImage, uint8_t*, sourceImage.width() )
    }

    static void Conv_YUYVu8_to_GRAYALPHAu8( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_YUYVu8_to_GRAYALPHAu8, dstImage, uint8_t*, sourceImage, uint8_t*, sourceImage.width() )
    }

    static void Conv_YUYVu8_to_GRAYf( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        SIMD* simd = SIMD::instance();
        const uint8_t* src;
        const uint8_t* sbase;
        size_t sstride;
        size_t dstride;
        uint8_t* dst;
        uint8_t* dbase;
        size_t h;

        CONV( Conv_YUYVu8_to_GRAYf, dstImage, float*, sourceImage, uint8_t*, sourceImage.width() )
    }

#undef CONV

    void Conv_BAYER_RGGB_to_RGBAu8( Image & dstImage, const Image & sourceImage, IConvertFlags flags )
    {
        const uint32_t* src1;
        const uint32_t* src2;
        const uint32_t* src3;
        const uint32_t* src4;
        const uint32_t* src5;
        const uint8_t* osrc;
        uint32_t* dst;
        uint8_t* odst;
        size_t sstride;
        size_t dstride;
        size_t h;
        size_t w;
        size_t i;

        SIMD* simd = SIMD::instance();

        osrc = sourceImage.map( &sstride );
        src1 = ( uint32_t* ) osrc;
        sstride = sstride >> 2;
        src2 = src1 + sstride;
        src3 = src2 + sstride;
        src4 = src3 + sstride;
        src5 = src4 + sstride;
        w = sourceImage.width();
        h = sourceImage.height();
        odst = dstImage.map( &dstride );
        dst = ( uint32_t* ) odst;
        dstride = dstride >> 2;

		if( flags & ICONVERT_DEBAYER_HQLINEAR ) {
			simd->debayer_EVEN_RGGBu8_RGBAu8( dst, src2, src1, src2, w >> 2 );
			dst += dstride;
			simd->debayer_ODD_RGGBu8_RGBAu8( dst, src1, src2, src3, w >> 2 );
			dst += dstride;
			for( i = 2; i < h - 2; i += 2 ) {
				simd->debayerhq_EVEN_RGGBu8_RGBAu8( dst, src1, src2, src3, src4, src5, w );
				dst += dstride;
				src1 += sstride;
				src2 += sstride;
				src3 += sstride;
				src4 += sstride;
				src5 += sstride;
				simd->debayerhq_ODD_RGGBu8_RGBAu8( dst, src1, src2, src3, src4, src5, w );
				dst += dstride;
				src1 += sstride;
				src2 += sstride;
				src3 += sstride;
				src4 += sstride;
				src5 += sstride;
			}
			if( h - 2 == i ) {
				simd->debayer_EVEN_RGGBu8_RGBAu8( dst, src2, src3, src4, w >> 2 );
				dst += dstride;
				simd->debayer_ODD_RGGBu8_RGBAu8( dst, src3, src4, src3, w >> 2 );
			} else {
				simd->debayerhq_EVEN_RGGBu8_RGBAu8( dst, src1, src2, src3, src4, src5, w );
				dst += dstride;
				simd->debayer_ODD_RGGBu8_RGBAu8( dst, src3, src4, src5, w >> 2 );
				dst += dstride;
				simd->debayer_EVEN_RGGBu8_RGBAu8( dst, src3, src4, src3, w >> 2 );
			}

		} else {
			simd->debayer_EVEN_RGGBu8_RGBAu8( dst, src2, src1, src2, w >> 2 );
			dst += dstride;
			for( i = 1; i < h - 1; i += 2 ) {
				simd->debayer_ODD_RGGBu8_RGBAu8( dst, src1, src2, src3, w >> 2 );
				dst += dstride;
				src1 += sstride;
				src2 += sstride;
				src3 += sstride;
				simd->debayer_EVEN_RGGBu8_RGBAu8( dst, src1, src2, src3, w >> 2 );
				dst += dstride;
				src1 += sstride;
				src2 += sstride;
				src3 += sstride;
			}
			if( h - 1 == i )
				simd->debayer_ODD_RGGBu8_RGBAu8( dst, src1, src2, src1, w >> 2 );
			else {
				simd->debayer_ODD_RGGBu8_RGBAu8( dst, src1, src2, src3, w >> 2 );
				dst += dstride;
				simd->debayer_EVEN_RGGBu8_RGBAu8( dst, src2, src3, src2, w >> 2 );
			}
		}

        dstImage.unmap( odst );
        sourceImage.unmap( osrc );
    }

    void Conv_BAYER_RGGB_to_BGRAu8( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        const uint32_t* src1;
        const uint32_t* src2;
        const uint32_t* src3;
        const uint8_t* osrc;
        uint32_t* dst;
        uint8_t* odst;
        size_t sstride;
        size_t dstride;
        size_t h;
        size_t w;
        size_t i;

        SIMD* simd = SIMD::instance();

        osrc = sourceImage.map( &sstride );
        src1 = ( uint32_t* ) osrc;
        sstride = sstride >> 2;
        src2 = src1 + sstride;
        src3 = src2 + sstride;
        w = sourceImage.width();
        h = sourceImage.height();
        odst = dstImage.map( &dstride );
        dst = ( uint32_t* ) odst;
        dstride = dstride >> 2;

        simd->debayer_EVEN_RGGBu8_BGRAu8( dst, src2, src1, src2, w >> 2 );
        dst += dstride;
        for( i = 1; i < h - 1; i += 2 ) {
            simd->debayer_ODD_RGGBu8_BGRAu8( dst, src1, src2, src3, w >> 2 );
            dst += dstride;
            src1 += sstride;
            src2 += sstride;
            src3 += sstride;
            simd->debayer_EVEN_RGGBu8_BGRAu8( dst, src1, src2, src3, w >> 2 );
            dst += dstride;
            src1 += sstride;
            src2 += sstride;
            src3 += sstride;
        }
        if( i < h ) {
            if( i & 1 ) {
                simd->debayer_ODD_RGGBu8_BGRAu8( dst, src1, src2, src1, w >> 2 );
            } else {
                simd->debayer_EVEN_RGGBu8_BGRAu8( dst, src1, src2, src1, w >> 2 );
            }
        }
        dstImage.unmap( odst );
        sourceImage.unmap( osrc );
    }

    void Conv_BAYER_RGGB_to_GRAYu8( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        const uint32_t* src1;
        const uint32_t* src2;
        const uint32_t* src3;
        const uint8_t* osrc;
        uint32_t* dst;
        uint8_t* odst;
        size_t sstride;
        size_t dstride;
        size_t h;
        size_t w;
        size_t i;

        SIMD* simd = SIMD::instance();

        osrc = sourceImage.map( &sstride );
        src1 = ( uint32_t* ) osrc;
        sstride = sstride >> 2;
        src2 = src1 + sstride;
        src3 = src2 + sstride;
        w = sourceImage.width();
        h = sourceImage.height();
        odst = dstImage.map( &dstride );
        dst = ( uint32_t* ) odst;
        dstride = dstride >> 2;

        simd->debayer_EVEN_RGGBu8_GRAYu8( dst, src2, src1, src2, w >> 2 );
        dst += dstride;
        for( i = 1; i < h - 1; i += 2 ) {
            simd->debayer_ODD_RGGBu8_GRAYu8( dst, src1, src2, src3, w >> 2 );
            dst += dstride;
            src1 += sstride;
            src2 += sstride;
            src3 += sstride;
            simd->debayer_EVEN_RGGBu8_GRAYu8( dst, src1, src2, src3, w >> 2 );
            dst += dstride;
            src1 += sstride;
            src2 += sstride;
            src3 += sstride;
        }
        if( i < h ) {
            if( ++i & 1 ) {
                simd->debayer_ODD_RGGBu8_GRAYu8( dst, src1, src2, src1, w >> 2 );
            } else {
                simd->debayer_EVEN_RGGBu8_GRAYu8( dst, src1, src2, src1, w >> 2 );
            }
        }
        simd->debayer_EVEN_RGGBu8_GRAYu8( dst, src2, src1, src2, w >> 2 );
        dstImage.unmap( odst );
        sourceImage.unmap( osrc );
    }

    void Conv_BAYER_GBRG_to_RGBAu8( Image & dstImage, const Image & sourceImage, IConvertFlags flags )
    {
        const uint32_t* src1;
        const uint32_t* src2;
        const uint32_t* src3;
        const uint32_t* src4;
        const uint32_t* src5;
        const uint8_t* osrc;
        uint32_t* dst;
        uint8_t* odst;
        size_t sstride;
        size_t dstride;
        size_t h;
        size_t w;
        size_t i;

        SIMD* simd = SIMD::instance();

        osrc = sourceImage.map( &sstride );
        src1 = ( uint32_t* ) osrc;
        sstride = sstride >> 2;
        src2 = src1 + sstride;
        src3 = src2 + sstride;
        src4 = src3 + sstride;
        src5 = src4 + sstride;
        w = sourceImage.width();
        h = sourceImage.height();
        odst = dstImage.map( &dstride );
        dst = ( uint32_t* ) odst;
        dstride = dstride >> 2;

		if( flags & ICONVERT_DEBAYER_HQLINEAR ) {
			simd->debayer_ODD_RGGBu8_RGBAu8( dst, src2, src1, src2, w >> 2 );
			dst += dstride;
			simd->debayer_EVEN_RGGBu8_RGBAu8( dst, src1, src2, src3, w >> 2 );
			dst += dstride;
			for( i = 2; i < h - 2; i += 2 ) {
				simd->debayerhq_ODD_RGGBu8_RGBAu8( dst, src1, src2, src3, src4, src5, w );
				dst += dstride;
				src1 += sstride;
				src2 += sstride;
				src3 += sstride;
				src4 += sstride;
				src5 += sstride;
				simd->debayerhq_EVEN_RGGBu8_RGBAu8( dst, src1, src2, src3, src4, src5, w );
				dst += dstride;
				src1 += sstride;
				src2 += sstride;
				src3 += sstride;
				src4 += sstride;
				src5 += sstride;
			}
			if( h - 2 == i ) {
				simd->debayer_ODD_RGGBu8_RGBAu8( dst, src2, src3, src4, w >> 2 );
				dst += dstride;
				simd->debayer_EVEN_RGGBu8_RGBAu8( dst, src3, src4, src3, w >> 2 );
			} else {
				simd->debayerhq_ODD_RGGBu8_RGBAu8( dst, src1, src2, src3, src4, src5, w );
				dst += dstride;
				simd->debayer_EVEN_RGGBu8_RGBAu8( dst, src3, src4, src5, w >> 2 );
				dst += dstride;
				simd->debayer_ODD_RGGBu8_RGBAu8( dst, src3, src4, src3, w >> 2 );
			}

		} else {
			simd->debayer_ODD_RGGBu8_RGBAu8( dst, src2, src1, src2, w >> 2 );
			dst += dstride;
			for( i = 1; i < h - 1; i += 2 ) {
				simd->debayer_EVEN_RGGBu8_RGBAu8( dst, src1, src2, src3, w >> 2 );
				dst += dstride;
				src1 += sstride;
				src2 += sstride;
				src3 += sstride;
				simd->debayer_ODD_RGGBu8_RGBAu8( dst, src1, src2, src3, w >> 2 );
				dst += dstride;
				src1 += sstride;
				src2 += sstride;
				src3 += sstride;
			}
			if( h - 1 == i )
				simd->debayer_EVEN_RGGBu8_RGBAu8( dst, src1, src2, src1, w >> 2 );
			else {
				simd->debayer_EVEN_RGGBu8_RGBAu8( dst, src1, src2, src3, w >> 2 );
				dst += dstride;
				simd->debayer_ODD_RGGBu8_RGBAu8( dst, src2, src3, src2, w >> 2 );
			}
		}

            dstImage.unmap( odst );
            sourceImage.unmap( osrc );
    }

    void Conv_BAYER_GBRG_to_GRAYu8( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        const uint32_t* src1;
        const uint32_t* src2;
        const uint32_t* src3;
        const uint8_t* osrc;
        uint32_t* dst;
        uint8_t* odst;
        size_t sstride;
        size_t dstride;
        size_t h;
        size_t w;
        size_t i;

        SIMD* simd = SIMD::instance();

        osrc = sourceImage.map( &sstride );
        src1 = ( uint32_t* ) osrc;
        sstride = sstride >> 2;
        src2 = src1 + sstride;
        src3 = src2 + sstride;
        w = sourceImage.width();
        h = sourceImage.height();
        odst = dstImage.map( &dstride );
        dst = ( uint32_t* ) odst;
        dstride = dstride >> 2;

        simd->debayer_ODD_RGGBu8_GRAYu8( dst, src2, src1, src2, w >> 2 );
        dst += dstride;
        for( i = 1; i < h - 1; i += 2 ) {
            simd->debayer_EVEN_RGGBu8_GRAYu8( dst, src1, src2, src3, w >> 2 );
            dst += dstride;
            src1 += sstride;
            src2 += sstride;
            src3 += sstride;
            simd->debayer_ODD_RGGBu8_GRAYu8( dst, src1, src2, src3, w >> 2 );
            dst += dstride;
            src1 += sstride;
            src2 += sstride;
            src3 += sstride;
        }
        if( i < h ) {
            if( ++i & 1 ) {
                simd->debayer_EVEN_RGGBu8_GRAYu8( dst, src1, src2, src1, w >> 2 );
            } else {
                simd->debayer_ODD_RGGBu8_GRAYu8( dst, src1, src2, src1, w >> 2 );
            }
        }
        simd->debayer_ODD_RGGBu8_GRAYu8( dst, src2, src1, src2, w >> 2 );
        dstImage.unmap( odst );
        sourceImage.unmap( osrc );
    }

    void Conv_BAYER_GBRG_to_BGRAu8( Image & dstImage, const Image & sourceImage, IConvertFlags )
    {
        const uint32_t* src1;
        const uint32_t* src2;
        const uint32_t* src3;
        const uint8_t* osrc;
        uint32_t* dst;
        uint8_t* odst;
        size_t sstride;
        size_t dstride;
        size_t h;
        size_t w;
        size_t i;

        SIMD* simd = SIMD::instance();

        osrc = sourceImage.map( &sstride );
        src1 = ( uint32_t* ) osrc;
        sstride = sstride >> 2;
        src2 = src1 + sstride;
        src3 = src2 + sstride;
        w = sourceImage.width();
        h = sourceImage.height();
        odst = dstImage.map( &dstride );
        dst = ( uint32_t* ) odst;
        dstride = dstride >> 2;

        simd->debayer_ODD_RGGBu8_BGRAu8( dst, src2, src1, src2, w >> 2 );
        dst += dstride;
        for( i = 1; i < h - 1; i += 2 ) {
            simd->debayer_EVEN_RGGBu8_BGRAu8( dst, src1, src2, src3, w >> 2 );
            dst += dstride;
            src1 += sstride;
            src2 += sstride;
            src3 += sstride;
            simd->debayer_ODD_RGGBu8_BGRAu8( dst, src1, src2, src3, w >> 2 );
            dst += dstride;
            src1 += sstride;
            src2 += sstride;
            src3 += sstride;
        }
        if( i < h ) {
            if( i & 1 ) {
                simd->debayer_EVEN_RGGBu8_BGRAu8( dst, src1, src2, src1, w >> 2 );
            } else {
                simd->debayer_ODD_RGGBu8_BGRAu8( dst, src1, src2, src1, w >> 2 );
            }
        }
        dstImage.unmap( odst );
        sourceImage.unmap( osrc );
    }

    IConvert::IConvert():
        _convertFuncs( 0 )
    {
        _convertFuncs = new ConversionFunction[ Math::sqr( (int)LAST_FORMAT ) ]();
        //memset( _convertFuncs, 0, Math::sqr( (int)LAST_FORMAT ) );

        this->initTable();
    }

    void IConvert::initTable()
    {
        /* GRAY_UINT8 TO X */
        TABLE( _convertFuncs, IFORMAT_GRAY_UINT8, IFORMAT_GRAY_FLOAT ) = &Conv_u8_to_f;
        TABLE( _convertFuncs, IFORMAT_GRAY_UINT8, IFORMAT_BGRA_UINT8 ) = &Conv_GRAYu8_to_XXXAu8;
        TABLE( _convertFuncs, IFORMAT_GRAY_UINT8, IFORMAT_RGBA_UINT8 ) = &Conv_GRAYu8_to_XXXAu8;

        /* GRAY_UINT16 TO X */
        TABLE( _convertFuncs, IFORMAT_GRAY_UINT16, IFORMAT_GRAY_FLOAT ) = &Conv_u16_to_f;
        TABLE( _convertFuncs, IFORMAT_GRAY_UINT16, IFORMAT_GRAY_UINT8 ) = &Conv_u16_to_u8;
        TABLE( _convertFuncs, IFORMAT_GRAY_UINT16, IFORMAT_RGBA_UINT8 ) = &Conv_u16_to_XXXAu8;
        TABLE( _convertFuncs, IFORMAT_GRAY_UINT16, IFORMAT_BGRA_UINT8 ) = &Conv_u16_to_XXXAu8;

        /* GRAY_INT16 TO X */
        TABLE( _convertFuncs, IFORMAT_GRAY_INT16, IFORMAT_GRAY_UINT8 ) = &Conv_s16_to_u8;

        /* GRAY_FLOAT TO X */
        TABLE( _convertFuncs, IFORMAT_GRAY_FLOAT, IFORMAT_GRAY_UINT8 )  = &Conv_GRAYf_to_GRAYu8;
        TABLE( _convertFuncs, IFORMAT_GRAY_FLOAT, IFORMAT_GRAY_UINT16 ) = &Conv_f_to_u16;
        TABLE( _convertFuncs, IFORMAT_GRAY_FLOAT, IFORMAT_RGBA_FLOAT )  = &Conv_GRAYf_to_XXXAf;
        TABLE( _convertFuncs, IFORMAT_GRAY_FLOAT, IFORMAT_BGRA_FLOAT )  = &Conv_GRAYf_to_XXXAf;

        /* GRAYALPHA_UINT8 TO X */
        TABLE( _convertFuncs, IFORMAT_GRAYALPHA_UINT8, IFORMAT_GRAYALPHA_FLOAT ) = &Conv_u8_to_f;

        /* GRAYALPHA_UINT16 TO X */
        TABLE( _convertFuncs, IFORMAT_GRAYALPHA_UINT16, IFORMAT_GRAYALPHA_FLOAT ) = &Conv_u16_to_f;
        TABLE( _convertFuncs, IFORMAT_GRAYALPHA_UINT16, IFORMAT_GRAYALPHA_UINT8 ) = &Conv_u16_to_u8;

        /* GRAYALPHA_FLOAT TO X */
        TABLE( _convertFuncs, IFORMAT_GRAYALPHA_FLOAT, IFORMAT_GRAYALPHA_UINT8 )  = &Conv_f_to_u8;
        TABLE( _convertFuncs, IFORMAT_GRAYALPHA_FLOAT, IFORMAT_GRAYALPHA_UINT16 ) = &Conv_f_to_u16;
        TABLE( _convertFuncs, IFORMAT_GRAYALPHA_FLOAT, IFORMAT_GRAY_FLOAT )       = &Conv_GRAYALPHAf_to_GRAYf;

        /* RGBA_UINT8 TO X */
        TABLE( _convertFuncs, IFORMAT_RGBA_UINT8, IFORMAT_GRAY_UINT8 ) = &Conv_RGBAu8_to_GRAYu8;
        TABLE( _convertFuncs, IFORMAT_RGBA_UINT8, IFORMAT_GRAY_FLOAT ) = &Conv_RGBAu8_to_GRAYf;
        TABLE( _convertFuncs, IFORMAT_RGBA_UINT8, IFORMAT_RGBA_FLOAT ) = &Conv_XXXAu8_to_XXXAf;
        TABLE( _convertFuncs, IFORMAT_RGBA_UINT8, IFORMAT_BGRA_UINT8 ) = &Conv_XYZAu8_to_ZYXAu8;
        TABLE( _convertFuncs, IFORMAT_RGBA_UINT8, IFORMAT_BGRA_FLOAT ) = &Conv_XYZAu8_to_ZYXAf;

        /* RGBA_UINT16 TO X */
        TABLE( _convertFuncs, IFORMAT_RGBA_UINT16, IFORMAT_RGBA_FLOAT ) = &Conv_u16_to_f;

        /* RGBA_FLOAT TO X */
        TABLE( _convertFuncs, IFORMAT_RGBA_FLOAT, IFORMAT_RGBA_UINT8 )  = &Conv_XXXAf_to_XXXAu8;
        TABLE( _convertFuncs, IFORMAT_RGBA_FLOAT, IFORMAT_RGBA_UINT16 ) = &Conv_f_to_u16;
        TABLE( _convertFuncs, IFORMAT_RGBA_FLOAT, IFORMAT_BGRA_UINT8 )  = &Conv_XYZAf_to_ZYXAu8;
        TABLE( _convertFuncs, IFORMAT_RGBA_FLOAT, IFORMAT_BGRA_FLOAT )  = &Conv_XYZAf_to_ZYXAf;
        TABLE( _convertFuncs, IFORMAT_RGBA_FLOAT, IFORMAT_GRAY_FLOAT )  = &Conv_RGBAf_to_GRAYf;

        /* BGRA_UINT8 TO X */
        TABLE( _convertFuncs, IFORMAT_BGRA_UINT8, IFORMAT_GRAY_UINT8 ) = &Conv_BGRAu8_to_GRAYu8;
        TABLE( _convertFuncs, IFORMAT_BGRA_UINT8, IFORMAT_GRAY_FLOAT ) = &Conv_BGRAu8_to_GRAYf;
        TABLE( _convertFuncs, IFORMAT_BGRA_UINT8, IFORMAT_RGBA_UINT8 ) = &Conv_XYZAu8_to_ZYXAu8;
        TABLE( _convertFuncs, IFORMAT_BGRA_UINT8, IFORMAT_RGBA_FLOAT ) = &Conv_XYZAu8_to_ZYXAf;
        TABLE( _convertFuncs, IFORMAT_BGRA_UINT8, IFORMAT_BGRA_FLOAT ) = &Conv_XXXAu8_to_XXXAf;

        /* RGBA_UINT16 TO X */
        TABLE( _convertFuncs, IFORMAT_BGRA_UINT16, IFORMAT_BGRA_FLOAT ) = &Conv_u16_to_f;

        /* BGRA_FLOAT TO X */
        TABLE( _convertFuncs, IFORMAT_BGRA_FLOAT, IFORMAT_BGRA_UINT8 )  = &Conv_XXXAf_to_XXXAu8;
        TABLE( _convertFuncs, IFORMAT_BGRA_FLOAT, IFORMAT_BGRA_UINT16 ) = &Conv_f_to_u16;
        TABLE( _convertFuncs, IFORMAT_BGRA_FLOAT, IFORMAT_RGBA_UINT8 )  = &Conv_XYZAf_to_ZYXAu8;
        TABLE( _convertFuncs, IFORMAT_BGRA_FLOAT, IFORMAT_RGBA_FLOAT )  = &Conv_XYZAf_to_ZYXAf;
        TABLE( _convertFuncs, IFORMAT_BGRA_FLOAT, IFORMAT_GRAY_FLOAT )  = &Conv_BGRAf_to_GRAYf;

        /* RGGB_UINT8 to X */
        TABLE( _convertFuncs, IFORMAT_BAYER_RGGB_UINT8, IFORMAT_GRAY_UINT8 ) = &Conv_BAYER_RGGB_to_GRAYu8;
        TABLE( _convertFuncs, IFORMAT_BAYER_RGGB_UINT8, IFORMAT_RGBA_UINT8 ) = &Conv_BAYER_RGGB_to_RGBAu8;
        TABLE( _convertFuncs, IFORMAT_BAYER_RGGB_UINT8, IFORMAT_BGRA_UINT8 ) = &Conv_BAYER_RGGB_to_BGRAu8;

        /* GBRG_UINT8 to X */
        TABLE( _convertFuncs, IFORMAT_BAYER_GBRG_UINT8, IFORMAT_GRAY_UINT8 ) = &Conv_BAYER_RGGB_to_GRAYu8;
        TABLE( _convertFuncs, IFORMAT_BAYER_GBRG_UINT8, IFORMAT_RGBA_UINT8 ) = &Conv_BAYER_GBRG_to_RGBAu8;
        TABLE( _convertFuncs, IFORMAT_BAYER_GBRG_UINT8, IFORMAT_BGRA_UINT8 ) = &Conv_BAYER_GBRG_to_BGRAu8;

        /* YUYV_UINT8 to X */
        TABLE( _convertFuncs, IFORMAT_YUYV_UINT8, IFORMAT_GRAY_UINT8 ) = &Conv_YUYVu8_to_GRAYu8;
        TABLE( _convertFuncs, IFORMAT_YUYV_UINT8, IFORMAT_GRAYALPHA_UINT8 ) = &Conv_YUYVu8_to_GRAYALPHAu8;
        TABLE( _convertFuncs, IFORMAT_YUYV_UINT8, IFORMAT_RGBA_UINT8 ) = &Conv_YUYVu8_to_RGBAu8;
        TABLE( _convertFuncs, IFORMAT_YUYV_UINT8, IFORMAT_BGRA_UINT8 ) = &Conv_YUYVu8_to_BGRAu8;
        TABLE( _convertFuncs, IFORMAT_YUYV_UINT8, IFORMAT_GRAY_FLOAT ) = &Conv_YUYVu8_to_GRAYf;

        /* UYVY_UINT8 to X */
        TABLE( _convertFuncs, IFORMAT_UYVY_UINT8, IFORMAT_GRAY_UINT8 ) = &Conv_UYVYu8_to_GRAYu8;
        TABLE( _convertFuncs, IFORMAT_UYVY_UINT8, IFORMAT_GRAYALPHA_UINT8 ) = &Conv_UYVYu8_to_GRAYALPHAu8;
        TABLE( _convertFuncs, IFORMAT_UYVY_UINT8, IFORMAT_RGBA_UINT8 ) = &Conv_UYVYu8_to_RGBAu8;
        TABLE( _convertFuncs, IFORMAT_UYVY_UINT8, IFORMAT_BGRA_UINT8 ) = &Conv_UYVYu8_to_BGRAu8;
        TABLE( _convertFuncs, IFORMAT_UYVY_UINT8, IFORMAT_GRAY_FLOAT ) = &Conv_UYVYu8_to_GRAYf;
    }

    const IConvert& IConvert::instance()
    {
        if( IConvert::_instance == 0 ){
            _instance = new IConvert();
        }

        return *_instance;
    }


    void IConvert::convert( Image & dst, const Image & src, IConvertFlags flags )
    {
        if( src.format() == dst.format() ) {
            dst = src;
            return;
        }

        IFormatID sourceID = src.format().formatID;
        IFormatID dstID = dst.format().formatID;

        if( sourceID > LAST_FORMAT )
            throw CVTException( "Source format unkown" );
        if( dstID > LAST_FORMAT )
            throw CVTException( "Destination format unkown" );

        const IConvert& self = IConvert::instance();
        if( self.TABLE( _convertFuncs, sourceID, dstID ) ){
            self.TABLE( _convertFuncs, sourceID, dstID)( dst, src, flags );
        } else {
            std::cerr << "CONVERSION MISSING: " << src.format() << " -> " << dst.format() << std::endl;
            throw CVTException( "Conversion not implemented!" );
        }
    }

}
