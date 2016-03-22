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

#include <cvt/util/SIMDSSE.h>
#include <xmmintrin.h>


namespace cvt {

#define SSE_AAOP1_FLOAT( name, sseop, cop ) \
void SIMDSSE::name( float* dst, const float* src1, const float* src2, const size_t n ) const \
{                                                                                            \
		size_t i = n >> 2;                                                                   \
		size_t i2 = i & 0x07;																 \
        __m128 d, s1, s2;                                                                    \
																						     \
		i >>= 3;                                                                             \
        if( ( ( size_t )src1 | ( size_t )src2 | ( size_t )dst ) & 0xf ){                     \
			while( i-- ) {                                                                   \
                s1 = _mm_loadu_ps( src1 );                                                   \
                s2 = _mm_loadu_ps( src2 );                                                   \
                d = sseop( s1, s2 );														 \
                _mm_storeu_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
                src2 += 4;                                                                   \
                s1 = _mm_loadu_ps( src1 );                                                   \
                s2 = _mm_loadu_ps( src2 );                                                   \
                d = sseop( s1, s2 );														 \
                _mm_storeu_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
                src2 += 4;                                                                   \
                s1 = _mm_loadu_ps( src1 );                                                   \
                s2 = _mm_loadu_ps( src2 );                                                   \
                d = sseop( s1, s2 );														 \
                _mm_storeu_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
                src2 += 4;                                                                   \
                s1 = _mm_loadu_ps( src1 );                                                   \
                s2 = _mm_loadu_ps( src2 );                                                   \
                d = sseop( s1, s2 );														 \
                _mm_storeu_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
                src2 += 4;                                                                   \
                s1 = _mm_loadu_ps( src1 );                                                   \
                s2 = _mm_loadu_ps( src2 );                                                   \
                d = sseop( s1, s2 );														 \
                _mm_storeu_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
                src2 += 4;                                                                   \
                s1 = _mm_loadu_ps( src1 );                                                   \
                s2 = _mm_loadu_ps( src2 );                                                   \
                d = sseop( s1, s2 );														 \
                _mm_storeu_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
                src2 += 4;                                                                   \
                s1 = _mm_loadu_ps( src1 );                                                   \
                s2 = _mm_loadu_ps( src2 );                                                   \
                d = sseop( s1, s2 );														 \
                _mm_storeu_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
                src2 += 4;                                                                   \
																							 \
                s1 = _mm_loadu_ps( src1 );                                                   \
                s2 = _mm_loadu_ps( src2 );                                                   \
                d = sseop( s1, s2 );														 \
                _mm_storeu_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
                src2 += 4;                                                                   \
            }                                                                                \
                                                                                             \
            while( i2-- ) {                                                                  \
                s1 = _mm_loadu_ps( src1 );                                                   \
                s2 = _mm_loadu_ps( src2 );                                                   \
                d = sseop( s1, s2 );														 \
                _mm_storeu_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
                src2 += 4;                                                                   \
            }                                                                                \
        } else {                                                                             \
			  while( i-- ) {                                                                 \
                s1 = _mm_load_ps( src1 );                                                    \
                s2 = _mm_load_ps( src2 );                                                    \
                d = sseop( s1, s2 );														 \
                _mm_stream_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
                src2 += 4;                                                                   \
                s1 = _mm_load_ps( src1 );                                                    \
                s2 = _mm_load_ps( src2 );                                                    \
                d = sseop( s1, s2 );														 \
                _mm_stream_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
                src2 += 4;                                                                   \
                s1 = _mm_load_ps( src1 );                                                    \
                s2 = _mm_load_ps( src2 );                                                    \
                d = sseop( s1, s2 );														 \
                _mm_stream_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
                src2 += 4;                                                                   \
                s1 = _mm_load_ps( src1 );                                                    \
                s2 = _mm_load_ps( src2 );                                                    \
                d = sseop( s1, s2 );														 \
                _mm_stream_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
                src2 += 4;                                                                   \
                s1 = _mm_load_ps( src1 );                                                    \
                s2 = _mm_load_ps( src2 );                                                    \
                d = sseop( s1, s2 );														 \
                _mm_stream_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
                src2 += 4;                                                                   \
                s1 = _mm_load_ps( src1 );                                                    \
                s2 = _mm_load_ps( src2 );                                                    \
                d = sseop( s1, s2 );														 \
                _mm_stream_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
                src2 += 4;                                                                   \
                s1 = _mm_load_ps( src1 );                                                    \
                s2 = _mm_load_ps( src2 );                                                    \
                d = sseop( s1, s2 );														 \
                _mm_stream_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
                src2 += 4;                                                                   \
                                                                                             \
                s1 = _mm_load_ps( src1 );                                                    \
                s2 = _mm_load_ps( src2 );                                                    \
                d = sseop( s1, s2 );														 \
                _mm_stream_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
                src2 += 4;                                                                   \
            }                                                                                \
                                                                                             \
            while( i2-- ) {                                                                  \
                s1 = _mm_load_ps( src1 );                                                    \
                s2 = _mm_load_ps( src2 );                                                    \
                d = sseop( s1, s2 );														 \
                _mm_stream_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
                src2 += 4;                                                                   \
            }                                                                                \
        }                                                                                    \
                                                                                             \
		i = n & 0x03;                                                                        \
		while( i-- )                                                                         \
			*dst++ = *src1++ cop *src2++;                                                    \
}


SSE_AAOP1_FLOAT( Add, _mm_add_ps, + )
SSE_AAOP1_FLOAT( Sub, _mm_sub_ps, - )
SSE_AAOP1_FLOAT( Mul, _mm_mul_ps, * )
SSE_AAOP1_FLOAT( Div, _mm_div_ps, / )

#define SSE_ACOP1_FLOAT( name, sseop, cop ) \
void SIMDSSE::name( float* dst, const float* src1, const float value, const size_t n ) const \
{                                                                                            \
		size_t i = n >> 2;                                                                   \
		size_t i2 = i & 0x07;																 \
        __m128 d, s1;														                 \
		const __m128 v = _mm_set1_ps( value );												 \
																						     \
		i >>= 3;                                                                             \
        if( ( ( size_t )src1 | ( size_t )dst ) & 0xf ) {				                     \
			while( i-- ) {                                                                   \
				s1 = _mm_loadu_ps( src1 );                                                   \
				d = sseop( s1, v );															 \
				_mm_storeu_ps( dst, d );                                                     \
                                                                                             \
				dst += 4;                                                                    \
				src1 += 4;                                                                   \
				s1 = _mm_loadu_ps( src1 );                                                   \
				d = sseop( s1, v );															 \
				_mm_storeu_ps( dst, d );                                                     \
                                                                                             \
				dst += 4;                                                                    \
				src1 += 4;                                                                   \
				s1 = _mm_loadu_ps( src1 );                                                   \
				d = sseop( s1, v );															 \
				_mm_storeu_ps( dst, d );                                                     \
                                                                                             \
				dst += 4;                                                                    \
				src1 += 4;                                                                   \
				s1 = _mm_loadu_ps( src1 );                                                   \
				d = sseop( s1, v );															 \
				_mm_storeu_ps( dst, d );                                                     \
                                                                                             \
				dst += 4;                                                                    \
				src1 += 4;                                                                   \
				s1 = _mm_loadu_ps( src1 );                                                   \
				d = sseop( s1, v );															 \
				_mm_storeu_ps( dst, d );                                                     \
                                                                                             \
				dst += 4;                                                                    \
				src1 += 4;                                                                   \
				s1 = _mm_loadu_ps( src1 );                                                   \
				d = sseop( s1, v );															 \
				_mm_storeu_ps( dst, d );                                                     \
                                                                                             \
				dst += 4;                                                                    \
				src1 += 4;                                                                   \
				s1 = _mm_loadu_ps( src1 );                                                   \
				d = sseop( s1, v );															 \
				_mm_storeu_ps( dst, d );                                                     \
                                                                                             \
				dst += 4;                                                                    \
				src1 += 4;                                                                   \
				s1 = _mm_loadu_ps( src1 );                                                   \
				d = sseop( s1, v );															 \
				_mm_storeu_ps( dst, d );                                                     \
                                                                                             \
				dst += 4;                                                                    \
				src1 += 4;                                                                   \
			}                                                                                \
                                                                                             \
            while( i2-- ) {                                                                  \
                s1 = _mm_loadu_ps( src1 );                                                   \
                d = sseop( s1, v );															 \
                _mm_storeu_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
            }                                                                                \
        } else {                                                                             \
			  while( i-- ) {                                                                 \
                s1 = _mm_load_ps( src1 );                                                    \
                d = sseop( s1, v );															 \
                _mm_stream_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
                s1 = _mm_load_ps( src1 );                                                    \
                d = sseop( s1, v );															 \
                _mm_stream_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
                s1 = _mm_load_ps( src1 );                                                    \
                d = sseop( s1, v );															 \
                _mm_stream_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
                s1 = _mm_load_ps( src1 );                                                    \
                d = sseop( s1, v );															 \
                _mm_stream_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
                s1 = _mm_load_ps( src1 );                                                    \
                d = sseop( s1, v );															 \
                _mm_stream_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
                s1 = _mm_load_ps( src1 );                                                    \
                d = sseop( s1, v );															 \
                _mm_stream_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
                s1 = _mm_load_ps( src1 );                                                    \
                d = sseop( s1, v );															 \
                _mm_stream_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
                s1 = _mm_load_ps( src1 );                                                    \
                d = sseop( s1, v );															 \
                _mm_stream_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
            }                                                                                \
                                                                                             \
            while( i2-- ) {                                                                  \
                s1 = _mm_load_ps( src1 );                                                    \
                d = sseop( s1, v );															 \
                _mm_stream_ps( dst, d );                                                     \
                                                                                             \
                dst += 4;                                                                    \
                src1 += 4;                                                                   \
            }                                                                                \
        }                                                                                    \
                                                                                             \
		i = n & 0x03;                                                                        \
		while( i-- )                                                                         \
			*dst++ = *src1++ cop value;	                                                     \
}

SSE_ACOP1_FLOAT( AddValue1f, _mm_add_ps, + )
SSE_ACOP1_FLOAT( SubValue1f, _mm_sub_ps, - )
SSE_ACOP1_FLOAT( MulValue1f, _mm_mul_ps, * )
SSE_ACOP1_FLOAT( DivValue1f, _mm_div_ps, / )


#define SSE_ACOP1_AOP2_FLOAT( name, sseop1, cop1, sseop2, cop2 ) \
	void SIMDSSE::name( float* dst, float const* src1, const float value, const size_t n ) const  \
	{                                                                                             \
        if( value == 0.0f )                                                                       \
            return;                                                                               \
                                                                                                  \
        size_t i = n >> 2;                                                                        \
		size_t i2 = i & 0x7;                                                                      \
        __m128 v = _mm_set1_ps( value );                                                          \
        __m128 d, s;                                                                              \
                                                                                                  \
		i = i >> 3;																				  \
        if( ( ( size_t )src1 | ( size_t )dst ) & 0xf ){                                           \
			while( i-- ){                                                                         \
				d = _mm_loadu_ps( dst );                                                          \
				s = _mm_loadu_ps( src1 );                                                         \
				s = sseop1( s, v );                                                               \
				d = sseop2( d, s );                                                               \
                                                                                                  \
				_mm_storeu_ps( dst, d );                                                          \
                                                                                                  \
				dst  += 4;                                                                        \
				src1 += 4;                                                                        \
				d = _mm_loadu_ps( dst );                                                          \
				s = _mm_loadu_ps( src1 );                                                         \
				s = sseop1( s, v );                                                               \
				d = sseop2( d, s );                                                               \
                                                                                                  \
				_mm_storeu_ps( dst, d );                                                          \
                                                                                                  \
				dst  += 4;                                                                        \
				src1 += 4;                                                                        \
				d = _mm_loadu_ps( dst );                                                          \
				s = _mm_loadu_ps( src1 );                                                         \
				s = sseop1( s, v );                                                               \
				d = sseop2( d, s );                                                               \
                                                                                                  \
				_mm_storeu_ps( dst, d );                                                          \
                                                                                                  \
				dst  += 4;                                                                        \
				src1 += 4;                                                                        \
				d = _mm_loadu_ps( dst );                                                          \
				s = _mm_loadu_ps( src1 );                                                         \
				s = sseop1( s, v );                                                               \
				d = sseop2( d, s );                                                               \
                                                                                                  \
				_mm_storeu_ps( dst, d );                                                          \
                                                                                                  \
				dst  += 4;                                                                        \
				src1 += 4;                                                                        \
				d = _mm_loadu_ps( dst );                                                          \
				s = _mm_loadu_ps( src1 );                                                         \
				s = sseop1( s, v );                                                               \
				d = sseop2( d, s );                                                               \
                                                                                                  \
				_mm_storeu_ps( dst, d );                                                          \
                                                                                                  \
				dst  += 4;                                                                        \
				src1 += 4;                                                                        \
				d = _mm_loadu_ps( dst );                                                          \
				s = _mm_loadu_ps( src1 );                                                         \
				s = sseop1( s, v );                                                               \
				d = sseop2( d, s );                                                               \
                                                                                                  \
				_mm_storeu_ps( dst, d );                                                          \
                                                                                                  \
				dst  += 4;                                                                        \
				src1 += 4;                                                                        \
				d = _mm_loadu_ps( dst );                                                          \
				s = _mm_loadu_ps( src1 );                                                         \
				s = sseop1( s, v );                                                               \
				d = sseop2( d, s );                                                               \
                                                                                                  \
				_mm_storeu_ps( dst, d );                                                          \
                                                                                                  \
				dst  += 4;                                                                        \
				src1 += 4;                                                                        \
                                                                                                  \
				d = _mm_loadu_ps( dst );                                                          \
				s = _mm_loadu_ps( src1 );                                                         \
				s = sseop1( s, v );                                                               \
				d = sseop2( d, s );                                                               \
                                                                                                  \
				_mm_storeu_ps( dst, d );                                                          \
                                                                                                  \
				dst  += 4;                                                                        \
				src1 += 4;                                                                        \
			}                                                                                     \
                                                                                                  \
            while( i2-- ){                                                                        \
                d = _mm_loadu_ps( dst );                                                          \
                s = _mm_loadu_ps( src1 );                                                         \
                s = sseop1( s, v );                                                               \
                d = sseop2( d, s );                                                               \
                                                                                                  \
                _mm_storeu_ps( dst, d );                                                          \
                                                                                                  \
                dst  += 4;                                                                        \
                src1 += 4;                                                                        \
            }                                                                                     \
        } else {                                                                                  \
            while( i-- ){                                                                         \
                d = _mm_load_ps( dst );                                                           \
                s = _mm_load_ps( src1 );                                                          \
                s = sseop1( s, v );                                                               \
                d = sseop2( d, s );                                                               \
                                                                                                  \
                _mm_store_ps( dst, d );                                                           \
                                                                                                  \
                dst  += 4;                                                                        \
                src1 += 4;                                                                        \
                d = _mm_load_ps( dst );                                                           \
                s = _mm_load_ps( src1 );                                                          \
                s = sseop1( s, v );                                                               \
                d = sseop2( d, s );                                                               \
                                                                                                  \
                _mm_store_ps( dst, d );                                                           \
                                                                                                  \
                dst  += 4;                                                                        \
                src1 += 4;                                                                        \
                d = _mm_load_ps( dst );                                                           \
                s = _mm_load_ps( src1 );                                                          \
                s = sseop1( s, v );                                                               \
                d = sseop2( d, s );                                                               \
                                                                                                  \
                _mm_store_ps( dst, d );                                                           \
                                                                                                  \
                dst  += 4;                                                                        \
                src1 += 4;                                                                        \
                                                                                                  \
                d = _mm_load_ps( dst );                                                           \
                s = _mm_load_ps( src1 );                                                          \
                s = sseop1( s, v );                                                               \
                d = sseop2( d, s );                                                               \
                                                                                                  \
                _mm_store_ps( dst, d );                                                           \
                                                                                                  \
                dst  += 4;                                                                        \
                src1 += 4;                                                                        \
                d = _mm_load_ps( dst );                                                           \
                s = _mm_load_ps( src1 );                                                          \
                s = sseop1( s, v );                                                               \
                d = sseop2( d, s );                                                               \
                                                                                                  \
                _mm_store_ps( dst, d );                                                           \
                                                                                                  \
                dst  += 4;                                                                        \
                src1 += 4;                                                                        \
                d = _mm_load_ps( dst );                                                           \
                s = _mm_load_ps( src1 );                                                          \
                s = sseop1( s, v );                                                               \
                d = sseop2( d, s );                                                               \
                                                                                                  \
                _mm_store_ps( dst, d );                                                           \
                                                                                                  \
                dst  += 4;                                                                        \
                src1 += 4;                                                                        \
                d = _mm_load_ps( dst );                                                           \
                s = _mm_load_ps( src1 );                                                          \
                s = sseop1( s, v );                                                               \
                d = sseop2( d, s );                                                               \
                                                                                                  \
                _mm_store_ps( dst, d );                                                           \
                                                                                                  \
                dst  += 4;                                                                        \
                src1 += 4;                                                                        \
                                                                                                  \
                d = _mm_load_ps( dst );                                                           \
                s = _mm_load_ps( src1 );                                                          \
                s = sseop1( s, v );                                                               \
                d = sseop2( d, s );                                                               \
                                                                                                  \
                _mm_store_ps( dst, d );                                                           \
                                                                                                  \
                dst  += 4;                                                                        \
                src1 += 4;                                                                        \
            }                                                                                     \
			while( i2-- ){                                                                        \
                d = _mm_load_ps( dst );                                                           \
                s = _mm_load_ps( src1 );                                                          \
                s = sseop1( s, v );                                                               \
                d = sseop2( d, s );                                                               \
                                                                                                  \
                _mm_store_ps( dst, d );                                                           \
                                                                                                  \
                dst  += 4;                                                                        \
                src1 += 4;                                                                        \
            }                                                                                     \
                                                                                                  \
        }                                                                                         \
                                                                                                  \
        i = n & 0x03;                                                                             \
        while( i-- ){                                                                             \
            *dst = *dst cop2 *src1 cop1 value;                                                    \
            dst++; src1++;                                                                        \
        }                                                                                         \
	}                                                                                             \

SSE_ACOP1_AOP2_FLOAT( MulAddValue1f, _mm_mul_ps, *, _mm_add_ps, + )
SSE_ACOP1_AOP2_FLOAT( MulSubValue1f, _mm_mul_ps, *, _mm_sub_ps, - )

	void SIMDSSE::Conv_GRAYALPHAf_to_GRAYf( float* dst, const float* src, const size_t n ) const
	{
		__m128 a, b;
		size_t i = n >> 2;

		if( ( ( size_t ) dst | ( size_t ) src ) & 0xF ) {
			while( i-- ) {
				a = _mm_loadu_ps( src );
				b = _mm_loadu_ps( src + 4 );
				a = _mm_shuffle_ps( a, b, _MM_SHUFFLE( 2, 0, 2, 0 ) );
				_mm_storeu_ps( dst, a  );
				src += 8;
				dst += 4;
			}
		} else {
			while( i-- ) {
				a = _mm_load_ps( src );
				b = _mm_load_ps( src + 4 );
				a = _mm_shuffle_ps( a, b, _MM_SHUFFLE( 2, 0, 2, 0 ) );
				_mm_stream_ps( dst, a  );
				src += 8;
				dst += 4;
			}
		}

		i = n & 0x03;
		while( i-- ) {
			*dst++ = *src++;
			src++;
		}
	}

	void SIMDSSE::Conv_XYZAf_to_ZYXAf( float* dst, float const* src, const size_t n ) const
	{
		size_t i = n;
		__m128 x;

		if( ( ( size_t ) dst | ( size_t ) src ) & 0xF ) {
			while( i-- ) {
				x = _mm_loadu_ps( src );
				x = _mm_shuffle_ps( x, x, _MM_SHUFFLE( 3, 0, 1, 2 ) );
				_mm_storeu_ps( dst, x  );
				src += 4;
				dst += 4;
			}
		} else {
			while( i-- ) {
				x = _mm_load_ps( src );
				x = _mm_shuffle_ps( x, x, _MM_SHUFFLE( 3, 0, 1, 2 ) );
				_mm_stream_ps( dst, x  );
				src += 4;
				dst += 4;
			}
		}
	}


	void SIMDSSE::warpLinePerspectiveBilinear4f( float* dst, const float* _src, size_t srcStride, size_t srcWidth, size_t srcHeight, const float* point, const float* normal, const size_t n ) const
	{
		const uint8_t* src = ( const uint8_t* ) _src;
		float px, py, pz;
		size_t i = n;
		__m128 r, v1, v2, v3, v4, ax1, ax2, ay1, ay2, one;

		one = _mm_set1_ps( 1.0f );

		px = point[ 0 ];
		py = point[ 1 ];
		pz = point[ 2 ];

		while( i-- )
		{
			float fx, fy;

			fx = px / pz;
			fy = py / pz;

			ax1 = _mm_set1_ps( fx + 1 - ( float ) ( int )( fx + 1 ) );
			ax2 = _mm_sub_ps( one, ax1 );
			ay1 = _mm_set1_ps( fy + 1 - ( float ) ( int )( fy + 1 ) );
			ay2 = _mm_sub_ps( one, ay1 );

#define VAL( fx, fy ) ( ( fx ) >= 0 && ( fx ) < ( int ) srcWidth && ( fy ) >= 0 && ( fy ) < ( int ) srcHeight ) ? _mm_loadu_ps( ( float* ) ( src + srcStride * ( fy ) + sizeof( float ) * ( ( fx ) * 4 ) ) ) : _mm_loadu_ps( dst );

			int lx = -1 + ( int )( fx + 1 );
			int ly = -1 + ( int )( fy + 1 );

			v3 = VAL( lx, ly );
			v4 = VAL( lx + 1, ly );
			v1 = _mm_add_ps( _mm_mul_ps( v3, ax2 ), _mm_mul_ps( v4, ax1 ) );
			v3 = VAL( lx, ly + 1 );
			v4 = VAL( lx + 1, ly + 1 );
			v2 = _mm_add_ps( _mm_mul_ps( v3, ax2 ), _mm_mul_ps( v4, ax1 ) );
			r = _mm_add_ps( _mm_mul_ps( v1, ay2 ), _mm_mul_ps( v2, ay1 ) );
			_mm_storeu_ps( dst, r );
			dst += 4;

			px += normal[ 0 ];
			py += normal[ 1 ];
			pz += normal[ 2 ];
#undef VAL
		}

	}

#define BOXFILTER_PREFIXSUM1F_ONCE()												\
																	\
	/* once */														\
	r1 = _mm_loadu_ps( A );											\
	r2 = _mm_loadu_ps( B );											\
	r3 = _mm_loadu_ps( C );											\
	r4 = _mm_loadu_ps( D );											\
																	\
	r1 = _mm_sub_ps( r1, r2 );	/* r1 = a - b */					\
	r1 = _mm_add_ps( r1, r3 );	/* r1 = a - b + c */				\
	r1 = _mm_sub_ps( r1, r4 );	/* r1 = a - b + c - d */			\
	r1 = _mm_mul_ps( r1, r5 );	/* r1 = (a - b + c - d) / scale; */	\
																	\
	_mm_storeu_ps(dst, r1);											\
																	\
	dst += 4;														\
	A += 4;															\
	B += 4;															\
	C += 4;															\
	D += 4;															\

	static inline void boxFilterLineInternal1_f_to_f( float *dst, const float *A, const float *B, const float *C, const float *D, float scale, size_t n )
	{
		__m128 r1, r2, r3, r4;
		const __m128 r5 = _mm_set1_ps( scale );

		// loop unroll: process 8 blocks of 4 floats at a time
		size_t i = n >> 5;
		while( i-- ) {
			BOXFILTER_PREFIXSUM1F_ONCE();
			BOXFILTER_PREFIXSUM1F_ONCE();
			BOXFILTER_PREFIXSUM1F_ONCE();
			BOXFILTER_PREFIXSUM1F_ONCE();

			BOXFILTER_PREFIXSUM1F_ONCE();
			BOXFILTER_PREFIXSUM1F_ONCE();
			BOXFILTER_PREFIXSUM1F_ONCE();
			BOXFILTER_PREFIXSUM1F_ONCE();
		}

		// process up to 4 blocks of 4 floats
		i = (n >> 2) & 0x07;
		while ( i-- ) {
			BOXFILTER_PREFIXSUM1F_ONCE();
		}

		i = n & 0x03;
		while ( i-- )
		{
			*dst++ = ( *A++ - *B++ - *D++ + *C++ ) * scale;
		}
	}

	void SIMDSSE::boxFilterPrefixSum1_f_to_f( float* dst, size_t dststride, const float* src, size_t srcstride, size_t width, size_t height, size_t boxwidth, size_t boxheight ) const
	{
		// FIXME
		srcstride >>= 2;
		dststride >>= 2;

		size_t x;
		size_t y;
		size_t boxwr = boxwidth >> 1;
		size_t boxhr = boxheight >> 1;
		size_t hend = height - boxhr;
		size_t wend = width - boxwr;
		const float* A = src + srcstride * boxhr + boxwr;
		const float* B = src + boxwr;
		const float* C = src;
		const float* D = src + srcstride * boxhr;
		const float scale = 1.0f / ( boxwidth * boxheight );

		for( y = 0; y <= boxhr; y++ ) {
			for( x = 0; x <= boxwr; x++ ) {
				dst[ x ] = A[ x ] / ( float )( ( boxwr + 1 + x ) * ( boxhr + 1 + y ) );
			}
			for( ; x < wend; x++ ) {
				dst[ x ] = ( A[ x ] - D[ x - ( boxwr + 1 ) ]  ) / ( float )( boxwidth * ( boxhr + 1 + y ) );
			}
			for( ; x < width; x++ ) {
				dst[ x ] = ( A[ width - 1 - boxwr ] - D[ x - ( boxwr + 1 ) ]  ) / ( float )( ( boxwr + 1 + ( width - 1 - x ) ) * ( boxhr + 1 + y ) );
			}
			A += srcstride;
			D += srcstride;
			dst += dststride;
		}

		for( ; y < hend; y++ ) {
			for( x = 0; x <= boxwr; x++ ) {
				dst[ x ] = ( A[ x ] - B[ x ] ) / ( float )( ( boxwr + 1 + x) * ( boxheight ) );
			}
			boxFilterLineInternal1_f_to_f( dst + x, A + x, B + x, C, D, scale, wend - x );
			x = wend;
			for( ; x < width; x++ ) {
				dst[ x ] = ( A[ width - 1 - boxwr ] - D[ x - ( boxwr + 1 )] - B[ width - 1 - boxwr ] + C[ x - ( boxwr + 1) ]  ) / ( float ) ( ( boxwr + 1 + ( width - 1 - x ) ) * boxheight );
			}

			A += srcstride;
			B += srcstride;
			C += srcstride;
			D += srcstride;
			dst += dststride;
		}

		A -= srcstride;
		D -= srcstride;

		for( ; y < height; y++ ) {
			for( x = 0; x <= boxwr; x++ ) {
				dst[ x ] = ( A[ x ] - B[ x ] ) / ( float )( ( boxwr + 1 + x) * ( boxhr + 1 + ( height - 1 - y ) ) );
			}
			boxFilterLineInternal1_f_to_f( dst + x, A + x, B + x, C + x - ( boxwr + 1 ), D + x - ( boxwr + 1 ), 1.0 / ( boxwidth * ( boxhr + 1 + ( height - 1 - y ) ) ), wend - x );
			x = wend;
			for( ; x < width; x++ ) {
				dst[ x ] = ( A[ width - 1 - boxwr ] - D[ x - ( boxwr + 1 )] - B[ width - 1 - boxwr ] + C[ x - ( boxwr + 1) ]  ) / ( float ) ( ( boxwr + 1 + ( width - 1 - x ) ) * ( boxhr + 1 + ( height - 1 - y ) ) );
			}


			B += srcstride;
			C += srcstride;
			dst += dststride;
		}

	}

	void SIMDSSE::Memcpy( uint8_t* dst, uint8_t const* src, const size_t n ) const
	{
		size_t n2 = n >> 4;
		__m128i tmp;

		if( ( ( size_t ) src | ( size_t ) dst ) & 0xf ) {
			while( n2-- ) {
				tmp = _mm_loadu_si128( ( __m128i* ) src );
				_mm_storeu_si128( ( __m128i* ) dst, tmp );
				src += 16;
				dst += 16;
			}
		} else {
			while( n2-- ) {
				tmp = _mm_load_si128( ( __m128i* ) src );
				_mm_stream_si128( ( __m128i* ) dst, tmp );
				src += 16;
				dst += 16;
			}
		}
		n2 = n & 0xf;
		while( n2-- )
			*dst++ = *src++;
	}
}
