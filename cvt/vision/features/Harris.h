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

#ifndef CVT_HARRIS_H
#define CVT_HARRIS_H

#include <cvt/vision/features/FeatureDetector.h>
#include <cvt/gfx/Image.h>
#include <cvt/gfx/ifilter/BoxFilter.h>
#include <cvt/gfx/IMapScoped.h>
#include <cvt/util/ScopedBuffer.h>


namespace cvt
{
	class Harris : public FeatureDetector
	{
		public:
			Harris( float threshold = 5e-5f, size_t border = 3 );
			~Harris();

			void detect( FeatureSet& features, const Image& image );
			void detect( FeatureSet& features, const ImagePyramid& image );

			void setThreshold( float threshold ) { _threshold = threshold; }
			float threshold() const	{ return _threshold; }

			void setBorder( size_t border )	{ _border = border; }
			size_t border() const	{ return _border; }

		private:
			void detectFloat( FeatureSet& features, const Image& image );
			void detectU8( FeatureSet& features, const Image& image );

			float		_threshold;
            size_t		_border;

			static const float  _kappa;
			static const int	_radius;
	};

	inline Harris::Harris( float threshold, size_t border ) :
		_threshold( threshold ),
		_border( border )
	{
	}

	inline Harris::~Harris()
	{
	}

	inline void Harris::detect( FeatureSet& features, const Image& image )
	{
		if( image.format() == IFormat::GRAY_FLOAT )
			detectFloat( features, image );
		else if( image.format() == IFormat::GRAY_UINT8 )
			detectU8( features, image );
		else
			throw CVTException( "Input Image format must be GRAY_FLOAT or GRAY_UINT8" );

	}

	inline void Harris::detectFloat( FeatureSet& features, const Image& image )
	{
		size_t w, h;

		w = image.width();
		h = image.height();

		Image dx( w, h, IFormat::GRAY_FLOAT );
		Image dy( w, h, IFormat::GRAY_FLOAT );
		Image dxy( w, h, IFormat::GRAY_FLOAT );

		image.convolve( dx, IKernel::HAAR_HORIZONTAL_3 );
		image.convolve( dy, IKernel::HAAR_VERTICAL_3 );

		dxy = dx;
		dxy.mul( dy );
		dx.mul( dx );
		dy.mul( dy );

		dx.boxfilter( dx, _radius );
		dy.boxfilter( dy, _radius );
		dxy.boxfilter( dxy, _radius );

		IMapScoped<const float> dxmap( dx );
		IMapScoped<const float> dymap( dy );
		IMapScoped<const float> dxymap( dxy );

		SIMD* simd = SIMD::instance();
		size_t yend = h - _border;
		size_t xend = w - _border;
		ScopedBuffer<float,true> scorebuf( w );
		dxmap.setLine( _border );
		dymap.setLine( _border );
		dxymap.setLine( _border );

		for( size_t y = _border; y < yend; y++ ) {
			float* ptr = scorebuf.ptr();
			simd->harrisScore1f( ptr, dxmap.ptr(), dymap.ptr(), dxymap.ptr(), _kappa, w );
			for( size_t x = _border;  x < xend; x++ ) {
				if( ptr[ x ] > _threshold  )
					features.add( Feature( x, y, 0, 0, ptr[ x ] ) );
			}
			dxmap++;
			dymap++;
			dxymap++;
		}
	}

	inline void Harris::detectU8( FeatureSet& features, const Image& image )
	{
		throw CVTException( "uint8 detection not implemented" );
	}

	inline void Harris::detect( FeatureSet& features, const ImagePyramid& image )
	{		
		throw CVTException( "multiscale detection not implemented" );
	}

}

#endif
