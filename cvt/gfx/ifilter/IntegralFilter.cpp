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

#include <cvt/gfx/ifilter/IntegralFilter.h>

#include <cvt/cl/kernel/prefixsum/prefixsum_pblock.h>
#include <cvt/cl/kernel/prefixsum/prefixsum_pblock_mul2.h>
#include <cvt/cl/kernel/prefixsum/prefixsum_pblock_mul2_shifted.h>
#include <cvt/cl/kernel/prefixsum/prefixsum_pblock_sqr.h>
#include <cvt/cl/kernel/prefixsum/prefixsum_pblock_outerrgb.h>
#include <cvt/cl/kernel/prefixsum/prefixsum_horiz.h>
#include <cvt/cl/kernel/prefixsum/prefixsum_vert.h>
#include <cvt/cl/kernel/prefixsum/prefixsum_block2.h>

namespace cvt {
	static ParamInfoTyped<Image*> pin( "Input", true );
	static ParamInfoTyped<Image*> pin2( "Input2", true );
	static ParamInfoTyped<Image*> pout( "Output", false );

	static ParamInfo * _params[ 3 ] = {
		&pin,
		&pin2,
		&pout,
	};

	IntegralFilter::IntegralFilter() :
		IFilter( "IntegralFilter", _params, 3, IFILTER_CPU | IFILTER_OPENCL ),
		_clprefixsum_blockp( _prefixsum_pblock_source, "prefixsum_pblock" ),
		_clprefixsum_blockp_sqr( _prefixsum_pblock_sqr_source, "prefixsum_pblock_sqr" ),
		_clprefixsum_blockp_mul2( _prefixsum_pblock_mul2_source, "prefixsum_pblock_mul2" ),
		_clprefixsum_blockp_mul2_shifted( _prefixsum_pblock_mul2_shifted_source, "prefixsum_pblock_mul2_shifted" ),
		_clprefixsum_blockp_outerrgb( _prefixsum_pblock_outerrgb_source, "prefixsum_pblock_outerrgb" ),
		_clprefixsum_horiz( _prefixsum_horiz_source, "prefixsum_horiz" ),
		_clprefixsum_vert( _prefixsum_vert_source, "prefixsum_vert" ),
		_clprefixsum_block2( _prefixsum_block2_source, "prefixsum_block2" ),
		_blocksize( 16 )
	{
		size_t maxwg = Math::max( _clprefixsum_blockp.maxWorkGroupSize(), _clprefixsum_block2.maxWorkGroupSize() );
		while( _blocksize * _blocksize > maxwg  ) {
			_blocksize >>= 1;
		}
	}

	void IntegralFilter::apply( Image& dst, const Image& src, const Image* src2 ) const
	{
		dst.reallocate( src.width(), src.height(), IFormat::floatEquivalent( src.format() ), IALLOCATOR_CL );
		// FIXME: hardcoded work-group size

		if( src2 ) {
			if( &src == src2 ) {
				_clprefixsum_blockp_sqr.setArg( 0, dst );
				_clprefixsum_blockp_sqr.setArg( 1, src );
				_clprefixsum_blockp_sqr.setArg( 2, CLLocalSpace( sizeof( cl_float4 ) * _blocksize * _blocksize ) );
				_clprefixsum_blockp_sqr.run( CLNDRange( Math::pad( src.width(), _blocksize ), Math::pad( src.height(), _blocksize ) ), CLNDRange( _blocksize, _blocksize ) );
			} else {
				_clprefixsum_blockp_mul2.setArg( 0, dst );
				_clprefixsum_blockp_mul2.setArg( 1, src );
				_clprefixsum_blockp_mul2.setArg( 2, *src2 );
				_clprefixsum_blockp_mul2.setArg( 3, CLLocalSpace( sizeof( cl_float4 ) * _blocksize * _blocksize ) );
				_clprefixsum_blockp_mul2.run( CLNDRange( Math::pad( src.width(), _blocksize ), Math::pad( src.height(), _blocksize ) ), CLNDRange( _blocksize, _blocksize ) );
			}
		} else {
			_clprefixsum_blockp.setArg( 0, dst );
			_clprefixsum_blockp.setArg( 1, src );
			_clprefixsum_blockp.setArg( 2, CLLocalSpace( sizeof( cl_float4 ) * _blocksize * _blocksize ) );
			_clprefixsum_blockp.run( CLNDRange( Math::pad( src.width(), _blocksize ), Math::pad( src.height(), _blocksize ) ), CLNDRange( _blocksize, _blocksize ) );

		}

		_clprefixsum_horiz.setArg( 0, dst );
		_clprefixsum_horiz.setArg( 1, dst );
		_clprefixsum_horiz.setArg<int>( 2, _blocksize );
		_clprefixsum_horiz.run( CLNDRange( Math::pad( dst.height(), _blocksize ) ), CLNDRange( _blocksize ) );

		_clprefixsum_vert.setArg( 0, dst );
		_clprefixsum_vert.setArg( 1, dst );
		_clprefixsum_vert.setArg<int>( 2, _blocksize );
		_clprefixsum_vert.run( CLNDRange( Math::pad( dst.width(), _blocksize ) ), CLNDRange( _blocksize ) );

		_clprefixsum_block2.setArg( 0, dst );
		_clprefixsum_block2.setArg( 1, dst );
//		_clprefixsum_block2.setArg( 2, CLLocalSpace( sizeof( cl_float4 ) * _blocksize ) );
//		_clprefixsum_block2.setArg( 3, CLLocalSpace( sizeof( cl_float4 ) * _blocksize ) );
		_clprefixsum_block2.run( CLNDRange( Math::pad( dst.width(), _blocksize ), Math::pad( dst.height(), _blocksize ) ), CLNDRange( _blocksize, _blocksize ) );
	}


	void IntegralFilter::applyOuterRGB( Image& dst_RR_RG_RB, Image& dst_GG_GB_BB, const Image& src ) const
	{
		_clprefixsum_blockp_outerrgb.setArg( 0, dst_RR_RG_RB );
		_clprefixsum_blockp_outerrgb.setArg( 1, dst_GG_GB_BB );
		_clprefixsum_blockp_outerrgb.setArg( 2, src );
		_clprefixsum_blockp_outerrgb.setArg( 3, CLLocalSpace( sizeof( cl_float8 ) * _blocksize * _blocksize ) );
		_clprefixsum_blockp_outerrgb.run( CLNDRange( Math::pad( src.width(), _blocksize ), Math::pad( src.height(), _blocksize ) ), CLNDRange( _blocksize, _blocksize ) );

		_clprefixsum_horiz.setArg( 0, dst_RR_RG_RB );
		_clprefixsum_horiz.setArg( 1, dst_RR_RG_RB );
		_clprefixsum_horiz.setArg<int>( 2, _blocksize );
		_clprefixsum_horiz.run( CLNDRange( Math::pad( dst_RR_RG_RB.height(), _blocksize ) ), CLNDRange( _blocksize ) );

		_clprefixsum_horiz.setArg( 0, dst_GG_GB_BB );
		_clprefixsum_horiz.setArg( 1, dst_GG_GB_BB );
		_clprefixsum_horiz.setArg<int>( 2, _blocksize );
		_clprefixsum_horiz.run( CLNDRange( Math::pad( dst_GG_GB_BB.height(), _blocksize ) ), CLNDRange( _blocksize ) );

		_clprefixsum_vert.setArg( 0, dst_RR_RG_RB );
		_clprefixsum_vert.setArg( 1, dst_RR_RG_RB );
		_clprefixsum_vert.setArg<int>( 2, _blocksize );
		_clprefixsum_vert.run( CLNDRange( Math::pad( dst_RR_RG_RB.width(), _blocksize ) ), CLNDRange( _blocksize ) );

		_clprefixsum_vert.setArg( 0, dst_GG_GB_BB );
		_clprefixsum_vert.setArg( 1, dst_GG_GB_BB );
		_clprefixsum_vert.setArg<int>( 2, _blocksize );
		_clprefixsum_vert.run( CLNDRange( Math::pad( dst_GG_GB_BB.width(), _blocksize ) ), CLNDRange( _blocksize ) );

		_clprefixsum_block2.setArg( 0, dst_RR_RG_RB );
		_clprefixsum_block2.setArg( 1, dst_RR_RG_RB );
//		_clprefixsum_block2.setArg( 2, CLLocalSpace( sizeof( cl_float4 ) * _blocksize ) );
//		_clprefixsum_block2.setArg( 3, CLLocalSpace( sizeof( cl_float4 ) * _blocksize ) );
		_clprefixsum_block2.run( CLNDRange( Math::pad( dst_RR_RG_RB.width(), _blocksize ), Math::pad( dst_RR_RG_RB.height(), _blocksize ) ), CLNDRange( _blocksize, _blocksize ) );

		_clprefixsum_block2.setArg( 0, dst_GG_GB_BB );
		_clprefixsum_block2.setArg( 1, dst_GG_GB_BB );
//		_clprefixsum_block2.setArg( 2, CLLocalSpace( sizeof( cl_float4 ) * _blocksize ) );
//		_clprefixsum_block2.setArg( 3, CLLocalSpace( sizeof( cl_float4 ) * _blocksize ) );
		_clprefixsum_block2.run( CLNDRange( Math::pad( dst_GG_GB_BB.width(), _blocksize ), Math::pad( dst_GG_GB_BB.height(), _blocksize ) ), CLNDRange( _blocksize, _blocksize ) );
	}


	void IntegralFilter::applyShifted( Image& dst, const Image& src, const Image& src2, float dx, float dy ) const
	{
		Vector2f shift( dx, dy );

		_clprefixsum_blockp_mul2_shifted.setArg( 0, dst );
		_clprefixsum_blockp_mul2_shifted.setArg( 1, src );
		_clprefixsum_blockp_mul2_shifted.setArg( 2, src2 );
		_clprefixsum_blockp_mul2_shifted.setArg( 3, shift );
		_clprefixsum_blockp_mul2_shifted.setArg( 4, CLLocalSpace( sizeof( cl_float4 ) * _blocksize * _blocksize ) );
		_clprefixsum_blockp_mul2_shifted.run( CLNDRange( Math::pad( src.width(), _blocksize ), Math::pad( src.height(), _blocksize ) ), CLNDRange( _blocksize, _blocksize ) );

		_clprefixsum_horiz.setArg( 0, dst );
		_clprefixsum_horiz.setArg( 1, dst );
		_clprefixsum_horiz.setArg<int>( 2, _blocksize );
		_clprefixsum_horiz.run( CLNDRange( Math::pad( dst.height(), _blocksize ) ), CLNDRange( _blocksize ) );

		_clprefixsum_vert.setArg( 0, dst );
		_clprefixsum_vert.setArg( 1, dst );
		_clprefixsum_vert.setArg<int>( 2, _blocksize );
		_clprefixsum_vert.run( CLNDRange( Math::pad( dst.width(), _blocksize ) ), CLNDRange( _blocksize ) );

		_clprefixsum_block2.setArg( 0, dst );
		_clprefixsum_block2.setArg( 1, dst );
		//		_clprefixsum_block2.setArg( 2, CLLocalSpace( sizeof( cl_float4 ) * _blocksize ) );
		//		_clprefixsum_block2.setArg( 3, CLLocalSpace( sizeof( cl_float4 ) * _blocksize ) );
		_clprefixsum_block2.run( CLNDRange( Math::pad( dst.width(), _blocksize ), Math::pad( dst.height(), _blocksize ) ), CLNDRange( _blocksize, _blocksize ) );
	}


	void IntegralFilter::apply( const ParamSet* set, IFilterType t ) const
	{
		Image * in = set->arg<Image*>( 0 );
		Image * in2 = set->arg<Image*>( 1 );
		Image * out = set->arg<Image*>( 2 );

		switch ( t ) {
			case IFILTER_OPENCL:
				this->apply( *out, *in, in2 );
				break;
			default:
				throw CVTException( "Not implemented" );
		}
	}

}
