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

#ifndef SIMD_H
#define SIMD_H
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <string>

#include <cvt/gfx/IScaleFilter.h>
#include <cvt/gfx/IBorder.h>
#include <cvt/math/Matrix.h>
#include <cvt/math/Vector.h>

namespace cvt {
    enum SIMDType {
        SIMD_BASE = 0,
        SIMD_SSE,
        SIMD_SSE2,
        SIMD_SSE3,
        SIMD_SSSE3,
        SIMD_SSE41,
        SIMD_SSE42,
        SIMD_AVX,
        SIMD_BEST
    };

    class Application;

    class SIMD
    {
        friend class Application;
        protected:
            SIMD() {}
            SIMD( const SIMD& ) {}

        public:
            virtual ~SIMD() {}

            virtual void Memcpy( uint8_t* dst, uint8_t const* src, const size_t n ) const;

            /* set to constant */
            virtual void SetValueU8( uint8_t* dst, const uint8_t value, const size_t n ) const;
            virtual void SetValueU16( uint16_t* dst, const uint16_t value, const size_t n ) const;
            virtual void SetValueU32( uint32_t* dst, const uint32_t value, const size_t n ) const;
            virtual void SetValue1f( float* dst, const float value, const size_t n ) const;
            virtual void SetValue2f( float* dst, const float (&value)[ 2 ], const size_t n ) const;
            virtual void SetValue4f( float* dst, const float (&value)[ 4 ], const size_t n ) const;

            /* memory block and constants */
            virtual void AddValue1f( float* dst, float const* src, const float value, const size_t n ) const;
            virtual void SubValue1f( float* dst, float const* src, const float value, const size_t n ) const;
            virtual void MulValue1f( float* dst, float const* src, const float value, const size_t n ) const;
            virtual void DivValue1f( float* dst, float const* src, const float value, const size_t n ) const;

            virtual void MulValue1ui16( uint16_t* dst, uint16_t const* src, float value, size_t n ) const;

            virtual void AddValue4f( float* dst, float const* src, const float (&value)[ 4 ], const size_t n ) const;
            virtual void SubValue4f( float* dst, float const* src, const float (&value)[ 4 ], const size_t n ) const;
            virtual void MulValue4f( float* dst, float const* src, const float (&value)[ 4 ], const size_t n ) const;
            virtual void DivValue4f( float* dst, float const* src, const float (&value)[ 4 ], const size_t n ) const;

            virtual void MulAddValue1f( float* dst, float const* src1, const float value, const size_t n ) const;
            virtual void MulSubValue1f( float* dst, float const* src1, const float value, const size_t n ) const;

            virtual void MulAddValue4f( float* dst, float const* src1, const float (&value)[ 4 ], const size_t n ) const;
            virtual void MulSubValue4f( float* dst, float const* src1, const float (&value)[ 4 ], const size_t n ) const;

			virtual void MinValueU8( uint8_t* dst, const uint8_t* src1, const uint8_t* src2, size_t n ) const;
			virtual void MinValueU16( uint16_t* dst, const uint16_t* src1, const uint16_t* src2, size_t n ) const;
			virtual void MinValue1f( float* dst, const float* src1, const float* src2, size_t n ) const;

			virtual void MaxValueU8( uint8_t* dst, const uint8_t* src1, const uint8_t* src2, size_t n ) const;
			virtual void MaxValueU16( uint16_t* dst, const uint16_t* src1, const uint16_t* src2, size_t n ) const;
			virtual void MaxValue1f( float* dst, const float* src1, const float* src2, size_t n ) const;

            virtual void MinValueVertU8( uint8_t* dst, const uint8_t** bufs, size_t numbufs, size_t n ) const;
            virtual void MinValueVertU16( uint16_t* dst, const uint16_t** bufs, size_t numbufs, size_t n ) const;
            virtual void MinValueVert1f( float* dst, const float** bufs, size_t numbufs, size_t n ) const;

            virtual void MaxValueVertU8( uint8_t* dst, const uint8_t** bufs, size_t numbufs, size_t n ) const;
            virtual void MaxValueVertU16( uint16_t* dst, const uint16_t** bufs, size_t numbufs, size_t n ) const;
            virtual void MaxValueVert1f( float* dst, const float** bufs, size_t numbufs, size_t n ) const;


			/* morphological ops */
			virtual void erodeSpanU8( uint8_t* dst, const uint8_t* src, size_t n, size_t radius ) const;
			virtual void erodeSpanU16( uint16_t* dst, const uint16_t* src, size_t n, size_t radius ) const;
			virtual void erodeSpan1f( float* dst, const float* src, size_t n, size_t radius ) const;

			virtual void dilateSpanU8( uint8_t* dst, const uint8_t* src, size_t n, size_t radius ) const;
			virtual void dilateSpanU16( uint16_t* dst, const uint16_t* src, size_t n, size_t radius ) const;
			virtual void dilateSpan1f( float* dst, const float* src, size_t n, size_t radius ) const;

            /* memory blocks */

            /* floating point numbers */
            virtual void Add( float* dst, float const* src1, float const* src2, const size_t n ) const;
            virtual void Sub( float* dst, float const* src1, float const* src2, const size_t n ) const;
            virtual void Mul( float* dst, float const* src1, float const* src2, const size_t n ) const;
            virtual void Div( float* dst, float const* src1, float const* src2, const size_t n ) const;

            /* Fixed point numbers */
            virtual void MulValue1fx( Fixed * dst, const Fixed * src, Fixed value, size_t n ) const;
            virtual void MulAddValue1fx( Fixed* dst, const Fixed* src, Fixed value, size_t n ) const;
            virtual void MulU8Value1fx( Fixed * dst, const uint8_t* src, Fixed value, size_t n ) const;
            virtual void MulAddU8Value1fx( Fixed* dst, const uint8_t* src, Fixed value, size_t n ) const;


            virtual void MulU8Value1f( float* dst, const uint8_t* src, float value, size_t n ) const;

            /* binary swap routines / endian conversion */
            virtual void BSwap16( uint16_t* dst, const uint16_t* src, size_t size ) const;
            virtual void BSwap32( uint32_t* dst, const uint32_t* src, size_t size ) const;
            virtual void BSwap64( uint64_t* dst, const uint64_t* src, size_t size ) const;

            /* memory block reductions */
            /**
             * @brief SSD   sum of squared differences
             * @param src1  first input
             * @param src2  second input
             * @param n     size of array
             * @return \sum_i ( src1[ i ] - src2[ i ] )^2
             */
            virtual float SSD( const float* src1, const float* src2, const size_t n ) const;
            virtual float SSD( uint8_t const* src1, uint8_t const* src2, const size_t n ) const;

            /**
             * @brief sumSqr - compute sum of squares
             * @param src   input values
             * @param n     size of array
             * @return \sum_i ( src[ i ] )^2
             */
            virtual float sumSqr( float const* src, const size_t n ) const;

            virtual float SAD( const float* src1, const float* src2, const size_t n ) const;
            virtual size_t SAD( uint8_t const* src1, uint8_t const* src2, const size_t n ) const;

            virtual float NCC( const float* src1, const float* src2, const size_t n ) const;
            
            /* Infinite Impulse Response */
            /* IIR_#coefs_#pass_#direction_#channels_inputType */
            virtual void IIR4FwdHorizontal4Fx( Fixed* dst, const uint8_t * src, size_t width, const Fixed * n,
                                               const Fixed * d, const Fixed & b1 ) const;

            virtual void IIR4BwdHorizontal4Fx( uint8_t * dst, const Fixed* fwdRes, const uint8_t * src, size_t w,
                                               const Fixed * n, const Fixed * d, const Fixed & b ) const;

            virtual void IIR4FwdVertical4Fx( Fixed* buffer, const uint8_t * src, size_t sstride,
                                             size_t h, const Fixed * n, const Fixed * d, const Fixed & b ) const;

            virtual void IIR4BwdVertical4Fx( uint8_t * dst, size_t dstride, Fixed* fwdRes,
                                             size_t h, const Fixed * n, const Fixed * d, const Fixed & b ) const;

			/* add vertical */
			virtual void AddVert_f( float* dst, const float**bufs, size_t numbufs, size_t width ) const;
			virtual void AddVert_f_to_u8( uint8_t* dst, const float**bufs, size_t numbufs, size_t width ) const;
			virtual void AddVert_f_to_s16( int16_t* dst, const float**bufs, size_t numbufs, size_t width ) const;

			virtual void AddVert_fx_to_u8( uint8_t* dst, const Fixed** bufs, size_t numbufs, size_t width ) const;

			/* Convolution */
            virtual void ConvolveHorizontal1f( float* dst, const float* src, const size_t width, float const* weights, const size_t wn, IBorderType btype ) const;
            virtual void ConvolveHorizontal2f( float* dst, const float* src, const size_t width, float const* weights, const size_t wn, IBorderType btype ) const;
            virtual void ConvolveHorizontal4f( float* dst, const float* src, const size_t width, float const* weights, const size_t wn, IBorderType btype ) const;

            virtual void ConvolveHorizontalSym1f( float* dst, const float* src, const size_t width, float const* weights, const size_t wn, IBorderType btype ) const;
            virtual void ConvolveHorizontalSym2f( float* dst, const float* src, const size_t width, float const* weights, const size_t wn, IBorderType btype ) const;
            virtual void ConvolveHorizontalSym4f( float* dst, const float* src, const size_t width, float const* weights, const size_t wn, IBorderType btype ) const;

            virtual void ConvolveHorizontal1u8_to_fx( Fixed* dst, const uint8_t* src, const size_t width, const Fixed* weights, const size_t wn, IBorderType btype ) const;
            virtual void ConvolveHorizontal2u8_to_fx( Fixed* dst, const uint8_t* src, const size_t width, const Fixed* weights, const size_t wn, IBorderType btype ) const;
            virtual void ConvolveHorizontal4u8_to_fx( Fixed* dst, const uint8_t* src, const size_t width, const Fixed* weights, const size_t wn, IBorderType btype ) const;

            virtual void ConvolveHorizontalSym1u8_to_fx( Fixed* dst, const uint8_t* src, const size_t width, const Fixed* weights, const size_t wn, IBorderType btype ) const;
            virtual void ConvolveHorizontalSym2u8_to_fx( Fixed* dst, const uint8_t* src, const size_t width, const Fixed* weights, const size_t wn, IBorderType btype ) const;
            virtual void ConvolveHorizontalSym4u8_to_fx( Fixed* dst, const uint8_t* src, const size_t width, const Fixed* weights, const size_t wn, IBorderType btype ) const;

            virtual void ConvolveHorizontal1u8_to_f( float* dst, const uint8_t* src, const size_t width, const float* weights, const size_t wn, IBorderType btype ) const;
            virtual void ConvolveHorizontal2u8_to_f( float* dst, const uint8_t* src, const size_t width, const float* weights, const size_t wn, IBorderType btype ) const;
            virtual void ConvolveHorizontal4u8_to_f( float* dst, const uint8_t* src, const size_t width, const float* weights, const size_t wn, IBorderType btype ) const;

            virtual void ConvolveHorizontalSym1u8_to_f( float* dst, const uint8_t* src, const size_t width, const float* weights, const size_t wn, IBorderType btype ) const;
            virtual void ConvolveHorizontalSym2u8_to_f( float* dst, const uint8_t* src, const size_t width, const float* weights, const size_t wn, IBorderType btype ) const;
            virtual void ConvolveHorizontalSym4u8_to_f( float* dst, const uint8_t* src, const size_t width, const float* weights, const size_t wn, IBorderType btype ) const;

            virtual void ConvolveClampVert_fx_to_u8( uint8_t* dst, const Fixed** bufs, const Fixed* weights, size_t numw, size_t width ) const;
            virtual void ConvolveClampVert_fx_to_s16( int16_t* dst, const Fixed** bufs, const Fixed* weights, size_t numw, size_t width ) const;
            virtual void ConvolveClampVert_f( float* dst, const float** bufs, const float* weights, size_t numw, size_t width ) const;
            virtual void ConvolveClampVert_f_to_u8( uint8_t* dst, const float** bufs, const float* weights, size_t numw, size_t width ) const;
            virtual void ConvolveClampVert_f_to_s16( int16_t* dst, const float** bufs, const float* weights, size_t numw, size_t width ) const;

            virtual void ConvolveClampVertSym_fx_to_u8( uint8_t* dst, const Fixed** bufs, const Fixed* weights, size_t numw, size_t width ) const;
            virtual void ConvolveClampVertSym_f( float* dst, const float** bufs, const float* weights, size_t numw, size_t width ) const;
            virtual void ConvolveClampVertSym_f_to_u8( uint8_t* dst, const float** bufs, const float* weights, size_t numw, size_t width ) const;
            virtual void ConvolveClampVertSym_f_to_s16( int16_t* dst, const float** bufs, const float* weights, size_t numw, size_t width ) const;

            virtual void ConvolveAdaptiveClamp1f( float* _dst, float const* _src, const size_t width, IConvolveAdaptivef* conva ) const;
            virtual void ConvolveAdaptiveClamp2f( float* _dst, float const* _src, const size_t width, IConvolveAdaptivef* conva ) const;
            virtual void ConvolveAdaptiveClamp4f( float* _dst, float const* _src, const size_t width, IConvolveAdaptivef* conva ) const;
            virtual void ConvolveAdaptive1Fixed( Fixed* _dst, uint8_t const* _src, size_t width, IConvolveAdaptiveFixed* conva ) const;
            virtual void ConvolveAdaptive2Fixed( Fixed* _dst, uint8_t const* _src, size_t width, IConvolveAdaptiveFixed* conva ) const;
            virtual void ConvolveAdaptive4Fixed( Fixed* _dst, uint8_t const* _src, size_t width, IConvolveAdaptiveFixed* conva ) const;

            virtual void Conv_f_to_u8( uint8_t* dst, float const* src, const size_t n ) const;
            virtual void Conv_f_to_u16( uint16_t* dst, const float* src, const size_t n ) const;
            virtual void Conv_fx_to_u8( uint8_t* dst, const Fixed* src, const size_t n ) const;
            virtual void Conv_fx_to_s16( int16_t* dst, const Fixed* src, const size_t n ) const;

            // uint8_t -> ...
            virtual void Conv_u8_to_f( float* dst, const uint8_t* src, const size_t n ) const;

            // int16_t -> ...
            virtual void Conv_s16_to_u8( uint8_t* dst, int16_t const* src, const size_t n ) const;

            virtual void Conv_u16_to_f( float* dst, const uint16_t* src, const size_t n ) const;
            virtual void Conv_u16_to_u8( uint8_t* dst, const uint16_t* src, const size_t n ) const;
            virtual void Conv_u16_to_XXXAu8( uint8_t* dst, const uint16_t* src, const size_t n ) const;
            virtual void Conv_GRAYf_to_GRAYu8( uint8_t* _dst, const float* src, const size_t n ) const;
            virtual void Conv_GRAYf_to_XXXAf( float* dst, const float* src, const size_t n ) const;
            virtual void Conv_GRAYALPHAf_to_GRAYf( float* dst, const float* src, const size_t n ) const;
            virtual void Conv_GRAYu8_to_XXXAu8( uint8_t* _dst, const uint8_t* src, const size_t n ) const;
            virtual void Conv_XXXAf_to_XXXAu8( uint8_t* dst, float const* src, const size_t n ) const;
            virtual void Conv_XYZAf_to_ZYXAu8( uint8_t* dst, float const* src, const size_t n ) const;
            virtual void Conv_XYZAf_to_ZYXAf( float* dst, float const* src, const size_t n ) const;
            virtual void Conv_XXXAu8_to_XXXAf( float* dst, uint8_t const* src, const size_t n ) const;
            virtual void Conv_XYZAu8_to_ZYXAf( float* dst, uint8_t const* src, const size_t n ) const;
            virtual void Conv_XYZAu8_to_ZYXAu8( uint8_t* dst, uint8_t const* src, const size_t n ) const;
            virtual void Conv_RGBAu8_to_GRAYf( float* _dst, uint8_t const* _src, const size_t n ) const;
            virtual void Conv_BGRAu8_to_GRAYf( float* _dst, uint8_t const* _src, const size_t n ) const;
            virtual void Conv_RGBAu8_to_GRAYu8( uint8_t* _dst, uint8_t const* _src, const size_t n ) const;
            virtual void Conv_BGRAu8_to_GRAYu8( uint8_t* _dst, uint8_t const* _src, const size_t n ) const;
            virtual void Conv_RGBAf_to_GRAYf( float* _dst, const float* _src, const size_t n ) const;
            virtual void Conv_BGRAf_to_GRAYf( float* _dst, const float* _src, const size_t n ) const;

            virtual void Conv_XXXf_to_XXXAf(float * dst, const float* src, size_t n) const;
            virtual void Conv_XXXAf_to_XXXf(float * dst, const float* src, size_t n) const;

            virtual void Conv_XXXu8_to_XXXAu8(uint8_t * dst, const uint8_t* src, size_t n) const;
            virtual void Conv_XXXAu8_to_XXXu8(uint8_t * dst, const uint8_t* src, size_t n) const;

            virtual void Conv_YUYVu8_to_RGBAu8( uint8_t* dst, const uint8_t* src, const size_t n ) const;
            virtual void Conv_YUYVu8_to_BGRAu8( uint8_t* dst, const uint8_t* src, const size_t n ) const;
            virtual void Conv_UYVYu8_to_RGBAu8( uint8_t* dst, const uint8_t* src, const size_t n ) const;
            virtual void Conv_UYVYu8_to_BGRAu8( uint8_t* dst, const uint8_t* src, const size_t n ) const;

            virtual void Conv_YUYVu8_to_GRAYu8( uint8_t* dst, const uint8_t* src, const size_t n ) const;
            virtual void Conv_YUYVu8_to_GRAYALPHAu8( uint8_t* dst, const uint8_t* src, const size_t n ) const;
            virtual void Conv_YUYVu8_to_GRAYf( float* dst, const uint8_t* src, const size_t n ) const;

            virtual void Conv_UYVYu8_to_GRAYu8( uint8_t* dst, const uint8_t* src, const size_t n ) const;
            virtual void Conv_UYVYu8_to_GRAYALPHAu8( uint8_t* dst, const uint8_t* src, const size_t n ) const;
            virtual void Conv_UYVYu8_to_GRAYf( float* dst, const uint8_t* src, const size_t n ) const;

            virtual void Conv_YUV420u8_to_RGBAu8( uint8_t* dst, const uint8_t* srcy, const uint8_t* srcu, const uint8_t* srcv, const size_t n ) const;
            virtual void Conv_YUV420u8_to_BGRAu8( uint8_t* dst, const uint8_t* srcy, const uint8_t* srcu, const uint8_t* srcv, const size_t n ) const;


            virtual void Decompose_4f( float* dst1, float* dst2, float* dst3, float* dst4, const float* src, size_t n ) const;
            virtual void Decompose_4f_to_3f( float* dst1, float* dst2, float* dst3, const float* src, size_t n ) const;
            virtual void Decompose_2f( float* dst1, float* dst2, const float* src, size_t n ) const;

            virtual void Decompose_4u8( uint8_t* dst1, uint8_t* dst2, uint8_t* dst3, uint8_t* dst4, const uint8_t* src, size_t n ) const;
            virtual void Decompose_4u8_to_3u8( uint8_t* dst1, uint8_t* dst2, uint8_t* dst3, const uint8_t* src, size_t n ) const;
            virtual void Decompose_2u8( uint8_t* dst1, uint8_t* dst2, const uint8_t* src, size_t n ) const;


			virtual void BoxFilterHorizontal_1u8_to_f( float* dst, const uint8_t* src, size_t radius, size_t width ) const;
			virtual void BoxFilterHorizontal_1f( float* dst, const float* src, size_t radius, size_t width ) const;

			virtual void BoxFilterVert_f_to_u8( uint8_t* dst, float* accum, const float* add, const float* sub, size_t radius, size_t width ) const;
			virtual void BoxFilterVert_f( float* dst, float* accum, const float* add, const float* sub, size_t radius, size_t width ) const;

			/*virtual void BoxFilter_2u8_to_f( float* dst, const uint8_t* src, size_t radius, size_t width ) const;
			virtual void BoxFilter_4u8_to_f( float* dst, const uint8_t* src, size_t radius, size_t width ) const;

			virtual void BoxFilter_1f( float* dst, const float* src, size_t radius, size_t width ) const;
			virtual void BoxFilter_2f( float* dst, const float* src, size_t radius, size_t width ) const;
			virtual void BoxFilter_4f( float* dst, const float* src, size_t radius, size_t width ) const;*/

            /* convolve with horizontal gaussian [ 1 4 6 4 1 ] and store the odd results in u16 - even results are discarded/not calculated */
            virtual void pyrdownHalfHorizontal_1u8_to_1u16( uint16_t* dst, const uint8_t* src, size_t n ) const;
            /* convolve with vertical gaussian [ 1 4 6 4 1 ] and store the odd rows in u8 dst by >> 8 */
            virtual void pyrdownHalfVertical_1u16_to_1u8( uint8_t* dst, uint16_t* rows[ 5 ], size_t n ) const;

            virtual void warpLinePerspectiveBilinear1f( float* dst, const float* src, size_t srcStride, size_t srcWidth, size_t srcHeight,
                                                        const float* point, const float* normal, const size_t n ) const;
            virtual void warpLinePerspectiveBilinear4f( float* dst, const float* src, size_t srcStride, size_t srcWidth, size_t srcHeight,
                                                        const float* point, const float* normal, const size_t n ) const;
            virtual void warpLinePerspectiveBilinear1u8( uint8_t* dst, const uint8_t* _src, size_t srcStride, size_t srcWidth, size_t srcHeight,
                                                         const float* point, const float* normal, const size_t n ) const;
            virtual void warpLinePerspectiveBilinear4u8( uint8_t* dst, const uint8_t* _src, size_t srcStride, size_t srcWidth, size_t srcHeight,
                                                         const float* point, const float* normal, const size_t n ) const;


            virtual void warpBilinear1f( float* dst, const float* coords, const float* src, size_t srcStride, size_t srcWidth, size_t srcHeight, float fillcolor, size_t n ) const;
            virtual void warpBilinear4f( float* dst, const float* coords, const float* src, size_t srcStride, size_t srcWidth, size_t srcHeight, const float* fillcolor, size_t n ) const;
            virtual void warpBilinear1u8( uint8_t* dst, const float* coords, const uint8_t* src, size_t srcStride, size_t srcWidth, size_t srcHeight, uint8_t fill, size_t n ) const;
            virtual void warpBilinear4u8( uint8_t* dst, const float* coords, const uint8_t* src, size_t srcStride, size_t srcWidth, size_t srcHeight, uint32_t fill, size_t n ) const;

			virtual void harrisScore1f( float* dst, const float* boxdx2, const float* boxdy2, const float* boxdxdy, float kappa, size_t width ) const;

            virtual float harrisResponse1u8( const uint8_t* _src, size_t srcStride, size_t w, size_t h, const float k ) const;
            virtual float harrisResponse1u8( float & xx, float & xy, float& yy, const uint8_t* _src, size_t srcStride, size_t w, size_t h, const float k ) const;
            virtual float harrisResponseCircular1u8( float & xx, float & xy, float & yy, const uint8_t* _src, size_t srcStride, const float k ) const;

            virtual void debayer_EVEN_RGGBu8_RGBAu8( uint32_t* dst, const uint32_t* src1, const uint32_t* src2, const uint32_t* src3, size_t n ) const;
            virtual void debayer_ODD_RGGBu8_RGBAu8( uint32_t* dst, const uint32_t* src1, const uint32_t* src2, const uint32_t* src3, size_t n ) const;

			virtual void debayerhq_EVEN_RGGBu8_RGBAu8( uint32_t* dst, const uint32_t* src1, const uint32_t* src2, const uint32_t* src3,
													   const uint32_t* src4, const uint32_t* src5, size_t n ) const;
			virtual void debayerhq_ODD_RGGBu8_RGBAu8( uint32_t* dst, const uint32_t* src1, const uint32_t* src2, const uint32_t* src3,
													   const uint32_t* src4, const uint32_t* src5, size_t n ) const;

            virtual void debayer_EVEN_RGGBu8_BGRAu8( uint32_t* dst, const uint32_t* src1, const uint32_t* src2, const uint32_t* src3, size_t n ) const;
            virtual void debayer_ODD_RGGBu8_BGRAu8( uint32_t* dst, const uint32_t* src1, const uint32_t* src2, const uint32_t* src3, size_t n ) const;

            virtual void debayer_EVEN_RGGBu8_GRAYu8( uint32_t* dst, const uint32_t* src1, const uint32_t* src2, const uint32_t* src3, size_t n ) const;
            virtual void debayer_ODD_RGGBu8_GRAYu8( uint32_t* dst, const uint32_t* src1, const uint32_t* src2, const uint32_t* src3, size_t n ) const;

            virtual size_t hammingDistance( const uint8_t* src1, const uint8_t* src2, size_t n ) const;

			// prefix sum for 1 channel images
			virtual void prefixSum1_u8_to_f( float * dst, size_t dstStride, const uint8_t* src, size_t srcStride, size_t width, size_t height ) const;
			virtual void prefixSum1_f_to_f( float * dst, size_t dstStride, const float* src, size_t srcStride, size_t width, size_t height ) const;
			virtual void prefixSum1_xxxxu8_to_f( float * dst, size_t dstStride, const uint8_t * src, size_t srcStride, size_t width, size_t height ) const;

			// prefix sum and square sum
			virtual void prefixSumSqr1_u8_to_f( float * dst, size_t dStride, const uint8_t * src, size_t srcStride, size_t width, size_t height ) const;
			virtual void prefixSumSqr1_f_to_f( float * dst, size_t dStride, const float* src, size_t srcStride, size_t width, size_t height ) const;

			virtual void boxFilterPrefixSum1_f_to_f( float* dst, size_t dstride, const float* src, size_t srcstride, size_t width, size_t height, size_t boxwidth, size_t boxheight ) const;
			virtual void boxFilterPrefixSum1_f_to_u8( uint8_t* dst, size_t dstride, const float* src, size_t srcstride, size_t width, size_t height, size_t boxwidth, size_t boxheight ) const;

			virtual void adaptiveThreshold1_f_to_u8( uint8_t* dst, const float* src, const float* srcmean, size_t n, float t ) const;
			virtual void adaptiveThreshold1_f_to_f( float* dst, const float* src, const float* srcmean, size_t n, float t ) const;

			virtual void threshold1_f_to_u8( uint8_t* dst, const float* src, size_t n, float t ) const;
			virtual void threshold1_f_to_f( float* dst, const float* src, size_t n, float t ) const;
			virtual void threshold1_u8_to_u8( uint8_t* dst, const uint8_t* src, size_t n, uint8_t t ) const;
			virtual void threshold1_u8_to_f( float* dst, const uint8_t* src, size_t n, uint8_t t ) const;

            virtual void sumPoints( Vector2f& dst, const Vector2f* src, size_t n ) const;
            virtual void sumPoints( Vector3f& dst, const Vector3f* src, size_t n ) const;

            virtual void scalePoints( Vector2f* dst, const Vector2f* src, const Vector2f& scale, size_t n ) const;
            virtual void scalePoints( Vector3f* dst, const Vector3f* src, const Vector3f& scale, size_t n ) const;
            virtual void scalePoints( Vector4f* dst, const Vector4f* src, const Vector4f& scale, size_t n ) const;

            virtual void translatePoints( Vector2f* dst, const Vector2f* src, const Vector2f& translation, size_t n ) const;
            virtual void translatePoints( Vector3f* dst, const Vector3f* src, const Vector3f& translation, size_t n ) const;
            virtual void translatePoints( Vector4f* dst, const Vector4f* src, const Vector4f& translation, size_t n ) const;

            virtual void transformPoints( Vector2f* dst, const Matrix2f& mat, const Vector2f* src, size_t n ) const;
            virtual void transformPoints( Vector2f* dst, const Matrix3f& mat, const Vector2f* src, size_t n ) const;
            virtual void transformPoints( Vector3f* dst, const Matrix3f& mat, const Vector3f* src, size_t n ) const;
            virtual void transformPoints( Vector3f* dst, const Matrix4f& mat, const Vector3f* src, size_t n ) const;
            virtual void transformPoints( Vector3d* dst, const Matrix4d& mat, const Vector3d* src, size_t n ) const;
            virtual void transformPoints( Vector4f* dst, const Matrix4f& mat, const Vector4f* src, size_t n ) const;

            virtual void transformPointsHomogenize( Vector2f* dst, const Matrix3f& mat, const Vector2f* src, size_t n ) const;
            virtual void transformPointsHomogenize( Vector3f* dst, const Matrix4f& mat, const Vector3f* src, size_t n ) const;

            /**
             *  \brief project 3D points to 2D coordinates
             *  \param dst      the resulting 2D positions
             *  \param mat      the projection matrix (only 3x4 part will be taken!)
             *  \param src      the 3D source points
             *  \param n        the number of points to transform
             */
            virtual void projectPoints( Vector2f* dst, const Matrix4f& mat, const Vector3f* src, size_t n ) const;
            virtual void projectPoints( Vector2d* dst, const Matrix4d& mat, const Vector3d* src, size_t n ) const;

            virtual std::string name() const;
            virtual SIMDType type() const;

            static SIMD* get( SIMDType type = SIMD_BEST );
            static void  force( SIMDType type );
            static SIMD* instance();
            static SIMDType bestSupportedType();

        private:
            static void cleanup();

            static SIMD* _simd;
    };

    inline void SIMD::Memcpy( uint8_t* dst, uint8_t const* src, const size_t n ) const
    {
        ::memcpy( dst, src, n );
    }

    inline std::string SIMD::name() const
    {
        return "SIMD-BASE";
    }

    inline SIMDType SIMD::type() const
    {
        return SIMD_BASE;
    }
}

#endif
