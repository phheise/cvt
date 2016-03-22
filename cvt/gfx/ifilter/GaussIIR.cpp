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

#include <cvt/gfx/ifilter/GaussIIR.h>
#include <cvt/math/Math.h>

#include <cvt/cl/CLContext.h>
#include <cvt/cl/kernel/gaussiir.h>
#include <cvt/cl/kernel/gaussiir2.h>
#include <cvt/math/Fixed.h>


namespace cvt {

	#define L1 ( _IIR[ order ][ 0 ][ 3 ] )
	#define L2 ( _IIR[ order ][ 1 ][ 3 ] )
	#define W1 ( _IIR[ order ][ 0 ][ 2 ] )
	#define W2 ( _IIR[ order ][ 1 ][ 2 ] )
	#define B1 ( _IIR[ order ][ 0 ][ 1 ] )
	#define B2 ( _IIR[ order ][ 1 ][ 1 ] )
	#define A1 ( _IIR[ order ][ 0 ][ 0 ] )
	#define A2 ( _IIR[ order ][ 1 ][ 0 ] )

	static const float _IIR[ 3 ][ 2 ][ 4 ] =
	{
		{
			{ 1.680f  ,  3.735f,  0.6318f, -1.783f },
			{ -0.6803f , -0.2598,  1.997f,  -1.723f },
		},
		{
			{ -0.6472f, -4.531f,  0.6719f, -1.527f },
			{ 0.6494f,  0.9557f, 2.072f,  -1.516f },
		},
		{
			{ -1.331f  ,  3.661f, 0.7480f, -1.240f },
			{ 0.3225f , -1.738f, 2.166f,  -1.314f }
		}
	};

	static void _GaussIIRCoeff_D( float sigma, int order, Vector4f& d, float& SD )
	{
		const float cw1 = Math::cos( W1 / sigma );
		const float cw2 = Math::cos( W2 / sigma );
		const float el1 = Math::exp( L1 / sigma );
		const float el2 = Math::exp( L2 / sigma );

		d[ 0 ] = -2.0f * ( el2 * cw2 + el1 * cw1 );
		d[ 1 ] = 4.0f * el1 * el2 * cw1 * cw2 + Math::sqr( el1 ) + Math::sqr( el2 );
		d[ 2 ] = -2.0f * el1 * Math::sqr( el2 ) * cw1 - 2.0f * Math::sqr( el1 ) * el2 * cw2;
		d[ 3 ] = Math::sqr( el1 * el2 );

		SD = 1.0f + d[ 0 ] + d[ 1 ] + d[ 2 ] + d[ 3 ];
	}

	static void _GaussIIRCoeff_N( float sigma, int order, Vector4f& n, float& SN )
	{
		const float cw1 = Math::cos( W1 / sigma );
		const float cw2 = Math::cos( W2 / sigma );
		const float sw1 = Math::sin( W1 / sigma );
		const float sw2 = Math::sin( W2 / sigma );
		const float el1 = Math::exp( L1 / sigma );
		const float el2 = Math::exp( L2 / sigma );

		n[ 0 ] = A1 + A2;
		n[ 1 ] = el2 * ( B2 * sw2 - ( A2 + 2.0f * A1 ) * cw2 ) + el1 * ( B1 * sw1 - ( A1 + 2.0f * A2 ) * cw1 );
		n[ 2 ] = 2.0f * el1 *el2 * ( ( A1 + A2 ) * cw2 * cw1 - B1 * cw2 * sw1 - B2 * cw1 * sw2  ) + A2 * Math::sqr( el1 ) + A1 * Math::sqr( el2 );
		n[ 3 ] = Math::sqr( el1 ) * el2 * ( B2 *sw2 - A2 * cw2 ) + el1 * Math::sqr( el2 ) * ( B1 * sw1 - A1 * cw1 );

		SN = n[ 0 ] + n[ 1 ] + n[ 2 ] + n[ 3 ];
	}

	static void _GaussIIRCoeff( float sigma, int order, Vector4f& n, Vector4f& m, Vector4f& d )
	{
		order = Math::clamp( order, 0, 2 );
		sigma = Math::max( sigma, 0.05f );
		float SN, SD;

		_GaussIIRCoeff_D( sigma, order, d, SD );
		_GaussIIRCoeff_N( sigma, order, n, SN );

		if( order == 0 ) {
			float alpha = 2.0f * SN / SD - n[ 0 ];
			n[ 0 ] = n[ 0 ] / alpha;
			n[ 1 ] = n[ 1 ] / alpha;
			n[ 2 ] = n[ 2 ] / alpha;
			n[ 3 ] = n[ 3 ] / alpha;
		} else if( order == 1 ) {
			float DN = n[ 1 ] + 2.0f * n[ 2 ] + 3.0f * n[ 3 ];
			float DD = d[ 0 ] + 2.0f * d[ 1 ] + 3.0f * d[ 2 ] + 4.0f * d[ 3 ];
			float alpha =  2.0f * ( SN * DD - DN * SD ) / Math::sqr( SD );
			n[ 0 ] = n[ 0 ] / alpha;
			n[ 1 ] = n[ 1 ] / alpha;
			n[ 2 ] = n[ 2 ] / alpha;
			n[ 3 ] = n[ 3 ] / alpha;
		} else {
			Vector4f n0, d0;
			float SN0, SD0, DN0;
			_GaussIIRCoeff_D( sigma, 0, d0, SD0 );
			_GaussIIRCoeff_N( sigma, 0, n0, SN0 );
			float DN = n[ 1 ] + 2.0f * n[ 2 ] + 3.0f * n[ 3 ];
			float DD = d[ 0 ] + 2.0f * d[ 1 ] + 3.0f * d[ 2 ] + 4.0f * d[ 3 ];
			float beta = -( 2.0f * SN - SD * n[ 0 ] ) / ( 2.0f * SN0 - SD * n0[ 0 ] );
			float EN = n[ 1 ] + 4.0f * n[ 2 ] + 9.0f * n[ 3 ];
			float EN0 = n0[ 1 ] + 4.0f * n0[ 2 ] + 9.0f * n0[ 3 ];
			float ED = d[ 0 ] + 4.0f * d[ 1 ] + 9.0f * d[ 3 ] + 16.0f * d[ 4 ];
		    float SN2 = SN + beta * SN0;
			float DN2 = DN + beta * DN0;
			float EN2 = EN + beta * EN0;
			float alpha = EN2 * Math::sqr( SD ) - ED * SN2 * SD - 2.0f * DN2 * DD * SD + 2.0f * Math::sqr( DD ) * SN;
			alpha /= SD * SD * SD;
			n[ 0 ] = ( n[ 0 ] + beta * n0[ 0 ] ) / alpha;
			n[ 1 ] = ( n[ 1 ] + beta * n0[ 1 ] ) / alpha;
			n[ 2 ] = ( n[ 2 ] + beta * n0[ 2 ] ) / alpha;
			n[ 3 ] = ( n[ 3 ] + beta * n0[ 3 ] ) / alpha;
		}

		float n0 = n[ 0 ];
		m[ 0 ] = n[ 1 ] - d[ 0 ] * n0;
		m[ 1 ] = n[ 2 ] - d[ 1 ] * n0;
		m[ 2 ] = n[ 3 ] - d[ 2 ] * n0;
		m[ 3 ] =		- d[ 3 ] * n0;

		if( order == 1 ) {
			m[ 0 ] = -m[ 0 ];
			m[ 1 ] = -m[ 1 ];
			m[ 2 ] = -m[ 2 ];
			m[ 3 ] = -m[ 3 ];
		}
	}

	static ParamInfoTyped<Image*> pin( "Input", true /* inputParam */ );
	static ParamInfoTyped<Image*> pout( "Output", false );
	static ParamInfoTyped<float> psigma( "Sigma", true );
	static ParamInfoTyped<int> porder( "Order", 0 /* min */, 2 /* max */, 0 /* default */, true );

	static ParamInfo* _params[ 4 ] = {
		&pin,
		&pout,
		&psigma,
		&porder
	};

	GaussIIR::GaussIIR() : IFilter( "GaussIIR", _params, 4, IFILTER_OPENCL ), _kernelIIR( 0 ), _kernelIIR2( 0 )
	{
	}

	GaussIIR::~GaussIIR()
	{
		if( _kernelIIR )
			delete _kernelIIR;
		if( _kernelIIR2 )
			delete _kernelIIR2;

	}

	void GaussIIR::apply( const ParamSet* set, IFilterType t ) const
	{
		Image * in = set->arg<Image*>( 0 );
		Image * out = set->arg<Image*>( 1 );
		float sigma = set->arg<float>( 2 );
		int   order = set->arg<int>( 3 );

		Vector4f n, m, d;
		_GaussIIRCoeff( sigma, order, n, m, d );

		switch ( t ) {
			case IFILTER_OPENCL:
				// TODO: check mem type to be opencl!
				this->applyOpenCL( *out, *in, n, m, d );
				break;
			case IFILTER_CPU:
				switch( in->format().type ){
					case IFORMAT_TYPE_FLOAT:
						this->applyCPUf( *out, *in, n, m, d );
						break;
					case IFORMAT_TYPE_UINT8:
						this->applyCPUu8( *out, *in, n, m, d );
						break;
					default:
						throw CVTException( "GaussIIR CPU not implemented for given Image format" );
						break;
				}
				break;
			default:
				throw CVTException( "Not implemented" );
		}
	}

	void GaussIIR::applyOpenCL( Image& dst, const Image& src, const Vector4f & n, const Vector4f & m, const Vector4f & d ) const
	{
		// TODO: THIS IS A HACK -> we change members of a const object!
		std::string log;
		if( _kernelIIR == 0 ){
			_kernelIIR = new CLKernel( _gaussiir_source, "gaussiir" );
		}

		if( _kernelIIR2 == 0 ){
			_kernelIIR2 = new CLKernel( _gaussiir2_source, "gaussiir2" );
		}

		int w = src.width();
		int h = src.height();


		CLBuffer buf( sizeof( cl_float4 ) * w * h );
		CLBuffer buf2( sizeof( cl_float4 ) * w * h );

		_kernelIIR->setArg( 0, buf );
		_kernelIIR->setArg( 1, src );
		_kernelIIR->setArg( 2, sizeof( int ), &w );
		_kernelIIR->setArg( 3, sizeof( int ), &h );
		_kernelIIR->setArg( 4, n );
		_kernelIIR->setArg( 5, m );
		_kernelIIR->setArg( 6, d );

		//std::cout << "W GCD( " << w << ", " << _kernelIIR->workGroupSize() << " ) = " << Math::gcd<size_t>( h, _kernelIIR->workGroupSize() ) << std::endl;
		_kernelIIR->run( CLNDRange( Math::pad16( h ) ), _kernelIIR->bestLocalRange1d( CLNDRange( Math::pad16( h ) ) ) );

		_kernelIIR2->setArg( 0, dst );
		_kernelIIR2->setArg( 1, buf );
		_kernelIIR2->setArg( 2, buf2 );
		_kernelIIR2->setArg( 3, sizeof( int ), &w );
		_kernelIIR2->setArg( 4, sizeof( int ), &h );
		_kernelIIR2->setArg( 5, n );
		_kernelIIR2->setArg( 6, m );
		_kernelIIR2->setArg( 7, d );
		//std::cout << "H GCD: " << Math::gcd<size_t>( w, _kernelIIR2->workGroupSize() ) << std::endl;
		_kernelIIR2->run( CLNDRange( Math::pad16( w ) ), _kernelIIR->bestLocalRange1d( CLNDRange( Math::pad16( w ) ) ) );
	}

	void GaussIIR::applyCPUf( Image& dst, const Image& src, const Vector4f & n, const Vector4f & m, const Vector4f & d ) const
	{
		float b1 = ( n[ 0 ] + n[ 1 ] + n[ 2 ] + n[ 3 ] ) / ( d[ 0 ] + d[ 1 ] + d[ 2 ] + d[ 3 ] + 1.0f );
		float b2 = ( m[ 0 ] + m[ 1 ] + m[ 2 ] + m[ 3 ] ) / ( d[ 0 ] + d[ 1 ] + d[ 2 ] + d[ 3 ] + 1.0f );

		size_t sstride, dstride;
		const float * inTmp = src.map<float>( &sstride );
		float * outTmp = dst.map<float>( &dstride );
		const float * in = inTmp;
		float * out = outTmp;

		uint32_t h = dst.height();
		uint32_t w = dst.width();
		uint32_t channels = dst.channels();

		/* last four values */
		const float* x;
		float* y;

		float tmpLine[ channels * w ];
		float * tmpBuf = tmpLine + channels * ( w - 1 );

		// horizontal pass
		for( uint32_t k = 0; k < h; k++ ){
			x = in;
			y = out;

			for( uint32_t c = 0; c < channels; c++ ){
				y[ c ] = n[ 0 ] * x[ c ] + n[ 1 ] * x[ c ] + n[ 2 ] * x[ c ] + n[ 3 ] * x[ c ]
				- b1 * ( d[ 0 ] * x[ c ] + d[ 1 ] * x[ c ] + d[ 2 ] * x[ c ] + d[ 3 ] * x[ c ] );

				y[ c + channels  ] = n[ 0 ] * x[ c + channels ] + n[ 1 ] * x[ c ] + n[ 2 ] * x[ c ] + n[ 3 ] * x[ c ]
				- d[ 0 ] * y[ c ] - b1 * ( d[ 1 ] * x[ c ] + d[ 2 ] * x[ c ] + d[ 3 ] * x[ c ] );

				y[ c + 2 * channels ] = n[ 0 ] * x[ c + 2 * channels ] + n[ 1 ] * x[ c + channels ] + n[ 2 ] * x[ c ] + n[ 3 ] * x[ c ]
				- d[ 0 ] * y[ c + channels ] - d[ 1 ] * y[ c ] - b1 * (  d[ 2 ] * x[ c ] + d[ 3 ] * x[ c ] );

				y[ c + 3 * channels ] = n[ 0 ] * x[ c + 3 * channels ] + n[ 1 ] * x[ c + 2 * channels ] + n[ 2 ] * x[ c + channels ] + n[ 3 ] * x[ c ]
				- d[ 0 ] * y[ c + 2 * channels ] - d[ 1 ] * y[ c + channels ] - d[ 2 ] * y[ c ] - b1 * d[ 3 ] * x[ c ];
			}

			// forward pass
			for( uint32_t i = 4; i < w; i++ ){
				x+=channels; /* last 4 pixels, x[ 3*channels ] is current pixel */

				for( uint32_t c = 0; c < channels; c++ ){
					y[ 4 * channels + c ] = n[ 0 ] * x[ 3 * channels + c ] + n[ 1 ] * x[ 2 * channels + c ] + n[ 2 ] * x[ channels + c ] + n[ 3 ] * x[ c ]
					- d[ 0 ] * y[ 3 * channels + c ] - d[ 1 ] * y[ 2 * channels + c ] - d[ 2 ] * y[ channels + c ] - d[ 3 ] * y[ c ];
				}
				y+=channels;
			}

			// backward border init:
			x = ( in + channels * ( w - 1 ) );
			y = ( out + channels * ( w - 1 ) );
			for( uint32_t c = 0; c < channels; c++ ){
				tmpBuf[ c ] = m[ 0 ] * x[ c ] + m[ 1 ] * x[ c ] + m[ 2 ] * x[ c ] + m[ 3 ] * x[ c ]
				- b2 * ( d[ 0 ] * x[ c ] + d[ 1 ] * x[ c ] + d[ 2 ] * x[ c ] + d[ 3 ] * x[ c ] );

				*( tmpBuf - channels + c ) = m[ 0 ] * *( x - channels + c ) + m[ 1 ] * x[ c ] + m[ 2 ] * x[ c ] + m[ 3 ] * x[ c ]
				- d[ 0 ] * tmpBuf[ c ] - b2 * ( d[ 1 ] * x[ c ] + d[ 2 ] * x[ c ] + d[ 3 ] * x[ c ] );

				*( tmpBuf - 2 * channels + c ) = m[ 0 ] * *( x - 2 * channels + c ) + m[ 1 ] * *( x - channels + c ) + m[ 2 ] * x[ c ] + m[ 3 ] * x[ c ]
				- d[ 0 ] * *( tmpBuf - channels + c ) - d[ 1 ] * tmpBuf[ c ] - b2 * (  d[ 2 ] * x[ c ] + d[ 3 ] * x[ c ] );

				*( tmpBuf - 3 * channels + c ) = m[ 0 ] * *( x - 3 * channels + c ) + m[ 1 ] * *( x - 2 * channels + c ) + m[ 2 ] * *( x - channels + c ) + m[ 3 ] * x[ c ]
				- d[ 0 ] * *( tmpBuf - 2 * channels + c ) - d[ 1 ] * *( tmpBuf - channels + c ) - d[ 2 ] * tmpBuf[ c ] - b2 * d[ 3 ] * x[ c ];

				y[ c ] += tmpBuf[ c ];
				*( y - channels + c ) += *( tmpBuf - channels + c );
				*( y - 2 * channels + c ) += *( tmpBuf - 2 * channels + c );
				*( y - 3 * channels + c ) += *( tmpBuf - 3 * channels + c );
			}

			// backward pass
			y -= ( 4 * channels );
			for (int i = w-4; i > 0; i--) {
				x-=channels;
				for( uint32_t c = 0; c < channels; c++ ){
					*( tmpBuf - 4 * channels + c ) = ( m[ 0 ] * *( x - 3 * channels + c ) + m[ 1 ] * *( x - 2 * channels + c ) + m[ 2 ] * *( x - channels + c ) + m[ 3 ] * x[ c ]
					- d[ 0 ] * *( tmpBuf - 3 * channels + c ) - d[ 1 ] * *( tmpBuf - 2 * channels + c ) - d[ 2 ] * *( tmpBuf - channels + c ) - d[ 3 ] * tmpBuf[ c ] );
					y[ c ] += *( tmpBuf - 4 * channels + c );
				}
				tmpBuf-=channels;
			}

			in += sstride;
			out += dstride;
		}

		// vertical pass
		const float * x0 = inTmp;
		const float * x1 = x0 + sstride;
		const float * x2 = x1 + sstride;
		const float * x3 = x2 + sstride;
		float * y0 = outTmp;
		float * y1 = y0 + dstride;
		float * y2 = y1 + dstride;
		float * y3 = y2 + dstride;
		for( size_t i = 0; i < w; i++ ){
			for( uint32_t c = 0; c < channels; c++ ){
				y0[ c ] = n[ 0 ] * x0[ c ] + n[ 1 ] * x0[ c ] + n[ 2 ] * x0[ c ] + n[ 3 ] * x0[ c ]
				- b1 * ( d[ 0 ] * x0[ c ] + d[ 1 ] * x0[ c ] + d[ 2 ] * x0[ c ] + d[ 3 ] * x0[ c ] );

				y1[ c ] = n[ 0 ] * x1[ c ] + n[ 1 ] * x0[ c ] + n[ 2 ] * x0[ c ] + n[ 3 ] * x0[ c ]
				- d[ 0 ] * y0[ c ] - b1 * ( d[ 1 ] * x0[ c ] + d[ 2 ] * x0[ c ] + d[ 3 ] * x0[ c ] );

				y2[ c ] = n[ 0 ] * x2[ c ] + n[ 1 ] * x1[ c ] + n[ 2 ] * x0[ c ] + n[ 3 ] * x0[ c ]
				- d[ 0 ] * y1[ c ] - d[ 1 ] * y0[ c ] - b1 * (  d[ 2 ] * x0[ c ] + d[ 3 ] * x0[ c ] );

				y3[ c ] = n[ 0 ] * x3[ c ] + n[ 1 ] * x2[ c ] + n[ 2 ] * x1[ c ] + n[ 3 ] * x0[ c ]
				- d[ 0 ] * y2[ c ] - d[ 1 ] * y1[ c ] - d[ 2 ] * y0[ c ] - b1 * d[ 3 ] * x0[ c ];
			}
			x0+=channels; x1+=channels; x2+=channels; x3+=channels;
			y0+=channels; y1+=channels; y2+=channels; y3+=channels;
		}

		float * currOut;
		// from fourth line to end:
		for( uint32_t i = 4; i < h; i++ ){
			x0 = inTmp + i * sstride;
			x1 = x0 + sstride;
			x2 = x1 + sstride;
			x3 = x2 + sstride;

			y0 = outTmp + ( i - 4 ) * dstride;
			y1 = y0 + dstride;
			y2 = y1 + dstride;
			y3 = y2 + dstride;
			currOut = y3 + dstride;

			for( uint32_t j = 0; j < w; j++ ){
				for( uint32_t c = 0; c < channels; c++ ){
					currOut[ c ] = n[ 0 ] * x3[ c ] + n[ 1 ] * x2[ c ] + n[ 2 ] * x1[ c ] + n[ 3 ] * x0[ c ]
					- d[ 0 ] * y3[ c ] - d[ 1 ] * y2[ c ] - d[ 2 ] * y1[ c ] - d[ 3 ] * y0[ c ];
				}
				x0+=channels; x1+=channels; x2+=channels; x3+=channels;
				y0+=channels; y1+=channels; y2+=channels; y3+=channels;
				currOut+=channels;
			}
		}

		x0 = inTmp + ( h - 1 ) * sstride;
		x1 = x0 - sstride;
		x2 = x1 - sstride;
		x3 = x2 - sstride;

		float tmpBuffer[ 4 * w * channels ];
		y0 = outTmp + ( h - 1 ) * dstride;
		y1 = y0 - dstride;
		y2 = y1 - dstride;
		y3 = y2 - dstride;

		float * t0 = ( tmpBuffer + 3 * w * channels );
		float * t1 = ( tmpBuffer + 2 * w * channels );
		float * t2 = ( tmpBuffer + 1 * w * channels );
		float * t3 = ( tmpBuffer );

		for( size_t i = 0; i < w; i++ ){
			for( uint32_t c = 0; c < channels; c++ ){
				t0[ c ] = m[ 0 ] * x0[ c ] + m[ 1 ] * x0[ c ] + m[ 2 ] * x0[ c ] + m[ 3 ] * x0[ c ]
				- b1 * ( d[ 0 ] * x0[ c ] + d[ 1 ] * x0[ c ] + d[ 2 ] * x0[ c ] + d[ 3 ] * x0[ c ] );
				y0[ c ] += t0[ c ];

				t1[ c ] = m[ 0 ] * x1[ c ] + m[ 1 ] * x0[ c ] + m[ 2 ] * x0[ c ] + m[ 3 ] * x0[ c ]
				- d[ 0 ] * t0[ c ] - b1 * ( d[ 1 ] * x0[ c ] + d[ 2 ] * x0[ c ] + d[ 3 ] * x0[ c ] );
				y1[ c ] += t1[ c ];

				t2[ c ] = m[ 0 ] * x2[ c ] + m[ 1 ] * x1[ c ] + m[ 2 ] * x0[ c ] + m[ 3 ] * x0[ c ]
				- d[ 0 ] * t1[ c ] - d[ 1 ] * t0[ c ] - b1 * (  d[ 2 ] * x0[ c ] + d[ 3 ] * x0[ c ] );
				y2[ c ] += t2[ c ];

				t3[ c ] = m[ 0 ] * x3[ c ] + m[ 1 ] * x2[ c ] + m[ 2 ] * x1[ c ] + m[ 3 ] * x0[ c ]
				- d[ 0 ] * t2[ c ] - d[ 1 ] * t1[ c ] - d[ 2 ] * t0[ c ] - b1 * d[ 3 ] * x0[ c ];
				y3[ c ] += t3[ c ];
			}
			x0+=channels; x1+=channels; x2+=channels; x3+=channels;
			y0+=channels; y1+=channels; y2+=channels; y3+=channels;
			t0+=channels; t1+=channels; t2+=channels; t3+=channels;
		}

		float* tmp;
		t0 = ( tmpBuffer + 3 * w * channels );
		t1 = ( tmpBuffer + 2 * w * channels );
		t2 = ( tmpBuffer + 1 * w * channels );
		t3 = ( tmpBuffer );
		for( uint32_t i = h - 4; i > 0; i-- ){
			x0 = inTmp + ( i + 2 ) * sstride;
			x1 = x0 - sstride;
			x2 = x1 - sstride;
			x3 = x2 - sstride;

			currOut = outTmp + ( i - 1 ) * dstride;

			for( uint32_t j = 0; j < w; j++ ){
				for( uint32_t c = 0; c < channels; c++ ){
					t0[ j * channels + c ] = n[ 0 ] * x3[ c ] + n[ 1 ] * x2[ c ] + n[ 2 ] * x1[ c ] + n[ 3 ] * x0[ c ]
					- d[ 0 ] * t3[ j * channels + c ] - d[ 1 ] * t2[ j * channels + c ] - d[ 2 ] * t1[ j * channels + c ] - d[ 3 ] * t0[ j * channels + c ];
					currOut[ c ] += t0[ j * channels + c ];
				}

				// next pixel of row
				x0+=channels; x1+=channels; x2+=channels; x3+=channels;
				currOut+=channels;
			}
			tmp = t0;
			t0 = t1; t1 = t2; t2 = t3; t3 = tmp;
		}

		src.unmap( inTmp );
		dst.unmap( outTmp );
	}

	void GaussIIR::applyCPUu8( Image& dst, const Image& src, const Vector4f & _n, const Vector4f & _m, const Vector4f & _d ) const
	{
		uint32_t h = dst.height();
		uint32_t w = dst.width();
		uint32_t channels = dst.channels();
		
		void (SIMD::*fwdHoriz)( Fixed* dst, const uint8_t * src, size_t width, const Fixed * n, const Fixed * d, const Fixed & b1 ) const;
		void (SIMD::*bwdHoriz)( uint8_t * dst, const Fixed* fwdRes, const uint8_t * src, size_t w, const Fixed * n, const Fixed * d, const Fixed & b  ) const;
		void (SIMD::*fwdVert)( Fixed* buffer, const uint8_t * src, size_t sstride, size_t h, const Fixed * n, const Fixed * d, const Fixed & b ) const;
		void (SIMD::*bwdVert)( uint8_t * dst, size_t dstride, Fixed* fwdRes, size_t h, const Fixed * n, const Fixed * d, const Fixed & b ) const;
		
		SIMD * simd = SIMD::instance();		
		switch ( channels ) {
			case 4:
				fwdHoriz = &SIMD::IIR4FwdHorizontal4Fx;
				bwdHoriz = &SIMD::IIR4BwdHorizontal4Fx;
				fwdVert = &SIMD::IIR4FwdVertical4Fx;
				bwdVert = &SIMD::IIR4BwdVertical4Fx;
				break;
			default:
				String msg;
				msg.sprintf( "Not implement for number of channles: %d", channels );
				throw CVTException( msg.c_str() );
				break;
		}
		
		
		Fixed n[ 4 ], m[ 4 ], d[ 4 ];
		n[ 0 ] = _n[ 0 ]; n[ 1 ] = _n[ 1 ]; n[ 2 ] = _n[ 2 ]; n[ 3 ] = _n[ 3 ];
		m[ 0 ] = _m[ 0 ]; m[ 1 ] = _m[ 1 ]; m[ 2 ] = _m[ 2 ]; m[ 3 ] = _m[ 3 ];
		d[ 0 ] = _d[ 0 ]; d[ 1 ] = _d[ 1 ]; d[ 2 ] = _d[ 2 ]; d[ 3 ] = _d[ 3 ];

		Fixed one( 1.0f );
		Fixed b1 = ( n[ 0 ] + n[ 1 ] + n[ 2 ] + n[ 3 ] ) / ( d[ 0 ] + d[ 1 ] + d[ 2 ] + d[ 3 ] + one );
		Fixed b2 = ( m[ 0 ] + m[ 1 ] + m[ 2 ] + m[ 3 ] ) / ( d[ 0 ] + d[ 1 ] + d[ 2 ] + d[ 3 ] + one );

		size_t sstride, dstride;
		const uint8_t * in = src.map<uint8_t>( &sstride );
		uint8_t * out = dst.map<uint8_t>( &dstride );

		/* last four values */
		const uint8_t* x = in;
		uint8_t* y = out;

		Fixed* tmpLine = new Fixed[ channels * w ];		

		// horizontal pass
		for( uint32_t k = 0; k < h; k++ ){
			( simd->*fwdHoriz )( tmpLine, x, w, n, d, b1 );
			( simd->*bwdHoriz )( y, tmpLine, x, w, m, d, b2 );

			x += sstride;
			y += dstride;
		}
        delete[] tmpLine;

		// vertical pass:
		// buffer for 4 lines:
		Fixed* column = new Fixed[ channels * h ];
		x = in;
		y = out;
		for( uint32_t k = 0; k < w; k++ ){
			( simd->*fwdVert )( column, y, dstride, h, n, d, b1 );
			( simd->*bwdVert )( y, dstride, column, h, m, d, b2 );
			x += channels;
			y += channels;
		}
        delete[] column;


		src.unmap( in );
		dst.unmap( out );
	}
}
