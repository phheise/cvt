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

#include <cvt/gfx/ifilter/IWarp.h>
#include <cvt/math/Vector.h>

namespace cvt {

	static ParamInfoTyped<Image*>   _pinput( "Input" );
	static ParamInfoTyped<Image*>   _pwarp( "Warp" );
	static ParamInfoTyped<Image*>	_poutput( "Output", NULL, false );

	static ParamInfo* _itransform_params[] =
	{
		&_pinput,
		&_pwarp,
		&_poutput
	};

	IWarp::IWarp() : IFilter( "ImageWarp", _itransform_params, 3, IFILTER_CPU )
	{
	}

	IWarp::~IWarp()
	{
	}

	void IWarp::apply( Image& dst, const Image& src, const Image& warp )
	{
		if( warp.format() != IFormat::GRAYALPHA_FLOAT )
			throw CVTException( "Unsupported warp image type" );

		dst.reallocate( warp.width(), warp.height(), src.format() );

		switch( src.format().formatID ) {
			case IFORMAT_GRAY_FLOAT: return applyFC1( dst, src, warp );
			case IFORMAT_GRAY_UINT8: return applyU8C1( dst, src, warp );
			case IFORMAT_RGBA_FLOAT:
			case IFORMAT_BGRA_FLOAT: return applyFC4( dst, src, warp );
			case IFORMAT_RGBA_UINT8:
			case IFORMAT_BGRA_UINT8: return applyU8C4( dst, src, warp );
			default: throw CVTException( "Unsupported image format!" );
		}
	}


	void IWarp::apply( const ParamSet* attribs, IFilterType iftype ) const
	{
	}


	void IWarp::warpTunnel( Image& idst, float radius, float cx, float cy )
	{
		if( idst.format() != IFormat::GRAYALPHA_FLOAT )
			throw CVTException( "Unsupported warp image type" );

		uint8_t* dst;
		float* pdst;
		size_t stride;
		size_t w, h;

		dst = idst.map( &stride );
		w = idst.width();
		h = idst.height();

		Vector2f c( cx, cy );
		for( size_t y = 0; y < h; y++ ) {
			pdst = ( float* ) ( dst + y * stride );
			for( size_t x = 0; x < w; x++ ) {
				Vector2f p( x, y );
				p -= c;
				float r = p.length();
				r = Math::min( r, radius );
				float angle = Math::atan2( p.y, p.x );
				*pdst++ = Math::clamp<float>( r * Math::cos( angle ) + c.x, 0, w - 1 );
				*pdst++ = Math::clamp<float>( r * Math::sin( angle ) + c.y, 0, h - 1 );
			}
		}
		idst.unmap( dst );
	}

	void IWarp::warpFishEye( Image& idst, float strength, float cx, float cy )
	{
		if( idst.format() != IFormat::GRAYALPHA_FLOAT )
			throw CVTException( "Unsupported warp image type" );

		uint8_t* dst;
		float* pdst;
		size_t stride;
		size_t w, h;

		dst = idst.map( &stride );
		w = idst.width();
		h = idst.height();

		float R = Vector2f( Math::max( cx, w - 1 - cx ), Math::max( cy, h - 1 - cy ) ).length();

		Vector2f c( cx, cy );
		for( size_t y = 0; y < h; y++ ) {
			pdst = ( float* ) ( dst + y * stride );
			for( size_t x = 0; x < w; x++ ) {
				Vector2f p( x, y );
				p -= c;
				float r = p.length();
				r = R * Math::pow( r / R, strength * 0.5f );
				float angle = Math::atan2( p.y, p.x );
				*pdst++ = Math::clamp<float>( r * Math::cos( angle ) + c.x, 0, w - 1 );
				*pdst++ = Math::clamp<float>( r * Math::sin( angle ) + c.y, 0, h - 1 );
			}
		}
		idst.unmap( dst );
	}

	void IWarp::warpUndistort( Image& idst, float k1, float k2, float cx, float cy, float fx, float fy, size_t srcWidth, size_t srcHeight, float k3, float p1, float p2 )
	{
		if( idst.format() != IFormat::GRAYALPHA_FLOAT )
			idst.reallocate( srcWidth, srcHeight, IFormat::GRAYALPHA_FLOAT );

		uint8_t* dst;
		float* pdst;
		size_t stride;
		size_t w, h;
		float invfx = 1.0f / fx;
		float invfy = 1.0f / fy;

		dst = idst.map( &stride );
		w = idst.width();
		h = idst.height();

		Vector2f c1( cx +  ( ( float ) w - ( float ) srcWidth ) / 2.0f, cy + ( ( float ) h - ( float ) srcHeight ) / 2.0f );
		Vector2f c2( cx, cy );
		for( size_t y = 0; y < h; y++ ) {
			pdst = ( float* ) ( dst + y * stride );
			for( size_t x = 0; x < w; x++ ) {
				Vector2f p( x, y );
				p -= c1;
				p.x *= invfx;
				p.y *= invfy;
				float r2 = p.lengthSqr();
				float r4 = Math::sqr( r2 );
				float r6 = r2 * r4;
				float poly = ( 1.0f + k1 * r2 + k2 * r4 + k3 * r6 );
				float xy2 = 2.0f * p.x * p.y;
				*pdst++ = fx * ( p.x * poly + xy2 * p1 + p2 * ( r2 + 2.0f * p.x * p.x ) ) + c2.x;
				*pdst++ = fy * ( p.y * poly + xy2 * p2 + p1 * ( r2 + 2.0f * p.y * p.y ) ) + c2.y;
			}
		}
		idst.unmap( dst );
	}

	void IWarp::warpUndistort( Image& idst, Matrix3f& Knew, const CameraCalibration& calib, size_t srcWidth, size_t srcHeight, float alpha, float beta  )
	{
		if( idst.format() != IFormat::GRAYALPHA_FLOAT )
			idst.reallocate( srcWidth, srcHeight, IFormat::GRAYALPHA_FLOAT );

		uint8_t* dst;
		float* pdst;
		size_t stride;
		size_t w, h;
		float sx, sy, scale, tx, ty;
		Rectf min, max, rect;
		Matrix3f transform;
		Vector2f c = calib.center();
		Vector2f f = calib.focalLength();
		Vector3f radial = calib.radialDistortion();
		Vector2f tangential = calib.tangentialDistortion();

		dst = idst.map( &stride );
		w = idst.width();
		h = idst.height();

		calib.calcUndistortRects( min, max, Rectf( 0, 0, srcWidth, srcHeight ) );
		rect.x = Math::mix( min.x, max.x, alpha );
		rect.y = Math::mix( min.y, max.y, alpha );
		rect.width = Math::mix( min.width, max.width, alpha );
		rect.height = Math::mix( min.height, max.height, alpha );

		sx = rect.width  / ( float ) ( idst.width()  - 1 );
		sy = rect.height / ( float ) ( idst.height() - 1 );

		if( sx > sy ) {
			scale = sx;
			sx = sy;
			sy = scale;
		}
		scale = Math::mix( sx, sy, beta );
		tx = ( rect.width - ( float ) ( idst.width() - 1 ) * scale ) / 2.0f + rect.x;
		ty = ( rect.height - ( float ) ( idst.height() - 1 ) * scale ) / 2.0f + rect.y;


		transform.setIdentity();
		transform[ 0 ][ 0 ] = scale;
		transform[ 1 ][ 1 ] = scale;
		transform[ 0 ][ 2 ] = tx;
		transform[ 1 ][ 2 ] = ty;

		Knew.setIdentity();
		Knew[ 0 ][ 0 ] = f.x / scale;
		Knew[ 1 ][ 1 ] = f.y / scale;
		Knew[ 0 ][ 2 ] = ( c.x - tx ) / scale;
		Knew[ 1 ][ 2 ] = ( c.y - ty ) / scale;

//		std::cout << min << std::endl;
//		std::cout << max << std::endl;
//		std::cout << rect << std::endl;
//		std::cout <<  transform * Vector2f( 0, 0 ) << std::endl;
//		std::cout <<  transform * Vector2f( idst.width() - 1, 0 ) << std::endl;
//		std::cout <<  transform * Vector2f( 0, idst.height() - 1 ) << std::endl;
//		std::cout <<  transform * Vector2f( idst.width() - 1, idst.height() - 1 ) << std::endl;

		Vector2f pnew;
		for( size_t y = 0; y < h; y++ ) {
			pdst = ( float* ) ( dst + y * stride );
			for( size_t x = 0; x < w; x++ ) {
				Vector2f p( x, y );
				p = transform * p;
				p -= c;
				p /= f;
				float r2 = p.lengthSqr();
				float r4 = Math::sqr( r2 );
				float r6 = r2 * r4;
				float poly = ( 1.0f + radial[ 0 ] * r2 + radial[ 1 ] * r4 + radial[ 2 ] * r6 );
				float xy2 = 2.0f * p.x * p.y;
				pnew.x = f.x * ( p.x * poly + xy2 * tangential[ 0 ] + tangential[ 1 ] * ( r2 + 2.0f * p.x * p.x ) ) + c.x;
				pnew.y = f.y * ( p.y * poly + xy2 * tangential[ 1 ] + tangential[ 0 ] * ( r2 + 2.0f * p.y * p.y ) ) + c.y;
				*pdst++ = pnew.x;
				*pdst++ = pnew.y;
			}
		}
		idst.unmap( dst );
	}

	void IWarp::applyFC1( Image& idst, const Image& isrc, const Image& iwarp )
	{
		const uint8_t* src;
		uint8_t* dst;
		const uint8_t* wrp;
		uint8_t* pdst;
		const uint8_t* pwrp;
		size_t sstride, dstride, wstride, w, h, sw, sh;

		pdst = dst = idst.map( &dstride );
		pwrp = wrp = iwarp.map( &wstride );
		src = isrc.map( &sstride );

		SIMD* simd = SIMD::instance();

		sw = isrc.width();
		sh = isrc.height();
		w = iwarp.width();
		h = iwarp.height();
		while( h-- ) {
			simd->warpBilinear1f( ( float* ) pdst, ( const float* ) pwrp, ( const float* ) src, sstride, sw, sh, 0.0f, w );
			pdst += dstride;
			pwrp += wstride;
		}

		idst.unmap( dst );
		isrc.unmap( src );
		iwarp.unmap( wrp );
	}

	void IWarp::applyFC4( Image& idst, const Image& isrc, const Image& iwarp )
	{
		const uint8_t* src;
		uint8_t* dst;
		const uint8_t* wrp;
		uint8_t* pdst;
		const uint8_t* pwrp;
		size_t sstride, dstride, wstride, w, h, sw, sh;
		float black[ ] = { 0.0f, 0.0f, 0.0f, 1.0f };

		pdst = dst = idst.map( &dstride );
		pwrp = wrp = iwarp.map( &wstride );
		src = isrc.map( &sstride );

		SIMD* simd = SIMD::instance();

		sw = isrc.width();
		sh = isrc.height();
		w = iwarp.width();
		h = iwarp.height();
		while( h-- ) {
			simd->warpBilinear4f( ( float* ) pdst, ( const float* ) pwrp, ( const float* ) src, sstride, sw, sh, black, w );
			pdst += dstride;
			pwrp += wstride;
		}

		idst.unmap( dst );
		isrc.unmap( src );
		iwarp.unmap( wrp );
	}

	void IWarp::applyU8C1( Image& idst, const Image& isrc, const Image& iwarp )
	{
		const uint8_t* src;
		uint8_t* dst;
		const uint8_t* wrp;
		uint8_t* pdst;
		const uint8_t* pwrp;
		size_t sstride, dstride, wstride, w, h, sw, sh;

		pdst = dst = idst.map( &dstride );
		pwrp = wrp = iwarp.map( &wstride );
		src = isrc.map( &sstride );

		SIMD* simd = SIMD::instance();

		sw = isrc.width();
		sh = isrc.height();
		w = iwarp.width();
		h = iwarp.height();
		while( h-- ) {
			simd->warpBilinear1u8( pdst, ( const float* ) pwrp, src, sstride, sw, sh, 0, w );
			pdst += dstride;
			pwrp += wstride;
		}

		idst.unmap( dst );
		isrc.unmap( src );
		iwarp.unmap( wrp );
	}

	void IWarp::applyU8C4( Image& idst, const Image& isrc, const Image& iwarp )
	{
		const uint8_t* src;
		uint8_t* dst;
		const uint8_t* wrp;
		uint8_t* pdst;
		const uint8_t* pwrp;
		size_t sstride, dstride, wstride, w, h, sw, sh;
		uint32_t black = 0xff000000;

		pdst = dst = idst.map( &dstride );
		pwrp = wrp = iwarp.map( &wstride );
		src = isrc.map( &sstride );

		SIMD* simd = SIMD::instance();

		sw = isrc.width();
		sh = isrc.height();
		w = iwarp.width();
		h = iwarp.height();
		while( h-- ) {
			simd->warpBilinear4u8( pdst, ( const float* ) pwrp, src, sstride, sw, sh, black, w );
			pdst += dstride;
			pwrp += wstride;
		}

		idst.unmap( dst );
		isrc.unmap( src );
		iwarp.unmap( wrp );
	}

}
