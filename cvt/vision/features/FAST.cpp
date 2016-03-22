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

#include <cvt/vision/features/FAST.h>
#include <cvt/gfx/IScaleFilter.h>

namespace cvt
{

	FAST::FAST( FASTSize size, uint8_t threshold, size_t border ) :
        _fastSize( size ),
		_threshold( threshold )
	{
		setBorder( border );
	}

	FAST::~FAST()
	{
	}

	void FAST::detect( FeatureSet& featureset, const Image& img )
	{
		if( img.format() != IFormat::GRAY_UINT8 )
			throw CVTException( "Input Image format must be GRAY_UINT8" );

		FeatureSetWrapper features( featureset );

		switch ( _fastSize ) {
			case SEGMENT_9:
				detect9( img, _threshold, features, _border );
				break;
			case SEGMENT_10:
				detect10( img, _threshold, features, _border );
				break;
			case SEGMENT_11:
				detect11( img, _threshold, features, _border );
				break;
			case SEGMENT_12:
				detect12( img, _threshold, features, _border );
				break;
			default:
				throw CVTException( "Unkown FAST size" );
				break;
		}
	}

	void FAST::detect( FeatureSet& featureset, const ImagePyramid& imgpyr )
	{
		if( imgpyr[ 0 ].format() != IFormat::GRAY_UINT8 )
			throw CVTException( "Input Image format must be GRAY_UINT8" );

		for( size_t coctave = 0; coctave < imgpyr.octaves(); coctave++ ) {
			float cscale = Math::pow( imgpyr.scaleFactor(), -( float )coctave );
			FeatureSetWrapper features( featureset, cscale, coctave );

			switch ( _fastSize ) {
				case SEGMENT_9:
					detect9( imgpyr[ coctave ], _threshold, features, _border );
					break;
				case SEGMENT_10:
					detect10( imgpyr[ coctave ], _threshold, features, _border );
					break;
				case SEGMENT_11:
					detect11( imgpyr[ coctave ], _threshold, features, _border );
					break;
				case SEGMENT_12:
					detect12( imgpyr[ coctave ], _threshold, features, _border );
					break;
				default:
					throw CVTException( "Unkown FAST size" );
					break;
			}
		}
	}

	inline void FAST::detect9( const Image& img, uint8_t threshold, FeatureSetWrapper& features, size_t border )
	{
		// check the cpu flags to determine the right version
		CPUFeatures cpu = cpuFeatures();
		if( cpu & CPU_SSE2 ){
			detect9simd( img, threshold, features, border );
			//detect9cpu( img, threshold, features, border );
		} else {
			detect9cpu( img, threshold, features, border );
		}
	}

	inline void FAST::detect9cpu( const Image& img, uint8_t threshold, FeatureSetWrapper& features, size_t border )
	{
		size_t stride;
		const uint8_t * im = img.map( &stride );

		size_t x, y;
		size_t xsize = img.width() - border;
		size_t ysize = img.height() - border;

		int offsets[ 16 ];
		make_offsets( offsets, stride );

		for( y=border; y < ysize; y++ ){
			for( x=border; x < xsize; x++ ){
				const uint8_t* p = im + y*stride + x;

				if( isCorner9( p, offsets, threshold ) )
					features( x, y, score9Pixel( p, offsets, threshold ) );
			}
		}
		img.unmap( im );
	}

	inline void FAST::detect9simd( const Image& img, uint8_t threshold, FeatureSetWrapper& features, size_t border )
	{
	#define CHECK_BARRIER(lo, hi, other, flags)		\
		{                                               \
			__m128i diff = _mm_subs_epu8(lo, other);	\
			__m128i diff2 = _mm_subs_epu8(other, hi);	\
			__m128i z = _mm_setzero_si128();			\
			diff = _mm_cmpeq_epi8(diff, z);				\
			diff2 = _mm_cmpeq_epi8(diff2, z);			\
			flags = ~(_mm_movemask_epi8(diff) | (_mm_movemask_epi8(diff2) << 16)); \
		}

		size_t stride;
		const uint8_t * iptr = img.map( &stride );

		int offsets[ 16 ];
		make_offsets( offsets, stride );

		const size_t tripleStride = 3 * stride;

		// The compiler refuses to reserve a register for this
		const __m128i barriers = _mm_set1_epi8( threshold  );

		// xend is the beginning of the last pixels in the row that need to be processed in the normal way
		size_t width = img.width();
		size_t height = img.height();
		size_t xend = width - border - ( width - border ) % 16;
		size_t aligned_start = ( (int)( border / 16 ) + 1 ) << 4;


		const uint8_t* im = iptr;
		im += ( border * stride );
		const uint8_t * ptr;

		for ( size_t y = border; y < height - border; y++ ) {
			ptr = im + border;
			for ( size_t x = border; x < aligned_start; x++ ){
				if( isCorner9( ptr, offsets, threshold ) )
					features( x, y, score9Pixel( ptr, offsets, threshold ) );
				ptr++;
			}

			for ( size_t x = aligned_start; x < xend; x += 16, ptr += 16 ) {
				__m128i lo, hi;
				{
					const __m128i here = _mm_load_si128( (const __m128i*)ptr );
					lo = _mm_subs_epu8( here, barriers );
					hi = _mm_adds_epu8( here, barriers );
				}



				uint32_t ans_0, ans_8, possible;
				{
					__m128i top = _mm_load_si128( ( const __m128i* )( ptr - tripleStride ) );
					__m128i bottom = _mm_load_si128( ( const __m128i* )( ptr + tripleStride ) );

					CHECK_BARRIER( lo, hi, top, ans_0 );
					CHECK_BARRIER( lo, hi, bottom, ans_8 );

					possible = ans_0 | ans_8;

					if ( !possible ){
						continue;
					}
				}

				uint32_t ans_15, ans_1;
				{
					__m128i a = _mm_loadu_si128( ( const __m128i* )( ptr - 1 - tripleStride ) );
					__m128i c = _mm_insert_epi16( _mm_srli_si128( a, 2 ), *( const uint16_t* ) (ptr + 15 - tripleStride), 7 );
					CHECK_BARRIER( lo, hi, a, ans_15 );
					CHECK_BARRIER( lo, hi, c, ans_1 );
					// 8 or (15 and 1 )
					possible &= ans_8 | (ans_15 & ans_1);

					if ( !possible )
						continue;
				}

				uint32_t ans_9, ans_7;
				{
					__m128i d = _mm_loadu_si128( ( const __m128i* )( ptr - 1 + tripleStride ) );
					__m128i f = _mm_insert_epi16( _mm_srli_si128( d, 2 ), *( const uint16_t* )( ptr + 15 + tripleStride ), 7 );
					CHECK_BARRIER( lo, hi, d, ans_9 );
					CHECK_BARRIER( lo, hi, f, ans_7 );
					possible &= ans_9 | ( ans_0 & ans_1 );
					possible &= ans_7 | ( ans_15 & ans_0 );

					if ( !possible )
						continue;
				}

				uint32_t ans_12, ans_4;
				{
					__m128i left = _mm_loadu_si128( ( const __m128i* )( ptr - 3 ) );
					__m128i right = _mm_loadu_si128( ( const __m128i* )( ptr + 3 ) );
					CHECK_BARRIER( lo, hi, left, ans_12 );
					CHECK_BARRIER( lo, hi, right, ans_4 );
					possible &= ans_12 | ( ans_4 & ( ans_1 | ans_7 ) );
					possible &= ans_4 | ( ans_12 & ( ans_9 | ans_15 ) );

					if ( !possible )
						continue;
				}

				uint32_t ans_14, ans_6;
				{
					__m128i ul = _mm_loadu_si128( ( const __m128i* ) ( ptr - 2 - 2 * stride ) );
					__m128i lr = _mm_loadu_si128( ( const __m128i* ) ( ptr + 2 + 2 * stride ) );
					CHECK_BARRIER( lo, hi, ul, ans_14 );
					CHECK_BARRIER( lo, hi, lr, ans_6 );
					{
						const unsigned int ans_6_7 = ans_6 & ans_7;
						possible &= ans_14 | (ans_6_7 & (ans_4 | (ans_8 & ans_9)));
						possible &= ans_1 | (ans_6_7) | ans_12;
					}
					{
						const unsigned int ans_14_15 = ans_14 & ans_15;
						possible &= ans_6 | (ans_14_15 & (ans_12 | (ans_0 & ans_1)));
						possible &= ans_9 | (ans_14_15) | ans_4;
					}

					if ( !possible )
						continue;
				}

				uint32_t ans_10, ans_2;
				{
					__m128i ll = _mm_loadu_si128( ( const __m128i* ) (ptr - 2 + 2 * stride) );
					__m128i ur = _mm_loadu_si128( ( const __m128i* ) (ptr + 2 - 2 * stride) );
					CHECK_BARRIER( lo, hi, ll, ans_10 );
					CHECK_BARRIER( lo, hi, ur, ans_2 );
					{
						const unsigned int ans_1_2 = ans_1 & ans_2;
						possible &= ans_10 | (ans_1_2 & ((ans_0 & ans_15) | ans_4));
						possible &= ans_12 | (ans_1_2) | (ans_6 & ans_7);
					}
					{
						const unsigned int ans_9_10 = ans_9 & ans_10;
						possible &= ans_2 | (ans_9_10 & ((ans_7 & ans_8) | ans_12));
						possible &= ans_4 | (ans_9_10) | (ans_14 & ans_15);
					}
					possible &= ans_8 | ans_14 | ans_2;
					possible &= ans_0 | ans_10 | ans_6;

					if ( !possible )
						continue;
				}

				uint32_t ans_13, ans_5;
				{
					__m128i g = _mm_loadu_si128( ( const __m128i* ) (ptr - 3 - stride ) );
					__m128i l = _mm_loadu_si128( ( const __m128i* ) (ptr + 3 + stride ) );
					CHECK_BARRIER( lo, hi, g, ans_13 );
					CHECK_BARRIER( lo, hi, l, ans_5 );
					const uint32_t ans_15_0 = ans_15 & ans_0;
					const uint32_t ans_7_8 = ans_7 & ans_8;
					{
						const uint32_t ans_12_13 = ans_12 & ans_13;
						possible &= ans_5 | (ans_12_13 & ans_14 & ((ans_15_0) | ans_10));
						possible &= ans_7 | (ans_1 & ans_2) | (ans_12_13);
						possible &= ans_2 | (ans_12_13) | (ans_7_8);
					}
					{
						const uint32_t ans_4_5 = ans_4 & ans_5;
						const uint32_t ans_9_10 = ans_9 & ans_10;
						possible &= ans_13 | (ans_4_5 & ans_6 & ((ans_7_8) | ans_2));
						possible &= ans_15 | (ans_4_5) | (ans_9_10);
						possible &= ans_10 | (ans_4_5) | (ans_15_0);
						possible &= ans_15 | (ans_9_10) | (ans_4_5);
					}

					possible &= ans_8 | (ans_13 & ans_14) | ans_2;
					possible &= ans_0 | (ans_5 & ans_6) | ans_10;

					if ( !possible )
						continue;
				}


				uint32_t ans_11, ans_3;
				{
					__m128i ii = _mm_loadu_si128( ( const __m128i* )( ptr - 3 + stride ) );
					__m128i jj = _mm_loadu_si128( ( const __m128i* )( ptr + 3 - stride ) );
					CHECK_BARRIER( lo, hi, ii, ans_11 );
					CHECK_BARRIER( lo, hi, jj, ans_3 );
					{
						const uint32_t ans_2_3 = ans_2 & ans_3;
						possible &= ans_11 | (ans_2_3 & ans_4 & ((ans_0 & ans_1) | (ans_5 & ans_6)));
						possible &= ans_13 | (ans_7 & ans_8) | (ans_2_3);
						possible &= ans_8 | (ans_2_3) | (ans_13 & ans_14);
					}
					{
						const uint32_t ans_11_12 = ans_11 & ans_12;
						possible &= ans_3 | (ans_10 & ans_11_12 & ((ans_8 & ans_9) | (ans_13 & ans_14)));
						possible &= ans_1 | (ans_11_12) | (ans_6 & ans_7);
						possible &= ans_6 | (ans_0 & ans_1) | (ans_11_12);
					}
					{
						const uint32_t ans_3_4 = ans_3 & ans_4;
						possible &= ans_9 | (ans_3_4) | (ans_14 & ans_15);
						possible &= ans_14 | (ans_8 & ans_9) | (ans_3_4);
					}
					{
						const uint32_t ans_10_11 = ans_10 & ans_11;
						possible &= ans_5 | (ans_15 & ans_0) | (ans_10_11);
						possible &= ans_0 | (ans_10_11) | (ans_5 & ans_6);
					}

					if ( !possible )
						continue;

				}

				possible |= (possible >> 16);

				//if(possible & 0x0f) //Does this make it faster?
				{
					if ( possible & (1 << 0) )
						features( x, y, score9Pixel( ptr, offsets, threshold ) );
					if ( possible & (1 << 1) )
						features( x + 1, y, score9Pixel( ptr + 1, offsets, threshold ) );
					if ( possible & (1 << 2) )
						features( x + 2, y, score9Pixel( ptr + 2, offsets, threshold ) );
					if ( possible & (1 << 3) )
						features( x + 3, y, score9Pixel( ptr + 3, offsets, threshold ) );
					if ( possible & (1 << 4) )
						features( x + 4, y, score9Pixel( ptr + 4, offsets, threshold ) );
					if ( possible & (1 << 5) )
						features( x + 5, y, score9Pixel( ptr + 5, offsets, threshold ) );
					if ( possible & (1 << 6) )
						features( x + 6, y, score9Pixel( ptr + 6, offsets, threshold ) );
					if ( possible & (1 << 7) )
						features( x + 7, y, score9Pixel( ptr + 7, offsets, threshold ) );
				}

				//if(possible & 0xf0) //Does this mak( ,  fast)r?
				{
					if ( possible & (1 << 8) )
						features( x + 8, y, score9Pixel( ptr + 8, offsets, threshold ) );
					if ( possible & (1 << 9) )
						features( x + 9, y, score9Pixel( ptr + 9, offsets, threshold ) );
					if ( possible & (1 << 10) )
						features( x + 10, y, score9Pixel( ptr + 10, offsets, threshold ) );
					if ( possible & (1 << 11) )
						features( x + 11, y, score9Pixel( ptr + 11, offsets, threshold ) );
					if ( possible & (1 << 12) )
						features( x + 12, y, score9Pixel( ptr + 12, offsets, threshold ) );
					if ( possible & (1 << 13) )
						features( x + 13, y, score9Pixel( ptr + 13, offsets, threshold ) );
					if ( possible & (1 << 14) )
						features( x + 14, y, score9Pixel( ptr + 14, offsets, threshold ) );
					if ( possible & (1 << 15) )
						features( x + 15, y, score9Pixel( ptr + 15, offsets, threshold ) );
				}
			}

			for ( size_t x = xend; x < width - border; x++ ){
				if( isCorner9( ptr, offsets, threshold ) )
					features( x, y, score9Pixel( ptr, offsets, threshold ) );
				ptr++;
			}
			im += stride;
		}
		img.unmap( iptr );

	#undef CHECK_BARRIER
	}

	inline void FAST::detect10( const Image& img, uint8_t threshold, FeatureSetWrapper& features, size_t border )
	{
		size_t stride;
		const uint8_t * im = img.map( &stride );

		size_t x, y;
		size_t xsize = img.width() - border;
		size_t ysize = img.height() - border;

		int offsets[ 16 ];
		make_offsets( offsets, stride );

		for( y=border; y < ysize; y++ ){
			for( x=border; x < xsize; x++ ){
				const uint8_t* p = im + y*stride + x;

				if( isCorner10( p, offsets, threshold ) )
					features( x, y, score10Pixel( p, offsets, threshold ) );
			}
		}
		img.unmap( im );
	}

	inline void FAST::detect11( const Image& img, uint8_t threshold, FeatureSetWrapper& features, size_t border )
	{
		size_t stride;
		const uint8_t * im = img.map( &stride );

		size_t x, y;
		size_t xsize = img.width() - border;
		size_t ysize = img.height() - border;

		int offsets[ 16 ];
		make_offsets( offsets, stride );

		for( y=border; y < ysize; y++ ){
			for( x=border; x < xsize; x++ ){
				const uint8_t* p = im + y*stride + x;

				if( isCorner11( p, offsets, threshold ) )
					features( x, y, score11Pixel( p, offsets, threshold ) );
			}
		}
		img.unmap( im );
	}

	inline void FAST::detect12( const Image& img, uint8_t threshold, FeatureSetWrapper& features, size_t border )
	{
		size_t stride;
		const uint8_t * im = img.map( &stride );

		size_t x, y;
		size_t xsize = img.width() - border;
		size_t ysize = img.height() - border;

		int offsets[ 16 ];
		make_offsets( offsets, stride );

		for( y=border; y < ysize; y++ ){
			for( x=border; x < xsize; x++ ){
				const uint8_t* p = im + y*stride + x;

				if( isCorner12( p, offsets, threshold ) )
					features( x, y, score12Pixel( p, offsets, threshold ) );
			}
		}
		img.unmap( im );
	}

}
