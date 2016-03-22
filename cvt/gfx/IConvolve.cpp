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

#include <cvt/gfx/IConvolve.h>
#include <cvt/gfx/Image.h>
#include <cvt/gfx/IMapScoped.h>
#include <cvt/gfx/IBorder.h>
#include <cvt/util/SIMD.h>
#include <cvt/util/ScopedBuffer.h>

namespace cvt {

	/* general template use for separable convolution ( except the constant border case ) */
	template<typename DSTTYPE, typename SRCTYPE, typename BUFTYPE, typename KERNTYPE>
	static void convolveSeparableTemplate( Image& dst, const Image& src, const KERNTYPE* hkern, size_t kw, const KERNTYPE* vkern, size_t kh,
										   void ( SIMD::*hconv )( BUFTYPE*, const SRCTYPE*, size_t, const KERNTYPE* , size_t, IBorderType type ) const,
										   void ( SIMD::*vconv )( DSTTYPE*, const BUFTYPE**, const KERNTYPE* , size_t, size_t ) const,
										   IBorderType btype
										 )
	{
		SIMD* simd = SIMD::instance();
		ssize_t w = src.width();
		ssize_t h = src.height();
		size_t widthchannels = w * src.channels();
		size_t bstride = Math::pad16( sizeof( BUFTYPE ) * widthchannels ) / sizeof( BUFTYPE ); //FIXME: does this always work - it should
		BUFTYPE** buf;
		ssize_t cy;

		IMapScoped<DSTTYPE> mapdst( dst );
		IMapScoped<const SRCTYPE> mapsrc( src );

		ssize_t b1 = ( kh >> 1 );
		ssize_t b2 = kh - b1 - 1;

		/* allocate buffers and fill buffer*/
		ScopedBuffer<BUFTYPE,true> bufmem( bstride * kh );
		ScopedBuffer<BUFTYPE*,true> bufptr( kh );

		buf = bufptr.ptr();
		buf[ 0 ] = bufmem.ptr();
		for( size_t i = 1; i < kh; i++ )
			buf[ i ] = buf[ i - 1 ] + bstride;

		for( ssize_t k = -b1; k <= b2; k++ ) {
			ssize_t y = IBorder::value( k, h, btype );
			( simd->*hconv )( buf[ k + b1 ], mapsrc.line( y ), w, hkern, kw, btype );
		}
		/* process first line */
		( simd->*vconv )( mapdst.ptr(), ( const BUFTYPE** ) buf, vkern, kh, widthchannels );
		mapdst++;

		mapsrc.setLine( b2 + 1 );
		/* main part */
		for( cy = 1; cy < h - b2; cy++ )
		{
				BUFTYPE* tmp = buf[ 0 ];
				for( size_t k = 0; k < kh - 1; k++ )
					buf[ k ] = buf[ k + 1 ];
				buf[ kh - 1 ] = tmp;
				( simd->*hconv )( tmp, mapsrc.ptr(), w, hkern, kw, btype );
				( simd->*vconv )( mapdst.ptr(), ( const BUFTYPE** ) buf, vkern, kh, widthchannels );
				mapdst++;
				mapsrc++;

		}

		/* lower border */
		for( ; cy < h; cy++ ) {
			BUFTYPE* tmp = buf[ 0 ];
			for( size_t k = 0; k < kh - 1; k++ )
				buf[ k ] = buf[ k + 1 ];
			buf[ kh - 1 ] = tmp;
			ssize_t y = IBorder::value( cy + b2, h, btype );
			( simd->*hconv )( tmp, mapsrc.line( y ), w, hkern, kw, btype );
			( simd->*vconv )( mapdst.ptr(), ( const BUFTYPE** ) buf, vkern, kh, widthchannels );
			mapdst++;
		}

	}

	/* general template use for convolution ( except the constant border case ) */
	template<typename DSTTYPE, typename SRCTYPE, typename BUFTYPE, typename KERNTYPE>
	static void convolveTemplate( Image& dst, const Image& src, const KERNTYPE* kern, ssize_t kw, ssize_t kh,
										   void ( SIMD::*conv )( BUFTYPE*, const SRCTYPE*, size_t, const KERNTYPE* , size_t, IBorderType type ) const,
										   void ( SIMD::*avg )( DSTTYPE*, const BUFTYPE**, size_t, size_t ) const,
										   IBorderType btype
										 )
	{
		SIMD* simd = SIMD::instance();
		ssize_t w = src.width();
		ssize_t h = src.height();
		size_t widthchannels = w * src.channels();
		size_t bstride = Math::pad16( sizeof( BUFTYPE ) * widthchannels ) / sizeof( BUFTYPE ); //FIXME: does this always work - it should
		BUFTYPE** buf;
		ssize_t cy;

		IMapScoped<DSTTYPE> mapdst( dst );
		IMapScoped<const SRCTYPE> mapsrc( src );

		ssize_t b1 = ( kh >> 1 );
		ssize_t b2 = kh - b1 - 1;

		/* allocate buffers and fill buffer*/
		ScopedBuffer<BUFTYPE,true> bufmem( bstride * kh );
		ScopedBuffer<BUFTYPE*,true> bufptr( kh );

		buf = bufptr.ptr();
		buf[ 0 ] = bufmem.ptr();
		for( ssize_t i = 1; i < kh; i++ )
			buf[ i ] = buf[ i - 1 ] + bstride;


		/* upper border */
		for( cy = 0; cy < b1; cy++ ) {
			for( ssize_t k = 0; k < kh; k++ ) {
				ssize_t y = IBorder::value( cy - b1 + k, h, btype );
				( simd->*conv )( buf[ k ], mapsrc.line( y ), w, kern + kw * k, kw, btype );
			}
			( simd->*avg )( mapdst.ptr(), ( const BUFTYPE** ) buf, kh, widthchannels );
			mapdst++;
		}

		/* main part */
		for( ; cy < h - b2; cy++ )
		{
			for( ssize_t k = 0; k < kh; k++ ) {
				( simd->*conv )( buf[ k ], mapsrc.line( cy - b1 + k ), w, kern + kw * k, kw, btype );
			}
			( simd->*avg )( mapdst.ptr(), ( const BUFTYPE** ) buf, kh, widthchannels );
			mapdst++;
		}

		/* lower border */
		for( ; cy < h; cy++ ) {
			for( ssize_t k = 0; k < kh; k++ ) {
				ssize_t y = IBorder::value( cy - b1 + k, h, btype );
				( simd->*conv )( buf[ k ], mapsrc.line( y ), w, kern + kw * k, kw, btype );
			}
			( simd->*avg )( mapdst.ptr(), ( const BUFTYPE** ) buf, kh, widthchannels );
			mapdst++;
		}

	}

	void IConvolve::convolve( Image& dst, const Image& src, const IKernel& kernel, IBorderType btype, const Color& )
	{
		if( src.format().type == IFORMAT_TYPE_FLOAT && dst.format().type == IFORMAT_TYPE_FLOAT ) {
			if( src.channels() == 1 )
				return convolveTemplate<float,float,float,float>( dst, src, kernel.ptr(), kernel.width(), kernel.height(),
																		  &SIMD::ConvolveHorizontal1f, &SIMD::AddVert_f, btype );
			else if( src.channels() == 2 )
				return convolveTemplate<float,float,float,float>( dst, src, kernel.ptr(), kernel.width(), kernel.height(),
																		  &SIMD::ConvolveHorizontal2f, &SIMD::AddVert_f, btype );
			else if( src.channels() == 4 )
				return convolveTemplate<float,float,float,float>( dst, src, kernel.ptr(), kernel.width(), kernel.height(),
																		  &SIMD::ConvolveHorizontal4f, &SIMD::AddVert_f, btype );
		}  else if( src.format().type == IFORMAT_TYPE_UINT8 && dst.format().type == IFORMAT_TYPE_UINT8 ) {
#if 0
			if( src.channels() == 1 )
				return convolveTemplate<uint8_t,uint8_t,float,float>( dst, src, kernel.ptr(), kernel.width(), kernel.height(),
																		  &SIMD::ConvolveHorizontal1u8_to_f, &SIMD::AddVert_f_to_u8, btype );
			else if( src.channels() == 2 )
				return convolveTemplate<uint8_t,uint8_t,float,float>( dst, src, kernel.ptr(), kernel.width(), kernel.height(),
																		  &SIMD::ConvolveHorizontal2u8_to_f, &SIMD::AddVert_f_to_u8, btype );
			else if( src.channels() == 4 )
				return convolveTemplate<uint8_t,uint8_t,float,float>( dst, src, kernel.ptr(), kernel.width(), kernel.height(),
																		  &SIMD::ConvolveHorizontal4u8_to_f, &SIMD::AddVert_f_to_u8, btype );
#else
			ScopedBuffer<Fixed,true> kern( kernel.width() * kernel.height() );
			for( size_t i = 0; i < kernel.width() * kernel.height(); i++ )
					kern.ptr()[ i ] = kernel.ptr()[ i ];

			if( src.channels() == 1 )
				return convolveTemplate<uint8_t,uint8_t,Fixed,Fixed>( dst, src, kern.ptr(), kernel.width(), kernel.height(),
																		  &SIMD::ConvolveHorizontal1u8_to_fx, &SIMD::AddVert_fx_to_u8, btype );
			else if( src.channels() == 2 )
				return convolveTemplate<uint8_t,uint8_t,Fixed,Fixed>( dst, src, kern.ptr(), kernel.width(), kernel.height(),
																		  &SIMD::ConvolveHorizontal2u8_to_fx, &SIMD::AddVert_fx_to_u8, btype );
			else if( src.channels() == 4 )
				return convolveTemplate<uint8_t,uint8_t,Fixed,Fixed>( dst, src, kern.ptr(), kernel.width(), kernel.height(),
																		  &SIMD::ConvolveHorizontal4u8_to_fx, &SIMD::AddVert_fx_to_u8, btype );

#endif
		}

	}

	void IConvolve::convolve( Image& dst, const Image& src, const IKernel& hkernel, const IKernel& vkernel, IBorderType btype, const Color& )
	{
		// TODO: check for compatible formats or reallocate
		bool symh = hkernel.isSymmetrical();
		bool symv = vkernel.isSymmetrical();

		if( src.format().type == IFORMAT_TYPE_FLOAT && dst.format().type == IFORMAT_TYPE_FLOAT ) {

			if( symh && !symv ) {
				if( src.channels() == 1 )
					return convolveSeparableTemplate<float,float,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontalSym1f, &SIMD::ConvolveClampVert_f, btype );
				else if( src.channels() == 2 )
					return convolveSeparableTemplate<float,float,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontalSym2f, &SIMD::ConvolveClampVert_f, btype );
				else if( src.channels() == 4 )
					return convolveSeparableTemplate<float,float,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontalSym4f, &SIMD::ConvolveClampVert_f, btype );
			} else if( !symh && symv ) {
				if( src.channels() == 1 )
					return convolveSeparableTemplate<float,float,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontal1f, &SIMD::ConvolveClampVertSym_f, btype );
				else if( src.channels() == 2 )
					return convolveSeparableTemplate<float,float,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontal2f, &SIMD::ConvolveClampVertSym_f, btype );
				else if( src.channels() == 4 )
					return convolveSeparableTemplate<float,float,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontal4f, &SIMD::ConvolveClampVertSym_f, btype );
			} else if( symh && symv ) {
				if( src.channels() == 1 )
					return convolveSeparableTemplate<float,float,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontalSym1f, &SIMD::ConvolveClampVertSym_f, btype );
				else if( src.channels() == 2 )
					return convolveSeparableTemplate<float,float,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontalSym2f, &SIMD::ConvolveClampVertSym_f, btype );
				else if( src.channels() == 4 )
					return convolveSeparableTemplate<float,float,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontalSym4f, &SIMD::ConvolveClampVertSym_f, btype );
			} else {
				if( src.channels() == 1 )
					return convolveSeparableTemplate<float,float,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontal1f, &SIMD::ConvolveClampVert_f, btype );
				else if( src.channels() == 2 )
					return convolveSeparableTemplate<float,float,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontal2f, &SIMD::ConvolveClampVert_f, btype );
				else if( src.channels() == 4 )
					return convolveSeparableTemplate<float,float,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontal4f, &SIMD::ConvolveClampVert_f, btype );
			}
		} else if( src.format().type == IFORMAT_TYPE_UINT8 && dst.format().type == IFORMAT_TYPE_UINT8 ) {
#if 0
			ScopedBuffer<Fixed,true> vkern( vkernel.height() );
			ScopedBuffer<Fixed,true> hkern( hkernel.width() );

			for( size_t kx = 0; kx < hkernel.width(); kx++ )
				hkern.ptr()[ kx ] = hkernel( kx, 0 );

			for( size_t ky = 0; ky < vkernel.height(); ky++ )
				vkern.ptr()[ ky ] = vkernel( 0, ky );

			if( symh && symv ) {
				if( src.channels() == 1 )
					return convolveSeparableTemplate<uint8_t,uint8_t,Fixed,Fixed>( dst, src, hkern.ptr(), hkernel.width(), vkern.ptr(), vkernel.height(),
																				  &SIMD::ConvolveHorizontalSym1u8_to_fx, &SIMD::ConvolveClampVertSym_fx_to_u8, btype );
				else if( src.channels() == 2 )
					return convolveSeparableTemplate<uint8_t,uint8_t,Fixed,Fixed>( dst, src, hkern.ptr(), hkernel.width(), vkern.ptr(), vkernel.height(),
																				  &SIMD::ConvolveHorizontalSym2u8_to_fx, &SIMD::ConvolveClampVertSym_fx_to_u8, btype );
				else if( src.channels() == 4 )
					return convolveSeparableTemplate<uint8_t,uint8_t,Fixed,Fixed>( dst, src, hkern.ptr(), hkernel.width(), vkern.ptr(), vkernel.height(),
																				  &SIMD::ConvolveHorizontalSym4u8_to_fx, &SIMD::ConvolveClampVertSym_fx_to_u8, btype );
			} else  {
				if( src.channels() == 1 )
					return convolveSeparableTemplate<uint8_t,uint8_t,Fixed,Fixed>( dst, src, hkern.ptr(), hkernel.width(), vkern.ptr(), vkernel.height(),
																				  &SIMD::ConvolveHorizontal1u8_to_fx, &SIMD::ConvolveClampVert_fx_to_u8, btype );
				else if( src.channels() == 2 )
					return convolveSeparableTemplate<uint8_t,uint8_t,Fixed,Fixed>( dst, src, hkern.ptr(), hkernel.width(), vkern.ptr(), vkernel.height(),
																				  &SIMD::ConvolveHorizontal2u8_to_fx, &SIMD::ConvolveClampVert_fx_to_u8, btype );
				else if( src.channels() == 4 )
					return convolveSeparableTemplate<uint8_t,uint8_t,Fixed,Fixed>( dst, src, hkern.ptr(), hkernel.width(), vkern.ptr(), vkernel.height(),
																				  &SIMD::ConvolveHorizontal4u8_to_fx, &SIMD::ConvolveClampVert_fx_to_u8, btype );
			}
#else
			if( symh && !symv ) {
				if( src.channels() == 1 )
					return convolveSeparableTemplate<uint8_t,uint8_t,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontalSym1u8_to_f, &SIMD::ConvolveClampVert_f_to_u8, btype );
				else if( src.channels() == 2 )
					return convolveSeparableTemplate<uint8_t,uint8_t,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontalSym2u8_to_f, &SIMD::ConvolveClampVert_f_to_u8, btype );
				else if( src.channels() == 4 )
					return convolveSeparableTemplate<uint8_t,uint8_t,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontalSym4u8_to_f, &SIMD::ConvolveClampVert_f_to_u8, btype );
			} else if( !symh && symv ) {
				if( src.channels() == 1 )
					return convolveSeparableTemplate<uint8_t,uint8_t,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontal1u8_to_f, &SIMD::ConvolveClampVertSym_f_to_u8, btype );
				else if( src.channels() == 2 )
					return convolveSeparableTemplate<uint8_t,uint8_t,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontal2u8_to_f, &SIMD::ConvolveClampVertSym_f_to_u8, btype );
				else if( src.channels() == 4 )
					return convolveSeparableTemplate<uint8_t,uint8_t,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontal4u8_to_f, &SIMD::ConvolveClampVertSym_f_to_u8, btype );
			} else if( symh && symv ) {
				if( src.channels() == 1 )
					return convolveSeparableTemplate<uint8_t,uint8_t,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontalSym1u8_to_f, &SIMD::ConvolveClampVertSym_f_to_u8, btype );
				else if( src.channels() == 2 )
					return convolveSeparableTemplate<uint8_t,uint8_t,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontalSym2u8_to_f, &SIMD::ConvolveClampVertSym_f_to_u8, btype );
				else if( src.channels() == 4 )
					return convolveSeparableTemplate<uint8_t,uint8_t,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontalSym4u8_to_f, &SIMD::ConvolveClampVertSym_f_to_u8, btype );
			} else {
				if( src.channels() == 1 )
					return convolveSeparableTemplate<uint8_t,uint8_t,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																				  &SIMD::ConvolveHorizontal1u8_to_f, &SIMD::ConvolveClampVert_f_to_u8, btype );
				else if( src.channels() == 2 )
					return convolveSeparableTemplate<uint8_t,uint8_t,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																				  &SIMD::ConvolveHorizontal2u8_to_f, &SIMD::ConvolveClampVert_f_to_u8, btype );
				else if( src.channels() == 4 )
					return convolveSeparableTemplate<uint8_t,uint8_t,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																				  &SIMD::ConvolveHorizontal4u8_to_f, &SIMD::ConvolveClampVert_f_to_u8, btype );
			}
#endif
		} else if( src.format().type == IFORMAT_TYPE_UINT8 && dst.format().type == IFORMAT_TYPE_INT16 ) {
			if( symh && !symv ) {
				if( src.channels() == 1 )
					return convolveSeparableTemplate<int16_t,uint8_t,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontalSym1u8_to_f, &SIMD::ConvolveClampVert_f_to_s16, btype );
				else if( src.channels() == 2 )
					return convolveSeparableTemplate<int16_t,uint8_t,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontalSym2u8_to_f, &SIMD::ConvolveClampVert_f_to_s16, btype );
				else if( src.channels() == 4 )
					return convolveSeparableTemplate<int16_t,uint8_t,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontalSym4u8_to_f, &SIMD::ConvolveClampVert_f_to_s16, btype );
			} else if( !symh && symv ) {
				if( src.channels() == 1 )
					return convolveSeparableTemplate<int16_t,uint8_t,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontal1u8_to_f, &SIMD::ConvolveClampVertSym_f_to_s16, btype );
				else if( src.channels() == 2 )
					return convolveSeparableTemplate<int16_t,uint8_t,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontal2u8_to_f, &SIMD::ConvolveClampVertSym_f_to_s16, btype );
				else if( src.channels() == 4 )
					return convolveSeparableTemplate<int16_t,uint8_t,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontal4u8_to_f, &SIMD::ConvolveClampVertSym_f_to_s16, btype );
			} else if( symh && symv ) {
				if( src.channels() == 1 )
					return convolveSeparableTemplate<int16_t,uint8_t,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontalSym1u8_to_f, &SIMD::ConvolveClampVertSym_f_to_s16, btype );
				else if( src.channels() == 2 )
					return convolveSeparableTemplate<int16_t,uint8_t,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontalSym2u8_to_f, &SIMD::ConvolveClampVertSym_f_to_s16, btype );
				else if( src.channels() == 4 )
					return convolveSeparableTemplate<int16_t,uint8_t,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																			  &SIMD::ConvolveHorizontalSym4u8_to_f, &SIMD::ConvolveClampVertSym_f_to_s16, btype );
			} else {
				if( src.channels() == 1 )
					return convolveSeparableTemplate<int16_t,uint8_t,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																				  &SIMD::ConvolveHorizontal1u8_to_f, &SIMD::ConvolveClampVert_f_to_s16, btype );
				else if( src.channels() == 2 )
					return convolveSeparableTemplate<int16_t,uint8_t,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																				  &SIMD::ConvolveHorizontal2u8_to_f, &SIMD::ConvolveClampVert_f_to_s16, btype );
				else if( src.channels() == 4 )
					return convolveSeparableTemplate<int16_t,uint8_t,float,float>( dst, src, hkernel.ptr(), hkernel.width(), vkernel.ptr(), vkernel.height(),
																				  &SIMD::ConvolveHorizontal4u8_to_f, &SIMD::ConvolveClampVert_f_to_s16, btype );
			}
		}

		throw CVTException("Unimplemented");
	}

}
