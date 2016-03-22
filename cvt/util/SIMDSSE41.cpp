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

#include <cvt/util/SIMDSSE41.h>

#include <xmmintrin.h>
#include <smmintrin.h>

namespace cvt {

	static inline __m128i _mm_loadl_epi32( __m128i const* p )
	{
		__m128i xmm;
		__asm__("movd (%1), %0;\n\t"
				: "=x"(xmm)
				: "r"(p)
				:
			   );
		return xmm;
	}

	void SIMDSSE41::Conv_XXXAu8_to_XXXAf( float* dst, uint8_t const* src, const size_t n ) const
	{
		__m128i v;
		__m128 fsqr, f, forig;
		size_t n2 = n >> 1;
		size_t n3 = n & 0x01;

		__m128i zero = _mm_setzero_si128();
		__m128 A = _mm_set1_ps( 0.28387f );
		__m128 B = _mm_set1_ps( 1.0f - 0.28387f );
		__m128 C = _mm_set1_ps( 1.0f / 255.0f );


		if( ( ( size_t ) dst | ( size_t ) src ) & 0x0f ) {
			while( n2-- ) {
				v = _mm_loadl_epi64( ( __m128i* ) src );
				src += 8;

				v = _mm_unpacklo_epi8( v, zero );
				__m128i hi = _mm_unpackhi_epi16( v, zero );
				__m128i lo = _mm_unpacklo_epi16( v, zero );

				f = _mm_cvtepi32_ps( lo );
				f = _mm_mul_ps( f, C );
				forig = f;
				fsqr = _mm_mul_ps( f, f );
				f = _mm_mul_ps( f, A );
				f = _mm_add_ps( f, B );
				f = _mm_mul_ps( f, fsqr );
				f = _mm_insert_ps( f, forig, ( 3 << 6 ) | ( 3 << 4 ) );

				_mm_storeu_ps( dst, f );
				dst += 4;

				f = _mm_cvtepi32_ps( hi );
				f = _mm_mul_ps( f, C );
				forig = f;
				fsqr = _mm_mul_ps( f, f );
				f = _mm_mul_ps( f, A );
				f = _mm_add_ps( f, B );
				f = _mm_mul_ps( f, fsqr );
				f = _mm_insert_ps( f, forig, ( 3 << 6 ) | ( 3 << 4 ) );

				_mm_storeu_ps( dst, f );
				dst += 4;
			}
			if( n3 ) {
				v = _mm_loadl_epi32( ( __m128i* ) src );
				v = _mm_unpacklo_epi8( v, zero );
				__m128i lo = _mm_unpacklo_epi16( v, zero );

				f = _mm_cvtepi32_ps( lo );
				f = _mm_mul_ps( f, C );
				forig = f;
				fsqr = _mm_mul_ps( f, f );
				f = _mm_mul_ps( f, A );
				f = _mm_add_ps( f, B );
				f = _mm_mul_ps( f, fsqr );
				f = _mm_insert_ps( f, forig, ( 3 << 6 ) | ( 3 << 4 ) );
				_mm_storeu_ps( dst, f );
			}
		} else {
			while( n2-- ) {
				v = _mm_loadl_epi64( ( __m128i* ) src );
				src += 8;

				v = _mm_unpacklo_epi8( v, zero );
				__m128i hi = _mm_unpackhi_epi16( v, zero );
				__m128i lo = _mm_unpacklo_epi16( v, zero );

				f = _mm_cvtepi32_ps( lo );
				f = _mm_mul_ps( f, C );
				forig = f;
				fsqr = _mm_mul_ps( f, f );
				f = _mm_mul_ps( f, A );
				f = _mm_add_ps( f, B );
				f = _mm_mul_ps( f, fsqr );
				f = _mm_insert_ps( f, forig, ( 3 << 6 ) | ( 3 << 4 ) );

				_mm_stream_ps( dst, f );
				dst += 4;

				f = _mm_cvtepi32_ps( hi );
				f = _mm_mul_ps( f, C );
				forig = f;
				fsqr = _mm_mul_ps( f, f );
				f = _mm_mul_ps( f, A );
				f = _mm_add_ps( f, B );
				f = _mm_mul_ps( f, fsqr );
				f = _mm_insert_ps( f, forig, ( 3 << 6 ) | ( 3 << 4 ) );

				_mm_stream_ps( dst, f );
				dst += 4;
			}
			if( n3 ) {
				v = _mm_loadl_epi32( ( __m128i* ) src );
				v = _mm_unpacklo_epi8( v, zero );
				__m128i lo = _mm_unpacklo_epi16( v, zero );

				f = _mm_cvtepi32_ps( lo );
				f = _mm_mul_ps( f, C );
				forig = f;
				fsqr = _mm_mul_ps( f, f );
				f = _mm_mul_ps( f, A );
				f = _mm_add_ps( f, B );
				f = _mm_mul_ps( f, fsqr );
				f = _mm_insert_ps( f, forig, ( 3 << 6 ) | ( 3 << 4 ) );
				_mm_stream_ps( dst, f );
			}
		}
		return;
	}


	void SIMDSSE41::Conv_XYZAu8_to_ZYXAf( float* dst, uint8_t const* src, const size_t n ) const
	{
		__m128i v;
		__m128 fsqr, f, forig;
		size_t n2 = n >> 1;
		size_t n3 = n & 0x01;

		__m128i zero = _mm_setzero_si128();
		__m128 A = _mm_set1_ps( 0.28387f );
		__m128 B = _mm_set1_ps( 1.0f - 0.28387f );
		__m128 C = _mm_set1_ps( 1.0f / 255.0f );


		if( ( ( size_t ) dst | ( size_t ) src ) & 0x0f ) {
			while( n2-- ) {
				v = _mm_loadl_epi64( ( __m128i* ) src );
				src += 8;

				v = _mm_unpacklo_epi8( v, zero );
				__m128i hi = _mm_unpackhi_epi16( v, zero );
				__m128i lo = _mm_unpacklo_epi16( v, zero );

				f = _mm_cvtepi32_ps( lo );
				f = _mm_mul_ps( f, C );
				forig = f;
				fsqr = _mm_mul_ps( f, f );
				f = _mm_mul_ps( f, A );
				f = _mm_add_ps( f, B );
				f = _mm_mul_ps( f, fsqr );
				f = _mm_insert_ps( f, forig, ( 3 << 6 ) | ( 3 << 4 ) );
				f = _mm_shuffle_ps( f, f, _MM_SHUFFLE( 3, 0, 1, 2 ) );

				_mm_storeu_ps( dst, f );
				dst += 4;

				f = _mm_cvtepi32_ps( hi );
				f = _mm_mul_ps( f, C );
				forig = f;
				fsqr = _mm_mul_ps( f, f );
				f = _mm_mul_ps( f, A );
				f = _mm_add_ps( f, B );
				f = _mm_mul_ps( f, fsqr );
				f = _mm_insert_ps( f, forig, ( 3 << 6 ) | ( 3 << 4 ) );
				f = _mm_shuffle_ps( f, f, _MM_SHUFFLE( 3, 0, 1, 2 ) );

				_mm_storeu_ps( dst, f );
				dst += 4;
			}
			if( n3 ) {
				v = _mm_loadl_epi32( ( __m128i* ) src );
				v = _mm_unpacklo_epi8( v, zero );
				__m128i lo = _mm_unpacklo_epi16( v, zero );

				f = _mm_cvtepi32_ps( lo );
				f = _mm_mul_ps( f, C );
				forig = f;
				fsqr = _mm_mul_ps( f, f );
				f = _mm_mul_ps( f, A );
				f = _mm_add_ps( f, B );
				f = _mm_mul_ps( f, fsqr );
				f = _mm_insert_ps( f, forig, ( 3 << 6 ) | ( 3 << 4 ) );
				f = _mm_shuffle_ps( f, f, _MM_SHUFFLE( 3, 0, 1, 2 ) );

				_mm_storeu_ps( dst, f );
			}
		} else {
			while( n2-- ) {
				v = _mm_loadl_epi64( ( __m128i* ) src );
				src += 8;

				v = _mm_unpacklo_epi8( v, zero );
				__m128i hi = _mm_unpackhi_epi16( v, zero );
				__m128i lo = _mm_unpacklo_epi16( v, zero );

				f = _mm_cvtepi32_ps( lo );
				f = _mm_mul_ps( f, C );
				forig = f;
				fsqr = _mm_mul_ps( f, f );
				f = _mm_mul_ps( f, A );
				f = _mm_add_ps( f, B );
				f = _mm_mul_ps( f, fsqr );
				f = _mm_insert_ps( f, forig, ( 3 << 6 ) | ( 3 << 4 ) );
				f = _mm_shuffle_ps( f, f, _MM_SHUFFLE( 3, 0, 1, 2 ) );

				_mm_stream_ps( dst, f );
				dst += 4;

				f = _mm_cvtepi32_ps( hi );
				f = _mm_mul_ps( f, C );
				forig = f;
				fsqr = _mm_mul_ps( f, f );
				f = _mm_mul_ps( f, A );
				f = _mm_add_ps( f, B );
				f = _mm_mul_ps( f, fsqr );
				f = _mm_insert_ps( f, forig, ( 3 << 6 ) | ( 3 << 4 ) );
				f = _mm_shuffle_ps( f, f, _MM_SHUFFLE( 3, 0, 1, 2 ) );

				_mm_stream_ps( dst, f );
				dst += 4;
			}
			if( n3 ) {
				v = _mm_loadl_epi32( ( __m128i* ) src );
				v = _mm_unpacklo_epi8( v, zero );
				__m128i lo = _mm_unpacklo_epi16( v, zero );

				f = _mm_cvtepi32_ps( lo );
				f = _mm_mul_ps( f, C );
				forig = f;
				fsqr = _mm_mul_ps( f, f );
				f = _mm_mul_ps( f, A );
				f = _mm_add_ps( f, B );
				f = _mm_mul_ps( f, fsqr );
				f = _mm_insert_ps( f, forig, ( 3 << 6 ) | ( 3 << 4 ) );
				f = _mm_shuffle_ps( f, f, _MM_SHUFFLE( 3, 0, 1, 2 ) );

				_mm_stream_ps( dst, f );
			}
		}
		return;
	}
}
