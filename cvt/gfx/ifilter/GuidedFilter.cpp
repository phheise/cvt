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

#include <cvt/gfx/ifilter/GuidedFilter.h>

#include <cvt/cl/kernel/guidedfilter/guidedfilter_calcab.h>
#include <cvt/cl/kernel/guidedfilter/guidedfilter_calcab_outerrgb.h>
#include <cvt/cl/kernel/guidedfilter/guidedfilter_applyab_gc.h>
#include <cvt/cl/kernel/guidedfilter/guidedfilter_applyab_gc_outer.h>
#include <cvt/cl/kernel/guidedfilter/guidedfilter_applyab_cc.h>

namespace cvt {
	static ParamInfoTyped<Image*> pin( "Input", true );
	static ParamInfoTyped<Image*> pinguide( "Guide", true );
	static ParamInfoTyped<Image*> pout( "Output", false );
	static ParamInfoTyped<int>	  pradius( "Radius", true );
	static ParamInfoTyped<float>  pepsilon( "Epsilon", true );

	static ParamInfo * _params[ 5 ] = {
		&pin,
		&pinguide,
		&pout,
		&pradius,
		&pepsilon,
	};

	GuidedFilter::GuidedFilter() :
		IFilter( "GuidedFilter", _params, 5, IFILTER_CPU | IFILTER_OPENCL ),
		_clguidedfilter_calcab( _guidedfilter_calcab_source, "guidedfilter_calcab" ),
		_clguidedfilter_calcab_outerrgb( _guidedfilter_calcab_outerrgb_source, "guidedfilter_calcab_outerrgb" ),
		_clguidedfilter_applyab_gc( _guidedfilter_applyab_gc_source, "guidedfilter_applyab_gc" ),
		_clguidedfilter_applyab_gc_outer( _guidedfilter_applyab_gc_outer_source, "guidedfilter_applyab_gc_outer" ),
		_clguidedfilter_applyab_cc( _guidedfilter_applyab_cc_source, "guidedfilter_applyab_cc" )
	{
	}

	void GuidedFilter::apply( Image& dst, const Image& src, const Image& guide, const int radius, const float epsilon, bool rgbcovariance ) const
	{
		// G guidance image, S source image

		if( rgbcovariance ) {
			applyGC_COV( dst, src, guide, radius, epsilon );
		} else if( src.format().channels <= 2 ) {
			applyGC( dst, src, guide, radius, epsilon );
		} else
			applyCC( dst, src, guide, radius, epsilon );
	}

	void GuidedFilter::applyGC( Image& dst, const Image& src, const Image& guide, const int radius, const float epsilon ) const
	{
		Image ia( src.width(), src.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image ib( src.width(), src.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );

		Image iint( src.width(), src.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL ); // FIXME: use only GRAY/GRAYALPHA for src and RGBA/GRAY/GRAYALPHA for GUIDE
		Image imeanG( src.width(), src.height(), IFormat::floatEquivalent( guide.format() ), IALLOCATOR_CL );
		Image imeanS( src.width(), src.height(), IFormat::floatEquivalent( src.format() ), IALLOCATOR_CL );
		Image imeanGS( src.width(), src.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL ); // FIXME: use only RGBA/GRAY/GRAYALPHA for SRC * GUIDE
		Image imeanGG( src.width(), src.height(), IFormat::floatEquivalent( guide.format() ), IALLOCATOR_CL );

		_intfilter.apply( iint, guide );
		_boxfilter.apply( imeanG, iint, radius, IFILTER_OPENCL );
		_intfilter.apply( iint, src );
		_boxfilter.apply( imeanS, iint, radius, IFILTER_OPENCL );
		_intfilter.apply( iint, guide, &src );
		_boxfilter.apply( imeanGS, iint, radius, IFILTER_OPENCL );
		_intfilter.apply( iint, guide, &guide );
		_boxfilter.apply( imeanGG, iint, radius, IFILTER_OPENCL );

		CLNDRange global( Math::pad16( src.width() ), Math::pad16( src.height() ) );
		CLNDRange local( 16, 16 );

		_clguidedfilter_calcab.setArg( 0, ia );
		_clguidedfilter_calcab.setArg( 1, ib );
		_clguidedfilter_calcab.setArg( 2, imeanG );
		_clguidedfilter_calcab.setArg( 3, imeanS );
		_clguidedfilter_calcab.setArg( 4, imeanGS );
		_clguidedfilter_calcab.setArg( 5, imeanGG );
		_clguidedfilter_calcab.setArg( 6, epsilon );
		_clguidedfilter_calcab.run( global, local);

		_intfilter.apply( iint, ia );
		_boxfilter.apply( ia, iint, radius, IFILTER_OPENCL );
		_intfilter.apply( iint, ib );
		_boxfilter.apply( ib, iint, radius, IFILTER_OPENCL );

		dst.reallocate( src.width(), src.height(), src.format(), IALLOCATOR_CL );
		_clguidedfilter_applyab_gc.setArg( 0, dst );
		_clguidedfilter_applyab_gc.setArg( 1, guide );
		_clguidedfilter_applyab_gc.setArg( 2, ia );
		_clguidedfilter_applyab_gc.setArg( 3, ib );
		_clguidedfilter_applyab_gc.run( global, local );
	}

	void GuidedFilter::applyGC_COV( Image& dst, const Image& src, const Image& guide, const int radius, const float epsilon ) const
	{
		Image ia( src.width(), src.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image ib( src.width(), src.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );

		Image iint( src.width(), src.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image iint2( src.width(), src.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image imeanG( src.width(), src.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image imeanS( src.width(), src.height(), IFormat::floatEquivalent( src.format() ), IALLOCATOR_CL );
		Image imeanGS( src.width(), src.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image imean_RR_RG_RB( src.width(), src.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image imean_GG_GB_BB( src.width(), src.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );

		_intfilter.apply( iint, guide );
		_boxfilter.apply( imeanG, iint, radius, IFILTER_OPENCL );
		_intfilter.apply( iint, src );
		_boxfilter.apply( imeanS, iint, radius, IFILTER_OPENCL );
		_intfilter.apply( iint, guide, &src );
		_boxfilter.apply( imeanGS, iint, radius, IFILTER_OPENCL );
		_intfilter.applyOuterRGB( iint, iint2, guide );
		_boxfilter.apply( imean_RR_RG_RB, iint, radius, IFILTER_OPENCL );
		_boxfilter.apply( imean_GG_GB_BB, iint2, radius, IFILTER_OPENCL );


		CLNDRange global( Math::pad16( src.width() ), Math::pad16( src.height() ) );
		CLNDRange local( 16, 16 );

		_clguidedfilter_calcab_outerrgb.setArg( 0, ia );
		_clguidedfilter_calcab_outerrgb.setArg( 1, ib );
		_clguidedfilter_calcab_outerrgb.setArg( 2, imeanG );
		_clguidedfilter_calcab_outerrgb.setArg( 3, imeanS );
		_clguidedfilter_calcab_outerrgb.setArg( 4, imeanGS );
		_clguidedfilter_calcab_outerrgb.setArg( 5, imean_RR_RG_RB );
		_clguidedfilter_calcab_outerrgb.setArg( 6, imean_GG_GB_BB );
		_clguidedfilter_calcab_outerrgb.setArg( 7, epsilon );
		_clguidedfilter_calcab_outerrgb.run( global, local );

		_intfilter.apply( iint, ia );
		_boxfilter.apply( ia, iint, radius, IFILTER_OPENCL );
		_intfilter.apply( iint, ib );
		_boxfilter.apply( ib, iint, radius, IFILTER_OPENCL );

		dst.reallocate( src.width(), src.height(), src.format(), IALLOCATOR_CL );
		_clguidedfilter_applyab_gc_outer.setArg( 0, dst );
		_clguidedfilter_applyab_gc_outer.setArg( 1, guide );
		_clguidedfilter_applyab_gc_outer.setArg( 2, ia );
		_clguidedfilter_applyab_gc_outer.setArg( 3, ib );
		_clguidedfilter_applyab_gc_outer.run( global, local );
	}

	void GuidedFilter::applyCC( Image& dst, const Image& src, const Image& guide, const int radius, const float epsilon ) const
	{
		Image ia( src.width(), src.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image ib( src.width(), src.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );

		Image iint( src.width(), src.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image imeanG( src.width(), src.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image imeanS( src.width(), src.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image imeanGS( src.width(), src.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image imeanGG( src.width(), src.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );

		_intfilter.apply( iint, guide );
		_boxfilter.apply( imeanG, iint, radius, IFILTER_OPENCL );
		_intfilter.apply( iint, src );
		_boxfilter.apply( imeanS, iint, radius, IFILTER_OPENCL );
		_intfilter.apply( iint, guide, &src );
		_boxfilter.apply( imeanGS, iint, radius, IFILTER_OPENCL );
		_intfilter.apply( iint, guide, &guide );
		_boxfilter.apply( imeanGG, iint, radius, IFILTER_OPENCL );

		CLNDRange global( Math::pad16( src.width() ), Math::pad16( src.height() ) );
		CLNDRange local( 16, 16 );

		_clguidedfilter_calcab.setArg( 0, ia );
		_clguidedfilter_calcab.setArg( 1, ib );
		_clguidedfilter_calcab.setArg( 2, imeanG );
		_clguidedfilter_calcab.setArg( 3, imeanS );
		_clguidedfilter_calcab.setArg( 4, imeanGS );
		_clguidedfilter_calcab.setArg( 5, imeanGG );
		_clguidedfilter_calcab.setArg( 6, epsilon );
		_clguidedfilter_calcab.run( global, local );

		_intfilter.apply( iint, ia );
		_boxfilter.apply( ia, iint, radius, IFILTER_OPENCL );
		_intfilter.apply( iint, ib );
		_boxfilter.apply( ib, iint, radius, IFILTER_OPENCL );

		dst.reallocate( src.width(), src.height(), src.format(), IALLOCATOR_CL );
		_clguidedfilter_applyab_cc.setArg( 0, dst );
		_clguidedfilter_applyab_cc.setArg( 1, guide );
		_clguidedfilter_applyab_cc.setArg( 2, ia );
		_clguidedfilter_applyab_cc.setArg( 3, ib );
		_clguidedfilter_applyab_cc.run( global, local );
	}


	void GuidedFilter::apply( const ParamSet* set, IFilterType t ) const
	{
		Image * in = set->arg<Image*>( 0 );
		Image * guide = set->arg<Image*>( 1 );
		Image * out = set->arg<Image*>( 2 );
		int radius = set->arg<int>( 3 );
		float epsilon = set->arg<float>( 4 );


		switch ( t ) {
			case IFILTER_OPENCL:
				this->apply( *out, *in, guide?*guide:*in, radius, epsilon );
				break;
			default:
				throw CVTException( "Not implemented" );
		}
	}

}
