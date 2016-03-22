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

#include <cvt/gfx/IScaleFilter.h>
#include <cvt/math/Math.h>

namespace cvt {
	/* Blackman window */
	static float _filter_window_blackman( float x )
	{
		if( x < -1.0f ) return 0.0f;
		if( x > 1.0f ) return 0.0f;
		return 0.42f + 0.50f * Math::cos( Math::PI * x ) + 0.08f * Math::cos( 2.0f * Math::PI * x );
	}

	/* Blackman-Harris window */
	static float _filter_window_blackmanharris( float x )
	{
		if( x < -1.0f ) return 0.0f;
		if( x > 1.0f ) return 0.0f;
		return 0.35875f + 0.48829f * Math::cos( Math::PI * x ) + 0.14128f * Math::cos( 2.0f * Math::PI * x ) + 0.01168f * Math::cos( 3.0f * Math::PI * x );
	}

	/* Lanczos window */
	static float _filter_window_lanczos( float x )
	{
		if( x < -1.0f ) return 0.0f;
		if( x > 1.0f ) return 0.0f;
		return x == 0.0f ? 1.0f : Math::sin( Math::PI * x ) / ( Math::PI * x );
	}

	/* Hamming window */
	/*static float _filter_window_hamming( float x )
	{
		if( x < -1.0f ) return 0.0f;
		if( x > 1.0f ) return 0.0f;
		return 0.54f + 0.46f * Math::cos( Math::PI * x );
	}*/

	/* triangle filter, bilinear */
	static float _filter_triangle( float x )
	{
		if( x < -1.0f ) return 0.0f;
		if( x < 0.0f ) return 1.0f + x;
		if( x < 1.0f ) return 1.0f - x;
		return 0.0f;
	}

	static float _filter_gauss( float x, float sigma )
	{
		return Math::exp( -( ( Math::sqr( x ) ) / ( 2.0f * sigma * sigma ) ) ) / ( sigma * 2.506628275f );
	}

	/* 4th order (cubic) b-spline */
	static float _filter_cubic( float x )
	{
		float t;

		if( x < -2.0f ) return 0.0f;
		if( x < -1.0f ) { t = 2.0f + x; return ( t * t * t ) / 6.0f; }
		if( x < 0.0f ) return ( 4.0f + x * x * ( -6.0f + x * -3.0f ) ) / 6.0f;
		if( x < 1.0f ) return ( 4.0f + x * x * ( -6.0f + x * 3.0f ) ) / 6.0f;
		if( x < 2.0f ) { t = 2.0f - x ; return ( t * t * t ) / 6.0f; }
		return 0.0f;
	}

	/* Catmull-Rom spline, Overhauser spline */
	static float _filter_catrom( float x )
	{
		if( x < -2.0f ) return 0.0f;
		if( x < -1.0f ) return 0.5f * ( 4.0f + x * ( 8.0f + x * ( 5.0f + x ) ) );
		if( x < 0.0f ) return 0.5f * ( 2.0f + x * x * ( -5.0f + x * -3.0f ) );
		if( x < 1.0f ) return 0.5f * ( 2.0f + x * x * ( -5.0f + x * 3.0f ) );
		if( x < 2.0f ) return 0.5f * ( 4.0f + x * ( -8.0f + x * ( 5.0f - x ) ) );
		return 0.0f;
	}

	/* infinite sinc */
	static float _filter_sinc( float x )
	{
		return x == 0.0f ? 1.0f : Math::sin( Math::PI * x ) / ( Math::PI * x );
	}

	static float _filter_mitchell( float x )
	{
		float B = 1.0f / 3.0f;
		float C = B;

		x = Math::abs( x );
		if( x > 2.0f )
			return 0;

		if( x > 1.0f )
			return ( ( -B - 6.0f * C ) * x * x * x + ( 6.0f * B + 30.0f * C ) * x * x +
					( -12.0f * B - 48.0f * C ) * x + ( 8.0f * B + 24.0f * C ) ) * ( 1.0f / 6.0f );
		else
			return ( ( 12 - 9 * B - 6 * C ) * x * x * x +
					( -18 + 12 * B + 6 * C ) * x * x +
					( 6 - 2 * B ) ) * ( 1.0f / 6.0f );
	}


	size_t IScaleFilter::getAdaptiveConvolutionWeights( size_t dst, size_t src, IConvolveAdaptivef& conva, bool nonegincr ) const
	{
		IConvolveAdaptiveSize* sweights;
		float*  weights;
		uint32_t numweight;
		size_t i;
		ssize_t x1, x2, lx, x;
		float center = 0.0;
		float offset;
		float incr;
		float norm;
		float a;
		size_t support;
		size_t maxsupport = 0;

		/* set center to zero and substract 1 from src and dst to retain the borders while scaling */
		if( /*incr <= 1.0*/ dst >= src ) {  /* upscale */
			incr = ( float ) ( src )  / ( float ) ( dst );
			a = 1.0f;
			support = ( size_t ) Math::ceil( _support );
			offset = 0.5f * incr - 0.5f;
		} else { /* downscale */
			incr = ( float ) ( src )  / ( float ) ( dst );
			a = incr * 0.5f + 0.1f + _sharpsmooth; /* add a little bit blur to avoid the strongest aliasing artifacts */
			support = ( size_t ) Math::ceil( _support * incr * 0.5f );
			offset =  0.5f * incr - 0.5f;
		}

#define SRC2DST( n ) ( ( float ) ( n * src ) / ( float ) ( dst ) + offset  )

		conva.size = new IConvolveAdaptiveSize[ dst ];
		conva.weights = new float[ dst * support * 2 ];
		i = 0;
		sweights = conva.size;
		weights = conva.weights;
		lx = 0;

		/* border */
		while( ( x1 = ( ( ssize_t ) Math::ceil( center ) ) - support ) < 0 && i < dst ) {
			x2 = x1 + 2 * support;
			numweight = 0;
			norm = 0.0f;

			for( x = 0; x < x2 && x < ( ssize_t ) src; x++ )
				norm += eval( ( ( ( float ) x ) - center ) / a );

			sweights->incr = 0 - lx;
			lx = 0;

			for( x = 0; x < x2 && x < ( ssize_t ) src; x++ ) {
				*weights = ( eval( ( ( ( float ) x ) - center ) / a ) / norm );
				/* skip leading zeros */
				if( Math::abs( *weights ) >= Math::EPSILONF || numweight || nonegincr ) {
					weights++;
					numweight++;
				} else {
					sweights->incr++;
					lx++;
				}
			}
			/* remove trailing zeros */
			while( numweight && Math::abs( *( weights - 1 ) ) < Math::EPSILONF ) {
				numweight--;
				weights--;
			}

			if( numweight > maxsupport )
				maxsupport = numweight;

			sweights->numw = numweight;
			sweights++;
			i++;
			center = SRC2DST( i );
		}

		/* center */
		while( ( x2 = ( ( ssize_t ) Math::ceil( center ) ) + support ) <= ( ssize_t ) src && i < dst ) { /* x2 <= src, x2 is exclusive bound*/
			x1 = x2 - 2 * support;
			numweight = 0;
			norm = 0.0f;

			for( x = x1; x < x2; x++ )
				norm += eval( ( ( ( float ) x ) - center ) / a );

			sweights->incr = x1 - lx;
			lx = x1;

			for( x = x1; x < x2; x++ ) {
				*weights = eval( ( ( ( float ) x ) - center ) / a ) / norm;
				/* skip leading zeros */
				if( Math::abs( *weights ) >= Math::EPSILONF || numweight || nonegincr ) {
					weights++;
					numweight++;
				} else {
					sweights->incr++;
					lx++;
				}
			}
			/* remove trailing zeros */
			while( numweight && Math::abs( *( weights - 1 ) ) < Math::EPSILONF ) {
				numweight--;
				weights--;
			}

			if( numweight > maxsupport )
				maxsupport = numweight;

			sweights->numw = numweight;
			sweights++;
			i++;
			center = SRC2DST( i );
		}

		/* border2 */
		while( i < dst ) {
			x1 = ( ( ssize_t ) Math::ceil( center ) ) - support;
			numweight = 0;
			norm = 0.0f;

			for( x = x1; x < ( ssize_t ) src; x++ )
				norm += eval( ( ( ( float ) x ) - center ) / a );

			sweights->incr = x1 - lx;
			lx = x1;

			for( x = x1; x < ( ssize_t ) src; x++ ) {
				*weights = eval( ( ( ( float ) x ) - center ) / a ) / norm;
				/* skip leading zeros */
				if( Math::abs( *weights ) >= Math::EPSILONF || numweight || nonegincr ) {
					weights++;
					numweight++;
				} else {
					sweights->incr++;
					lx++;
				}
			}
			/* remove trailing zeros */
			while( numweight && Math::abs( *( weights - 1 ) ) < Math::EPSILONF ) {
				numweight--;
				weights--;
			}

			if( numweight > maxsupport )
				maxsupport = numweight;

			sweights->numw = numweight;
			sweights++;
			i++;
			center = SRC2DST( i );
		}
		return maxsupport;
	}


	size_t IScaleFilter::getAdaptiveConvolutionWeights( size_t dst, size_t src, IConvolveAdaptiveFixed& conva, bool nonegincr ) const
	{
		IConvolveAdaptiveSize* sweights;
		Fixed*  weights;
		uint32_t numweight;
		size_t i;
		ssize_t x1, x2, lx, x;
		float center = 0.0;
		float offset;
		float incr;
		float norm;
		float a;
		size_t support;
		size_t maxsupport = 0;
		
		/* set center to zero and substract 1 from src and dst to retain the borders while scaling */
		if( /*incr <= 1.0*/ dst >= src ) {  /* upscale */
			incr = ( float ) ( src )  / ( float ) ( dst );
			a = 1.0f;
			support = ( size_t ) Math::ceil( _support );
			offset = 0.5f * incr - 0.5f;
		} else { /* downscale */
			incr = ( float ) ( src )  / ( float ) ( dst );
			a = incr * 0.5f + 0.1f + _sharpsmooth; /* add a little bit blur to avoid the strongest aliasing artifacts */
			support = ( size_t ) Math::ceil( _support * incr * 0.5f );
			offset =  0.5f * incr - 0.5f;
		}
		
		conva.size = new IConvolveAdaptiveSize[ dst ];
		conva.weights = new Fixed[ dst * support * 2 ];
		i = 0;
		sweights = conva.size;
		weights = conva.weights;
		lx = 0;
		
		Fixed ZeroFixed = 0.0f;
		
		/* border */
		while( ( x1 = ( ( ssize_t ) Math::ceil( center ) ) - support ) < 0 && i < dst ) {
			x2 = x1 + 2 * support;
			numweight = 0;
			norm = 0.0f;
			
			for( x = 0; x < x2 && x < ( ssize_t ) src; x++ )
				norm += eval( ( ( ( float ) x ) - center ) / a );
			
			sweights->incr = 0 - lx;
			lx = 0;
			
			for( x = 0; x < x2 && x < ( ssize_t ) src; x++ ) {
				*weights = ( eval( ( ( ( float ) x ) - center ) / a ) / norm );
				/* skip leading zeros */
				if( *weights != ZeroFixed || numweight || nonegincr ) {
					weights++;
					numweight++;
				} else {
					sweights->incr++;
					lx++;
				}
			}
			/* remove trailing zeros */
			while( numweight && *( weights - 1 ) == ZeroFixed ) {
				numweight--;
				weights--;
			}
			
			if( numweight > maxsupport )
				maxsupport = numweight;
			
			sweights->numw = numweight;
			sweights++;
			i++;
			center = SRC2DST( i );
		}
		
		/* center */
		while( ( x2 = ( ( ssize_t ) Math::ceil( center ) ) + support ) <= ( ssize_t ) src && i < dst ) { /* x2 <= src, x2 is exclusive bound*/
			x1 = x2 - 2 * support;
			numweight = 0;
			norm = 0.0f;
			
			for( x = x1; x < x2; x++ )
				norm += eval( ( ( ( float ) x ) - center ) / a );
			
			sweights->incr = x1 - lx;
			lx = x1;
			
			for( x = x1; x < x2; x++ ) {
				*weights = eval( ( ( ( float ) x ) - center ) / a ) / norm;
				/* skip leading zeros */
				if( *weights != ZeroFixed || numweight || nonegincr ) {
					weights++;
					numweight++;
				} else {
					sweights->incr++;
					lx++;
				}
			}
			/* remove trailing zeros */
			while( numweight && *( weights - 1 ) == ZeroFixed ) {
				numweight--;
				weights--;
			}
			
			if( numweight > maxsupport )
				maxsupport = numweight;
			
			sweights->numw = numweight;
			sweights++;
			i++;
			center = SRC2DST( i );
		}
		
		/* border2 */
		while( i < dst ) {
			x1 = ( ( ssize_t ) Math::ceil( center ) ) - support;
			numweight = 0;
			norm = 0.0f;
			
			for( x = x1; x < ( ssize_t ) src; x++ )
				norm += eval( ( ( ( float ) x ) - center ) / a );
			
			sweights->incr = x1 - lx;
			lx = x1;
			
			for( x = x1; x < ( ssize_t ) src; x++ ) {
				*weights = eval( ( ( ( float ) x ) - center ) / a ) / norm;
				/* skip leading zeros */
				if( *weights != ZeroFixed || numweight || nonegincr ) {
					weights++;
					numweight++;
				} else {
					sweights->incr++;
					lx++;
				}
			}
			/* remove trailing zeros */
			while( numweight && *( weights - 1 ) == ZeroFixed ) {
				numweight--;
				weights--;
			}
			
			if( numweight > maxsupport )
				maxsupport = numweight;
			
			sweights->numw = numweight;
			sweights++;
			i++;
			center = SRC2DST( i );
		}
		return maxsupport;
	}

	float IScaleFilterBilinear::eval( float x ) const
	{
		return _filter_triangle( x );
	}

	float IScaleFilterCubic::eval( float x ) const
	{
		return _filter_cubic( x );
	}

	float IScaleFilterCatmullRom::eval( float x ) const
	{
		return _filter_catrom( x );
	}

	float IScaleFilterMitchell::eval( float x ) const
	{
		return _filter_mitchell( x );
	}

	float IScaleFilterLanczos::eval( float x ) const
	{
		return _filter_sinc( x ) * _filter_window_lanczos( x / _support );
	}

	float IScaleFilterBlackman::eval( float x ) const
	{
		return _filter_sinc( x ) * _filter_window_blackman( x / _support );
	}

	float IScaleFilterBlackmanHarris::eval( float x ) const
	{
		return _filter_sinc( x ) * _filter_window_blackmanharris( x / _support );
	}

	float IScaleFilterGauss::eval( float x ) const
	{
		return _filter_gauss( x, _support * 0.5f );
	}

}
