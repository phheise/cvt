/*
   The MIT License (MIT)

   Copyright (c) 2011 - 2013, Philipp Heise and Sebastian Klose
   Copyright (c) 2016, BMW Car IT GmbH, Philipp Heise (philipp.heise@bmw.de)

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
#include <cvt/gfx/IMapScoped.h>
#include <cvt/util/SIMD.h>

namespace cvt {

    #define CONV_GENERIC( func, dsttype, srctype, width )                           \
    static void func( Image & dstImage, const Image & sourceImage, IConvertFlags )  \
    {                                                                               \
        SIMD* simd = SIMD::instance();                                              \
        const uint8_t* src;                                                         \
        const uint8_t* sbase;                                                       \
        size_t sstride;                                                             \
        size_t dstride;                                                             \
        uint8_t* dst;                                                               \
        uint8_t* dbase;                                                             \
        size_t h;                                                                   \
        const size_t w = width;                                                     \
                                                                                    \
        sbase = src = sourceImage.map( &sstride );                                  \
        dbase = dst = dstImage.map( &dstride );                                     \
        h = sourceImage.height();                                                   \
        while( h-- ) {                                                              \
            simd->func( ( dsttype ) dst, ( const srctype ) src, w );                \
            src += sstride;                                                         \
            dst += dstride;                                                         \
        }                                                                           \
        sourceImage.unmap( sbase );                                                 \
        dstImage.unmap( dbase );                                                    \
        return;                                                                     \
    }

    CONV_GENERIC( Conv_XYZAf_to_ZYXAf, float*, float*, sourceImage.width() )
    CONV_GENERIC( Conv_XYZAu8_to_ZYXAu8, uint8_t*, uint8_t*, sourceImage.width() )
    CONV_GENERIC( Conv_u8_to_f, float*, uint8_t*, sourceImage.width() * dstImage.channels() )
    CONV_GENERIC( Conv_u16_to_u8, uint8_t*, uint16_t*, sourceImage.width() * dstImage.channels() )
    CONV_GENERIC( Conv_u16_to_XXXAu8, uint8_t*, uint16_t*, sourceImage.width() )
    CONV_GENERIC( Conv_u16_to_f, float*, uint16_t*, sourceImage.width() * dstImage.channels() )
    CONV_GENERIC( Conv_f_to_u8, uint8_t*, float*, sourceImage.width() * dstImage.channels() )
    CONV_GENERIC( Conv_f_to_u16, uint16_t*, float*, sourceImage.width() * dstImage.channels() )
    CONV_GENERIC( Conv_s16_to_u8, uint8_t*, int16_t*, sourceImage.width() * dstImage.channels() )
    CONV_GENERIC( Conv_GRAYf_to_GRAYu8, uint8_t*, float*, sourceImage.width() * dstImage.channels() )
    CONV_GENERIC( Conv_GRAYALPHAf_to_GRAYf, float*, float*, sourceImage.width() )
    CONV_GENERIC( Conv_GRAYf_to_XXXAf, float*, float*, sourceImage.width() )
    CONV_GENERIC( Conv_RGBAu8_to_GRAYf, float*, uint8_t*, sourceImage.width() )
    CONV_GENERIC( Conv_RGBAu16_to_GRAYf, float*, uint16_t*, sourceImage.width() )
    CONV_GENERIC( Conv_BGRAu16_to_GRAYf, float*, uint16_t*, sourceImage.width() )
    CONV_GENERIC( Conv_GRAYu8_to_XXXAu8, uint8_t*, uint8_t*, sourceImage.width() )
    CONV_GENERIC( Conv_XXXAu8_to_XXXAf, float*, uint8_t*, sourceImage.width() )
    CONV_GENERIC( Conv_XXXAf_to_XXXAu8, uint8_t*, float*, sourceImage.width() )
    CONV_GENERIC( Conv_XYZAu8_to_ZYXAf, float*, uint8_t*, sourceImage.width() )
    CONV_GENERIC( Conv_XYZAf_to_ZYXAu8, uint8_t*, float*, sourceImage.width() )
    CONV_GENERIC( Conv_BGRAu8_to_GRAYu8, uint8_t*, uint8_t*, sourceImage.width() )
    CONV_GENERIC( Conv_RGBAu8_to_GRAYu8, uint8_t*, uint8_t*, sourceImage.width() )
    CONV_GENERIC( Conv_BGRAu8_to_GRAYf, float*, uint8_t*, sourceImage.width() )
    CONV_GENERIC( Conv_BGRAf_to_GRAYf, float*, float*, sourceImage.width() )
    CONV_GENERIC( Conv_RGBAf_to_GRAYf, float*, float*, sourceImage.width() )
    CONV_GENERIC( Conv_YUYVu8_to_RGBAu8, uint8_t*, uint8_t*, sourceImage.width() )
    CONV_GENERIC( Conv_YUYVu8_to_BGRAu8, uint8_t*, uint8_t*, sourceImage.width() )
    CONV_GENERIC( Conv_UYVYu8_to_RGBAu8, uint8_t*, uint8_t*, sourceImage.width() )
    CONV_GENERIC( Conv_UYVYu8_to_BGRAu8, uint8_t*, uint8_t*, sourceImage.width() )
    CONV_GENERIC( Conv_UYVYu8_to_GRAYu8, uint8_t*, uint8_t*, sourceImage.width() )
    CONV_GENERIC( Conv_UYVYu8_to_GRAYf, float*, uint8_t*, sourceImage.width() )
    CONV_GENERIC( Conv_UYVYu8_to_GRAYALPHAu8, uint8_t*, uint8_t*, sourceImage.width() )
    CONV_GENERIC( Conv_YUYVu8_to_GRAYu8, uint8_t*, uint8_t*, sourceImage.width() )
    CONV_GENERIC( Conv_YUYVu8_to_GRAYALPHAu8, uint8_t*, uint8_t*, sourceImage.width() )
    CONV_GENERIC( Conv_YUYVu8_to_GRAYf, float*, uint8_t*, sourceImage.width() )

#undef CONV_GENERIC


#define Conv_DEBAYER_BILINEAR(NAME, EVENFUNC, ODDFUNC ) \
    void Conv_DEBAYER_BILINEAR_##NAME( Image& dstImage, const Image& sourceImage, IConvertFlags ) \
    {                                                                                    \
        const uint32_t* src1;                                                            \
        const uint32_t* src2;                                                            \
        const uint32_t* src3;                                                            \
        const uint8_t* osrc;                                                             \
        uint32_t* dst;                                                                   \
        uint8_t* odst;                                                                   \
        size_t sstride;                                                                  \
        size_t dstride;                                                                  \
        size_t h;                                                                        \
        size_t w;                                                                        \
        size_t i;                                                                        \
                                                                                         \
        SIMD* simd = SIMD::instance();                                                   \
                                                                                         \
        osrc = sourceImage.map( &sstride );                                              \
        src1 = ( uint32_t* ) osrc;                                                       \
        sstride = sstride >> 2;                                                          \
        src2 = src1 + sstride;                                                           \
        src3 = src2 + sstride;                                                           \
        w = sourceImage.width();                                                         \
        h = sourceImage.height();                                                        \
        odst = dstImage.map( &dstride );                                                 \
        dst = ( uint32_t* ) odst;                                                        \
        dstride = dstride >> 2;                                                          \
                                                                                         \
        simd->EVENFUNC( dst, src2, src1, src2, w >> 2 );                                 \
        dst += dstride;                                                                  \
        i = ( h - 2 ) >> 1;                                                              \
        while( i-- ) {                                                                   \
            simd->ODDFUNC( dst, src1, src2, src3, w >> 2 );                              \
            dst += dstride;                                                              \
            src1 += sstride;                                                             \
            src2 += sstride;                                                             \
            src3 += sstride;                                                             \
            simd->EVENFUNC( dst, src1, src2, src3, w >> 2 );                             \
            dst += dstride;                                                              \
            src1 += sstride;                                                             \
            src2 += sstride;                                                             \
            src3 += sstride;                                                             \
        }                                                                                \
        if( h & 1 ) {                                                                    \
            simd->ODDFUNC( dst, src1, src2, src3, w >> 2 );                              \
            dst += dstride;                                                              \
            simd->EVENFUNC( dst, src2, src3, src2, w >> 2 );                             \
        } else {                                                                         \
            simd->ODDFUNC( dst, src1, src2, src1, w >> 2 );                              \
        }                                                                                \
                                                                                         \
        dstImage.unmap( odst );                                                          \
        sourceImage.unmap( osrc );                                                       \
    }

    Conv_DEBAYER_BILINEAR(GBRGu8_to_BGRAu8, debayer_ODD_RGGBu8_BGRAu8, debayer_EVEN_RGGBu8_BGRAu8)
    Conv_DEBAYER_BILINEAR(GBRGu8_to_RGBAu8, debayer_ODD_RGGBu8_RGBAu8, debayer_EVEN_RGGBu8_RGBAu8)

    Conv_DEBAYER_BILINEAR(RGGBu8_to_RGBAu8, debayer_EVEN_RGGBu8_RGBAu8, debayer_ODD_RGGBu8_RGBAu8)
    Conv_DEBAYER_BILINEAR(RGGBu8_to_BGRAu8, debayer_EVEN_RGGBu8_BGRAu8, debayer_ODD_RGGBu8_BGRAu8)


#define Conv_DEBAYER_BILINEAR_HQ(NAME, EVENFUNC, ODDFUNC, EVENFUNCHQ, ODDFUNCHQ ) \
    void Conv_DEBAYER_BILINEAR_HQ_##NAME( Image & dstImage, const Image & sourceImage, IConvertFlags flags ) \
    {                                                                                                        \
        const uint32_t* src1;                                                                                \
        const uint32_t* src2;                                                                                \
        const uint32_t* src3;                                                                                \
        const uint32_t* src4;                                                                                \
        const uint32_t* src5;                                                                                \
        const uint8_t* osrc;                                                                                 \
        uint32_t* dst;                                                                                       \
        uint8_t* odst;                                                                                       \
        size_t sstride;                                                                                      \
        size_t dstride;                                                                                      \
        size_t h;                                                                                            \
        size_t w;                                                                                            \
        size_t i;                                                                                            \
                                                                                                             \
        SIMD* simd = SIMD::instance();                                                                       \
                                                                                                             \
        osrc = sourceImage.map( &sstride );                                                                  \
        src1 = ( uint32_t* ) osrc;                                                                           \
        sstride = sstride >> 2;                                                                              \
        src2 = src1 + sstride;                                                                               \
        src3 = src2 + sstride;                                                                               \
        src4 = src3 + sstride;                                                                               \
        src5 = src4 + sstride;                                                                               \
        w = sourceImage.width();                                                                             \
        h = sourceImage.height();                                                                            \
        odst = dstImage.map( &dstride );                                                                     \
        dst = ( uint32_t* ) odst;                                                                            \
        dstride = dstride >> 2;                                                                              \
                                                                                                             \
        simd->EVENFUNC( dst, src2, src1, src2, w >> 2 );                                                     \
        dst += dstride;                                                                                      \
        simd->ODDFUNC( dst, src1, src2, src3, w >> 2 );                                                      \
        dst += dstride;                                                                                      \
        i = ( h - 4 ) >> 1;                                                                                  \
        while( i-- ) {                                                                                       \
            simd->EVENFUNCHQ( dst, src1, src2, src3, src4, src5, w );                                        \
            dst += dstride;                                                                                  \
            src1 += sstride;                                                                                 \
            src2 += sstride;                                                                                 \
            src3 += sstride;                                                                                 \
            src4 += sstride;                                                                                 \
            src5 += sstride;                                                                                 \
            simd->ODDFUNCHQ( dst, src1, src2, src3, src4, src5, w );                                         \
            dst += dstride;                                                                                  \
            src1 += sstride;                                                                                 \
            src2 += sstride;                                                                                 \
            src3 += sstride;                                                                                 \
            src4 += sstride;                                                                                 \
            src5 += sstride;                                                                                 \
        }                                                                                                    \
        if( h & 1 ) {                                                                                        \
            simd->EVENFUNCHQ( dst, src1, src2, src3, src4, src5, w );                                        \
            dst += dstride;                                                                                  \
            simd->ODDFUNC( dst, src3, src4, src5, w >> 2 );                                                  \
            dst += dstride;                                                                                  \
            simd->EVENFUNC( dst, src4, src5, src4, w >> 2 );                                                 \
        } else {                                                                                             \
            simd->EVENFUNC( dst, src2, src3, src4, w >> 2 );                                                 \
            dst += dstride;                                                                                  \
            simd->ODDFUNC( dst, src3, src4, src3, w >> 2 );                                                  \
        }                                                                                                    \
                                                                                                             \
        dstImage.unmap( odst );                                                                              \
        sourceImage.unmap( osrc );                                                                           \
    }

    Conv_DEBAYER_BILINEAR_HQ(RGGBu8_to_RGBAu8, debayer_EVEN_RGGBu8_RGBAu8, debayer_ODD_RGGBu8_RGBAu8, debayerhq_EVEN_RGGBu8_RGBAu8, debayerhq_ODD_RGGBu8_RGBAu8 )
    Conv_DEBAYER_BILINEAR_HQ(RGGBu8_to_BGRAu8, debayer_EVEN_RGGBu8_BGRAu8, debayer_ODD_RGGBu8_BGRAu8, debayerhq_EVEN_RGGBu8_BGRAu8, debayerhq_ODD_RGGBu8_BGRAu8 )

    Conv_DEBAYER_BILINEAR_HQ(GBRGu8_to_RGBAu8, debayer_ODD_RGGBu8_RGBAu8, debayer_EVEN_RGGBu8_RGBAu8, debayerhq_ODD_RGGBu8_RGBAu8, debayerhq_EVEN_RGGBu8_RGBAu8 )
    Conv_DEBAYER_BILINEAR_HQ(GBRGu8_to_BGRAu8, debayer_ODD_RGGBu8_BGRAu8, debayer_EVEN_RGGBu8_BGRAu8, debayerhq_ODD_RGGBu8_BGRAu8, debayerhq_EVEN_RGGBu8_BGRAu8 )

#define Conv_DEBAYER(NAME) \
void Conv_DEBAYER_##NAME( Image & dstImage, const Image & sourceImage, IConvertFlags flags ) \
{                                                                                            \
        if( flags & ICONVERT_DEBAYER_HQLINEAR )                                              \
            Conv_DEBAYER_BILINEAR_HQ_##NAME( dstImage, sourceImage, flags );                 \
        else                                                                                 \
            Conv_DEBAYER_BILINEAR_##NAME( dstImage, sourceImage, flags );                    \
}

    Conv_DEBAYER(RGGBu8_to_RGBAu8)
    Conv_DEBAYER(RGGBu8_to_BGRAu8)

    Conv_DEBAYER(GBRGu8_to_RGBAu8)
    Conv_DEBAYER(GBRGu8_to_BGRAu8)


#define Conv_DEBAYER_GRAYu8( PATTERN ) \
    void Conv_DEBAYER_##PATTERN##u8_to_GRAYu8( Image & dstImage, const Image & sourceImage, IConvertFlags ) \
    {                                                                                            \
        const uint32_t* src1;                                                                    \
        const uint32_t* src2;                                                                    \
        const uint32_t* src3;                                                                    \
        const uint8_t* osrc;                                                                     \
        uint32_t* dst;                                                                           \
        uint8_t* odst;                                                                           \
        size_t sstride;                                                                          \
        size_t dstride;                                                                          \
        size_t h;                                                                                \
        size_t w;                                                                                \
        size_t i;                                                                                \
                                                                                                 \
        SIMD* simd = SIMD::instance();                                                           \
                                                                                                 \
        osrc = sourceImage.map( &sstride );                                                      \
        src1 = ( uint32_t* ) osrc;                                                               \
        sstride = sstride >> 2;                                                                  \
        src2 = src1 + sstride;                                                                   \
        src3 = src2 + sstride;                                                                   \
        w = sourceImage.width();                                                                 \
        h = sourceImage.height();                                                                \
        odst = dstImage.map( &dstride );                                                         \
        dst = ( uint32_t* ) odst;                                                                \
        dstride = dstride >> 2;                                                                  \
                                                                                                 \
        simd->debayer_EVEN_##PATTERN##u8_GRAYu8( dst, src2, src1, src2, w >> 2 );                \
        dst += dstride;                                                                          \
        i = ( h - 2 ) >> 1;                                                                      \
        while( i-- ) {                                                                           \
            simd->debayer_ODD_##PATTERN##u8_GRAYu8( dst, src1, src2, src3, w >> 2 );             \
            dst += dstride;                                                                      \
            src1 += sstride;                                                                     \
            src2 += sstride;                                                                     \
            src3 += sstride;                                                                     \
            simd->debayer_EVEN_##PATTERN##u8_GRAYu8( dst, src1, src2, src3, w >> 2 );            \
            dst += dstride;                                                                      \
            src1 += sstride;                                                                     \
            src2 += sstride;                                                                     \
            src3 += sstride;                                                                     \
        }                                                                                        \
        if( h & 1 ) {                                                                            \
             simd->debayer_ODD_##PATTERN##u8_GRAYu8( dst, src1, src2, src3, w >> 2 );            \
             dst += dstride;                                                                     \
             simd->debayer_EVEN_##PATTERN##u8_GRAYu8( dst, src2, src3, src2, w >> 2 );           \
        } else {                                                                                 \
            simd->debayer_ODD_##PATTERN##u8_GRAYu8( dst, src1, src2, src1, w >> 2 );             \
        }                                                                                        \
                                                                                                 \
        dstImage.unmap( odst );                                                                  \
        sourceImage.unmap( osrc );                                                               \
    }

    Conv_DEBAYER_GRAYu8(RGGB)
    Conv_DEBAYER_GRAYu8(GBRG)
    Conv_DEBAYER_GRAYu8(GRBG)
    Conv_DEBAYER_GRAYu8(BGGR)


#define Conv_BAYER( NAME, PATTERN_SRC, PATTERN_DST1, PATTERN_DST2 ) \
    void Conv_BAYER_##NAME( Image & dstImage, const Image & sourceImage, IConvertFlags ) \
    {                                                                                    \
        IMapScoped<uint8_t> dstMap( dstImage );                                          \
        IMapScoped<const uint32_t> srcMap( sourceImage );                                \
        SIMD* simd = SIMD::instance();                                                   \
                                                                                         \
        const size_t w = srcMap.width();                                                 \
        size_t h = srcMap.height() >> 1;                                                 \
                                                                                         \
        while( h-- ) {                                                                   \
            simd->bayer_##PATTERN_SRC##_##PATTERN_DST1( dstMap.ptr(), srcMap.ptr(), w ); \
            dstMap++;                                                                    \
            srcMap++;                                                                    \
            simd->bayer_##PATTERN_SRC##_##PATTERN_DST2( dstMap.ptr(), srcMap.ptr(), w ); \
            dstMap++;                                                                    \
            srcMap++;                                                                    \
        }                                                                                \
        if( srcMap.height() & 1 )                                                        \
            simd->bayer_##PATTERN_SRC##_##PATTERN_DST1( dstMap.ptr(), srcMap.ptr(), w ); \
    }

    Conv_BAYER( RGBAu8_to_RGGBu8, RGBAu8, RGu8, GBu8 )
    Conv_BAYER( RGBAu8_to_GBRGu8, RGBAu8, GBu8, RGu8 )
    Conv_BAYER( RGBAu8_to_GRBGu8, RGBAu8, GRu8, BGu8 )
    Conv_BAYER( RGBAu8_to_BGGRu8, RGBAu8, BGu8, GRu8 )

    Conv_BAYER( BGRAu8_to_RGGBu8, RGBAu8, BGu8, GRu8 )
    Conv_BAYER( BGRAu8_to_GBRGu8, RGBAu8, GBu8, BGu8 )
    Conv_BAYER( BGRAu8_to_GRBGu8, RGBAu8, GBu8, RGu8 )
    Conv_BAYER( BGRAu8_to_BGGRu8, RGBAu8, RGu8, GBu8 )

#define LAST_FORMAT ( IFORMAT_UYVY_UINT8 )
#define TABLE( table, source, dst ) table[ ( ( source ) - 1 ) * LAST_FORMAT + ( dst ) - 1 ]

    IConvert::IConvert():
        _convertFuncs( 0 )
    {
        _convertFuncs = new ConversionFunction[ Math::sqr( (int) LAST_FORMAT ) ](); // zero init
        initTable();
    }

    IConvert::~IConvert()
    {
        delete[] _convertFuncs;
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

        /* RGBA_UINT8 TO BAYER */
        TABLE( _convertFuncs, IFORMAT_RGBA_UINT8, IFORMAT_BAYER_RGGB_UINT8 ) = &Conv_BAYER_RGBAu8_to_RGGBu8;
        TABLE( _convertFuncs, IFORMAT_RGBA_UINT8, IFORMAT_BAYER_GBRG_UINT8 ) = &Conv_BAYER_RGBAu8_to_GBRGu8;
        TABLE( _convertFuncs, IFORMAT_RGBA_UINT8, IFORMAT_BAYER_GRBG_UINT8 ) = &Conv_BAYER_RGBAu8_to_GRBGu8;
        TABLE( _convertFuncs, IFORMAT_RGBA_UINT8, IFORMAT_BAYER_BGGR_UINT8 ) = &Conv_BAYER_RGBAu8_to_BGGRu8;

        /* RGBA_UINT16 TO X */
        TABLE( _convertFuncs, IFORMAT_RGBA_UINT16, IFORMAT_RGBA_FLOAT ) = &Conv_u16_to_f;
        TABLE( _convertFuncs, IFORMAT_RGBA_UINT16, IFORMAT_GRAY_FLOAT ) = &Conv_RGBAu16_to_GRAYf;

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

        /* BGRA_UINT8 TO BAYER */
        TABLE( _convertFuncs, IFORMAT_BGRA_UINT8, IFORMAT_BAYER_RGGB_UINT8 ) = &Conv_BAYER_BGRAu8_to_RGGBu8;
        TABLE( _convertFuncs, IFORMAT_BGRA_UINT8, IFORMAT_BAYER_GBRG_UINT8 ) = &Conv_BAYER_BGRAu8_to_GBRGu8;
        TABLE( _convertFuncs, IFORMAT_BGRA_UINT8, IFORMAT_BAYER_GRBG_UINT8 ) = &Conv_BAYER_BGRAu8_to_GRBGu8;
        TABLE( _convertFuncs, IFORMAT_BGRA_UINT8, IFORMAT_BAYER_BGGR_UINT8 ) = &Conv_BAYER_BGRAu8_to_BGGRu8;

        /* BGRA_UINT16 TO X */
        TABLE( _convertFuncs, IFORMAT_BGRA_UINT16, IFORMAT_BGRA_FLOAT ) = &Conv_u16_to_f;
        TABLE( _convertFuncs, IFORMAT_BGRA_UINT16, IFORMAT_GRAY_FLOAT ) = &Conv_BGRAu16_to_GRAYf;

        /* BGRA_FLOAT TO X */
        TABLE( _convertFuncs, IFORMAT_BGRA_FLOAT, IFORMAT_BGRA_UINT8 )  = &Conv_XXXAf_to_XXXAu8;
        TABLE( _convertFuncs, IFORMAT_BGRA_FLOAT, IFORMAT_BGRA_UINT16 ) = &Conv_f_to_u16;
        TABLE( _convertFuncs, IFORMAT_BGRA_FLOAT, IFORMAT_RGBA_UINT8 )  = &Conv_XYZAf_to_ZYXAu8;
        TABLE( _convertFuncs, IFORMAT_BGRA_FLOAT, IFORMAT_RGBA_FLOAT )  = &Conv_XYZAf_to_ZYXAf;
        TABLE( _convertFuncs, IFORMAT_BGRA_FLOAT, IFORMAT_GRAY_FLOAT )  = &Conv_BGRAf_to_GRAYf;

        /* RGGB_UINT8 to X */
        TABLE( _convertFuncs, IFORMAT_BAYER_RGGB_UINT8, IFORMAT_GRAY_UINT8 ) = &Conv_DEBAYER_RGGBu8_to_GRAYu8;
        TABLE( _convertFuncs, IFORMAT_BAYER_RGGB_UINT8, IFORMAT_RGBA_UINT8 ) = &Conv_DEBAYER_RGGBu8_to_RGBAu8;
        TABLE( _convertFuncs, IFORMAT_BAYER_RGGB_UINT8, IFORMAT_BGRA_UINT8 ) = &Conv_DEBAYER_RGGBu8_to_BGRAu8;

        /* GBRG_UINT8 to X */
        TABLE( _convertFuncs, IFORMAT_BAYER_GBRG_UINT8, IFORMAT_GRAY_UINT8 ) = &Conv_DEBAYER_GBRGu8_to_GRAYu8;
        TABLE( _convertFuncs, IFORMAT_BAYER_GBRG_UINT8, IFORMAT_RGBA_UINT8 ) = &Conv_DEBAYER_GBRGu8_to_RGBAu8;
        TABLE( _convertFuncs, IFORMAT_BAYER_GBRG_UINT8, IFORMAT_BGRA_UINT8 ) = &Conv_DEBAYER_GBRGu8_to_BGRAu8;

        /* GRBG_UINT8 to X */
        TABLE( _convertFuncs, IFORMAT_BAYER_GRBG_UINT8, IFORMAT_GRAY_UINT8 ) = &Conv_DEBAYER_GRBGu8_to_GRAYu8;
        TABLE( _convertFuncs, IFORMAT_BAYER_GRBG_UINT8, IFORMAT_RGBA_UINT8 ) = &Conv_DEBAYER_GBRGu8_to_BGRAu8; // intentionally reusing GBRG for GRGB
        TABLE( _convertFuncs, IFORMAT_BAYER_GRBG_UINT8, IFORMAT_BGRA_UINT8 ) = &Conv_DEBAYER_GBRGu8_to_RGBAu8; // intentionally reusing GBRG for GRGB

        /* BGGR_UINT8 to X */
        TABLE( _convertFuncs, IFORMAT_BAYER_BGGR_UINT8, IFORMAT_GRAY_UINT8 ) = &Conv_DEBAYER_BGGRu8_to_GRAYu8;
        TABLE( _convertFuncs, IFORMAT_BAYER_BGGR_UINT8, IFORMAT_RGBA_UINT8 ) = &Conv_DEBAYER_RGGBu8_to_BGRAu8; // intentionally reusing RGGB for BGGR
        TABLE( _convertFuncs, IFORMAT_BAYER_BGGR_UINT8, IFORMAT_BGRA_UINT8 ) = &Conv_DEBAYER_RGGBu8_to_RGBAu8; // intentionally reusing RGGB for BGGR

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
        static IConvert _instance;
        return _instance;
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
