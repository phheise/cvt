/*
   The MIT License (MIT)

   Copyright (c) 2016, BMW Car IT GmbH, Philipp Heise (philipp.heise@bmw.de)

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

#include <cvt/vision/ProxStereoCL.h>

#include <cvt/cl/kernel/ProxStereo.h>
#include <cvt/cl/kernel/fill.h>
#include <cvt/cl/kernel/gradxy.h>
#include <cvt/cl/kernel/pyr/pyrupmul.h>
#include <cvt/cl/kernel/pyr/pyrup.h>

#define KX 32
#define KY 32

namespace cvt {

    ProxStereoCL::ProxStereoCL( float scale, int levels, int patchsize, int iterations ) :
        _patchsize( patchsize ),
        _iterations( iterations ),
        _warpiterations( 25 ),
        _clproxstereo( _ProxStereo_source, "prox_stereo" ),
        _clproxstereowarp( _ProxStereo_source, "prox_stereo_warp" ),
        _clfill( _fill_source, "fill" ),
        _clgrad( _gradxy_source, "grad_gray_xydiag" ),
        _clpyrupmul( _pyrupmul_source, "pyrup_mul4f" ),
        _clpyrup( _pyrup_source, "pyrup" ),
        _cldisparity( _ProxStereo_source, "prox_stereo_disparity" ),
        _clwarpimg( _ProxStereo_source, "prox_stereo_warp_image" ),
        _clpyr0( scale, levels ),
        _clpyr1( scale, levels )
    {
    }

    ProxStereoCL::~ProxStereoCL()
    {
    }

    void ProxStereoCL::depthmap( Image& output, const Image& image0, const Image& image1, float lambda, float depthmax )
    {
        Image outprev;
        Image outprevp;

        _clpyr0.update( image0 );
        _clpyr1.update( image1 );
        output.reallocate( image0.width(), image0.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );

        int lvlmax = _clpyr0.levels();
        for( int lvl = lvlmax - 1; lvl >= 0; lvl-- ) {
            Image grad0( _clpyr0[ lvl ].width(), _clpyr0[ lvl ].height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
            Image grad1( _clpyr1[ lvl ].width(), _clpyr1[ lvl ].height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
            Image out0( _clpyr0[ lvl ].width(), _clpyr0[ lvl ].height(), IFormat::GRAYALPHA_FLOAT, IALLOCATOR_CL );
            Image out1( _clpyr1[ lvl ].width(), _clpyr1[ lvl ].height(), IFormat::GRAYALPHA_FLOAT, IALLOCATOR_CL );
            Image outp0( _clpyr0[ lvl ].width(), _clpyr0[ lvl ].height(), IFormat::GRAYALPHA_FLOAT, IALLOCATOR_CL );
            Image outp1( _clpyr1[ lvl ].width(), _clpyr1[ lvl ].height(), IFormat::GRAYALPHA_FLOAT, IALLOCATOR_CL );
            Image warp( _clpyr0[ lvl ].width(), _clpyr0[ lvl ].height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
            Image precond( _clpyr0[ lvl ].width(), _clpyr0[ lvl ].height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );
            Image* out[] = { &out0, &out1 };
            Image* outp[] = { &outp0, &outp1 };

            gradient( grad0, _clpyr0[ lvl ] );
            gradient( grad1, _clpyr1[ lvl ] );
            _precond.precondEdgeClampSeparable3( precond, Vector3f( -1.0f, 1.0f, 0.0f ), Vector3f( -1.0f, 1.0f, 0.0f ) );

            if( lvl == lvlmax - 1 ) {
                fill( out1, Vector4f( 0.0f, 0.0f, 0.0f, 0.0f ) );
                fill( outp1, Vector4f( 0.0f, 0.0f, 0.0f, 0.0f ) );
            } else {
                Vector4f mul( ( ( float ) out1.width() ) / ( ( float ) outprev.width() ) );
                pyrUpMul( out1, outprev, mul );
                pyrUp( outp1, outprevp );
                //fill( outp1, Vector4f( 0.0f, 0.0f, 0.0f, 0.0f ) );
            }

            int swap = 0;
            for( int warpiter = 0; warpiter < _warpiterations; warpiter++ ) {
                proxStereoWarp( warp, *out[ 1 - swap ], _clpyr0[ lvl ], grad0, _clpyr1[ lvl ], grad1 );

#if 0
                {
                    Image tmp( _clpyr0[ lvl ].width(), _clpyr0[ lvl ].height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
                    warpImage( tmp,  *out[ 1 - swap ],  _clpyr1[ lvl ] );

                    tmp.save( "prox_warp.png" );
                    _clpyr0[ lvl ].save("prox_warp_dst.png");
                    _clpyr1[ lvl ].save("prox_warp_src.png");
                }
#endif

                for( int iter = 0; iter < _iterations; iter++ ) {
                    proxStereo( *out[ swap ], *out[ 1 - swap ],
                               *outp[ swap ], *outp[ 1 - swap ],
                               warp, lambda * ( ( ( float ) _clpyr0[ lvl ].width() ) / ( ( float ) _clpyr0[ 0 ].width() ) ),
                               depthmax * ( ( ( float ) _clpyr0[ lvl ].width() ) / ( ( float ) _clpyr0[ 0 ].width() ) ), precond );
                    swap = 1 - swap;
                }
#if 0
                {
                    Image tmp( _clpyr0[ lvl ].width(), _clpyr0[ lvl ].height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );
                    toDisparity( tmp, *out[ 1 - swap ], 1.0f );
                    Image tmp2;
                    tmp.colorCode( tmp2, ICOLORCODE_PM3DINV, 0.0f, ( ( ( float ) _clpyr0[ lvl ].width() ) / ( ( float ) _clpyr0[ 0 ].width() ) ) * depthmax );
                    tmp2.save( "prox_dmap.png" );
                    getchar();
                }
#endif
            }

            if( lvl > 0 ) {
                outprev.assign( *out[ 1 - swap ], IALLOCATOR_CL );
                outprevp.assign( *outp[ 1 - swap ], IALLOCATOR_CL );
            } else
                toDisparity( output, *out[ 1 - swap ], 1.0f );

        }
    }

    void ProxStereoCL::depthmapRefine( Image& output, const Image& image0, const Image& image1, const Image& initdisparity, float lambda, float depthmax )
    {
        output.reallocate( image0.width(), image0.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );

        Image grad0( image0.width(), image0.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image grad1( image1.width(), image1.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image out0( image0.width(), image0.height(), IFormat::GRAYALPHA_FLOAT, IALLOCATOR_CL );
        Image out1( image1.width(), image1.height(), IFormat::GRAYALPHA_FLOAT, IALLOCATOR_CL );
        Image outp0( image0.width(), image0.height(), IFormat::GRAYALPHA_FLOAT, IALLOCATOR_CL );
        Image outp1( image1.width(), image1.height(), IFormat::GRAYALPHA_FLOAT, IALLOCATOR_CL );
        Image warp( image0.width(), image0.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image precond( image0.width(), image0.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );
        Image* out[] = { &out0, &out1 };
        Image* outp[] = { &outp0, &outp1 };

        gradient( grad0, image0 );
        gradient( grad1, image1 );
        _precond.precondEdgeClampSeparable3( precond, Vector3f( -1.0f, 1.0f, 0.0f ), Vector3f( -1.0f, 1.0f, 0.0f ) );

        fill( outp1, Vector4f( 0.0f, 0.0f, 0.0f, 0.0f ) );
        out1.assign( initdisparity, IALLOCATOR_CL );

        int swap = 0;
        for( int warpiter = 0; warpiter < _warpiterations; warpiter++ ) {
            proxStereoWarp( warp, *out[ 1 - swap ], image0, grad0, image1, grad1 );

#if 1
            {
                Image tmp( image0.width(), image0.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
                warpImage( tmp,  *out[ 1 - swap ],  image1 );

                tmp.save( "prox_warp.png" );
                image0.save("prox_warp_dst.png");
                image1.save("prox_warp_src.png");
            }
#endif

            for( int iter = 0; iter < _iterations; iter++ ) {
                proxStereo( *out[ swap ], *out[ 1 - swap ],
                           *outp[ swap ], *outp[ 1 - swap ],
                           warp, lambda, depthmax, precond );
                swap = 1 - swap;
            }
#if 1
            {
                Image tmp( image0.width(), image0.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );
                toDisparity( tmp, *out[ 1 - swap ], 1.0f );
                Image tmp2;
                tmp.colorCode( tmp2, ICOLORCODE_PM3DINV, 0.0f, depthmax );
                tmp2.save( "prox_dmap.png" );
                getchar();
            }
#endif
        }

        toDisparity( output, *out[ 1 - swap ], 1.0f );
    }

    void ProxStereoCL::proxStereoWarp( Image& output, const Image& input,
                             const Image& image0, const Image& gradimage0,
                             const Image& image1, const Image& gradimage1 )
    {
        _clproxstereowarp.setArg( 0, output );
        _clproxstereowarp.setArg( 1, input );
        _clproxstereowarp.setArg( 2, image0 );
        _clproxstereowarp.setArg( 3, gradimage0 );
        _clproxstereowarp.setArg( 4, image1 );
        _clproxstereowarp.setArg( 5, gradimage1 );
        _clproxstereowarp.setArg<cl_int>( 6, _patchsize );
        _clproxstereowarp.run( CLNDRange( Math::pad( image0.width(), KX ), Math::pad( image0.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void ProxStereoCL::proxStereo( Image& output, const Image& input,
                                   Image& outputp, const Image& inputp,
                                   const Image& warp, float lambda, float depthmax, Image& precond )
    {
        _clproxstereo.setArg( 0, output );
        _clproxstereo.setArg( 1, input );
        _clproxstereo.setArg( 2, outputp );
        _clproxstereo.setArg( 3, inputp );
        _clproxstereo.setArg( 4, warp );
        _clproxstereo.setArg( 5, lambda );
        _clproxstereo.setArg( 6, depthmax );
        _clproxstereo.setArg( 7, precond );
        _clproxstereo.run( CLNDRange( Math::pad( warp.width(), KX ), Math::pad( warp.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void ProxStereoCL::pyrUpMul( Image& output, const Image& input, const Vector4f& mul )
    {
        cl_float4 val;
        val.x = mul.x;
        val.y = mul.y;
        val.z = mul.z;
        val.w = mul.w;

        _clpyrupmul.setArg( 0, output );
        _clpyrupmul.setArg( 1, input );
        _clpyrupmul.setArg( 2, val );
        _clpyrupmul.run( CLNDRange( Math::pad( output.width(), KX ), Math::pad( output.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void ProxStereoCL::pyrUp( Image& output, const Image& input )
    {
        _clpyrup.setArg( 0, output );
        _clpyrup.setArg( 1, input );
        _clpyrup.run( CLNDRange( Math::pad( output.width(), KX ), Math::pad( output.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void ProxStereoCL::gradient( Image& output, const Image& input )
    {
        _clgrad.setArg( 0, output );
        _clgrad.setArg( 1, input );
        _clgrad.run( CLNDRange( Math::pad( input.width(), KX ), Math::pad( input.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void ProxStereoCL::fill( Image& output, const Vector4f& value )
    {
        cl_float4 val;
        val.x = value.x;
        val.y = value.y;
        val.z = value.z;
        val.w = value.w;

        _clfill.setArg( 0, output );
        _clfill.setArg( 1, val );
        _clfill.run( CLNDRange( Math::pad( output.width(), KX ), Math::pad( output.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void ProxStereoCL::toDisparity( Image& output, const Image& input, float scale )
    {
        _cldisparity.setArg( 0, output );
        _cldisparity.setArg( 1, input );
        _cldisparity.setArg( 2, scale );
        _cldisparity.run( CLNDRange( Math::pad( input.width(), KX ), Math::pad( input.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void ProxStereoCL::warpImage( Image& output, const Image& input, const Image& source )
    {
        _clwarpimg.setArg( 0, output );
        _clwarpimg.setArg( 1, input );
        _clwarpimg.setArg( 2, source );
        _clwarpimg.run( CLNDRange( Math::pad( input.width(), KX ), Math::pad( input.height(), KY ) ), CLNDRange( KX, KY ) );
    }

}
