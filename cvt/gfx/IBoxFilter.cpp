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

#include <cvt/gfx/IBoxFilter.h>
#include <cvt/gfx/Image.h>
#include <cvt/gfx/IMapScoped.h>
#include <cvt/util/SIMD.h>
#include <cvt/util/ScopedBuffer.h>

namespace cvt
{

	/* general template use for separable convolution ( except the constant border case ) */
	template<typename TYPE, typename BUFTYPE>
	static void boxfilterTemplate( Image& dst, const Image& src, size_t hradius, size_t vradius,
								   void ( SIMD::*hbox )( BUFTYPE*, const TYPE*, size_t, size_t ) const,
								   void ( SIMD::*vconv )( TYPE*, BUFTYPE*, const BUFTYPE*, const BUFTYPE*, size_t, size_t ) const,
								   void ( SIMD::*mul )( BUFTYPE*, const BUFTYPE*, const BUFTYPE, size_t ) const,
								   void ( SIMD::*add )( BUFTYPE*, const BUFTYPE*, const BUFTYPE*, size_t ) const
								 )
	{
		SIMD* simd = SIMD::instance();
		size_t w = src.width();
		size_t h = src.height();
		size_t widthchannels = w * src.channels();
		size_t bstride = Math::pad16( sizeof( BUFTYPE ) * widthchannels ) / sizeof( BUFTYPE ); //FIXME: does this always work - it should
		BUFTYPE** buf;
		size_t bh = 2 * vradius + 1;
		size_t subidx = 0, addidx = vradius - 1, y;

		IMapScoped<TYPE> mapdst( dst );
		IMapScoped<const TYPE> mapsrc( src );

		/* allocate buffers and fill buffer*/
		ScopedBuffer<BUFTYPE,true> bufmem( bstride * ( bh + 1 ) );
		ScopedBuffer<BUFTYPE*,true> bufptr( bh + 1 );
		ScopedBuffer<BUFTYPE,true> accum( widthchannels );


		buf = bufptr.ptr();
		buf[ 0 ] = bufmem.ptr();
		for( size_t i = 1; i <= bh; i++ )
			buf[ i ] = buf[ i - 1 ] + bstride;

		/* boxfilter the vradius first lines and store into buf */
		for( size_t i = 0; i < vradius; i++ ) {
			( simd->*hbox )( buf[ i ], mapsrc.ptr(), hradius, w );
			mapsrc++;
		}

		( simd->*mul )( accum.ptr(), buf[ 0 ], vradius + 2, widthchannels );
		for( size_t i = 1; i < vradius; i++ )
			( simd->*add )( accum.ptr(), accum.ptr(), buf[ i ], widthchannels );


		for( y = 0; y <= vradius; y++ ) {
			addidx = ( addidx + 1 ) % ( bh + 1 );
			( simd->*hbox )( buf[ addidx ], mapsrc.ptr(), hradius, w );
			( simd->*vconv )( mapdst.ptr(), accum.ptr(), buf[ addidx ], buf[ 0 ], vradius, widthchannels );
			mapsrc++;
			mapdst++;
		}

		for( ; y < h - vradius ; y++ ) {
			addidx = ( addidx + 1 ) % ( bh + 1 );
			( simd->*hbox )( buf[ addidx ], mapsrc.ptr(), hradius, w );
			( simd->*vconv )( mapdst.ptr(), accum.ptr(), buf[ addidx ], buf[ subidx ], vradius, widthchannels );
			subidx = ( subidx + 1 ) % ( bh + 1 );
			mapsrc++;
			mapdst++;
		}

		for( ; y < h; y++ ) {
			( simd->*vconv )( mapdst.ptr(), accum.ptr(), buf[ addidx ], buf[ subidx ], vradius, widthchannels );
			subidx = ( subidx + 1 ) % ( bh + 1 );
			mapdst++;
		}


	}

	void IBoxFilter::boxfilter( Image& dst, const Image& src, size_t radiushorizontal, size_t radiusvertical )
	{
		radiushorizontal = Math::max<size_t>( radiushorizontal, 1 );
		if( !radiusvertical )
			radiusvertical = radiushorizontal;
		radiusvertical   = Math::max<size_t>( radiusvertical, 1 );

		if( src.format().type == IFORMAT_TYPE_UINT8 ) {
			dst.reallocate( src.width(), src.height(), IFormat::GRAY_UINT8 );

			if( src.channels() == 1 ) {
				return boxfilterTemplate<uint8_t,float>( dst, src, radiushorizontal, radiusvertical,
														&SIMD::BoxFilterHorizontal_1u8_to_f,
														&SIMD::BoxFilterVert_f_to_u8,
														&SIMD::MulValue1f,
														&SIMD::Add );
			} else
				throw CVTException("Unimplemented");
		} else if( src.format().type == IFORMAT_TYPE_FLOAT ) {
			dst.reallocate( src.width(), src.height(), IFormat::GRAY_FLOAT );

			if( src.channels() == 1 ) {
				return boxfilterTemplate<float,float>( dst, src, radiushorizontal, radiusvertical,
														&SIMD::BoxFilterHorizontal_1f,
														&SIMD::BoxFilterVert_f,
														&SIMD::MulValue1f,
														&SIMD::Add );
			} else
				throw CVTException("Unimplemented");
		} else
			throw CVTException("Unimplemented");
	}
}
