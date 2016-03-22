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

/*
 * File:   SIMDSSSE3.cpp
 * Author: sebi
 *
 * Created on August 1, 2011, 8:48 AM
 */

#include <cvt/util/SIMDSSSE3.h>

#include <tmmintrin.h>
#include <xmmintrin.h>

namespace cvt {
	void SIMDSSSE3::Conv_XYZAu8_to_ZYXAu8( uint8_t* _dst, uint8_t const* _src, const size_t n ) const
	{
		uint32_t* src = ( uint32_t* ) _src;
		uint32_t* dst = ( uint32_t* ) _dst;
		uint32_t tmp1, tmp2;
		__m128i x;
		__m128i mask = _mm_set_epi8( 0xf, 0xc, 0xd, 0xe,
									 0xb, 0x8, 0x9, 0xa,
									 0x7, 0x4, 0x5, 0x6,
									 0x3, 0x0, 0x1, 0x2 );

		size_t i = n >> 2;

		if( ( ( size_t ) dst | ( size_t ) src ) & 0xF ) {
			while( i-- ) {
				x = _mm_loadu_si128( ( __m128i* )src );
				x = _mm_shuffle_epi8( x, mask );
				_mm_storeu_si128( ( __m128i* ) dst, x );
				src += 4;
				dst += 4;
			}
		} else {
			while( i-- ) {
				x = _mm_load_si128( ( __m128i* ) src );
				x = _mm_shuffle_epi8( x, mask );
				_mm_stream_si128( ( __m128i* ) dst, x );
				src += 4;
				dst += 4;
			}
		}

		i = n & 0x3;
		while( i-- ) {
			tmp1 = *src++;
			tmp2 = ( tmp1 & 0xff00ff00 );
			tmp2 += ( tmp1 & 0xff0000 ) >> 16;
			tmp2 += ( tmp1 & 0xff ) << 16;
			*dst++ = tmp2;
		}
	}


    size_t SIMDSSSE3::hammingDistance(const uint8_t* src1, const uint8_t* src2, size_t n) const
	{
		size_t bitcount = 0;

		static const uint8_t __attribute__((aligned( 16 ))) LUT[ 16 ] = { 0, 1, 1, 2,
																	   	  1, 2, 2, 3,
																		  1, 2, 2, 3,
																		  2, 3, 3, 4 };

		__m128i s1, s2, xored, highNibbles, lowNibbles, zero, sum1, sum2, sum, sumUp1, sumUp2;

		const __m128i mask = _mm_set1_epi8( 0x0f );
		const __m128i lut  = _mm_loadu_si128( ( __m128i* )LUT );
        zero = _mm_setzero_si128();


		size_t n16 = n >> 4;
		size_t r   = n & 0xf;

        sum1 = sumUp1 = zero;
        sum2 = sumUp2 = zero;

        // calculate the number of 31 blocks we need to process
        // n = 16 * 31 * x + r -> x / 496 + 1
        size_t numBlocks = n / ( 16 * 63 ) + 1;


		size_t num;
        while( numBlocks-- ){
			num = 63;
            while( n16 && num ){
                s1 = _mm_loadu_si128( ( __m128i* )src1 );
                s2 = _mm_loadu_si128( ( __m128i* )src2 );

                // xor:
                xored = _mm_xor_si128( s1, s2 );

                // get the high and low nibbles:
                lowNibbles = _mm_and_si128( xored, mask );
                highNibbles = _mm_and_si128( _mm_srli_epi64( xored, 4 ), mask );

                // access the LUT
                sum1 = _mm_add_epi8( sum1, _mm_shuffle_epi8( lut, lowNibbles ) );
                sum2 = _mm_add_epi8( sum2, _mm_shuffle_epi8( lut, highNibbles ) );

                src1 += 16;
                src2 += 16;
                n16--;
                num--;
            }

			// each 31 loops, accumulate the result into bitcount to
            // avoid overflow
            sumUp1 = _mm_add_epi64( sumUp1, _mm_sad_epu8( sum1, zero ) );
            sumUp2 = _mm_add_epi64( sumUp2, _mm_sad_epu8( sum2, zero ) );

            /*
            sum = _mm_sad_epu8( sum1, zero );
            sum = _mm_add_epi64( sum, _mm_sad_epu8( sum2, zero ) );
            sum = _mm_add_epi64( _mm_srli_si128( sum, 8 ), sum );
            bitcount += ( ( uint64_t* )( &sum ) )[ 0 ];
            */
            sum1 = sum2 = zero;
        }

		sum = _mm_add_epi64( sumUp1, sumUp2 );
		sum = _mm_add_epi64( _mm_srli_si128( sum, 8 ), sum );

		uint64_t tmp;
		_mm_storel_epi64( ( __m128i* ) &tmp, sum );
        bitcount += tmp;

        if( r ){
            uint64_t a = 0, b = 0;
            uint64_t xored;

            Memcpy( ( uint8_t* )( &a ), src1, r );
			Memcpy( ( uint8_t* )( &b ), src2, r );

            xored = ( a^b );
            xored = ( ( xored & 0xAAAAAAAAAAAAAAAAll ) >> 1 ) + ( xored & 0x5555555555555555ll );
            xored = ( ( xored & 0xCCCCCCCCCCCCCCCCll ) >> 2 ) + ( xored & 0x3333333333333333ll );
            xored = ( ( xored & 0xF0F0F0F0F0F0F0F0ll ) >> 4 ) + ( xored & 0x0F0F0F0F0F0F0F0Fll );
            xored += xored >> 32;
            xored += xored >> 16;
            xored += xored >>  8;
            //  1 8-bit number
            bitcount += ( xored & 0xFF );
        }

		return bitcount;
	}


    void SIMDSSSE3::BSwap16( uint16_t* dst, const uint16_t* src, size_t size ) const
    {
		__m128i x;
        __m128i mask = _mm_set_epi8( 14, 15, 12, 13, 10, 11, 8, 9, 6, 7, 4, 5, 2, 3, 0, 1 );

        size_t i = size >> 3;
		if( ( ( size_t ) dst | ( size_t ) src ) & 0xF ) {
			while( i-- ) {
				x = _mm_loadu_si128( ( __m128i* ) src );
				x = _mm_shuffle_epi8( x, mask );
				_mm_storeu_si128( ( __m128i* ) dst, x );
				src += 8;
				dst += 8;
			}
		} else {
			while( i-- ) {
				x = _mm_load_si128( ( __m128i* ) src );
				x = _mm_shuffle_epi8( x, mask );
				_mm_stream_si128( ( __m128i* ) dst, x );
				src += 8;
				dst += 8;
			}
		}


        i = size & 0x07;
        while( i-- ) {
            uint16_t tmp = *src++;
            *dst++ = ( ( tmp << 8) & 0xff00 ) | ( ( tmp >> 8 ) & 0x00ff );
        }
    }

    void SIMDSSSE3::BSwap32( uint32_t* dst, const uint32_t* src, size_t size ) const
    {
		__m128i x;
        __m128i mask = _mm_set_epi8( 12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3 );

        size_t i = size >> 2;
		if( ( ( size_t ) dst | ( size_t ) src ) & 0xF ) {
			while( i-- ) {
				x = _mm_loadu_si128( ( __m128i* ) src );
				x = _mm_shuffle_epi8( x, mask );
				_mm_storeu_si128( ( __m128i* ) dst, x );
				src += 4;
				dst += 4;
			}
		} else {
			while( i-- ) {
				x = _mm_load_si128( ( __m128i* ) src );
				x = _mm_shuffle_epi8( x, mask );
				_mm_stream_si128( ( __m128i* ) dst, x );
				src += 4;
				dst += 4;
			}
		}


        i = size & 0x03;
        while( i-- ) {
            uint32_t tmp = *src++;
            *dst++ =  ( ( tmp << 24 ) & 0xff000000 ) |
                      ( ( tmp <<  8 ) & 0x00ff0000 ) |
                      ( ( tmp >>  8 ) & 0x0000ff00 ) |
                      ( ( tmp >> 24 ) & 0x000000ff );
        }
    }

    void SIMDSSSE3::BSwap64( uint64_t* dst, const uint64_t* src, size_t size ) const
    {
		__m128i x;
        __m128i mask = _mm_set_epi8( 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7 );

        size_t i = size >> 1;
		if( ( ( size_t ) dst | ( size_t ) src ) & 0xF ) {
			while( i-- ) {
				x = _mm_loadu_si128( ( __m128i* ) src );
				x = _mm_shuffle_epi8( x, mask );
				_mm_storeu_si128( ( __m128i* ) dst, x );
				src += 2;
				dst += 2;
			}
		} else {
			while( i-- ) {
				x = _mm_load_si128( ( __m128i* ) src );
				x = _mm_shuffle_epi8( x, mask );
				_mm_stream_si128( ( __m128i* ) dst, x );
				src += 2;
				dst += 2;
			}
		}


        if( size & 0x01 ) {
             uint64_t tmp = *src++;
            *dst++ = ( ( tmp << 56 ) & 0xff00000000000000UL ) |
                     ( ( tmp << 40 ) & 0x00ff000000000000UL ) |
                     ( ( tmp << 24 ) & 0x0000ff0000000000UL ) |
                     ( ( tmp <<  8 ) & 0x000000ff00000000UL ) |
                     ( ( tmp >>  8 ) & 0x00000000ff000000UL ) |
                     ( ( tmp >> 24 ) & 0x0000000000ff0000UL ) |
                     ( ( tmp >> 40 ) & 0x000000000000ff00UL ) |
                     ( ( tmp >> 56 ) & 0x00000000000000ffUL );
        }
    }

}

