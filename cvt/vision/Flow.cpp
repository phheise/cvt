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

#include <cvt/vision/Flow.h>

namespace cvt {
	namespace Flow {

#define NUMCOLORS 55
		static float _colorwheel[ NUMCOLORS * 4 ] = {
			1.000000f, 0.000000f, 0.000000f, 1.0f,
			1.000000f, 0.066667f, 0.000000f, 1.0f,
			1.000000f, 0.133333f, 0.000000f, 1.0f,
			1.000000f, 0.200000f, 0.000000f, 1.0f,
			1.000000f, 0.266667f, 0.000000f, 1.0f,
			1.000000f, 0.333333f, 0.000000f, 1.0f,
			1.000000f, 0.400000f, 0.000000f, 1.0f,
			1.000000f, 0.466667f, 0.000000f, 1.0f,
			1.000000f, 0.533333f, 0.000000f, 1.0f,
			1.000000f, 0.600000f, 0.000000f, 1.0f,
			1.000000f, 0.666667f, 0.000000f, 1.0f,
			1.000000f, 0.733333f, 0.000000f, 1.0f,
			1.000000f, 0.800000f, 0.000000f, 1.0f,
			1.000000f, 0.866667f, 0.000000f, 1.0f,
			1.000000f, 0.933333f, 0.000000f, 1.0f,
			1.000000f, 1.000000f, 0.000000f, 1.0f,
			0.833333f, 1.000000f, 0.000000f, 1.0f,
			0.666667f, 1.000000f, 0.000000f, 1.0f,
			0.500000f, 1.000000f, 0.000000f, 1.0f,
			0.333333f, 1.000000f, 0.000000f, 1.0f,
			0.166667f, 1.000000f, 0.000000f, 1.0f,
			0.000000f, 1.000000f, 0.000000f, 1.0f,
			0.000000f, 1.000000f, 0.250000f, 1.0f,
			0.000000f, 1.000000f, 0.500000f, 1.0f,
			0.000000f, 1.000000f, 0.750000f, 1.0f,
			0.000000f, 1.000000f, 1.000000f, 1.0f,
			0.000000f, 0.909091f, 1.000000f, 1.0f,
			0.000000f, 0.818182f, 1.000000f, 1.0f,
			0.000000f, 0.727273f, 1.000000f, 1.0f,
			0.000000f, 0.636364f, 1.000000f, 1.0f,
			0.000000f, 0.545455f, 1.000000f, 1.0f,
			0.000000f, 0.454545f, 1.000000f, 1.0f,
			0.000000f, 0.363636f, 1.000000f, 1.0f,
			0.000000f, 0.272727f, 1.000000f, 1.0f,
			0.000000f, 0.181818f, 1.000000f, 1.0f,
			0.000000f, 0.090909f, 1.000000f, 1.0f,
			0.000000f, 0.000000f, 1.000000f, 1.0f,
			0.076923f, 0.000000f, 1.000000f, 1.0f,
			0.153846f, 0.000000f, 1.000000f, 1.0f,
			0.230769f, 0.000000f, 1.000000f, 1.0f,
			0.307692f, 0.000000f, 1.000000f, 1.0f,
			0.384615f, 0.000000f, 1.000000f, 1.0f,
			0.461538f, 0.000000f, 1.000000f, 1.0f,
			0.538462f, 0.000000f, 1.000000f, 1.0f,
			0.615385f, 0.000000f, 1.000000f, 1.0f,
			0.692308f, 0.000000f, 1.000000f, 1.0f,
			0.769231f, 0.000000f, 1.000000f, 1.0f,
			0.846154f, 0.000000f, 1.000000f, 1.0f,
			0.923077f, 0.000000f, 1.000000f, 1.0f,
			1.000000f, 0.000000f, 1.000000f, 1.0f,
			1.000000f, 0.000000f, 0.833333f, 1.0f,
			1.000000f, 0.000000f, 0.666667f, 1.0f,
			1.000000f, 0.000000f, 0.500000f, 1.0f,
			1.000000f, 0.000000f, 0.333333f, 1.0f,
			1.000000f, 0.000000f, 0.166667f, 1.0f };

		static bool _unknown_flow( float u, float v ) {
#define UFLOW 1e9f
			return ( Math::abs(u) > UFLOW )	|| ( Math::abs(v) > UFLOW ); //|| isnan(u) || isnan(v);
#undef UFLOW
		}

		float AEE( const Image& flow, const Image& gt )
		{
			if( flow.width() != gt.width() ||
			   flow.height() != gt.height() ||
			   flow.format() != IFormat::GRAYALPHA_FLOAT ||
			   gt.format() != IFormat::GRAYALPHA_FLOAT )			   
				throw CVTException( "FlowAAE: illegal flow data" );

			size_t stride1, stride2;
			size_t w, h;
			uint8_t const* ptr1;
			uint8_t const* ptr2;
			float ee1, ee2;
			float aee = 0.0f;
			size_t unknown = 0;
			uint8_t const* optr1;
			uint8_t const* optr2;

			w = gt.width();
			h = gt.height();
			optr1 = ptr1 = flow.map( &stride1 );
			optr2 = ptr2 = gt.map( &stride2 );


			while( h-- ) {
				float const* d1 = ( float const* ) ptr1;
				float const* d2 = ( float const* ) ptr2;

				for( size_t i = 0; i < w; i++ ) {
					if( !_unknown_flow( d2[ i * 2], d2[ i * 2 + 1 ] ) ) {
						ee1 = ( d1[ i * 2 ] - d2[ i * 2 ] );
						ee2 = ( d1[ i * 2 + 1 ] - d2[ i * 2 + 1 ] );
						aee += Math::sqrt( Math::sqr( ee1 ) + Math::sqr( ee2 ) );
					} else
						unknown++;
				}
				ptr1 += stride1;
				ptr2 += stride2;
			}
			flow.unmap( optr1 );
			gt.unmap( optr2 );
			return  aee / ( ( float ) ( gt.width() * gt.height() - unknown ) );
		}

		float AAE( const Image& flow, const Image& gt )
		{
			if( flow.width() != gt.width() ||
			   flow.height() != gt.height() ||
			   flow.format() != IFormat::GRAYALPHA_FLOAT ||
			   gt.format() != IFormat::GRAYALPHA_FLOAT )
				throw CVTException( "FlowAAE: illegal flow data" );

			size_t stride1, stride2;
			size_t w, h;
			uint8_t const* ptr1;
			uint8_t const* ptr2;
			float dot ,dot1, dot2, v;
			float aae = 0.0f;
			size_t unknown = 0;
			uint8_t const* optr1;
			uint8_t const* optr2;


			w = flow.width();
			h = flow.height();
			optr1 = ptr1 = flow.map( &stride1 );
			optr2 = ptr2 = gt.map( &stride2 );


			while( h-- ) {
				float const* d1 = ( float const* ) ptr1;
				float const* d2 = ( float const* ) ptr2;

				for( size_t i = 0; i < w; i++ ) {
					if( !_unknown_flow( d2[ i * 2], d2[ i * 2 + 1 ] ) ) {
						dot = 1.0f + ( d1[ i * 2 ] * d2[ i * 2 ] + d1[ i * 2 + 1 ] * d2[ i * 2 + 1 ]);
						dot1 = 1.0f + ( d1[ i * 2 ] * d1[ i * 2 ] + d1[ i * 2 + 1 ] * d1[ i * 2 + 1 ]);
						dot2 = 1.0f + ( d2[ i * 2 ] * d2[ i * 2 ] + d2[ i * 2 + 1 ] * d2[ i * 2 + 1 ]);
						v = ( dot / ( Math::sqrt( dot1 ) * Math::sqrt( dot2 ) ) );
						if( v <= 1.0f )
							aae += Math::acos( v );
					} else
						unknown++;
				}
				ptr1 += stride1;
				ptr2 += stride2;
			}
			flow.unmap( optr1 );
			gt.unmap( optr2 );

			return Math::rad2Deg( aae / ( ( float ) ( gt.width() * gt.height() - unknown ) ) );
		}

		void colorCode( Image& idst, Image& flow, float maxflow )
		{
			if( flow.format() != IFormat::GRAYALPHA_FLOAT )
				throw CVTException( "Illegal flow data" );

			uint8_t* dst;
			uint8_t* src;
			size_t stridedst;
			size_t stridesrc;
			float* pdst;
			float* psrc;
			size_t w, h;
			float u[ 2 ];
			float color[ 4 ];
			float radius;
			float angle;
			float val;
			float alpha;
			size_t i1, i2;
			uint8_t const* optr1;
			uint8_t const* optr2;

			idst.reallocate( flow.width(), flow.height(), IFormat::BGRA_FLOAT );

			optr1 = dst = idst.map( &stridedst );
			optr2 = src = flow.map( &stridesrc );

			h = idst.height();
			while( h-- ) {
				pdst = ( float* ) dst;
				psrc = ( float* ) src;

				w = idst.width();
				while( w-- ) {
					u[ 0 ] = *psrc++;
					u[ 1 ] = *psrc++;
					radius = Math::sqrt( Math::sqr( u[ 0 ] ) + Math::sqr( u[ 1 ] ) );
					angle = Math::atan2( -u[ 1 ], -u[ 0 ] ) / Math::PI;
					val = ( ( angle + 1.0f ) / 2.0f ) *  ( float ) ( NUMCOLORS );
					alpha = val - Math::floor( val );
					i1 = ( ( size_t ) val ) % NUMCOLORS;
					i2 = ( i1 + 1 ) % NUMCOLORS;
					color[ 2 ] = Math::mix( _colorwheel[ i1 * 4 ], _colorwheel[ i2 * 4 ], alpha  );
					color[ 1 ] = Math::mix( _colorwheel[ i1 * 4 + 1 ], _colorwheel[ i2 * 4 + 1 ], alpha  );
					color[ 0 ] = Math::mix( _colorwheel[ i1 * 4 + 2 ], _colorwheel[ i2 * 4 + 2 ], alpha  );
					color[ 3 ] = 1.0f; //Math::mix( _colorwheel[ i1 * 4 + 3 ], _colorwheel[ i2 * 4 + 3 ], alpha  );
					color[ 0 ] = Math::mix( 1.0f, color[ 0 ], Math::min( radius / maxflow, 1.0f ) );
					color[ 1 ] = Math::mix( 1.0f, color[ 1 ], Math::min( radius / maxflow, 1.0f ) );
					color[ 2 ] = Math::mix( 1.0f, color[ 2 ], Math::min( radius / maxflow, 1.0f ) );
					/* alpha remains */
					*pdst++ = color[ 0 ];
					*pdst++ = color[ 1 ];
					*pdst++ = color[ 2 ];
					*pdst++ = color[ 3 ];
				}

				dst += stridedst;
				src += stridesrc;
			}
			idst.unmap( optr1 );
			flow.unmap( optr2 );
		}

	}
}
