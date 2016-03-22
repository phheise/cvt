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

#include <cvt/gfx/ifilter/ITransform.h>
#include <cvt/gfx/Clipping.h>
#include <cvt/gfx/IMapScoped.h>
#include <cvt/util/ScopedBuffer.h>

namespace cvt {

	static ParamInfoTyped<Image*>   _pinput( "Input" );
	static ParamInfoTyped<Matrix3f> _ptransform( "Transformation" );
	static ParamInfoTyped<uint32_t>	_pwidth( "Width", 0, true, 1, 0 );
	static ParamInfoTyped<uint32_t>	_pheight( "Height", 0, true, 1, 0 );
	static ParamInfoTyped<Image*>	_poutput( "Output", NULL, false );

	static ParamInfo* _itransform_params[] =
	{
		&_pinput,
		&_ptransform,
		&_pwidth,
		&_pheight,
		&_poutput
	};

	ITransform::ITransform() : IFilter( "ImageTransform", _itransform_params, 5, IFILTER_CPU )
	{
	}

	ITransform::~ITransform()
	{
	}

	void ITransform::apply( Image& dst, const Image& src, const Matrix3f& T, size_t width, size_t height )
	{
		if( !width )
			width = src.width();
		if( !height )
			height = src.height();
		dst.reallocate( width, height, src.format() );

		Matrix3f Tinv( T );
		if( !Tinv.inverseSelf() )
			return;

		switch( src.format().formatID ) {
			case IFORMAT_GRAY_FLOAT: return applyFC1( dst, src, T, Tinv );
			case IFORMAT_GRAY_UINT8: return applyU8C1( dst, src, T, Tinv );
			case IFORMAT_RGBA_FLOAT:
			case IFORMAT_BGRA_FLOAT: return applyFC4( dst, src, T, Tinv );
			case IFORMAT_RGBA_UINT8:
			case IFORMAT_BGRA_UINT8: return applyU8C4( dst, src, T, Tinv );
			default: throw CVTException( "Unsupported image format!" );
		}
	}

	void ITransform::apply( Image& dst, const Image& src, const Matrix3f& T, const Matrix3f& Tinv, size_t width, size_t height )
	{
		if( !width )
			width = src.width();
		if( !height )
			height = src.height();
		dst.reallocate( width, height, src.format() );

		switch( src.format().formatID ) {
			case IFORMAT_GRAY_FLOAT: return applyFC1( dst, src, T, Tinv );
			case IFORMAT_GRAY_UINT8: return applyU8C1( dst, src, T, Tinv );
			case IFORMAT_RGBA_FLOAT:
			case IFORMAT_BGRA_FLOAT: return applyFC4( dst, src, T, Tinv );
			case IFORMAT_RGBA_UINT8:
			case IFORMAT_BGRA_UINT8: return applyU8C4( dst, src, T, Tinv );
			default: throw CVTException( "Unsupported image format!" );
		}
	}

	void ITransform::apply( Image& dst, const Image& src, const Function<Vector2f, Vector2f>& warpFunc, size_t width, size_t height )
	{
		if( !width )
			width = src.width();
		if( !height )
			height = src.height();
		dst.reallocate( width, height, src.format() );

		switch( src.format().formatID ) {
			case IFORMAT_GRAY_FLOAT: return applyFC1( dst, src, warpFunc );
			case IFORMAT_GRAY_UINT8: return applyU8C1( dst, src, warpFunc );
			case IFORMAT_RGBA_FLOAT:
			case IFORMAT_BGRA_FLOAT: return applyFC4( dst, src, warpFunc );
			case IFORMAT_RGBA_UINT8:
			case IFORMAT_BGRA_UINT8: return applyU8C4( dst, src, warpFunc );
			default: throw CVTException( "Unsupported image format!" );
		}
	}

	void ITransform::apply( const ParamSet* attribs, IFilterType iftype ) const
	{
		if( !(getIFilterType() & iftype ) )
			throw CVTException("Invalid filter type (CPU/GPU)!");

	/*	Image* dst;
		Image* src;
		Matrix3f H;
		Color c;

		src = set->arg<Image*>( 0 );
		dst = set->arg<Image*>( 1 );
		H = set->arg<Matrix3f>( 2 );
		c = set->arg<Color>( 3 );*/
	}


	void ITransform::applyFC1( Image& idst, const Image& isrc, const Matrix3f& T, const Matrix3f& Tinv )
	{
		IMapScoped<const uint8_t> source( isrc );
		IMapScoped<uint8_t> dst( idst );

		ssize_t w, h;
		ssize_t sw, sh;
		float* pdst;
		Matrix3f Ttrans;
		Vector2f pt1( 0, 0 ), pt2( 0, 0 );
		SIMD* simd =SIMD::instance();

		w = idst.width();
		h = idst.height();
		sw = ( ssize_t ) isrc.width();
		sh = ( ssize_t ) isrc.height();

		Ttrans = T.transpose();

		Rectf r( 0 - 1.0f, 0 - 1.0f, sw + 2, sh + 2);
		Vector3f nx = Tinv * Vector3f( 1.0f, 0.0f, 0.0f );
		ssize_t ystart = 0, yend = h;

		Vector2f p = Tinv * Vector2f( 0.0f, 0.0f );
		p.y = Math::clamp<float>( p.y, 0, h );
		ystart = Math::min( ystart, ( ssize_t ) p.y );
		yend = Math::max( yend, ( ssize_t ) p.y );

		p = Tinv * Vector2f( sw, 0.0f );
		p.y = Math::clamp<float>( p.y, 0, h );
		ystart = Math::min( ystart, ( ssize_t ) p.y );
		yend = Math::max( yend, ( ssize_t ) p.y );

		p = Tinv * Vector2f( 0.0f, sh );
		p.y = Math::clamp<float>( p.y, 0, h );
		ystart = Math::min( ystart, ( ssize_t ) p.y );
		yend = Math::max( yend, ( ssize_t ) p.y );

		p = Tinv * Vector2f( sw, sh );
		p.y = Math::clamp<float>( p.y, 0, h );
		ystart = Math::min( ystart, ( ssize_t ) p.y );
		yend = Math::max( yend, ( ssize_t ) p.y );

		for( ssize_t y = ystart; y < yend; y++  ) {
			Line2Df l( Vector3f( 0, 1, -y ) );
			Line2Df l2( Ttrans * Vector3f( 0, 1, -y ) );

			pdst = ( float* ) dst.ptr();
			if( Clipping::clip( r, l2, pt1, pt2 ) ) {
				Vector2f px1, px2;
				px1 = T * pt1;
				px2 = T * pt2;

				if( px1.x > px2.x ) {
					Vector2f tmp = px1;
					px1 = px2;
					px2 = tmp;
				}

				ssize_t x1 =  Math::clamp<ssize_t>( px1.x, 0, w );
				ssize_t len =  Math::clamp<ssize_t>( px2.x + 1, 0, w ) -x1;
				Vector3f p = Tinv * Vector3f( x1, y, 1.0f );
				simd->warpLinePerspectiveBilinear1f( pdst + x1, ( const float* ) source.ptr(), source.stride(), sw, sh,
													 p.ptr(), nx.ptr(), len );
			}
			dst++;
		}
	}


	void ITransform::applyFC4( Image& idst, const Image& isrc, const Matrix3f& T, const Matrix3f& Tinv )
	{
		IMapScoped<const uint8_t> src( isrc );
		IMapScoped<uint8_t> dst( idst );
		ssize_t w, h;
		ssize_t sw, sh;
		Matrix3f Ttrans;
		float* pdst;
		Vector2f pt1( 0, 0 ), pt2( 0, 0 );
		SIMD* simd =SIMD::instance();

		w = idst.width();
		h = idst.height();
		sw = ( ssize_t ) isrc.width();
		sh = ( ssize_t ) isrc.height();

		Ttrans = T.transpose();

		Rectf r( 0 - 1.0f, 0 - 1.0f, sw + 2, sh + 2);
		Vector3f nx = Tinv * Vector3f( 1.0f, 0.0f, 0.0f );
		ssize_t ystart = 0, yend = h;

		Vector2f p = Tinv * Vector2f( 0.0f, 0.0f );
		p.y = Math::clamp<float>( p.y, 0, h );
		ystart = Math::min( ystart, ( ssize_t ) p.y );
		yend = Math::max( yend, ( ssize_t ) p.y );

		p = Tinv * Vector2f( sw, 0.0f );
		p.y = Math::clamp<float>( p.y, 0, h );
		ystart = Math::min( ystart, ( ssize_t ) p.y );
		yend = Math::max( yend, ( ssize_t ) p.y );

		p = Tinv * Vector2f( 0.0f, sh );
		p.y = Math::clamp<float>( p.y, 0, h );
		ystart = Math::min( ystart, ( ssize_t ) p.y );
		yend = Math::max( yend, ( ssize_t ) p.y );

		p = Tinv * Vector2f( sw, sh );
		p.y = Math::clamp<float>( p.y, 0, h );
		ystart = Math::min( ystart, ( ssize_t ) p.y );
		yend = Math::max( yend, ( ssize_t ) p.y );

		for( ssize_t y = ystart; y < yend; y++  ) {
			Line2Df l( Vector3f( 0, 1, -y ) );
			Line2Df l2( Ttrans * Vector3f( 0, 1, -y ) );

			pdst = ( float* ) dst.ptr();
			if( Clipping::clip( r, l2, pt1, pt2 ) ) {
				Vector2f px1, px2;
				px1 = T * pt1;
				px2 = T * pt2;

				if( px1.x > px2.x ) {
					Vector2f tmp = px1;
					px1 = px2;
					px2 = tmp;
				}

				ssize_t x1 =  Math::clamp<ssize_t>( px1.x, 0, w );
				ssize_t len =  Math::clamp<ssize_t>( px2.x + 1, 0, w ) -x1;
				Vector3f p = Tinv * Vector3f( x1, y, 1.0f );
				simd->warpLinePerspectiveBilinear4f( pdst + 4 * x1, ( const float* )src.ptr(), src.stride(), sw, sh,
													p.ptr(), nx.ptr(), len );
			}
			dst++;
		}
	}

	void ITransform::applyU8C1( Image& idst, const Image& isrc, const Matrix3f& T, const Matrix3f& Tinv )
	{
		IMapScoped<const uint8_t> src( isrc );
		IMapScoped<uint8_t> dst( idst );
		ssize_t w, h;
		ssize_t sw, sh;
		Matrix3f Ttrans;
		Vector2f pt1( 0, 0 ), pt2( 0, 0 );
		SIMD* simd = SIMD::instance();

		w = idst.width();
		h = idst.height();
		sw = ( ssize_t ) isrc.width();
		sh = ( ssize_t ) isrc.height();

		Ttrans = T.transpose();

		Rectf r( 0 - 1.0f, 0 - 1.0f, sw + 2, sh + 2);
		Vector3f nx = Tinv * Vector3f( 1.0f, 0.0f, 0.0f );
		ssize_t ystart = 0, yend = h;

		Vector2f p = Tinv * Vector2f( 0.0f, 0.0f );
		p.y = Math::clamp<float>( p.y, 0, h );
		ystart = Math::min( ystart, ( ssize_t ) p.y );
		yend = Math::max( yend, ( ssize_t ) p.y );

		p = Tinv * Vector2f( sw, 0.0f );
		p.y = Math::clamp<float>( p.y, 0, h );
		ystart = Math::min( ystart, ( ssize_t ) p.y );
		yend = Math::max( yend, ( ssize_t ) p.y );

		p = Tinv * Vector2f( 0.0f, sh );
		p.y = Math::clamp<float>( p.y, 0, h );
		ystart = Math::min( ystart, ( ssize_t ) p.y );
		yend = Math::max( yend, ( ssize_t ) p.y );

		p = Tinv * Vector2f( sw, sh );
		p.y = Math::clamp<float>( p.y, 0, h );
		ystart = Math::min( ystart, ( ssize_t ) p.y );
		yend = Math::max( yend, ( ssize_t ) p.y );

		for( ssize_t y = ystart; y < yend; y++  ) {
			Line2Df l( Vector3f( 0, 1, -y ) );
			Line2Df l2( Ttrans * Vector3f( 0, 1, -y ) );

			if( Clipping::clip( r, l2, pt1, pt2 ) ) {
				Vector2f px1, px2;
				px1 = T * pt1;
				px2 = T * pt2;

				if( px1.x > px2.x ) {
					Vector2f tmp = px1;
					px1 = px2;
					px2 = tmp;
				}

				ssize_t x1 =  Math::clamp<ssize_t>( px1.x, 0, w );
				ssize_t len =  Math::clamp<ssize_t>( px2.x + 1, 0, w ) -x1;
				Vector3f p = Tinv * Vector3f( x1, y, 1.0f );
				simd->warpLinePerspectiveBilinear1u8( dst.ptr() + x1, src.ptr(), src.stride(), sw, sh,
													  p.ptr(), nx.ptr(), len );
			}
			// next line
			dst++;
		}
	}

	void ITransform::applyU8C4( Image& idst, const Image& isrc, const Matrix3f& T, const Matrix3f& Tinv )
	{
		IMapScoped<const uint8_t> src( isrc );
		IMapScoped<uint8_t> dst( idst );
		ssize_t w, h;
		ssize_t sw, sh;
		Matrix3f Ttrans;
		Vector2f pt1( 0, 0 ), pt2( 0, 0 );
		SIMD* simd =SIMD::instance();

		w = idst.width();
		h = idst.height();
		sw = ( ssize_t ) isrc.width();
		sh = ( ssize_t ) isrc.height();

		Ttrans = T.transpose();

		Rectf r( 0 - 1.0f, 0 - 1.0f, sw + 2, sh + 2);
		Vector3f nx = Tinv * Vector3f( 1.0f, 0.0f, 0.0f );
		ssize_t ystart = 0, yend = h;

		Vector2f p = Tinv * Vector2f( 0.0f, 0.0f );
		p.y = Math::clamp<float>( p.y, 0, h );
		ystart = Math::min( ystart, ( ssize_t ) p.y );
		yend = Math::max( yend, ( ssize_t ) p.y );

		p = Tinv * Vector2f( sw, 0.0f );
		p.y = Math::clamp<float>( p.y, 0, h );
		ystart = Math::min( ystart, ( ssize_t ) p.y );
		yend = Math::max( yend, ( ssize_t ) p.y );

		p = Tinv * Vector2f( 0.0f, sh );
		p.y = Math::clamp<float>( p.y, 0, h );
		ystart = Math::min( ystart, ( ssize_t ) p.y );
		yend = Math::max( yend, ( ssize_t ) p.y );

		p = Tinv * Vector2f( sw, sh );
		p.y = Math::clamp<float>( p.y, 0, h );
		ystart = Math::min( ystart, ( ssize_t ) p.y );
		yend = Math::max( yend, ( ssize_t ) p.y );

		for( ssize_t y = ystart; y < yend; y++  ) {
			Line2Df l( Vector3f( 0, 1, -y ) );
			Line2Df l2( Ttrans * Vector3f( 0, 1, -y ) );

			if( Clipping::clip( r, l2, pt1, pt2 ) ) {
				Vector2f px1, px2;
				px1 = T * pt1;
				px2 = T * pt2;

				if( px1.x > px2.x ) {
					Vector2f tmp = px1;
					px1 = px2;
					px2 = tmp;
				}

				ssize_t x1 =  Math::clamp<ssize_t>( px1.x, 0, w );
				ssize_t len =  Math::clamp<ssize_t>( px2.x + 1, 0, w ) -x1;
				Vector3f p = Tinv * Vector3f( x1, y, 1.0f );
				simd->warpLinePerspectiveBilinear4u8( dst.ptr() + sizeof( uint32_t ) * x1, src.ptr(), src.stride(), sw, sh,
													p.ptr(), nx.ptr(), len );
			}
			dst++;
		}
	}


	void ITransform::applyU8C1( Image& dst, const Image& src, const Function<Vector2f, Vector2f>& warp )
	{
		size_t h  = dst.height();
		size_t w  = dst.width();
		size_t sh = src.height(); 
		size_t sw = src.width();

		IMapScoped<const uint8_t> source( src );
		IMapScoped<uint8_t> destination( dst );

		ScopedBuffer<float, true> buf( 2 * w );
		float * warpLookup = buf.ptr();

		SIMD* simd = SIMD::instance();

		Vector2f p, pp;
		for( size_t y = 0; y < h; y++ ) {
			p.y = y;
			// fill the line warp lookup
			for( size_t x = 0; x < w; x++ ) {
				p.x = x;				
				pp = warp( p );
				warpLookup[ 2 * x     ] = pp.x;
				warpLookup[ 2 * x + 1 ] = pp.y;
			}
			simd->warpBilinear1u8( destination.ptr(), warpLookup, source.ptr(), source.stride(), sw, sh, 0, w );
			destination++;
		}
	}
	
	void ITransform::applyU8C4( Image& dst, const Image& src, const Function<Vector2f, Vector2f>& warp )
	{
		size_t h = dst.height();
		size_t w = dst.width();
		size_t sh = src.height(); 
		size_t sw = src.width();

		IMapScoped<const uint8_t> source( src );
		IMapScoped<uint8_t> destination( dst );

		ScopedBuffer<float, true> buf( 2 * w );
		float * warpLookup = buf.ptr();
		uint32_t black = 0xff000000;

		SIMD* simd = SIMD::instance();

		Vector2f p, pp;
		for( size_t y = 0; y < h; y++ ) {
			p.y = y;
			// fill the line warp lookup
			for( size_t x = 0; x < w; x++ ) {
				p.x = x;				
				pp = warp( p );
				warpLookup[ 2 * x     ] = pp.x;
				warpLookup[ 2 * x + 1 ] = pp.y;
			}
			simd->warpBilinear4u8( destination.ptr(), warpLookup, source.ptr(), source.stride(), sw, sh, black, w );
			destination++;
		}
	}	
	void ITransform::applyFC1( Image& dst, const Image& src, const Function<Vector2f, Vector2f>& warp )
	{
		size_t h  = dst.height();
		size_t w  = dst.width();
		size_t sh = src.height(); 
		size_t sw = src.width();

		IMapScoped<const float> source( src );
		IMapScoped<float> destination( dst );

		ScopedBuffer<float, true> buf( 2 * w );
		float * warpLookup = buf.ptr();

		SIMD* simd = SIMD::instance();

		Vector2f p, pp;
		for( size_t y = 0; y < h; y++ ) {
			p.y = y;
			// fill the line warp lookup
			for( size_t x = 0; x < w; x++ ) {
				p.x = x;				
				pp = warp( p );
				warpLookup[ 2 * x     ] = pp.x;
				warpLookup[ 2 * x + 1 ] = pp.y;
			}
			simd->warpBilinear1f( destination.ptr(), warpLookup, source.ptr(), source.stride(), sw, sh, 0.0f, w );
			destination++;
		}
	}

	void ITransform::applyFC4( Image& dst, const Image& src, const Function<Vector2f, Vector2f>& warp )
	{
		size_t h  = dst.height();
		size_t w  = dst.width();
		size_t sh = src.height(); 
		size_t sw = src.width();

		IMapScoped<const float> source( src );
		IMapScoped<float> destination( dst );

		ScopedBuffer<float, true> buf( 2 * w );
		float * warpLookup = buf.ptr();
		float black[ ] = { 0.0f, 0.0f, 0.0f, 1.0f };

		SIMD* simd = SIMD::instance();

		Vector2f p, pp;
		for( size_t y = 0; y < h; y++ ) {
			p.y = y;
			// fill the line warp lookup
			for( size_t x = 0; x < w; x++ ) {
				p.x = x;				
				pp = warp( p );
				warpLookup[ 2 * x     ] = pp.x;
				warpLookup[ 2 * x + 1 ] = pp.y;
			}
			simd->warpBilinear4f( destination.ptr(), warpLookup, source.ptr(), source.stride(), sw, sh, black, w );
			destination++;
		}
	}


}
