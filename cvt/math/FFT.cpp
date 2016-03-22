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

#include <cvt/math/FFT.h>

namespace cvt {

	template<typename T>
	void FFT::fftRadix2( Complex<T>* data, size_t n, bool backward )
	{
		size_t i, k, l, klen, lstep;
		size_t mbit;
		float phi, tmp, dir = -1.0f;
		Complex<T> ctmp, ctmp2, cw, cwr;

		if( n & ( n - 1 ) )
			throw CVTException("data not power of 2!");

		if( backward )
			dir = 1.0f;

		/* do the bit reversal
		   zero and n-1 hold their position */
		mbit = n >> 1; /* highest bit occuring */
		k = mbit;
		for( i = 1 ; i < n - 1; i++) {
			if ( i < k ) {
				ctmp = data[ i ];
				data[ i ] = data[ k ];
				data[ k ] = ctmp;
			}

			/* add 1, but from the left */
			l = mbit;
			/* while highest bit set */
			while ( k & l ) {
				/* unset: k &= ( l - 1 );
				   but we use k -= l; since there is no higher bit than l -> we save one op
				 */
				k -= l;
				l >>= 1;
			}
			/* set the highest bit not set*/
			k |= l;
		}

		/* compute the butterfly */
		for( klen = 1; klen < n; klen = lstep ) {
			lstep = klen << 1;
			phi = dir * ( Math::PI / ( float ) klen );
			tmp = Math::sin( 0.5f * phi );
			cwr.set( -2.0f * tmp * tmp, sin( phi ) );
			cw.set( 1.0f, 0.0f );
			for( k = 0; k < klen; k++ ) {
				/*
				   We use a trigonometric recurrence outisde of the loop to calculate
				   cw.re = cos( M_PI * ( float ) dir * ( float ) k / ( float ) klen );
				   cw.im = sin( M_PI * ( float ) dir * ( float ) k / ( float ) klen );
				 */
				for( l = k; l < n; l += lstep ) {
					ctmp = data[ l ];
					ctmp2 = data[ l + klen ] * cw;
					data[ l ] += ctmp2;
					ctmp -= ctmp2;
					data[ l + klen ] = ctmp;
				}
				ctmp = cw;
				ctmp *= cwr;
				cw += ctmp;
			}
		}

		if( backward ) {
			tmp = 1.0f / ( float ) n;
			for( i = 0 ; i < n; i++)
				data[ i ] *= tmp;
		}
	}

	template void FFT::fftRadix2<float>( Complex<float>* data, size_t n, bool backward );
	template void FFT::fftRadix2<double>( Complex<double>* data, size_t n, bool backward );

	template<typename T>
	void FFT::fftStridedRadix2( Complex<T>* data, size_t n, size_t stride, bool backward )

	{
		size_t i, k, l, klen, lstep;
		size_t mbit;
		float phi, tmp, dir = -1.0f;
		Complex<T> ctmp, ctmp2, cw, cwr;

#define DATAN( x ) ( data[ ( x ) * stride ] )

		if( n & ( n - 1 ) )
			throw CVTException("data not power of 2!");

		if( backward )
			dir = 1.0f;

		/* do the bit reversal
		   zero and n-1 hold their position */
		mbit = n >> 1; /* highest bit occuring */
		k = mbit;
		for( i = 1 ; i < n - 1; i++) {
			if ( i < k ) {
				ctmp = DATAN( i );
				DATAN( i ) = DATAN( k );
				DATAN( k ) = ctmp;
			}

			/* add 1, but from the left */
			l = mbit;
			/* while highest bit set */
			while ( k & l ) {
				/* unset: k &= ( l - 1 );
				   but we use k -= l; since there is no higher bit than l -> we save one op
				 */
				k -= l;
				l >>= 1;
			}
			/* set the highest bit not set*/
			k |= l;
		}

		/* compute the butterfly */
		for( klen = 1; klen < n; klen = lstep ) {
			lstep = klen << 1;
			phi = dir * ( Math::PI / ( float ) klen );
			tmp = Math::sin( 0.5f * phi );
			cwr.set( -2.0f * tmp * tmp, sin( phi ) );
			cw.set( 1.0f, 0.0f );
			for( k = 0; k < klen; k++ ) {
				/*
				   We use a trigonometric recurrence outisde of the loop to calculate
				   cw.re = cos( M_PI * ( float ) dir * ( float ) k / ( float ) klen );
				   cw.im = sin( M_PI * ( float ) dir * ( float ) k / ( float ) klen );
				 */
				for( l = k; l < n; l += lstep ) {
					ctmp = DATAN( l );
					ctmp2 = DATAN( l + klen ) * cw;
					DATAN( l ) += ctmp2;
					ctmp -= ctmp2;
					DATAN( l + klen ) = ctmp;
				}
				ctmp = cw;
				ctmp *= cwr;
				cw += ctmp;
			}
		}

		if( backward ) {
			tmp = 1.0f / ( float ) n;
			for( i = 0 ; i < n; i++)
				DATAN( i ) *= tmp;
		}

#undef DATAN
	}

	template void FFT::fftStridedRadix2<float>( Complex<float>* data, size_t n, size_t stride, bool backward );
	template void FFT::fftStridedRadix2<double>( Complex<double>* data, size_t n, size_t stride, bool backward );



}
