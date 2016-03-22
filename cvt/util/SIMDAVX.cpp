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

#include <cvt/util/SIMDAVX.h>
#include <immintrin.h>

namespace cvt
{
	float SIMDAVX::SAD( const float* src1, const float* src2, const size_t n ) const
	{
		size_t i = n >> 3;

        const __m256i absmask =  _mm256_set1_epi32( 0x7fffffff );
		__m256 a, b, absdiff, sum;


		sum = _mm256_setzero_ps( );
		if( ( ( size_t ) src1 | ( size_t ) src2 ) & 0x1f ) {
			while( i-- ) {
				a = _mm256_loadu_ps( src1 );
				b = _mm256_loadu_ps( src2 );
				absdiff = _mm256_and_ps( _mm256_sub_ps( a, b ), ( __m256 ) absmask );
				sum = _mm256_add_ps( sum, absdiff );
				src1 += 8; src2 += 8;
			}
		} else {
			while( i-- ) {
				a = _mm256_load_ps( src1 );
				b = _mm256_load_ps( src2 );
				absdiff = _mm256_and_ps( _mm256_sub_ps( a, b ),( __m256 ) absmask );
				sum = _mm256_add_ps( sum, absdiff );
				src1 += 8; src2 += 8;
			}
		}

		float sad = 0.0f;

        __m128 sum2 = _mm_add_ps( _mm256_castps256_ps128( sum ), _mm256_extractf128_ps( sum, 1 ) );
        sum2 = _mm_add_ps( sum2, _mm_movehl_ps( sum2, sum2 ) );
        sum2 = _mm_add_ps( sum2, _mm_shuffle_ps( sum2, sum2, _MM_SHUFFLE( 0, 0, 0, 1 ) ) );
        _mm_store_ss( &sad, sum2 );

		i = n & 0x7;
		while( i-- ) {
			sad += Math::abs( *src1++ - *src2++ );
		}

		// Zero upper half of AVX registers to avoid AVX-SSE transition penalties
		_mm256_zeroupper( );

		return sad;
	}

	float SIMDAVX::SSD( const float* src1, const float* src2, const size_t n ) const
	{
		size_t i = n >> 3;

		__m256 a, b, diff, sqr, sum;
		sum = _mm256_setzero_ps( );
		if( ( ( size_t ) src1 | ( size_t ) src2 ) & 0x1f ) {
			while( i-- ) {
				a = _mm256_loadu_ps( src1 );
				b = _mm256_loadu_ps( src2 );
				diff = _mm256_sub_ps( a, b );
				sqr = _mm256_mul_ps( diff, diff );
				sum = _mm256_add_ps( sum, sqr );
				src1 += 8; src2 += 8;
			}
		} else {
			while( i-- ) {
				a = _mm256_load_ps( src1 );
				b = _mm256_load_ps( src2 );
				diff = _mm256_sub_ps( a, b );
				sqr = _mm256_mul_ps( diff, diff );
				sum = _mm256_add_ps( sum, sqr );
				src1 += 8; src2 += 8;
			}
		}

		float ssd = 0.0f;

        __m128 sum2 = _mm_add_ps( _mm256_castps256_ps128( sum ), _mm256_extractf128_ps( sum, 1 ) );
        sum2 = _mm_add_ps( sum2, _mm_movehl_ps( sum2, sum2 ) );
        sum2 = _mm_add_ps( sum2, _mm_shuffle_ps( sum2, sum2, _MM_SHUFFLE( 0, 0, 0, 1 ) ) );
        _mm_store_ss( &ssd, sum2 );

		i = n & 0x7;
		while( i-- ) {
			ssd += Math::sqr( *src1++ - *src2++ );
		}

		// Zero upper half of AVX registers to avoid AVX-SSE transition penalties
		_mm256_zeroupper( );

		return ssd;
	}

	float SIMDAVX::NCC( float const* src1, float const* src2, const size_t n ) const
	{
		size_t i = n >> 3;

		float mean1 = 0.0f;
		float mean2 = 0.0f;
		float mean12 = 0.0f;
		float meanSqr1 = 0.0f;
		float meanSqr2 = 0.0f;

		__m256 a, b, simdMean1, simdMean2, simdMean12, simdMeanSqr1, simdMeanSqr2;

		simdMean1 = _mm256_setzero_ps( );
		simdMean2 = _mm256_setzero_ps( );
		simdMean12 = _mm256_setzero_ps( );
		simdMeanSqr1 = _mm256_setzero_ps( );
		simdMeanSqr2 = _mm256_setzero_ps( );

		if( ( ( size_t ) src1 | ( size_t ) src2 ) & 0x1f ) {
			while( i-- ) {
				a = _mm256_loadu_ps( src1 );
				b = _mm256_loadu_ps( src2 );
				simdMean1 = _mm256_add_ps( simdMean1, a );
				simdMean2 = _mm256_add_ps( simdMean2, b );
				simdMean12 = _mm256_add_ps( simdMean12, _mm256_mul_ps( a, b ) );
				simdMeanSqr1 = _mm256_add_ps( simdMeanSqr1, _mm256_mul_ps( a, a ) );
				simdMeanSqr2 = _mm256_add_ps( simdMeanSqr2, _mm256_mul_ps( b, b ) );
				src1 += 8; src2 += 8;
			}
		} else {
			while( i-- ) {
				a = _mm256_load_ps( src1 );
				b = _mm256_load_ps( src2 );
				simdMean1 = _mm256_add_ps( simdMean1, a );
				simdMean2 = _mm256_add_ps( simdMean2, b );
				simdMean12 = _mm256_add_ps( simdMean12, _mm256_mul_ps( a, b ) );
				simdMeanSqr1 = _mm256_add_ps( simdMeanSqr1, _mm256_mul_ps( a, a ) );
				simdMeanSqr2 = _mm256_add_ps( simdMeanSqr2, _mm256_mul_ps( b, b ) );
				src1 += 8; src2 += 8;
			}
		}

#define avx_reduce_sum( ptr, r, r2 ) \
		__m128 r2 = _mm_add_ps( _mm256_castps256_ps128( r ), _mm256_extractf128_ps( r, 1 ) ); \
		r2 = _mm_add_ps( r2, _mm_movehl_ps( r2, r2 ) ); \
		r2 = _mm_add_ps( r2, _mm_shuffle_ps( r2, r2, _MM_SHUFFLE( 0, 0, 0, 1 ) ) ); \
		_mm_store_ss( ptr, r2 );

		avx_reduce_sum( &mean1, simdMean1, simdMean1_2 );
		avx_reduce_sum( &mean2, simdMean2, simdMean2_2 );
		avx_reduce_sum( &mean12, simdMean12, simdMean12_2 );
		avx_reduce_sum( &meanSqr1, simdMeanSqr1, simdMeanSqr1_2 );
		avx_reduce_sum( &meanSqr2, simdMeanSqr2, simdMeanSqr2_2 );

		// Zero upper half of AVX registers to avoid AVX-SSE transition penalties
		_mm256_zeroupper( );

#undef avx_reduce_sum

		i = n & 0x7;
		while( i-- ) {
			float v1 = *src1++;
			float v2 = *src2++;
			mean1 += v1;
			mean2 += v2;
			mean12 += v1 * v2;
			meanSqr1 += Math::sqr( v1 );
			meanSqr2 += Math::sqr( v2 );
		}

		float nInv = 1.0f / (float) n;
		mean1 *= nInv;
		mean2 *= nInv;
		mean12 *= nInv;
		meanSqr1 *= nInv;
		meanSqr2 *= nInv;

		float cov = mean12 - ( mean1 * mean2 );
		float var1var2 = ( meanSqr1 - Math::sqr( mean1 ) ) * ( meanSqr2 - Math::sqr( mean2 ) );

		// Avoid division by zero
		if( var1var2 == 0.0f ) {
			var1var2 = Math::floatNext( 0.0f );
		}

		return Math::invSqrt( var1var2 ) * cov;
	}

}
