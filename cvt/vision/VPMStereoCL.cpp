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

#include <cvt/vision/VPMStereoCL.h>

#include <cvt/cl/kernel/VPMStereo.h>
#include <cvt/cl/kernel/fill.h>
#include <cvt/cl/kernel/gradxy.h>

#define KX 32
#define KY 32

namespace cvt {

    VPMStereoCL::VPMStereoCL( int patchsize, int iterations, float disparitymax ) :
        _patchsize( patchsize ),
        _iterations( iterations ),
        _disparitymax( disparitymax ),
        _clvpmsinit( _VPMStereo_source, "vpmstereo_init" ),
        _clvpmsinitdisparity( _VPMStereo_source, "vpmstereo_init_disparity" ),
        _clvpmsitersimple( _VPMStereo_source, "vpmstereo_iter_simple" ),
        _clfill( _fill_source, "fill" ),
        _clgrad( _gradxy_source, "grad_gray_xydiag" ),
        _clvpmsdisparity( _VPMStereo_source, "vpmstereo_disparity" ),
        _clvpmswarpimg( _VPMStereo_source, "vpmstereo_warp_image" )
    {
    }

    VPMStereoCL::~VPMStereoCL()
    {
    }

    void VPMStereoCL::disparitySimple( Image& output, const Image& image0, const Image& image1, const Image* initdisparity )
    {
        Image grad0( image0.width(), image0.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image grad1( image1.width(), image1.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image out0( image0.width(), image0.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image out1( image1.width(), image1.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image* out[] = { &out0, &out1 };

        gradient( grad0, image0 );
        gradient( grad1, image1 );

        if( initdisparity )
            initDisparity( out1, *initdisparity, image0, grad0, image1, grad1 );
        else
            init( out1, image0, grad0, image1, grad1 );

        int swap = 0;
        for( int i = 0; i < _iterations; i++ ) {
            iterSimple( *out[ swap ], *out[ 1 - swap ], image0, grad0, image1, grad1, i );
            swap = 1 - swap;
#if 1
            {
                Image tmp( image0.width(), image0.height(), IFormat::GRAYALPHA_FLOAT, IALLOCATOR_CL );
                toDisparity( tmp, *out[ 1 - swap ], 1.0f / _disparitymax );
                tmp.save( "vpmsstereo.png" );
                getchar();
            }
#endif
        }

        toDisparity( output, *out[ 1 - swap ] );
    }

    void VPMStereoCL::init( Image& output,
                           const Image& colimg0, const Image& gradimg0,
                           const Image& colimg1, const Image& gradimg1 )
    {
        _clvpmsinit.setArg( 0, output );
        _clvpmsinit.setArg( 1, colimg0 );
        _clvpmsinit.setArg( 2, gradimg0 );
        _clvpmsinit.setArg( 3, colimg1 );
        _clvpmsinit.setArg( 4, gradimg1 );
        _clvpmsinit.setArg<cl_int>( 5, _patchsize );
        _clvpmsinit.setArg( 6, _disparitymax );
        _clvpmsinit.run( CLNDRange( Math::pad( output.width(), KX ), Math::pad( output.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void VPMStereoCL::initDisparity( Image& output, const Image& disparity,
                                    const Image& colimg0, const Image& gradimg0,
                                    const Image& colimg1, const Image& gradimg1 )
    {
        _clvpmsinitdisparity.setArg( 0, output );
        _clvpmsinitdisparity.setArg( 1, disparity );
        _clvpmsinitdisparity.setArg( 2, colimg0 );
        _clvpmsinitdisparity.setArg( 3, gradimg0 );
        _clvpmsinitdisparity.setArg( 4, colimg1 );
        _clvpmsinitdisparity.setArg( 5, gradimg1 );
        _clvpmsinitdisparity.setArg<cl_int>( 6, _patchsize );
        _clvpmsinitdisparity.setArg( 7, _disparitymax );
        _clvpmsinitdisparity.run( CLNDRange( Math::pad( output.width(), KX ), Math::pad( output.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void VPMStereoCL::iterSimple( Image& output, const Image& input,
                                 const Image& colimg0, const Image& gradimg0,
                                 const Image& colimg1, const Image& gradimg1, int iter )
    {
        _clvpmsitersimple.setArg( 0, output );
        _clvpmsitersimple.setArg( 1, input );
        _clvpmsitersimple.setArg( 2, colimg0 );
        _clvpmsitersimple.setArg( 3, gradimg0 );
        _clvpmsitersimple.setArg( 4, colimg1 );
        _clvpmsitersimple.setArg( 5, gradimg1 );
        _clvpmsitersimple.setArg<cl_int>( 6, _patchsize );
        _clvpmsitersimple.setArg( 7, _disparitymax );
        _clvpmsitersimple.setArg<cl_int>( 8, iter );
        _clvpmsitersimple.run( CLNDRange( Math::pad( output.width(), KX ), Math::pad( output.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void VPMStereoCL::fill( Image& output, const Vector4f& value )
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

    void VPMStereoCL::gradient( Image& output, const Image& input )
    {
        _clgrad.setArg( 0, output );
        _clgrad.setArg( 1, input );
        _clgrad.run( CLNDRange( Math::pad( input.width(), KX ), Math::pad( input.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void VPMStereoCL::toDisparity( Image& output, const Image& input, float scale )
    {
        _clvpmsdisparity.setArg( 0, output );
        _clvpmsdisparity.setArg( 1, input );
        _clvpmsdisparity.setArg( 2, scale );
        _clvpmsdisparity.run( CLNDRange( Math::pad( input.width(), KX ), Math::pad( input.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void VPMStereoCL::warpImage( Image& output, const Image& input, const Image& source )
    {
        _clvpmswarpimg.setArg( 0, output );
        _clvpmswarpimg.setArg( 1, input );
        _clvpmswarpimg.setArg( 2, source );
        _clvpmswarpimg.run( CLNDRange( Math::pad( input.width(), KX ), Math::pad( input.height(), KY ) ), CLNDRange( KX, KY ) );
    }

}
