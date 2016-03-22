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

#ifndef CVT_IKERNEL_H
#define CVT_IKERNEL_H

#include <cvt/util/SIMD.h>

namespace cvt {
	class Image;

	class IKernel {
		public:
			IKernel();
			IKernel( const IKernel& kernel );
			IKernel( size_t w, size_t h, float* data );
			IKernel( size_t w, size_t h );
			~IKernel();
			IKernel& operator=(const IKernel& kernel );

			size_t width() const;
			size_t height() const;

			float& operator()( int x, int y );
			const float& operator()( int x, int y ) const;
			const float* ptr() const;

			float sum() const;
			void normalize();
			void scale( float value );

			bool isSymmetrical() const;
			bool isPointSymmetrical() const;

			void toImage( Image& dst ) const;


			static IKernel createGaussian1D( float sigma, bool vertical = false );
			static IKernel createGaussian2D( float sigma );

			/**
				@param sigma sigma of the gaussian
				@param theta rotation in rad
				@param lamba wavelength of the sinusoidal factor
				@param gamma spatial aspect ratio
				@param psi   phase offset
			 */
			static IKernel createGabor( float sigma, float theta, float lambda, float gamma, float psi );

			static const IKernel IDENTITY;

			static const IKernel GAUSS_HORIZONTAL_3;
			static const IKernel GAUSS_HORIZONTAL_5;
			static const IKernel GAUSS_HORIZONTAL_7;

			static const IKernel MEAN_HORIZONTAL_3;
			static const IKernel MEAN_HORIZONTAL_5;
			static const IKernel MEAN_HORIZONTAL_7;

			static const IKernel HAAR_HORIZONTAL_2;
			static const IKernel HAAR_HORIZONTAL_3;

			static const IKernel GAUSS_VERTICAL_3;
			static const IKernel GAUSS_VERTICAL_5;
			static const IKernel GAUSS_VERTICAL_7;

			static const IKernel MEAN_VERTICAL_3;
			static const IKernel MEAN_VERTICAL_5;
			static const IKernel MEAN_VERTICAL_7;

			static const IKernel HAAR_VERTICAL_2;
			static const IKernel HAAR_VERTICAL_3;
			static const IKernel FIVEPOINT_DERIVATIVE_HORIZONTAL;
			static const IKernel FIVEPOINT_DERIVATIVE_VERTICAL;

			static const IKernel LAPLACE_33;

			static const IKernel LAPLACE_3_XX;
			static const IKernel LAPLACE_3_YY;

			static const IKernel LAPLACE_5_XX;
			static const IKernel LAPLACE_5_YY;

		private:
			size_t _width, _height;
			float* _mem;
			float* _data;
	};

	inline IKernel::IKernel() : _width( 0 ), _height( 0 ), _mem( NULL ), _data( NULL )
	{
	}

	inline IKernel::IKernel( const IKernel& kernel ) : _width( kernel._width ),
													   _height( kernel._height )
	{
		if( &kernel == this )
			return;
		_mem = new float[ _width * _height ];
		_data = _mem;
		SIMD* simd = SIMD::instance();
		simd->Memcpy( ( uint8_t* ) _data, ( const uint8_t* ) kernel._data, _width * _height * sizeof( float ) );
	}


	inline IKernel::IKernel( size_t w, size_t h ) : _width( w ),
												    _height( h )
	{
		_mem = new float[ _width * _height ];
		_data = _mem;
	}

	inline IKernel& IKernel::operator=(const IKernel& kernel )
	{
		if( &kernel == this )
			return *this;
		_width = kernel._width;
		_height = kernel._height;
		if( _mem )
			delete[] _mem;
		_mem = new float[ _width * _height ];
		_data = _mem;
		SIMD* simd = SIMD::instance();
		simd->Memcpy( ( uint8_t* ) _data, ( const uint8_t* ) kernel._data, _width * _height * sizeof( float ) );
		return *this;
	}

	inline IKernel::IKernel( size_t w, size_t h, float* data ) : _width( w ),
																	   _height( h ),
																	   _mem( NULL ),
																	   _data( data )
	{
	}

	inline IKernel::~IKernel()
	{
		if( _mem )
			delete[] _mem;
	}

	inline size_t IKernel::width() const
	{
		return _width;
	}

	inline size_t IKernel::height() const
	{
		return _height;
	}

	inline float& IKernel::operator()( int x, int y )
	{
		return _data[ y * _width + x ];
	}

	inline const float& IKernel::operator()( int x, int y ) const
	{
		return _data[ y * _width + x ];
	}

	inline const float* IKernel::ptr() const
	{
		return _data;
	}

	inline float IKernel::sum() const
	{
		size_t n = _width * _height;
		float* ptr = _data;
		float ret = 0;
		while( n-- )
			ret += *ptr++;
		return ret;
	}

	inline void IKernel::normalize()
	{
		size_t n = _width * _height;
		float* ptr = _data;
		float invsum = 1.0f / sum();
		while( n-- )
			*ptr++ *= invsum;
	}

	inline void IKernel::scale ( float value )
	{
		size_t n = _width * _height;
		float* ptr = _data;
		while( n-- )
			*ptr++ *= value;
	}


	inline bool IKernel::isSymmetrical() const
	{
		if( !( _width & 1 ) || !( _height & 1 ) )
			return false;

		const size_t iend = _width * _height;
		for( size_t i = 0; i < iend; i++ )
		{
			float a = _data[ i ];
			float b = _data[ iend - i - 1 ];
			if( a != b )
				return false;
		}
		return true;
	}

	inline bool IKernel::isPointSymmetrical() const
	{
		if( !( _width & 1 ) || !( _height & 1 ) )
			return false;

		const size_t iend = _width * _height;
		for( size_t i = 0; i < iend; i++ )
		{
			if( i == iend - i - 1 )
				continue;
			float a = _data[ i ];
			float b = _data[ iend - i - 1 ];
			if( a != -b )
				return false;
		}
		return true;
	}


	inline IKernel IKernel::createGaussian1D( float sigma, bool vertical )
	{
		const float nstds = 3.0f;
		int max = Math::max( ( int ) Math::ceil( nstds * sigma ), 1 );
		float mul = 1.0f / Math::sqrt( 2.0f * Math::PI * Math::sqr( sigma ) );

		if( vertical ) {
			IKernel ret( 1, 2 * max + 1 );
			for( int x = -max; x <= max; x++ ) {
				ret( 0, x + max ) = mul * Math::exp( -( Math::sqr( ( float ) x ) ) / ( 2.0f * Math::sqr( sigma ) ) );
			}
			return ret;
		} else {
			IKernel ret( 2 * max + 1, 1 );
			for( int x = -max; x <= max; x++ ) {
				ret( x + max, 0 ) = mul * Math::exp( -( Math::sqr( ( float ) x ) ) / ( 2.0f * Math::sqr( sigma ) ) );
			}
			return ret;
		}
	}

	inline IKernel IKernel::createGaussian2D( float sigma )
	{
		const float nstds = 3.0f;
		int max = Math::max( ( int ) Math::ceil( nstds * sigma ), 1 );
		float mul = 1.0f / ( 2.0f * Math::PI * Math::sqr( sigma ) );

		IKernel ret( 2 * max + 1, 2 * max + 1 );

		for( int y = -max; y <= max; y++ ) {
			for( int x = -max; x <= max; x++ ) {
				ret( x + max, y + max ) = mul * Math::exp( -( Math::sqr( ( float ) x ) + Math::sqr( ( float ) y ) ) / ( 2.0f * Math::sqr( sigma ) ) );
			}
		}

		return ret;
	}

	inline IKernel IKernel::createGabor( float sigma, float theta, float lambda, float gamma, float psi )
	{
		const float nstds = 3.0f;
		float sigma_x = sigma;
		float sigma_y = sigma / gamma;

		float c = Math::cos( theta );
		float s = Math::sin( theta );

		int xmax = Math::ceil( Math::max( Math::abs( nstds * sigma_x * c ), Math::abs( nstds * sigma_y * s ) ) );
			xmax = Math::max( 1, xmax );
		int ymax = Math::ceil( Math::max( Math::abs( nstds * sigma_x * s ), Math::abs( nstds * sigma_y * c ) ) );
			ymax = Math::max( 1, ymax );

		IKernel ret( 2 * xmax + 1, 2 * ymax + 1 );

		for( int y = -ymax; y <= ymax; y++ ) {
			for( int x = -xmax; x <= xmax; x++ ) {
				float rotx =  ( float ) x * c + ( float ) y * s;
				float roty = -( float ) x * s + ( float ) y * c;
				ret( x + xmax, y + ymax ) = Math::exp( -0.5f * ( Math::sqr( rotx / sigma_x ) + Math::sqr( roty / sigma_y ) ) ) *  Math::cos( 2.0f * Math::PI * rotx / lambda + psi );
			}
		}

		return ret;
	}

}

#endif
