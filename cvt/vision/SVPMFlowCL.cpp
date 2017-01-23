/*
   The MIT License (MIT)

   Copyright (c) 2011 - 2013, Philipp Heise and Sebastian Klose
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

#include <cvt/vision/SVPMFlowCL.h>
#include <cvt/util/Exception.h>
#include <cvt/cl/CLBuffer.h>
#include <cvt/cl/kernel/SVPMFlow.h>
#include <cvt/cl/kernel/pyr/pyrupmul.h>
#include <cvt/cl/kernel/pyr/pyrdown.h>
#include <cvt/vision/Flow.h>
#include <utility>

namespace cvt {

#define KX 16
#define KY 16
#define VIEWSAMPLES 6

#define MAXITER 200

    SVPMFlowCL::SVPMFlowCL() :
        _clsvpm_init( _SVPMFlow_source, "svpmflow_init" ),
        _clsvpm_initflow( _SVPMFlow_source, "svpmflow_init_from_flow" ),
        _clsvpm_prop( _SVPMFlow_source, "svpmflow_prop_local_tv_admm" ),
        _clsvpm_flow( _SVPMFlow_source, "svpmflow_flow" ),
        _clpyrdown( _pyrdown_source, "pyrdown" ),
        _clpyrupmul( _pyrupmul_source, "pyrup_mul" )
    {

    }

    SVPMFlowCL::~SVPMFlowCL()
    {
    }

    void SVPMFlowCL::flow( Image& flow, const Image& first, const Image& second, int patchsize, float flowmax, Image* gtflow )
    {

        Image clbuf1( first.width(), first.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image clbuf2( first.width(), first.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image clbufp1( first.width(), first.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image clbufp2( first.width(), first.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );

        Image* buf[] = { &clbuf1, &clbuf2 };
        Image* bufp[] = { &clbufp1, &clbufp2 };

        init( clbuf2, first, second, patchsize, flowmax );
        clbufp2.fill( Color( 0.0f, 0.0f, 0.0f, 0.0f ) );

        for( int i = 0; i < MAXITER; i++ ) {
            int swap = i & 1;
            propagate_tv( *buf[ swap ], *buf[ 1 - swap ], *bufp[ swap ], *bufp[ 1 - swap ], first, second, patchsize, flowmax, i + 1 );

             {
                Image tmp;
                toFlow( tmp, *buf[ swap ] );
                if( gtflow )
                    std::cout << i << " AEE: " << Flow::AEE( tmp, *gtflow ) <<  std::endl;
                Image colcode;
                Flow::colorCode( colcode, tmp, flowmax );
                colcode.save("svpmflow.png");
                getchar();
            }
        }

        toFlow( flow, *buf[ 1 - ( MAXITER & 1 ) ] );
    }

    void SVPMFlowCL::flowInit( Image& flow, const Image& first, const Image& second, const Image& init, int patchsize, float flowmax )
    {

        Image clbuf1( first.width(), first.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image clbuf2( first.width(), first.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );

        Image* buf[] = { &clbuf1, &clbuf2 };

        initFlow( clbuf2, first, second, init, patchsize, flowmax );

        for( int i = 0; i < MAXITER; i++ ) {
            int swap = i & 1;
            propagate( *buf[ swap ], *buf[ 1 - swap ], first, second, patchsize, flowmax, i + 1 );

            {
                Image tmp;
                toFlow( tmp, *buf[ swap ] );
                Image colcode;
                Flow::colorCode( colcode, tmp, flowmax );
                colcode.save("svpmflow.png");
                getchar();
            }
        }

        toFlow( flow, *buf[ 1 - ( MAXITER & 1 ) ] );
    }

    void SVPMFlowCL::flowPyramid( Image& flowout, const Image& first, const Image& second, int patchsize, float flowmax, Image* gtflow )
    {
        const float SCALEFACTOR = 0.5f;
        Image* a;
        Image* b;
        Image flowtmp;
        std::vector<std::pair<const Image*, const Image*> > pyramid;

        pyramid.push_back( std::make_pair( &first, &second ) );

        while( pyramid.back().first->width() * SCALEFACTOR > 50.0f &&  pyramid.back().first->height() * SCALEFACTOR > 50.0f ) {
            a = new Image();
            b = new Image();
            int neww = pyramid.back().first->width() * SCALEFACTOR + 0.5f;
            int newh = pyramid.back().first->height() * SCALEFACTOR + 0.5f;
            pyrDown( *a, neww, newh, *pyramid.back().first );
            pyrDown( *b, neww, newh, *pyramid.back().second );
            pyramid.push_back( std::make_pair( a, b ) );
        }

         int level = pyramid.size() - 1;
         std::vector< std::pair<const Image*, const Image*> >::reverse_iterator it = pyramid.rbegin();
         // first iteration
         flow( flowout, *it->first, *it->second, patchsize, flowmax * pow( SCALEFACTOR, level-- ) );
         ++it;
         while( it!= pyramid.rend() ) {
             pyrUpFlow( flowtmp, it->first->width(), it->first->height(), flowout );
            {
                Image colcode;
                Flow::colorCode( colcode, flowtmp, flowmax * pow( SCALEFACTOR, level ) );
                colcode.save("svpmflowup.png");
                getchar();
            }
             flowInit( flowout, *it->first, *it->second, flowtmp, patchsize, flowmax * pow( SCALEFACTOR, level-- ) );
             if( it->first != &first && it->second != &second ) {
                 delete it->first;
                 delete it->second;
             }
             ++it;
         }

         if( gtflow ) {
             Image tmp;
             toFlow( tmp, flowout );
             std::cout << " AEE: " << Flow::AEE( tmp, *gtflow ) <<  std::endl;
         }
    }

    void SVPMFlowCL::init( Image& output, const Image& first, const Image& second, int patchsize, float flowmax )
    {
        _clsvpm_init.setArg( 0, output );
        _clsvpm_init.setArg( 1, first );
        _clsvpm_init.setArg( 2, second );
        _clsvpm_init.setArg( 3, patchsize );
        _clsvpm_init.setArg( 4, flowmax );
        _clsvpm_init.run( CLNDRange( Math::pad( first.width(), KX ), Math::pad( first.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void SVPMFlowCL::initFlow( Image& output, const Image& first, const Image& second, const Image& flow, int patchsize, float flowmax )
    {
        _clsvpm_initflow.setArg( 0, output );
        _clsvpm_initflow.setArg( 1, first );
        _clsvpm_initflow.setArg( 2, second );
        _clsvpm_initflow.setArg( 3, flow );
        _clsvpm_initflow.setArg( 4, patchsize );
        _clsvpm_initflow.setArg( 5, flowmax );
        _clsvpm_initflow.run( CLNDRange( Math::pad( first.width(), KX ), Math::pad( first.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void SVPMFlowCL::propagate( Image& output, const Image& old, const Image& first, const Image& second, int patchsize, float flowmax, int iter )
    {
        _clsvpm_prop.setArg( 0, output );
        _clsvpm_prop.setArg( 1, old );
        _clsvpm_prop.setArg( 2, first );
        _clsvpm_prop.setArg( 3, second );
        _clsvpm_prop.setArg( 4, patchsize );
        _clsvpm_prop.setArg( 5, flowmax );
        _clsvpm_prop.setArg( 6, iter );
        _clsvpm_prop.run( CLNDRange( Math::pad( first.width(), KX ), Math::pad( first.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void SVPMFlowCL::propagate_tv( Image& output, const Image& old, Image& outputp, const Image& oldp, const Image& first, const Image& second, int patchsize, float flowmax, int iter )
    {
        _clsvpm_prop.setArg( 0, output );
        _clsvpm_prop.setArg( 1, old );
        _clsvpm_prop.setArg( 2, first );
        _clsvpm_prop.setArg( 3, second );
        _clsvpm_prop.setArg( 4, outputp );
        _clsvpm_prop.setArg( 5, oldp );
        _clsvpm_prop.setArg( 6, patchsize );
        _clsvpm_prop.setArg( 7, flowmax );
        _clsvpm_prop.setArg( 8, iter );
        _clsvpm_prop.run( CLNDRange( Math::pad( first.width(), KX ), Math::pad( first.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void SVPMFlowCL::toFlow( Image& flow, const Image& buffer )
    {
        flow.reallocate( buffer.width(), buffer.height(), IFormat::GRAYALPHA_FLOAT, IALLOCATOR_CL );
        _clsvpm_flow.setArg( 0, flow );
        _clsvpm_flow.setArg( 1, buffer );
        _clsvpm_flow.run( CLNDRange( Math::pad( buffer.width(), KX ), Math::pad( buffer.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void SVPMFlowCL::pyrDown( Image& output, int width, int height, const Image& input )
    {
        output.reallocate( width, height, input.format(), IALLOCATOR_CL );
        _clpyrdown.setArg( 0, output );
        _clpyrdown.setArg( 1, input );
        _clpyrdown.run( CLNDRange( Math::pad( output.width(), KX ), Math::pad( output.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void SVPMFlowCL::pyrUpFlow( Image& output, int width, int height, const Image& input )
    {
        output.reallocate( width, height, input.format(), IALLOCATOR_CL );
        _clpyrupmul.setArg( 0, output );
        _clpyrupmul.setArg( 1, input );
        _clpyrupmul.setArg( 2, ( ( float ) width ) / ( ( float ) input.width() ) );
        _clpyrupmul.run( CLNDRange( Math::pad( output.width(), KX ), Math::pad( output.height(), KY ) ), CLNDRange( KX, KY ) );
    }

}
