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

#include <cvt/gfx/ifilter/TVL1Stereo.h>

#include <cvt/cl/kernel/clear.h>
#include <cvt/cl/kernel/median3.h>
#include <cvt/cl/kernel/pyr/pyrupmul.h>
#include <cvt/cl/kernel/pyr/pyrdown_binom3.h>
#include <cvt/cl/kernel/pyr/pyrdown_binom5.h>
#include <cvt/cl/kernel/pyr/pyrdown.h>
#include <cvt/cl/kernel/tvl1stereo/tvl1.h>
#include <cvt/cl/kernel/tvl1stereo/tvl1_warp.h>

#include <cvt/vision/Flow.h>

namespace cvt {
		static ParamInfoTyped<Image*> pin( "Input", true );
		static ParamInfoTyped<Image*> pout( "Output", false );

		static ParamInfo * _params[ 2 ] = {
			&pin,
			&pout,
		};

		TVL1Stereo::TVL1Stereo( float scalefactor, size_t levels ) : IFilter( "TVL1Stereo", _params, 2, IFILTER_OPENCL ),
			_scalefactor( scalefactor ),
			_levels( levels ),
			_pyrup( _pyrupmul_source, "pyrup_mul" ),
			_pyrdown( _pyrdown_source, "pyrdown" ),
			_pyrdownbinom( _pyrdown_binom3_source, "pyrdown_binom3" ),
			_tvl1( _tvl1_source, "tvl1" ),
			_tvl1_warp( _tvl1_warp_source, "tvl1_warp" ),
			_clear( _clear_source, "clear" ),
			_median3( _median3_source, "median3" ),
			_lambda( 70.0f )
		{
			_pyr[ 0 ] = new Image[ levels ];
			_pyr[ 1 ] = new Image[ levels ];
		}

		TVL1Stereo::~TVL1Stereo()
		{
			delete[ ] _pyr[ 0 ];
			delete[ ] _pyr[ 1 ];
		}

		void TVL1Stereo::apply( Image& output, const Image& src1, const Image& src2 )
		{
			if( src1.width() != src2.width() ||
			    src1.height() != src2.height() )
				throw CVTException( "Image do not match in size!" );

			fillPyramidCL( src1, 0 );
			fillPyramidCL( src2, 1 );

#define THETA 0.08f
#define WARPWGSIZE 16
#define PYRUPWGSIZE 16
#define TVL1WGSIZE 16
#define MEDWGSIZE 16

			Image* flow = NULL;
			Image* flowold = NULL;

			for( int l = _levels - 1; l >= 0; l-- ) {
				if( flowold )
					delete flowold;
				if( flow )
					flowold = flow;

				flow = new Image( _pyr[ 0 ][ l ].width(), _pyr[ 0 ][ l ].height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );
				if( flowold ) {
					_pyrup.setArg( 0, *flow );
					_pyrup.setArg( 1, *flowold );
					_pyrup.setArg( 2, 1.0f / _scalefactor );
					_pyrup.run( CLNDRange( Math::pad( flow->width(), PYRUPWGSIZE ), Math::pad( flow->height(), PYRUPWGSIZE ) ), CLNDRange( PYRUPWGSIZE, PYRUPWGSIZE ) );
				} else {
					_clear.setArg( 0, *flow );
					_clear.run( CLNDRange(Math::pad( flow->width(), 16 ), Math::pad( flow->height(), 16 ) ), CLNDRange( 16, 16 ));
				}

//				float tmp = _lambda;
//				_lambda = _lambda * Math::pow( _scalefactor, l * 1.0f );
				solveTVL1( *flow, _pyr[ 0 ][ l ], _pyr[ 1 ][ l ], true );
//				_lambda = tmp;
			}

			if( flowold )
				delete flowold;

			//*flow = *flowtmp;
			output.reallocate( src1.width(), src2.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );
			output = *flow;

			//	_pyrup.setArg( 0, output );
			//	_pyrup.setArg( 1, *flow );
			//	_pyrup.setArg( 2, ( float ) output.width() / ( float ) flow->width() );
			//	_pyrup.run( CLNDRange( Math::pad( output.width(), 16 ), Math::pad( output.height(), 16 ) ), CLNDRange( 16, 16 ) );


			delete flow;
		}

		void TVL1Stereo::solveTVL1( Image& flow, const Image& src1, const Image& src2, bool median )
		{
			Image flowtmp( flow.width(), flow.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );
			Image flow0( flow.width(), flow.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );
			Image warp( flow.width(), flow.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );

			Image p0( flow.width(), flow.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
			Image p1( flow.width(), flow.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
			//Image p2( flow.width(), flow.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );

			_clear.setArg( 0, p1 );
			_clear.run( CLNDRange(Math::pad( flow.width(), 16 ), Math::pad( flow.height(), 16 ) ), CLNDRange( 16, 16 ));
		//	_clear.setArg( 0, p2 );
		//	_clear.run( CLNDRange(Math::pad( flow.width(), 16 ), Math::pad( flow.height(), 16 ) ), CLNDRange( 16, 16 ));

			Image* us[ 2 ] = { &flowtmp, &flow };

				Image* ps[ 3 ] = { &p0, &p1/*, &p2*/ };
			// WARPS
			for( int i = 0; i < 10; i++ ) {
				if( median ) {
					_median3.setArg( 0, flow0 );
					_median3.setArg( 1, *us[ 1 ] );
					_median3.setArg( 2,  CLLocalSpace( sizeof( cl_float4 ) * ( MEDWGSIZE + 2 ) * ( MEDWGSIZE + 2 ) ) );
					_median3.run( CLNDRange( Math::pad( flow.width(), MEDWGSIZE ), Math::pad( flow.height(), MEDWGSIZE ) ), CLNDRange( MEDWGSIZE, MEDWGSIZE ) );
				} else
					flow0 = *us[ 1 ];

				_tvl1_warp.setArg( 0, warp );
				_tvl1_warp.setArg( 1, flow0 );
				_tvl1_warp.setArg( 2, src1 );
				_tvl1_warp.setArg( 3, src2 );
				_tvl1_warp.setArg( 4, CLLocalSpace( sizeof( cl_float4 ) * ( WARPWGSIZE + 2 ) * ( WARPWGSIZE + 2 ) ) );
				_tvl1_warp.run( CLNDRange(Math::pad( flow.width(), WARPWGSIZE ), Math::pad( flow.height(), WARPWGSIZE ) ), CLNDRange( WARPWGSIZE, WARPWGSIZE ) );

				//_clear.setArg( 0, *ps[ 1 ] );
				//_clear.run( CLNDRange(Math::pad( flow.width(), 16 ), Math::pad( flow.height(), 16 ) ), CLNDRange( 16, 16 ));
				//_clear.setArg( 0, *ps[ 2 ] );
				//_clear.run( CLNDRange(Math::pad( flow.width(), 16 ), Math::pad( flow.height(), 16 ) ), CLNDRange( 16, 16 ));


				Image* tmp;
				// NUMBER of ROF/THRESHOLD iterations
#define ROFITER 50
				for( int k = 0; k < ROFITER; k++ ) {
					_tvl1.setArg( 0, *ps[ 0 ] );
					_tvl1.setArg( 1, *us[ 0 ] );
					_tvl1.setArg( 2, *us[ 1 ] );
					_tvl1.setArg( 3, flow0 );
					_tvl1.setArg( 4, warp );
					_tvl1.setArg( 5, *ps[ 1 ] );
				//	_tvl1.setArg( 6, *ps[ 2 ] );
					_tvl1.setArg( 6, _lambda );
//					_tvl1.setArg( 6, _lambda * ( Math::exp( -( float ) ( k / ( float ) ROFITER ) * ( k / ( float ) ROFITER ) * 1.0f ) ) );
//					_tvl1.setArg( 6, _lambda * ( ( Math::tanh( ( ( float ) ( -k ) + 0.5f * ( float ) ROFITER ) * 0.75f ) * 0.5f + 0.5f ) ) );
					_tvl1.setArg( 7, THETA );
				//	_tvl1.setArg( 9, ( told - 1.0f ) / t  );
					_tvl1.setArg( 8, CLLocalSpace( sizeof( cl_float4 ) * ( TVL1WGSIZE + 2 ) * ( TVL1WGSIZE + 2 ) ) );
					_tvl1.setArg( 9, CLLocalSpace( sizeof( cl_float4 ) * ( TVL1WGSIZE + 1 ) * ( TVL1WGSIZE + 1 ) ) );
					_tvl1.run( CLNDRange(Math::pad( flow.width(), TVL1WGSIZE ), Math::pad( flow.height(), TVL1WGSIZE ) ), CLNDRange( TVL1WGSIZE, TVL1WGSIZE ) );

					tmp = ps[ 0 ];
					ps[ 0 ] = ps[ 1 ];
					ps[ 1 ] = tmp;

/*					tmp = us[ 0 ];
					us[ 0 ] = us[ 1 ];
					us[ 1 ] = tmp;*/

					_median3.setArg( 0, *us[ 1 ] );
					_median3.setArg( 1, *us[ 0 ] );
					_median3.setArg( 2,  CLLocalSpace( sizeof( cl_float4 ) * ( MEDWGSIZE + 2 ) * ( MEDWGSIZE + 2 ) ) );
					_median3.run( CLNDRange( Math::pad( flow.width(), MEDWGSIZE ), Math::pad( flow.height(), MEDWGSIZE ) ), CLNDRange( MEDWGSIZE, MEDWGSIZE ) );



					/*{
						Image tmp( us[1]->width(), us[1]->height(), IFormat::BGRA_FLOAT );
						Flow::colorCode( tmp, *us[1],  2.0f );
						tmp.save("curflow.png");
					}*/
				}
//			if( us[ 1 ] != &flow )
//					flow = *us[ 1 ];

			}

			if( us[ 1 ] != &flow )
					flow = *us[ 1 ];
		}

		void TVL1Stereo::fillPyramidCL( const Image& img, size_t index )
		{
			Image* pyr = _pyr[ index ];

#define PYRWGSIZE 16

			//pyr[ 0 ].reallocate( img.width(), img.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
			//
			//Image tmp;
			img.convert( pyr[ 0 ], IFormat::RGBA_FLOAT, IALLOCATOR_CL );

			//	pyr[ 0 ].reallocate( img.width() * 3.0, img.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
			//	_pyrdown.setArg( 0, pyr[ 0 ] );
			//	_pyrdown.setArg( 1, tmp );
			//	_pyrdown.run( CLNDRange( Math::pad( pyr[ 0 ].width(), PYRWGSIZE ), Math::pad( pyr[ 0 ].height(), PYRWGSIZE ) ), CLNDRange( PYRWGSIZE, PYRWGSIZE ) );


			for( size_t l = 1; l < _levels; l++ ) {
				pyr[ l ].reallocate( pyr[ l - 1 ].width() * _scalefactor, pyr[ l - 1 ].height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
				_pyrdown.setArg( 0, pyr[ l ] );
				_pyrdown.setArg( 1, pyr[ l - 1 ] );
//				_pyrdown.setArg( 2, CLLocalSpace( sizeof( cl_float4 ) * ( PYRWGSIZE + 4 ) * ( PYRWGSIZE + 4 ) ) );
//				_pyrdown.setArg( 3, CLLocalSpace( sizeof( cl_float4 ) * ( PYRWGSIZE + 4 ) * PYRWGSIZE ) );
				_pyrdown.run( CLNDRange( Math::pad( pyr[ l ].width(), PYRWGSIZE ), Math::pad( pyr[ l ].height(), PYRWGSIZE ) ), CLNDRange( PYRWGSIZE, PYRWGSIZE ) );
			}
		}
}
