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

#include <cvt/gfx/ifilter/ROFDenoise.h>
#include <cvt/util/Exception.h>
#include <cvt/math/Math.h>

#include <iostream>

namespace cvt {
	static ParamInfoTyped<Image*> pin( "Input", true );
	static ParamInfoTyped<Image*> pout( "Output", false );
	static ParamInfoTyped<int> pit( "Iterations", true);
	static ParamInfoTyped<float> plambda( "Lambda", true );

	static ParamInfo * _params[ 4 ] = {
		&pin,
		&pout,
		&pit,
		&plambda
	};

	ROFDenoise::ROFDenoise() : IFilter( "ROFDenoise", _params, 4, IFILTER_CPU )
	{
	}

#if 0
	static void multadd2( Image& idst, Image& dx, Image& dy, float lambda )
	{
		uint8_t* dst;
		uint8_t* src1;
		uint8_t* src2;
		size_t stridedst;
		size_t stridesrc1;
		size_t stridesrc2;
		float* pdst;
		float* psrc1;
		float* psrc2;
		size_t w, h;

		dst = idst.data();
		stridedst = idst.stride();
		src1 = dx.data();
		stridesrc1 = dx.stride();
		src2 = dy.data();
		stridesrc2 = dy.stride();

		h = idst.height();
		while( h-- ) {
			pdst = ( float* ) dst;
			psrc1 = ( float* ) src1;
			psrc2 = ( float* ) src2;

			w = idst.width() * idst.channels();
			while( w-- ) {
				*pdst++ += lambda * ( *psrc1++ + *psrc2++ );
			}

			dst += stridedst;
			src1 += stridesrc1;
			src2 += stridesrc2;
		}
	}
#endif

	static void multadd3( Image& idst, const Image& isrc, const Image& dx, const Image& dy, float lambda )
	{
		uint8_t* dst;
		uint8_t* odst;
		const uint8_t* src1;
		const uint8_t* src2;
		const uint8_t* src3;
		const uint8_t* osrc1;
		const uint8_t* osrc2;
		const uint8_t* osrc3;
		size_t stridedst;
		size_t stridesrc1;
		size_t stridesrc2;
		size_t stridesrc3;
		float* pdst;
		const float* psrc1;
		const float* psrc2;
		const float* psrc3;
		size_t w, h;

		odst = dst = idst.map( &stridedst );
		osrc1 = src1 = isrc.map( &stridesrc1 );
		osrc2 = src2 = dx.map( &stridesrc2 );
		osrc3 = src3 = dy.map( &stridesrc3 );

		h = idst.height();
		while( h-- ) {
			pdst = ( float* ) dst;
			psrc1 = ( float* ) src1;
			psrc2 = ( float* ) src2;
			psrc3 = ( float* ) src3;

			w = idst.width() * idst.channels();
			while( w-- ) {
				*pdst++ = *psrc1++ + lambda * ( *psrc2++ + *psrc3++ );
			}

			dst += stridedst;
			src1 += stridesrc1;
			src2 += stridesrc2;
			src3 += stridesrc3;
		}
		dy.unmap( osrc3 );
		dx.unmap( osrc2 );
		isrc.unmap( osrc1 );
		idst.unmap( odst );
	}

	static void multadd2_th( Image& idst1, Image& idst2, const Image& dx, const Image& dy, float taulambda )
	{
		uint8_t* dst1;
		uint8_t* dst2;
		uint8_t* odst1;
		uint8_t* odst2;
		const uint8_t* src1;
		const uint8_t* src2;
		const uint8_t* osrc1;
		const uint8_t* osrc2;
		size_t stridedst1;
		size_t stridedst2;
		size_t stridesrc1;
		size_t stridesrc2;
		float* pdst1;
		float* pdst2;
		const float* psrc1;
		const float* psrc2;
		size_t w, h;
		float tmp1, tmp2, norm;

		odst1 = dst1 = idst1.map( &stridedst1 );
		odst2 = dst2 = idst2.map( &stridedst2 );
		osrc1 = src1 = dx.map( &stridesrc1 );
		osrc2 = src2 = dy.map( &stridesrc2 );

		h = idst1.height();
		while( h-- ) {
			pdst1 = ( float* ) dst1;
			pdst2 = ( float* ) dst2;
			psrc1 = ( float* ) src1;
			psrc2 = ( float* ) src2;

			w = idst1.width() * idst1.channels();
			while( w-- ) {
				tmp1 = *pdst1 + taulambda * *psrc1++;
				tmp2 = *pdst2 + taulambda * *psrc2++;
				norm = Math::min( 1.0f, Math::fastInvSqrtf( tmp1 * tmp1 + tmp2 * tmp2 ) );
				*pdst1++ = tmp1 * norm;
				*pdst2++ = tmp2 * norm;

				//				norm = 1.0f / Math::max( 1.0f,  Math::abs( tmp1 ) + Math::abs( tmp2 ) );
				//				*pdst1++ = tmp1 / Math::max( 1.0f, Math::abs( tmp1 ) );
				//				*pdst2++ = tmp2 / Math::max( 1.0f, Math::abs( tmp2 ) );
			}

			dst1 += stridedst1;
			dst2 += stridedst2;
			src1 += stridesrc1;
			src2 += stridesrc2;
		}
		dy.unmap( osrc2 );
		dx.unmap( osrc1 );
		idst2.unmap( odst2 );
		idst1.unmap( odst1 );
	}

	void ROFDenoise::apply( Image& dst, const Image& src, float lambda, uint64_t iter ) const
	{
		Image dx, dy, px, py;
#if 0
		Image kerndx( 5, 1, IOrder::GRAY, IType::FLOAT );
		Image kerndy( 1, 5, IOrder::GRAY, IType::FLOAT );
		Image kerndxrev( 5, 1, IOrder::GRAY, IType::FLOAT );
		Image kerndyrev( 1, 5, IOrder::GRAY, IType::FLOAT );

		{
			float* data;
			data = ( float* ) kerndx.data();
			*data++ =  0.1f;
			*data++ = -0.9f;
			*data++ =  0.9f;
			*data++ = -0.1f;
			*data++ =  0.0f;

			data = ( float* ) kerndy.scanline( 0 );
			*data++ =  0.1f;
			data = ( float* ) kerndy.scanline( 1 );
			*data++ = -0.9f;
			data = ( float* ) kerndy.scanline( 2 );
			*data++ =  0.9f;
			data = ( float* ) kerndy.scanline( 3 );
			*data++ = -0.1f;
			data = ( float* ) kerndy.scanline( 4 );
			*data++ =  0.0f;

			data = ( float* ) kerndxrev.data();
			*data++ =  0.0f;
			*data++ =  0.1f;
			*data++ = -0.9f;
			*data++ =  0.9f;
			*data++ = -0.1f;

			data = ( float* ) kerndyrev.scanline( 0 );
			*data++ =  0.0f;
			data = ( float* ) kerndyrev.scanline( 1 );
			*data++ =  0.1f;
			data = ( float* ) kerndyrev.scanline( 2 );
			*data++ = -0.9f;
			data = ( float* ) kerndyrev.scanline( 3 );
			*data++ =  0.9f;
			data = ( float* ) kerndyrev.scanline( 4 );
			*data++ = -0.1f;
		}
#else
		IKernel kerndx( IKernel::HAAR_HORIZONTAL_3 );
		IKernel kerndy( IKernel::HAAR_VERTICAL_3 );
		IKernel kerndxrev( IKernel::HAAR_HORIZONTAL_3 );
		//		kerndxrev.scale( -1.0f );
		IKernel kerndyrev( IKernel::HAAR_VERTICAL_3 );
		//		kerndyrev.scale( -1.0f );
#endif

#define TAU 0.249f
		dst = src;
		px.reallocate( src );
		py.reallocate( src );
		dx.reallocate( dst );
		dy.reallocate( dst );
		px.fill( Color( 0.0f, 0.0f ) );
		py.fill( Color( 0.0f, 0.0f ) );

		while( iter-- ) {
			dst.convolve( dx, kerndx );
			dst.convolve( dy, kerndy );
			//			dst.ddx( dx );
			//			dst.ddy( dy );
			multadd2_th( px, py, dx, dy, TAU / lambda );
			px.convolve( dx, kerndxrev );
			py.convolve( dy, kerndyrev );
			//			px.ddx( dx, false );
			//			py.ddy( dy, false );

			multadd3( dst, src, dx, dy, lambda );
		}

	}

	void ROFDenoise::apply( const ParamSet* set, IFilterType t ) const
	{
		if( !( getIFilterType() & t ) )
			throw CVTException("Invalid filter type (CPU/GPU)!");
		Image* dst;
		Image* src;
		float lambda;
		uint64_t iter;

		src = set->arg<Image*>( 0 );
		dst = set->arg<Image*>( 1 );
		iter = set->arg<int>( 2 );
		lambda = set->arg<float>( 3 );				

		apply( *dst, *src, lambda, iter );
	}

}
