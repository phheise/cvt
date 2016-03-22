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

#include <cvt/vision/PMHuberStereo.h>
#include <cvt/util/Exception.h>
#include <cvt/cl/CLBuffer.h>
#include <cvt/cl/kernel/pmhstereo.h>


namespace cvt {
#define KX 16
#define KY 16
#define VIEWSAMPLES 4

	struct PMHVIEWPROP {
		cl_int n;
		cl_float4 samples[ VIEWSAMPLES ];
	};


	PMHuberStereo::PMHuberStereo() :
		_clpmh_init( _pmhstereo_source, "pmhstereo_init" ),
		_clpmh_propagate( _pmhstereo_source, "pmhstereo_propagate_view" ),
		_clpmh_depthmap( _pmhstereo_source, "pmhstereo_depthmap" ),
		_clpmh_viewbufclear( _pmhstereo_source, "pmhstereo_viewbuf_clear" ),
		_clpmh_fill( _pmhstereo_source, "pmhstereo_fill_state" ),
		_clpmh_consistency( _pmhstereo_source, "pmhstereo_consistency" ),
		_clpmh_filldepthmap( _pmhstereo_source, "pmhstereo_fill_depthmap" ),
		_clpmh_fillnormalmap( _pmhstereo_source, "pmhstereo_fill_normalmap" ),
		_clpmh_normaldepth( _pmhstereo_source, "pmhstereo_normal_depth" ),
		_clpmh_clear( _pmhstereo_source, "pmhstereo_clear" ),
		_clpmh_occmap( _pmhstereo_source, "pmhstereo_occmap" ),
		_clpmh_gradxy( _pmhstereo_source, "pmhstereo_gradxy" ),
		_clpmh_weight( _pmhstereo_source, "pmhstereo_weight" )
	{

	}

	PMHuberStereo::~PMHuberStereo()
	{
	}

	void PMHuberStereo::depthMap( Image& dmap, const Image& left, const Image& right, size_t patchsize, float depthmax, size_t iterations, size_t viewsamples, float dscale, Image* normalmap )
	{
		if( left.width() != right.width() || left.height() != right.height() ||
		    left.memType() != IALLOCATOR_CL || right.memType() != IALLOCATOR_CL )
			throw CVTException( "Left/Right stereo images inconsistent or incompatible memory type" );

		const float maxdispdiff = 0.5f;
		const float maxanglediff = 10.0f;
		const float thetascale = 50.0f;
		float theta = 0.0f;

        if( dscale <= 0.0f )
			dscale = 1.0f / depthmax;

		CLBuffer viewbuf1( sizeof( PMHVIEWPROP ) * left.width() * left.height() );
		CLBuffer viewbuf2( sizeof( PMHVIEWPROP ) * right.width() * right.height() );

		Image leftgrad( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image rightgrad( right.width(), right.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image leftweight( left.width(), left.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );
		Image rightweight( right.width(), right.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );
		Image leftsmooth( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image rightsmooth( right.width(), right.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image clsmoothtmp( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image clsmoothtmp2( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );

		/* calculate gradient image */
		_clpmh_gradxy.setArg( 0, leftgrad );
		_clpmh_gradxy.setArg( 1, left );
		_clpmh_gradxy.run( CLNDRange( Math::pad( left.width(), KX ), Math::pad( left.height(), KY ) ), CLNDRange( KX, KY ) );

		_clpmh_gradxy.setArg( 0, rightgrad );
		_clpmh_gradxy.setArg( 1, right );
		_clpmh_gradxy.run( CLNDRange( Math::pad( right.width(), KX ), Math::pad( right.height(), KY ) ), CLNDRange( KX, KY ) );

		/* calculate huber weights */
		_clpmh_weight.setArg( 0, leftweight );
		_clpmh_weight.setArg( 1, left );
		_clpmh_weight.run( CLNDRange( Math::pad( left.width(), KX ), Math::pad( left.height(), KY ) ), CLNDRange( KX, KY ) );

		_clpmh_weight.setArg( 0, rightweight );
		_clpmh_weight.setArg( 1, right );
		_clpmh_weight.run( CLNDRange( Math::pad( right.width(), KX ), Math::pad( right.height(), KY ) ), CLNDRange( KX, KY ) );

		/* clear the smoothed images */
		_clpmh_clear.setArg( 0, leftsmooth );
		_clpmh_clear.run( CLNDRange( Math::pad( leftsmooth.width(), KX ), Math::pad( leftsmooth.height(), KY ) ), CLNDRange( KX, KY ) );

		_clpmh_clear.setArg( 0, rightsmooth );
		_clpmh_clear.run( CLNDRange( Math::pad( rightsmooth.width(), KX ), Math::pad( rightsmooth.height(), KY ) ), CLNDRange( KX, KY ) );


		Image clmatches1_1( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image clmatches1_2( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image* clmatches1[ 2 ] = { &clmatches1_1, &clmatches1_2 };

		Image clmatches2_1( right.width(), right.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image clmatches2_2( right.width(), right.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image* clmatches2[ 2 ] = { &clmatches2_1, &clmatches2_2 };

		/* PMH init */
		_clpmh_init.setArg( 0, *clmatches1[ 0 ] );
		_clpmh_init.setArg( 1, left );
		_clpmh_init.setArg( 2, right );
		_clpmh_init.setArg( 3, leftgrad );
		_clpmh_init.setArg( 4, rightgrad );
		_clpmh_init.setArg( 5, ( int ) patchsize );
		_clpmh_init.setArg( 6, depthmax );
		_clpmh_init.setArg<int>( 7, 1 );
		_clpmh_init.run( CLNDRange( Math::pad( left.width(), KX ), Math::pad( left.height(), KY ) ), CLNDRange( KX, KY ) );

		_clpmh_init.setArg( 0, *clmatches2[ 0 ] );
		_clpmh_init.setArg( 1, right );
		_clpmh_init.setArg( 2, left );
		_clpmh_init.setArg( 3, rightgrad );
		_clpmh_init.setArg( 4, leftgrad );
		_clpmh_init.setArg( 5, ( int ) patchsize );
		_clpmh_init.setArg( 6, depthmax );
		_clpmh_init.setArg<int>( 7, 0 );
		_clpmh_init.run( CLNDRange( Math::pad( right.width(), KX ), Math::pad( right.height(), KY ) ), CLNDRange( KX, KY ) );

		/* Clear view propagation buffer for the right view*/
		_clpmh_viewbufclear.setArg( 0, viewbuf2 );
		_clpmh_viewbufclear.setArg( 1, ( int ) right.width() );
		_clpmh_viewbufclear.setArg( 2, ( int ) right.height() );
		_clpmh_viewbufclear.run( CLNDRange( Math::pad( right.width(), KX ), Math::pad( right.height(), KY ) ), CLNDRange( KX, KY ) );

		for( size_t iter = 0; iter < iterations; iter++ ) {
			int swap = iter & 1;
#if 1
			std::cout << "Theta: " << theta << std::endl;
			_clpmh_depthmap.setArg( 0, clsmoothtmp );
			_clpmh_depthmap.setArg( 1, *clmatches1[ swap ] );
			_clpmh_depthmap.setArg( 2, dscale );
			_clpmh_depthmap.runWait( CLNDRange( Math::pad( left.width(), KX ), Math::pad( left.height(), KY ) ), CLNDRange( KX, KY ) );
			clsmoothtmp.save("stereo1.png");
			std::cout << "Wrote stereo1.png" << std::endl;

			_clpmh_depthmap.setArg( 0, clsmoothtmp );
			_clpmh_depthmap.setArg( 1, *clmatches2[ swap ]  );
			_clpmh_depthmap.setArg( 2, dscale );
			_clpmh_depthmap.runWait( CLNDRange( Math::pad( right.width(), KX ), Math::pad( right.height(), KY ) ), CLNDRange( KX, KY ) );
			clsmoothtmp.save("stereo2.png");
			std::cout << "Wrote stereo2.png" << std::endl;

/*			clconsistency.setArg( 0, cloutput1 );
			clconsistency.setArg( 1, *clmatches1[ swap ] );
			clconsistency.setArg( 2, *clmatches2[ swap ] );
			clconsistency.setArg( 3, lr );
			clconsistency.runWait( CLNDRange( Math::pad( clinput2.width(), KX ), Math::pad( clinput2.height(), KY ) ), CLNDRange( KX, KY ) );
			cloutput1.save("stereoconsistency.png");

			clfilldepthmap.setArg( 0, cloutput2 );
			clfilldepthmap.setArg( 1, cloutput1 );
			clfilldepthmap.setArg( 2, 4.0f / 255.0f );
			clfilldepthmap.runWait( CLNDRange( Math::pad( clinput2.width(), KX ), Math::pad( clinput2.height(), KY ) ), CLNDRange( KX, KY ) );

			cloutput2.save("stereofill.png");
			cloutput2.save("stereofill.cvtraw");
*/
			getchar();
#endif
			_clpmh_viewbufclear.setArg( 0, viewbuf1 );
			_clpmh_viewbufclear.setArg( 1, ( int ) left.width() );
			_clpmh_viewbufclear.setArg( 2, ( int ) left.height() );
			_clpmh_viewbufclear.runWait( CLNDRange( Math::pad( left.width(), KX ), Math::pad( left.height(), KY ) ), CLNDRange( KX, KY ) );

			_clpmh_propagate.setArg( 0, *clmatches1[ 1 - swap ] );
			_clpmh_propagate.setArg( 1, *clmatches1[ swap ] );
			_clpmh_propagate.setArg( 2, left );
			_clpmh_propagate.setArg( 3, right );
			_clpmh_propagate.setArg( 4, leftgrad );
			_clpmh_propagate.setArg( 5, rightgrad );
			_clpmh_propagate.setArg( 6, leftsmooth );
			_clpmh_propagate.setArg( 7, theta );
			_clpmh_propagate.setArg( 8, ( int ) patchsize );
			_clpmh_propagate.setArg( 9, depthmax );
			_clpmh_propagate.setArg<int>( 10, 1 ); // left to right
			_clpmh_propagate.setArg( 11, ( int ) iter );
			_clpmh_propagate.setArg( 12, viewbuf2 );
			_clpmh_propagate.setArg( 13, viewbuf1 );
			_clpmh_propagate.runWait( CLNDRange( Math::pad( left.width(), KX ), Math::pad( left.height(), KY ) ), CLNDRange( KX, KY ) );

			_clpmh_viewbufclear.setArg( 0, viewbuf2 );
			_clpmh_viewbufclear.setArg( 1, ( int ) right.width() );
			_clpmh_viewbufclear.setArg( 2, ( int ) right.height() );
			_clpmh_viewbufclear.runWait( CLNDRange( Math::pad( right.width(), KX ), Math::pad( right.height(), KY ) ), CLNDRange( KX, KY ) );

			_clpmh_propagate.setArg( 0, *clmatches2[ 1 - swap ] );
			_clpmh_propagate.setArg( 1, *clmatches2[ swap ] );
			_clpmh_propagate.setArg( 2, right );
			_clpmh_propagate.setArg( 3, left );
			_clpmh_propagate.setArg( 4, rightgrad );
			_clpmh_propagate.setArg( 5, leftgrad );
			_clpmh_propagate.setArg( 6, rightsmooth );
			_clpmh_propagate.setArg( 7, theta );
			_clpmh_propagate.setArg( 8, ( int ) patchsize );
			_clpmh_propagate.setArg( 9, depthmax );
			_clpmh_propagate.setArg<int>( 10, 0 ); // right to left
			_clpmh_propagate.setArg( 11, ( int ) iter );
			_clpmh_propagate.setArg( 12, viewbuf1 );
			_clpmh_propagate.setArg( 13, viewbuf2 );
			_clpmh_propagate.runWait( CLNDRange( Math::pad( right.width(), KX ), Math::pad( right.height(), KY ) ), CLNDRange( KX, KY ) );

			_clpmh_consistency.setArg( 0, clsmoothtmp );
			_clpmh_consistency.setArg( 1, *clmatches1[ 1 - swap ] );
			_clpmh_consistency.setArg( 2, *clmatches2[ 1 - swap ] );
			_clpmh_consistency.setArg( 3, maxdispdiff );
			_clpmh_consistency.setArg( 4, maxanglediff );
			_clpmh_consistency.setArg<int>( 5, 1 ); // left to right
			_clpmh_consistency.runWait( CLNDRange( Math::pad( clsmoothtmp.width(), KX ), Math::pad( clsmoothtmp.height(), KY ) ), CLNDRange( KX, KY ) );

			_clpmh_fill.setArg( 0, clsmoothtmp2 );
			_clpmh_fill.setArg( 1, clsmoothtmp );
			_clpmh_fill.setArg( 2, depthmax );
			_clpmh_fill.setArg<int>( 3, 1 ); // left to right
			_clpmh_fill.runWait( CLNDRange( Math::pad( clsmoothtmp.width(), KX ), Math::pad( clsmoothtmp.height(), KY ) ), CLNDRange( KX, KY ) );

//			clsmoothtmp2.save("stereosmoothorig1.png");
			_pdrof.apply( leftsmooth, clsmoothtmp2, leftweight, theta * thetascale + 5.0f, 250 );
//			leftsmooth.save("stereosmooth1.png");

			_clpmh_consistency.setArg( 0, clsmoothtmp );
			_clpmh_consistency.setArg( 1, *clmatches2[ 1 - swap ] );
			_clpmh_consistency.setArg( 2, *clmatches1[ 1 - swap ] );
			_clpmh_consistency.setArg( 3, maxdispdiff );
			_clpmh_consistency.setArg( 4, maxanglediff );
			_clpmh_consistency.setArg<int>( 5, 0 ); // right to left;
			_clpmh_consistency.runWait( CLNDRange( Math::pad( clsmoothtmp.width(), KX ), Math::pad( clsmoothtmp.height(), KY ) ), CLNDRange( KX, KY ) );

			_clpmh_fill.setArg( 0, clsmoothtmp2 );
			_clpmh_fill.setArg( 1, clsmoothtmp );
			_clpmh_fill.setArg( 2, depthmax );
			_clpmh_fill.setArg<int>( 3, 0 ); // right to left
			_clpmh_fill.runWait( CLNDRange( Math::pad( clsmoothtmp.width(), KX ), Math::pad( clsmoothtmp.height(), KY ) ), CLNDRange( KX, KY ) );

//			clsmoothtmp2.save("stereosmoothorig2.png");
			_pdrof.apply( rightsmooth, clsmoothtmp2, rightweight, theta * thetascale + 5.0f, 250 );
//			rightsmooth.save("stereosmooth2.png");

			if( iter >= 5 )
				theta = Math::smoothstep<float>( ( ( iter - 5.0f ) / ( ( float ) iterations - 5.0f ) )  ) * 1.0f;
		}

		_clpmh_consistency.setArg( 0, clsmoothtmp );
		_clpmh_consistency.setArg( 1, *clmatches1[ 1 - ( ( iterations - 1 ) & 1 ) ] );
		_clpmh_consistency.setArg( 2, *clmatches2[ 1 - ( ( iterations - 1 ) & 1 ) ] );
		_clpmh_consistency.setArg( 3, maxdispdiff );
		_clpmh_consistency.setArg( 4, maxanglediff );
		_clpmh_consistency.setArg<int>( 5, 1 ); // left to right
		_clpmh_consistency.runWait( CLNDRange( Math::pad( left.width(), KX ), Math::pad( left.height(), KY ) ), CLNDRange( KX, KY ) );
//		cloutput1.save("stereoconsistency.png");

	
		dmap.reallocate( left.width(), left.height(), ( dmap.channels() != 1 ) ? IFormat::GRAY_FLOAT : dmap.format(), IALLOCATOR_CL );
		_clpmh_filldepthmap.setArg( 0, dmap );
		_clpmh_filldepthmap.setArg( 1, clsmoothtmp );
		_clpmh_filldepthmap.setArg( 2, dscale );
		_clpmh_filldepthmap.runWait( CLNDRange( Math::pad( left.width(), KX ), Math::pad( left.height(), KY ) ), CLNDRange( KX, KY ) );
//		_clpmh_outputfinal.save( "stereofinal.png" );

		if( normalmap != NULL ) {
			normalmap->reallocate( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
			_clpmh_fillnormalmap.setArg( 0, *normalmap );
			_clpmh_fillnormalmap.setArg( 1, clsmoothtmp );
			_clpmh_fillnormalmap.runWait( CLNDRange( Math::pad( left.width(), KX ), Math::pad( left.height(), KY ) ), CLNDRange( KX, KY ) );
		}
	}

	void PMHuberStereo::depthMapInpaint( Image& dmap, const Image& left, const Image& right, size_t patchsize, float depthmax, size_t iterations, size_t viewsamples )
	{
		if( left.width() != right.width() || left.height() != right.height() ||
		    left.memType() != IALLOCATOR_CL || right.memType() != IALLOCATOR_CL )
			throw CVTException( "Left/Right stereo images inconsistent or incompatible memory type" );

		float theta = 0.0f;
		CLBuffer viewbuf1( sizeof( PMHVIEWPROP ) * left.width() * left.height() );
		CLBuffer viewbuf2( sizeof( PMHVIEWPROP ) * right.width() * right.height() );

		Image leftgrad( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image rightgrad( right.width(), right.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image leftweight( left.width(), left.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );
		Image rightweight( right.width(), right.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );
		Image leftsmooth( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image rightsmooth( right.width(), right.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image clsmoothtmp( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image clsmoothtmp2( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image cloccimg( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );

		/* calculate gradient image */
		_clpmh_gradxy.setArg( 0, leftgrad );
		_clpmh_gradxy.setArg( 1, left );
		_clpmh_gradxy.run( CLNDRange( Math::pad( left.width(), KX ), Math::pad( left.height(), KY ) ), CLNDRange( KX, KY ) );

		_clpmh_gradxy.setArg( 0, rightgrad );
		_clpmh_gradxy.setArg( 1, right );
		_clpmh_gradxy.run( CLNDRange( Math::pad( right.width(), KX ), Math::pad( right.height(), KY ) ), CLNDRange( KX, KY ) );

		/* calculate huber weights */
		_clpmh_weight.setArg( 0, leftweight );
		_clpmh_weight.setArg( 1, left );
		_clpmh_weight.run( CLNDRange( Math::pad( left.width(), KX ), Math::pad( left.height(), KY ) ), CLNDRange( KX, KY ) );

		_clpmh_weight.setArg( 0, rightweight );
		_clpmh_weight.setArg( 1, right );
		_clpmh_weight.run( CLNDRange( Math::pad( right.width(), KX ), Math::pad( right.height(), KY ) ), CLNDRange( KX, KY ) );

		/* clear the smoothed images */
		_clpmh_clear.setArg( 0, leftsmooth );
		_clpmh_clear.run( CLNDRange( Math::pad( leftsmooth.width(), KX ), Math::pad( leftsmooth.height(), KY ) ), CLNDRange( KX, KY ) );

		_clpmh_clear.setArg( 0, rightsmooth );
		_clpmh_clear.run( CLNDRange( Math::pad( rightsmooth.width(), KX ), Math::pad( rightsmooth.height(), KY ) ), CLNDRange( KX, KY ) );


		Image clmatches1_1( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image clmatches1_2( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image* clmatches1[ 2 ] = { &clmatches1_1, &clmatches1_2 };

		Image clmatches2_1( right.width(), right.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image clmatches2_2( right.width(), right.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
		Image* clmatches2[ 2 ] = { &clmatches2_1, &clmatches2_2 };

		/* PMH init */
		_clpmh_init.setArg( 0, *clmatches1[ 0 ] );
		_clpmh_init.setArg( 1, left );
		_clpmh_init.setArg( 2, right );
		_clpmh_init.setArg( 3, leftgrad );
		_clpmh_init.setArg( 4, rightgrad );
		_clpmh_init.setArg( 5, ( int ) patchsize );
		_clpmh_init.setArg( 6, depthmax );
		_clpmh_init.setArg<int>( 7, 1 );
		_clpmh_init.run( CLNDRange( Math::pad( left.width(), KX ), Math::pad( left.height(), KY ) ), CLNDRange( KX, KY ) );

		_clpmh_init.setArg( 0, *clmatches2[ 0 ] );
		_clpmh_init.setArg( 1, right );
		_clpmh_init.setArg( 2, left );
		_clpmh_init.setArg( 3, rightgrad );
		_clpmh_init.setArg( 4, leftgrad );
		_clpmh_init.setArg( 5, ( int ) patchsize );
		_clpmh_init.setArg( 6, depthmax );
		_clpmh_init.setArg<int>( 7, 0 );
		_clpmh_init.run( CLNDRange( Math::pad( right.width(), KX ), Math::pad( right.height(), KY ) ), CLNDRange( KX, KY ) );

		/* Clear view propagation buffer for the right view*/
		_clpmh_viewbufclear.setArg( 0, viewbuf2 );
		_clpmh_viewbufclear.setArg( 1, ( int ) right.width() );
		_clpmh_viewbufclear.setArg( 2, ( int ) right.height() );
		_clpmh_viewbufclear.run( CLNDRange( Math::pad( right.width(), KX ), Math::pad( right.height(), KY ) ), CLNDRange( KX, KY ) );

		for( size_t iter = 0; iter < iterations; iter++ ) {
			int swap = iter & 1;
#if 1
			std::cout << "Theta: " << theta << std::endl;
			_clpmh_depthmap.setArg( 0, clsmoothtmp );
			_clpmh_depthmap.setArg( 1, *clmatches1[ swap ] );
			_clpmh_depthmap.setArg( 2, depthmax );
			_clpmh_depthmap.runWait( CLNDRange( Math::pad( left.width(), KX ), Math::pad( left.height(), KY ) ), CLNDRange( KX, KY ) );
			clsmoothtmp.save("stereo1.png");
			std::cout << "Wrote stereo1.png" << std::endl;

			_clpmh_depthmap.setArg( 0, clsmoothtmp );
			_clpmh_depthmap.setArg( 1, *clmatches2[ swap ]  );
			_clpmh_depthmap.setArg( 2, depthmax );
			_clpmh_depthmap.runWait( CLNDRange( Math::pad( right.width(), KX ), Math::pad( right.height(), KY ) ), CLNDRange( KX, KY ) );
			clsmoothtmp.save("stereo2.png");
			std::cout << "Wrote stereo2.png" << std::endl;

/*			clconsistency.setArg( 0, cloutput1 );
			clconsistency.setArg( 1, *clmatches1[ swap ] );
			clconsistency.setArg( 2, *clmatches2[ swap ] );
			clconsistency.setArg( 3, lr );
			clconsistency.runWait( CLNDRange( Math::pad( clinput2.width(), KX ), Math::pad( clinput2.height(), KY ) ), CLNDRange( KX, KY ) );
			cloutput1.save("stereoconsistency.png");

			clfilldepthmap.setArg( 0, cloutput2 );
			clfilldepthmap.setArg( 1, cloutput1 );
			clfilldepthmap.setArg( 2, 4.0f / 255.0f );
			clfilldepthmap.runWait( CLNDRange( Math::pad( clinput2.width(), KX ), Math::pad( clinput2.height(), KY ) ), CLNDRange( KX, KY ) );

			cloutput2.save("stereofill.png");
			cloutput2.save("stereofill.cvtraw");
*/
			getchar();
#endif
			_clpmh_viewbufclear.setArg( 0, viewbuf1 );
			_clpmh_viewbufclear.setArg( 1, ( int ) left.width() );
			_clpmh_viewbufclear.setArg( 2, ( int ) left.height() );
			_clpmh_viewbufclear.runWait( CLNDRange( Math::pad( left.width(), KX ), Math::pad( left.height(), KY ) ), CLNDRange( KX, KY ) );

			_clpmh_propagate.setArg( 0, *clmatches1[ 1 - swap ] );
			_clpmh_propagate.setArg( 1, *clmatches1[ swap ] );
			_clpmh_propagate.setArg( 2, left );
			_clpmh_propagate.setArg( 3, right );
			_clpmh_propagate.setArg( 4, leftgrad );
			_clpmh_propagate.setArg( 5, rightgrad );
			_clpmh_propagate.setArg( 6, leftsmooth );
			_clpmh_propagate.setArg( 7, theta );
			_clpmh_propagate.setArg( 8, ( int ) patchsize );
			_clpmh_propagate.setArg( 9, depthmax );
			_clpmh_propagate.setArg<int>( 10, 1 ); // left to right
			_clpmh_propagate.setArg( 11, ( int ) iter );
			_clpmh_propagate.setArg( 12, viewbuf2 );
			_clpmh_propagate.setArg( 13, viewbuf1 );
			_clpmh_propagate.runWait( CLNDRange( Math::pad( left.width(), KX ), Math::pad( left.height(), KY ) ), CLNDRange( KX, KY ) );

			_clpmh_viewbufclear.setArg( 0, viewbuf2 );
			_clpmh_viewbufclear.setArg( 1, ( int ) right.width() );
			_clpmh_viewbufclear.setArg( 2, ( int ) right.height() );
			_clpmh_viewbufclear.runWait( CLNDRange( Math::pad( right.width(), KX ), Math::pad( right.height(), KY ) ), CLNDRange( KX, KY ) );

			_clpmh_propagate.setArg( 0, *clmatches2[ 1 - swap ] );
			_clpmh_propagate.setArg( 1, *clmatches2[ swap ] );
			_clpmh_propagate.setArg( 2, right );
			_clpmh_propagate.setArg( 3, left );
			_clpmh_propagate.setArg( 4, rightgrad );
			_clpmh_propagate.setArg( 5, leftgrad );
			_clpmh_propagate.setArg( 6, rightsmooth );
			_clpmh_propagate.setArg( 7, theta );
			_clpmh_propagate.setArg( 8, ( int ) patchsize );
			_clpmh_propagate.setArg( 9, depthmax );
			_clpmh_propagate.setArg<int>( 10, 0 ); // right to left
			_clpmh_propagate.setArg( 11, ( int ) iter );
			_clpmh_propagate.setArg( 12, viewbuf1 );
			_clpmh_propagate.setArg( 13, viewbuf2 );
			_clpmh_propagate.runWait( CLNDRange( Math::pad( right.width(), KX ), Math::pad( right.height(), KY ) ), CLNDRange( KX, KY ) );

			_clpmh_occmap.setArg( 0, cloccimg );
			_clpmh_occmap.setArg( 1, *clmatches1[ 1 - swap ] );
			_clpmh_occmap.setArg( 2, *clmatches2[ 1 - swap ] );
			_clpmh_occmap.setArg( 3, 0.5f );
			_clpmh_occmap.setArg<int>( 4, 1 ); // left to right
			_clpmh_occmap.runWait( CLNDRange( Math::pad( left.width(), KX ), Math::pad( left.height(), KY ) ), CLNDRange( KX, KY ) );
			cloccimg.save("stereoocc1.png");

			_clpmh_normaldepth.setArg( 0, clsmoothtmp );
			_clpmh_normaldepth.setArg( 1, *clmatches1[ 1 - swap ] );
			_clpmh_normaldepth.setArg( 2, depthmax );
			_clpmh_normaldepth.setArg<int>( 3, 1 ); // left to right
			_clpmh_normaldepth.runWait( CLNDRange( Math::pad( left.width(), KX ), Math::pad( left.height(), KY ) ), CLNDRange( KX, KY ) );
			clsmoothtmp.save("stereosmoothorig1.png");

			_pdrofinpaint.apply( leftsmooth, clsmoothtmp, leftweight, cloccimg, theta * 20.0f + 5.0f, 100 );
			leftsmooth.save("stereosmooth1.png");

			_clpmh_occmap.setArg( 0, cloccimg );
			_clpmh_occmap.setArg( 1, *clmatches2[ 1 - swap ] );
			_clpmh_occmap.setArg( 2, *clmatches1[ 1 - swap ] );
			_clpmh_occmap.setArg( 3, 0.5f );
			_clpmh_occmap.setArg<int>( 4, 0 ); // right to left
			_clpmh_occmap.runWait( CLNDRange( Math::pad( left.width(), KX ), Math::pad( left.height(), KY ) ), CLNDRange( KX, KY ) );
			cloccimg.save("stereoocc2.png");

			_clpmh_normaldepth.setArg( 0, clsmoothtmp );
			_clpmh_normaldepth.setArg( 1, *clmatches2[ 1 - swap ] );
			_clpmh_normaldepth.setArg( 2, depthmax );
			_clpmh_normaldepth.setArg<int>( 3, 0 ); // right to left
			_clpmh_normaldepth.runWait( CLNDRange( Math::pad( right.width(), KX ), Math::pad( right.height(), KY ) ), CLNDRange( KX, KY ) );
			clsmoothtmp.save("stereosmoothorig2.png");

			_pdrofinpaint.apply( rightsmooth, clsmoothtmp, rightweight, cloccimg, theta * 20.0f + 5.0f, 100 );
			rightsmooth.save("stereosmooth2.png");

			if( iter >= 4 )
				theta = Math::smoothstep<float>( ( ( iter - 4.0f ) / ( ( float ) iterations - 4.0f ) )  ) * 1.0f;
		}

		_clpmh_consistency.setArg( 0, clsmoothtmp );
		_clpmh_consistency.setArg( 1, *clmatches1[ 1 ] );
		_clpmh_consistency.setArg( 2, *clmatches2[ 1 ] );
		_clpmh_consistency.setArg( 3, 1.0f );
		_clpmh_consistency.setArg( 4, 5.0f );
		_clpmh_consistency.setArg<int>( 5, 1 ); // left to right
		_clpmh_consistency.runWait( CLNDRange( Math::pad( left.width(), KX ), Math::pad( left.height(), KY ) ), CLNDRange( KX, KY ) );
//		cloutput1.save("stereoconsistency.png");

		dmap.reallocate( left.width(), right.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );
		_clpmh_filldepthmap.setArg( 0, dmap );
		_clpmh_filldepthmap.setArg( 1, clsmoothtmp );
		_clpmh_filldepthmap.setArg( 2, 1.0f );
		_clpmh_filldepthmap.runWait( CLNDRange( Math::pad( left.width(), KX ), Math::pad( left.height(), KY ) ), CLNDRange( KX, KY ) );
//		_clpmh_outputfinal.save( "stereofinal.png" );

	}
}
