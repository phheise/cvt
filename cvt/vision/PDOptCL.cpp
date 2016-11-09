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

#include <cvt/vision/PDOptCL.h>

#include <cvt/util/Exception.h>

#include <cvt/cl/kernel/fill.h>
#include <cvt/cl/kernel/PDEdgeWeight.h>
#include <cvt/cl/kernel/PDDiffTensor.h>
#include <cvt/cl/kernel/PDROF.h>
#include <cvt/cl/kernel/PDDROF.h>
#include <cvt/cl/kernel/PDROFWeighted.h>
#include <cvt/cl/kernel/PDDROFWeighted.h>
#include <cvt/cl/kernel/PDDROFDiffTensor.h>
#include <cvt/cl/kernel/PDHuber.h>
#include <cvt/cl/kernel/PDDHuber.h>
#include <cvt/cl/kernel/PDHuberWeighted.h>
#include <cvt/cl/kernel/PDDHuberWeighted.h>
#include <cvt/cl/kernel/PDHuberWeightedInpaint.h>
#include <cvt/cl/kernel/PDDHuberDiffTensor.h>
#include <cvt/cl/kernel/PDHL.h>
#include <cvt/cl/kernel/PDMSC.h>
#include <cvt/cl/kernel/PDLOG1.h>

#include <cvt/cl/CLMatrix.h>

namespace cvt {
    PDOptCL::PDOptCL() :
        _clfill( _fill_source, "fill" ),
        _clweightColor( _PDEdgeWeight_source, "PDEdgeWeightColor" ),
        _clweightGray( _PDEdgeWeight_source, "PDEdgeWeightGray" ),
        _clweightColorConfid( _PDEdgeWeight_source, "PDEdgeWeightColorCondfidence" ),
        _cldiffTensorColor( _PDDiffTensor_source, "PDDiffTensorColor" ),
        _cldiffTensorGray( _PDDiffTensor_source, "PDDiffTensorGray" ),
        _cldiffTensorColorConfid( _PDDiffTensor_source, "PDDiffTensorConfidenceColor" ),
        _cldiffTensorGrayConfid( _PDDiffTensor_source, "PDDiffTensorConfidenceGray" ),
        _clrof( _PDROF_source, "PDROF" ),
        _clrofpdd( _PDDROF_source, "PDDROF" ),
        _clhuber( _PDHuber_source, "PDHuber" ),
        _clhuberpdd( _PDDHuber_source, "PDDHuber" ),
        _clwrof( _PDROFWeighted_source, "PDROFWeighted" ),
        _clwrofpdd( _PDDROFWeighted_source, "PDDROFWeighted" ),
        _clwhuber(_PDHuberWeighted_source, "PDHuberWeighted" ),
        _clwhuberpdd(_PDDHuberWeighted_source, "PDDHuberWeighted" ),
        _cldtrofpdd(_PDDROFDiffTensor_source, "PDDROFDiffTensor"),
        _cldthuberpdd(_PDDHuberDiffTensor_source, "PDDHuberDiffTensor"),
        _clinpaintdthuberpdd(_PDDHuberDiffTensor_source, "PDDHuberInpaintDiffTensor"),
        _clnddtrofpdd(_PDDROFDiffTensor_source, "PDDROFNDDiffTensor"),
        _clposdtrofpdd(_PDDROFDiffTensor_source, "PDDROFPOSDiffTensor"),
        _clinpaintwhuber( _PDHuberWeightedInpaint_source, "PDHuberWeightedInpaint" ),
        _clhl( _PDHL_source, "PDHL" ),
        _clhlcwise( _PDHL_source, "PDHL_CWISE" ),
        _clmsc( _PDMSC_source, "PDMSC" ),
        _clmsccwise( _PDMSC_source, "PDMSC_CWISE" ),
        _cllog1( _PDLOG1_source, "PDLOG1" ),
        _cllog1cwise( _PDLOG1_source, "PDLOG1_CWISE" ),
        _cllog18( _PDLOG1_source, "PDLOG1_8")
    {
    }

    PDOptCL::~PDOptCL()
    {
    }

    void PDOptCL::edgeWeight( Image& dst, const Image& input, float alpha, float beta ) const
    {
         if( input.memType() != IALLOCATOR_CL )
            throw CVTException( "Source must use IALLOCATOR_CL!" );

        dst.assign( input, IALLOCATOR_CL );

        if( input.format().channels == 4 ) {
            _clweightColor.setArg( 0, dst );
            _clweightColor.setArg( 1, input );
            _clweightColor.setArg( 2, alpha );
            _clweightColor.setArg( 3, beta );
            _clweightColor.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        } else {
            _clweightGray.setArg( 0, dst );
            _clweightGray.setArg( 1, input );
            _clweightGray.setArg( 2, alpha );
            _clweightGray.setArg( 3, beta );
            _clweightGray.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }
    }

    void PDOptCL::edgeWeightConfidence( Image& dst, const Image& input,const Image& cmap, float alpha, float beta ) const
    {
         if( input.memType() != IALLOCATOR_CL )
            throw CVTException( "Source must use IALLOCATOR_CL!" );

        dst.assign( input, IALLOCATOR_CL );

        if( input.format().channels == 4 ) {
            _clweightColorConfid.setArg( 0, dst );
            _clweightColorConfid.setArg( 1, input );
            _clweightColorConfid.setArg( 2, cmap );
            _clweightColorConfid.setArg( 3, alpha );
            _clweightColorConfid.setArg( 4, beta );
            _clweightColorConfid.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        } else {
            throw CVTException( "Unimplemented!" );
            //_clweightGray.setArg( 0, dst );
            //_clweightGray.setArg( 1, input );
            //_clweightGray.setArg( 2, alpha );
            //_clweightGray.setArg( 3, beta );
            //_clweightGray.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }
    }

    void PDOptCL::diffusionTensor( Image& dst, const Image& input, float alpha, float beta ) const
    {
         if( input.memType() != IALLOCATOR_CL )
            throw CVTException( "Source must use IALLOCATOR_CL!" );

        dst.reallocate( input.width(), input.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );

        if( input.format().channels == 4 ) {
            _cldiffTensorColor.setArg( 0, dst );
            _cldiffTensorColor.setArg( 1, input );
            _cldiffTensorColor.setArg( 2, alpha );
            _cldiffTensorColor.setArg( 3, beta );
            _cldiffTensorColor.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        } else if( input.format().channels == 1 ){
            _cldiffTensorGray.setArg( 0, dst );
            _cldiffTensorGray.setArg( 1, input );
            _cldiffTensorGray.setArg( 2, alpha );
            _cldiffTensorGray.setArg( 3, beta );
            _cldiffTensorGray.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        } else
            throw CVTException( "Unimplemented!" );
    }

    void PDOptCL::diffusionTensorConfidence( Image& dst, const Image& input, const Image& confid, float alpha, float beta ) const
    {
         if( input.memType() != IALLOCATOR_CL )
            throw CVTException( "Source must use IALLOCATOR_CL!" );

        dst.reallocate( input.width(), input.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );

        if( input.format().channels == 4 ) {
            _cldiffTensorColorConfid.setArg( 0, dst );
            _cldiffTensorColorConfid.setArg( 1, input );
            _cldiffTensorColorConfid.setArg( 2, confid );
            _cldiffTensorColorConfid.setArg( 3, alpha );
            _cldiffTensorColorConfid.setArg( 4, beta );
            _cldiffTensorColorConfid.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        } else if( input.format().channels == 1 ){
            _cldiffTensorGrayConfid.setArg( 0, dst );
            _cldiffTensorGrayConfid.setArg( 1, input );
            _cldiffTensorGrayConfid.setArg( 2, confid );
            _cldiffTensorGrayConfid.setArg( 3, alpha );
            _cldiffTensorGrayConfid.setArg( 4, beta );
            _cldiffTensorGrayConfid.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        } else
            throw CVTException( "Unimplemented!" );
    }

    void PDOptCL::denoiseROF( Image& output, const Image& input, float lambda, size_t iter ) const
    {
        if( input.memType() != IALLOCATOR_CL )
            throw CVTException( "Source must use IALLOCATOR_CL!" );

        output.assign( input, IALLOCATOR_CL );

        Image cltmp( input.width(), input.height(), input.format(), IALLOCATOR_CL );
        Image clp1( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clp2( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image* climg[ 2 ] = { &cltmp, &output };
        Image* clp[ 2 ] = { &clp1, &clp2 };

        cl_float4 fillvalue = {{ 0.0f, 0.0f, 0.0f, 0.0f }};

        fill( *clp[ 1 ], fillvalue );

        size_t swap = 0;
        for( size_t i = 0; i < iter; i++ ) {
            swap = i & 1;

            _clrof.setArg( 0, *climg[ swap ] );
            _clrof.setArg( 1, *clp[ swap ] );
            _clrof.setArg( 2, *climg[ 1 - swap ] );
            _clrof.setArg( 3, *clp[ 1 - swap ] );
            _clrof.setArg( 4, input );
            _clrof.setArg( 5, lambda );
            _clrof.setArg( 6, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clrof.setArg( 7, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clrof.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }

        if( !swap ) {
            _clrof.setArg( 0, *climg[ 1 ] );
            _clrof.setArg( 1, *clp[ 1 ] );
            _clrof.setArg( 2, *climg[ 0 ] );
            _clrof.setArg( 3, *clp[ 0 ] );
            _clrof.setArg( 4, input );
            _clrof.setArg( 5, lambda );
            _clrof.setArg( 6, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clrof.setArg( 7, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clrof.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }
    }

    void PDOptCL::denoiseHuber( Image& output, const Image& input, float lambda, float hubereps, size_t iter ) const
    {
        if( input.memType() != IALLOCATOR_CL )
            throw CVTException( "Source must use IALLOCATOR_CL!" );

        output.assign( input, IALLOCATOR_CL );

        Image cltmp( input.width(), input.height(), input.format(), IALLOCATOR_CL );
        Image clp1( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clp2( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image* climg[ 2 ] = { &cltmp, &output };
        Image* clp[ 2 ] = { &clp1, &clp2 };

        cl_float4 fillvalue = {{ 0.0f, 0.0f, 0.0f, 0.0f }};

        fill( *clp[ 1 ], fillvalue );

        size_t swap = 0;
        for( size_t i = 0; i < iter; i++ ) {
            swap = i & 1;

            _clhuber.setArg( 0, *climg[ swap ] );
            _clhuber.setArg( 1, *clp[ swap ] );
            _clhuber.setArg( 2, *climg[ 1 - swap ] );
            _clhuber.setArg( 3, *clp[ 1 - swap ] );
            _clhuber.setArg( 4, input );
            _clhuber.setArg( 5, lambda );
            _clhuber.setArg( 6, hubereps );
            _clhuber.setArg( 7, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clhuber.setArg( 8, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clhuber.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }

        if( !swap ) {
            _clhuber.setArg( 0, *climg[ 1 ] );
            _clhuber.setArg( 1, *clp[ 1 ] );
            _clhuber.setArg( 2, *climg[ 0 ] );
            _clhuber.setArg( 3, *clp[ 0 ] );
            _clhuber.setArg( 4, input );
            _clhuber.setArg( 5, lambda );
            _clhuber.setArg( 6, hubereps );
            _clhuber.setArg( 7, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clhuber.setArg( 8, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clhuber.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }
    }

    void PDOptCL::denoiseHL( Image& output, const Image& input, float lambda, float alpha, size_t iter ) const
    {
        const float gamma = 1.8f;
        float sigma = 0.5f;
        float tau   = 0.25f;
        float theta;

        if( input.memType() != IALLOCATOR_CL )
            throw CVTException( "Source must use IALLOCATOR_CL!" );

        output.assign( input, IALLOCATOR_CL );

        Image cltmp( input.width(), input.height(), input.format(), IALLOCATOR_CL );
        Image clp1( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clp2( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image* climg[ 2 ] = { &cltmp, &output };
        Image* clp[ 2 ] = { &clp1, &clp2 };

        cl_float4 fillvalue = {{ 0.0f, 0.0f, 0.0f, 0.0f }};

        fill( *clp[ 1 ], fillvalue );

        size_t swap = 0;
        for( size_t i = 0; i < iter; i++ ) {
            swap = i & 1;

            theta = 1.0f / Math::sqrt( 1.0f + 2.0f * gamma * tau );

            _clhl.setArg( 0, *climg[ swap ] );
            _clhl.setArg( 1, *clp[ swap ] );
            _clhl.setArg( 2, *climg[ 1 - swap ] );
            _clhl.setArg( 3, *clp[ 1 - swap ] );
            _clhl.setArg( 4, input );
            _clhl.setArg( 5, lambda );
            _clhl.setArg( 6, sigma );
            _clhl.setArg( 7, tau );
            _clhl.setArg( 8, theta );
            _clhl.setArg( 9, alpha );
            _clhl.setArg( 10, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clhl.setArg( 11, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clhl.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );

            tau = theta * tau;
            sigma = sigma / theta;
        }

        if( !swap ) {
            theta = 1.0f / Math::sqrt( 1.0f + 2.0f * gamma * tau );
            _clhl.setArg( 0, *climg[ 1 ] );
            _clhl.setArg( 1, *clp[ 1 ] );
            _clhl.setArg( 2, *climg[ 0 ] );
            _clhl.setArg( 3, *clp[ 0 ] );
            _clhl.setArg( 4, input );
            _clhl.setArg( 5, lambda );
            _clhl.setArg( 6, sigma );
            _clhl.setArg( 7, tau );
            _clhl.setArg( 8, theta );
            _clhl.setArg( 9, alpha );
            _clhl.setArg( 10, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clhl.setArg( 11, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clhl.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }
    }

    void PDOptCL::denoiseHL_cwise( Image& output, const Image& input, float lambda, float alpha, size_t iter ) const
    {
        const float gamma = 1.8f;
        float sigma = 0.5f;
        float tau   = 0.25f;
        float theta;

        if( input.memType() != IALLOCATOR_CL )
            throw CVTException( "Source must use IALLOCATOR_CL!" );

        output.assign( input, IALLOCATOR_CL );

        Image cltmp( input.width(), input.height(), input.format(), IALLOCATOR_CL );
        Image clp1( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clp2( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image* climg[ 2 ] = { &cltmp, &output };
        Image* clp[ 2 ] = { &clp1, &clp2 };

        cl_float4 fillvalue = {{ 0.0f, 0.0f, 0.0f, 0.0f }};

        fill( *clp[ 1 ], fillvalue );

        size_t swap = 0;
        for( size_t i = 0; i < iter; i++ ) {
            swap = i & 1;

            theta = 1.0f / Math::sqrt( 1.0f + 2.0f * gamma * tau );

            _clhlcwise.setArg( 0, *climg[ swap ] );
            _clhlcwise.setArg( 1, *clp[ swap ] );
            _clhlcwise.setArg( 2, *climg[ 1 - swap ] );
            _clhlcwise.setArg( 3, *clp[ 1 - swap ] );
            _clhlcwise.setArg( 4, input );
            _clhlcwise.setArg( 5, lambda );
            _clhlcwise.setArg( 6, sigma );
            _clhlcwise.setArg( 7, tau );
            _clhlcwise.setArg( 8, theta );
            _clhlcwise.setArg( 9, alpha );
            _clhlcwise.setArg( 10, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clhlcwise.setArg( 11, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clhlcwise.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );

            tau = theta * tau;
            sigma = sigma / theta;
        }

        if( !swap ) {
            theta = 1.0f / Math::sqrt( 1.0f + 2.0f * gamma * tau );
            _clhlcwise.setArg( 0, *climg[ 1 ] );
            _clhlcwise.setArg( 1, *clp[ 1 ] );
            _clhlcwise.setArg( 2, *climg[ 0 ] );
            _clhlcwise.setArg( 3, *clp[ 0 ] );
            _clhlcwise.setArg( 4, input );
            _clhlcwise.setArg( 5, lambda );
            _clhlcwise.setArg( 6, sigma );
            _clhlcwise.setArg( 7, tau );
            _clhlcwise.setArg( 8, theta );
            _clhlcwise.setArg( 9, alpha );
            _clhlcwise.setArg( 10, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clhlcwise.setArg( 11, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clhlcwise.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }
    }

    void PDOptCL::denoiseLOG1( Image& output, const Image& input, float lambda, float beta, size_t iter ) const
    {
        const float gamma = 1.8f;
        float sigma = 0.5f;
        float tau   = 0.25f;
        float theta;

        if( input.memType() != IALLOCATOR_CL )
            throw CVTException( "Source must use IALLOCATOR_CL!" );

        output.assign( input, IALLOCATOR_CL );

        Image cltmp( input.width(), input.height(), input.format(), IALLOCATOR_CL );
        Image clp1( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clp2( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image* climg[ 2 ] = { &cltmp, &output };
        Image* clp[ 2 ] = { &clp1, &clp2 };

        cl_float4 fillvalue = {{ 0.0f, 0.0f, 0.0f, 0.0f }};

        fill( *clp[ 1 ], fillvalue );

        size_t swap = 0;
        for( size_t i = 0; i < iter; i++ ) {
            swap = i & 1;

            theta = 1.0f / Math::sqrt( 1.0f + 2.0f * gamma * tau );

            _cllog1.setArg( 0, *climg[ swap ] );
            _cllog1.setArg( 1, *clp[ swap ] );
            _cllog1.setArg( 2, *climg[ 1 - swap ] );
            _cllog1.setArg( 3, *clp[ 1 - swap ] );
            _cllog1.setArg( 4, input );
            _cllog1.setArg( 5, lambda );
            _cllog1.setArg( 6, sigma );
            _cllog1.setArg( 7, tau );
            _cllog1.setArg( 8, theta );
            _cllog1.setArg( 9, beta );
            _cllog1.setArg( 10, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _cllog1.setArg( 11, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _cllog1.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );

            tau = theta * tau;
            sigma = sigma / theta;
        }

        if( !swap ) {
            theta = 1.0f / Math::sqrt( 1.0f + 2.0f * gamma * tau );
            _cllog1.setArg( 0, *climg[ 1 ] );
            _cllog1.setArg( 1, *clp[ 1 ] );
            _cllog1.setArg( 2, *climg[ 0 ] );
            _cllog1.setArg( 3, *clp[ 0 ] );
            _cllog1.setArg( 4, input );
            _cllog1.setArg( 5, lambda );
            _cllog1.setArg( 6, sigma );
            _cllog1.setArg( 7, tau );
            _cllog1.setArg( 8, theta );
            _cllog1.setArg( 9, beta );
            _cllog1.setArg( 10, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _cllog1.setArg( 11, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _cllog1.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }
    }

    void PDOptCL::denoiseLOG1( Image& output1, Image& output2, const Image& input1, const Image& input2, float lambda, float beta, size_t iter ) const
    {
        const float gamma = 1.8f;
        float sigma = 0.5f;
        float tau   = 0.25f;
        float theta;

        if( input1.memType() != IALLOCATOR_CL || input2.memType() != IALLOCATOR_CL )
            throw CVTException( "Source must use IALLOCATOR_CL!" );

        output1.assign( input1, IALLOCATOR_CL );
        output2.assign( input2, IALLOCATOR_CL );

        output1 = input1;
        output2 = input2;

        Image cltmp( input1.width(), input1.height(), input1.format(), IALLOCATOR_CL );
        Image cltmp2( input1.width(), input1.height(), input1.format(), IALLOCATOR_CL );
        Image clp1( input1.width()*4, input1.height(), IFormat::floatEquivalent( input1.format() ), IALLOCATOR_CL );
        Image clp2( input1.width()*4, input1.height(), IFormat::floatEquivalent( input1.format() ), IALLOCATOR_CL );
        Image* climg1[ 2 ] = { &cltmp, &output1 };
        Image* climg2[ 2 ] = { &cltmp2, &output2 };
        Image* clp[ 2 ] = { &clp1, &clp2 };

        cl_float4 fillvalue = {{ 0.0f, 0.0f, 0.0f, 0.0f }};
        fill( *clp[ 1 ], fillvalue );

        size_t swap = 0;
        for( size_t i = 0; i < iter; i++ ) {
            swap = i & 1;

            theta = 1.0f / Math::sqrt( 1.0f + 2.0f * gamma * tau );

            _cllog18.setArg( 0, *climg1[ swap ] );
            _cllog18.setArg( 1, *climg2[ swap ] );
            _cllog18.setArg( 2, *clp[ swap ] );
            _cllog18.setArg( 3, *climg1[ 1 - swap ] );
            _cllog18.setArg( 4, *climg2[ 1 - swap ] );
            _cllog18.setArg( 5, *clp[ 1 - swap ] );
            _cllog18.setArg( 6, input1 );
            _cllog18.setArg( 7, input2 );
            _cllog18.setArg( 8, lambda );
            _cllog18.setArg( 9, sigma );
            _cllog18.setArg( 10, tau );
            _cllog18.setArg( 11, theta );
            _cllog18.setArg( 12, beta );
            _cllog18.setArg( 13, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _cllog18.setArg( 14, CLLocalSpace( sizeof( cl_float16 ) * 18 * 18 ) );
            _cllog18.run( CLNDRange( Math::pad( input1.width(), 16 ), Math::pad( input1.height(), 16 ) ), CLNDRange( 16, 16 ) );

            tau = theta * tau;
            sigma = sigma / theta;
        }

        if( !swap ) {
            theta = 1.0f / Math::sqrt( 1.0f + 2.0f * gamma * tau );
            _cllog18.setArg( 0, *climg1[ 1 ] );
            _cllog18.setArg( 1, *climg2[ 1 ] );
            _cllog18.setArg( 2, *clp[ 1 ] );
            _cllog18.setArg( 3, *climg1[ 0 ] );
            _cllog18.setArg( 4, *climg2[ 0 ] );
            _cllog18.setArg( 5, *clp[ 0 ] );
            _cllog18.setArg( 6, input1 );
            _cllog18.setArg( 7, input2 );
            _cllog18.setArg( 8, lambda );
            _cllog18.setArg( 9, sigma );
            _cllog18.setArg( 10, tau );
            _cllog18.setArg( 11, theta );
            _cllog18.setArg( 12, beta );
            _cllog18.setArg( 13, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _cllog18.setArg( 14, CLLocalSpace( sizeof( cl_float16 ) * 18 * 18 ) );
            _cllog18.run( CLNDRange( Math::pad( input1.width(), 16 ), Math::pad( input1.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }
    }

    void PDOptCL::denoiseLOG1_cwise( Image& output, const Image& input, float lambda, float beta, size_t iter ) const
    {
        const float gamma = 1.8f;
        float sigma = 0.5f;
        float tau   = 0.25f;
        float theta;

        if( input.memType() != IALLOCATOR_CL )
            throw CVTException( "Source must use IALLOCATOR_CL!" );

        output.assign( input, IALLOCATOR_CL );

        Image cltmp( input.width(), input.height(), input.format(), IALLOCATOR_CL );
        Image clp1( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clp2( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image* climg[ 2 ] = { &cltmp, &output };
        Image* clp[ 2 ] = { &clp1, &clp2 };

        cl_float4 fillvalue = {{ 0.0f, 0.0f, 0.0f, 0.0f }};

        fill( *clp[ 1 ], fillvalue );

        size_t swap = 0;
        for( size_t i = 0; i < iter; i++ ) {
            swap = i & 1;

            theta = 1.0f / Math::sqrt( 1.0f + 2.0f * gamma * tau );

            _cllog1cwise.setArg( 0, *climg[ swap ] );
            _cllog1cwise.setArg( 1, *clp[ swap ] );
            _cllog1cwise.setArg( 2, *climg[ 1 - swap ] );
            _cllog1cwise.setArg( 3, *clp[ 1 - swap ] );
            _cllog1cwise.setArg( 4, input );
            _cllog1cwise.setArg( 5, lambda );
            _cllog1cwise.setArg( 6, sigma );
            _cllog1cwise.setArg( 7, tau );
            _cllog1cwise.setArg( 8, theta );
            _cllog1cwise.setArg( 9, beta );
            _cllog1cwise.setArg( 10, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _cllog1cwise.setArg( 11, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _cllog1cwise.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );

            tau = theta * tau;
            sigma = sigma / theta;
        }

        if( !swap ) {
            theta = 1.0f / Math::sqrt( 1.0f + 2.0f * gamma * tau );
            _cllog1cwise.setArg( 0, *climg[ 1 ] );
            _cllog1cwise.setArg( 1, *clp[ 1 ] );
            _cllog1cwise.setArg( 2, *climg[ 0 ] );
            _cllog1cwise.setArg( 3, *clp[ 0 ] );
            _cllog1cwise.setArg( 4, input );
            _cllog1cwise.setArg( 5, lambda );
            _cllog1cwise.setArg( 6, sigma );
            _cllog1cwise.setArg( 7, tau );
            _cllog1cwise.setArg( 8, theta );
            _cllog1cwise.setArg( 9, beta );
            _cllog1cwise.setArg( 10, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _cllog1cwise.setArg( 11, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _cllog1cwise.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }
    }

    void PDOptCL::mumfordShahConst( Image& output, const Image& input, float lambda, size_t iter ) const
    {
        float sigma = 0.5f;
        float tau   = 0.25f;
        float theta;

        if( input.memType() != IALLOCATOR_CL )
            throw CVTException( "Source must use IALLOCATOR_CL!" );

        output.assign( input, IALLOCATOR_CL );

        Image cltmp( input.width(), input.height(), input.format(), IALLOCATOR_CL );
        Image clp1( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clp2( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image* climg[ 2 ] = { &cltmp, &output };
        Image* clp[ 2 ] = { &clp1, &clp2 };

        cl_float4 fillvalue = {{ 0.0f, 0.0f, 0.0f, 0.0f }};

        fill( *clp[ 1 ], fillvalue );

        size_t swap = 0;
        for( size_t i = 0; i < iter; i++ ) {
            swap = i & 1;

            theta = 1.0f / Math::sqrt( 1.0f + 4.0f * tau );

            _clmsc.setArg( 0, *climg[ swap ] );
            _clmsc.setArg( 1, *clp[ swap ] );
            _clmsc.setArg( 2, *climg[ 1 - swap ] );
            _clmsc.setArg( 3, *clp[ 1 - swap ] );
            _clmsc.setArg( 4, input );
            _clmsc.setArg( 5, lambda );
            _clmsc.setArg( 6, sigma );
            _clmsc.setArg( 7, tau );
            _clmsc.setArg( 8, theta );
            _clmsc.setArg( 9, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clmsc.setArg( 10, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clmsc.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );

            tau = theta * tau;
            sigma = sigma / theta;
        }

        if( !swap ) {
            theta = 1.0f / Math::sqrt( 1.0f + 4.0f * tau );
            _clmsc.setArg( 0, *climg[ 1 ] );
            _clmsc.setArg( 1, *clp[ 1 ] );
            _clmsc.setArg( 2, *climg[ 0 ] );
            _clmsc.setArg( 3, *clp[ 0 ] );
            _clmsc.setArg( 4, input );
            _clmsc.setArg( 5, lambda );
            _clmsc.setArg( 6, sigma );
            _clmsc.setArg( 7, tau );
            _clmsc.setArg( 8, theta );
            _clmsc.setArg( 9, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clmsc.setArg( 10, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clmsc.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }
    }

    void PDOptCL::mumfordShahConst_cwise( Image& output, const Image& input, float lambda, size_t iter ) const
    {
        float sigma = 0.5f;
        float tau   = 0.25f;
        float theta;

        if( input.memType() != IALLOCATOR_CL )
            throw CVTException( "Source must use IALLOCATOR_CL!" );

        output.assign( input, IALLOCATOR_CL );

        Image cltmp( input.width(), input.height(), input.format(), IALLOCATOR_CL );
        Image clp1( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clp2( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image* climg[ 2 ] = { &cltmp, &output };
        Image* clp[ 2 ] = { &clp1, &clp2 };

        cl_float4 fillvalue = {{ 0.0f, 0.0f, 0.0f, 0.0f }};

        fill( *clp[ 1 ], fillvalue );

        size_t swap = 0;
        for( size_t i = 0; i < iter; i++ ) {
            swap = i & 1;

            theta = 1.0f / Math::sqrt( 1.0f + 4.0f * tau );

            _clmsccwise.setArg( 0, *climg[ swap ] );
            _clmsccwise.setArg( 1, *clp[ swap ] );
            _clmsccwise.setArg( 2, *climg[ 1 - swap ] );
            _clmsccwise.setArg( 3, *clp[ 1 - swap ] );
            _clmsccwise.setArg( 4, input );
            _clmsccwise.setArg( 5, lambda );
            _clmsccwise.setArg( 6, sigma );
            _clmsccwise.setArg( 7, tau );
            _clmsccwise.setArg( 8, theta );
            _clmsccwise.setArg( 9, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clmsccwise.setArg( 10, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clmsccwise.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );

            tau = theta * tau;
            sigma = sigma / theta;
        }

        if( !swap ) {
            theta = 1.0f / Math::sqrt( 1.0f + 4.0f * tau );
            _clmsccwise.setArg( 0, *climg[ 1 ] );
            _clmsccwise.setArg( 1, *clp[ 1 ] );
            _clmsccwise.setArg( 2, *climg[ 0 ] );
            _clmsccwise.setArg( 3, *clp[ 0 ] );
            _clmsccwise.setArg( 4, input );
            _clmsccwise.setArg( 5, lambda );
            _clmsccwise.setArg( 6, sigma );
            _clmsccwise.setArg( 7, tau );
            _clmsccwise.setArg( 8, theta );
            _clmsccwise.setArg( 9, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clmsccwise.setArg( 10, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clmsccwise.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }
    }

    void PDOptCL::denoiseROF_PDD( Image& output, const Image& input, float lambda, size_t iter ) const
    {
        if( input.memType() != IALLOCATOR_CL )
            throw CVTException( "Source must use IALLOCATOR_CL!" );

        output.assign( input, IALLOCATOR_CL );

        Image cltmp( input.width(), input.height(), input.format(), IALLOCATOR_CL );
        Image clp1( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clp2( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clq1( input.width(), input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clq2( input.width(), input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image* climg[ 2 ] = { &cltmp, &output };
        Image* clp[ 2 ] = { &clp1, &clp2 };
        Image* clq[ 2 ] = { &clq1, &clq2 };

        cl_float4 fillvalue = {{ 0.0f, 0.0f, 0.0f, 0.0f }};

        fill( *clp[ 1 ], fillvalue );
        fill( *clq[ 1 ], fillvalue );

        size_t swap = 0;
        for( size_t i = 0; i < iter; i++ ) {
            swap = i & 1;

            _clrofpdd.setArg( 0, *climg[ swap ] );
            _clrofpdd.setArg( 1, *clp[ swap ] );
            _clrofpdd.setArg( 2, *clq[ swap ] );
            _clrofpdd.setArg( 3, *climg[ 1 - swap ] );
            _clrofpdd.setArg( 4, *clp[ 1 - swap ] );
            _clrofpdd.setArg( 5, *clq[ 1 - swap ] );
            _clrofpdd.setArg( 6, input );
            _clrofpdd.setArg( 7, lambda );
            _clrofpdd.setArg( 8, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clrofpdd.setArg( 9, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clrofpdd.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }

        if( !swap ) {
            _clrofpdd.setArg( 0, *climg[ 1 ] );
            _clrofpdd.setArg( 1, *clp[ 1 ] );
            _clrofpdd.setArg( 2, *clq[ 1 ] );
            _clrofpdd.setArg( 3, *climg[ 0 ] );
            _clrofpdd.setArg( 4, *clp[ 0 ] );
            _clrofpdd.setArg( 5, *clq[ 0 ] );
            _clrofpdd.setArg( 6, input );
            _clrofpdd.setArg( 7, lambda );
            _clrofpdd.setArg( 8, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clrofpdd.setArg( 9, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clrofpdd.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }
    }

    void PDOptCL::denoiseHuber_PDD( Image& output, const Image& input, float lambda, float hubereps, size_t iter ) const
    {
        if( input.memType() != IALLOCATOR_CL )
            throw CVTException( "Source must use IALLOCATOR_CL!" );

        output.assign( input, IALLOCATOR_CL );

        Image cltmp( input.width(), input.height(), input.format(), IALLOCATOR_CL );
        Image clp1( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clp2( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clq1( input.width(), input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clq2( input.width(), input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image* climg[ 2 ] = { &cltmp, &output };
        Image* clp[ 2 ] = { &clp1, &clp2 };
        Image* clq[ 2 ] = { &clq1, &clq2 };

        cl_float4 fillvalue = {{ 0.0f, 0.0f, 0.0f, 0.0f }};

        fill( *clp[ 1 ], fillvalue );
        fill( *clq[ 1 ], fillvalue );

        size_t swap = 0;
        for( size_t i = 0; i < iter; i++ ) {
            swap = i & 1;

            _clhuberpdd.setArg( 0, *climg[ swap ] );
            _clhuberpdd.setArg( 1, *clp[ swap ] );
            _clhuberpdd.setArg( 2, *clq[ swap ] );
            _clhuberpdd.setArg( 3, *climg[ 1 - swap ] );
            _clhuberpdd.setArg( 4, *clp[ 1 - swap ] );
            _clhuberpdd.setArg( 5, *clq[ 1 - swap ] );
            _clhuberpdd.setArg( 6, input );
            _clhuberpdd.setArg( 7, lambda );
            _clhuberpdd.setArg( 8, hubereps );
            _clhuberpdd.setArg( 9, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clhuberpdd.setArg( 10, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clhuberpdd.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }

        if( !swap ) {
            _clhuberpdd.setArg( 0, *climg[ 1 ] );
            _clhuberpdd.setArg( 1, *clp[ 1 ] );
            _clhuberpdd.setArg( 2, *clq[ 1 ] );
            _clhuberpdd.setArg( 3, *climg[ 0 ] );
            _clhuberpdd.setArg( 4, *clp[ 0 ] );
            _clhuberpdd.setArg( 5, *clq[ 0 ] );
            _clhuberpdd.setArg( 6, input );
            _clhuberpdd.setArg( 7, lambda );
            _clhuberpdd.setArg( 8, hubereps );
            _clhuberpdd.setArg( 9, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clhuberpdd.setArg( 10, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clhuberpdd.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }
    }

    void PDOptCL::denoiseWeightedROF( Image& output, const Image& input, const Image& weight, float lambda, size_t iter ) const
    {
        if( input.memType() != IALLOCATOR_CL || weight.memType() != IALLOCATOR_CL )
            throw CVTException( "input/weight must use IALLOCATOR_CL!" );
        if( input.width() != weight.width() || input.height() != weight.height() )
            throw CVTException( "Source and weight-image must have the same size" );

        output.assign( input, IALLOCATOR_CL );

        Image cltmp( input.width(), input.height(), input.format(), IALLOCATOR_CL );
        Image clp1( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clp2( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image* climg[ 2 ] = { &cltmp, &output };
        Image* clp[ 2 ] = { &clp1, &clp2 };

        cl_float4 fillvalue = {{ 0.0f, 0.0f, 0.0f, 0.0f }};
        cl_float4 lambda4 = {{ lambda, lambda, lambda, lambda }};

        fill( *clp[ 1 ], fillvalue );

        size_t swap = 0;
        for( size_t i = 0; i < iter; i++ ) {
            swap = i & 1;

            _clwrof.setArg( 0, *climg[ swap ] );
            _clwrof.setArg( 1, *clp[ swap ] );
            _clwrof.setArg( 2, *climg[ 1 - swap ] );
            _clwrof.setArg( 3, *clp[ 1 - swap ] );
            _clwrof.setArg( 4, input );
            _clwrof.setArg( 5, weight );
            _clwrof.setArg( 6, lambda );
            _clwrof.setArg( 7, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clwrof.setArg( 8, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clwrof.setArg( 9, CLLocalSpace( sizeof( cl_float ) * 18 * 18 ) );
            _clwrof.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }

        if( !swap ) {
            _clwrof.setArg( 0, *climg[ 1 ] );
            _clwrof.setArg( 1, *clp[ 1 ] );
            _clwrof.setArg( 2, *climg[ 0 ] );
            _clwrof.setArg( 3, *clp[ 0 ] );
            _clwrof.setArg( 4, input );
            _clwrof.setArg( 5, weight );
            _clwrof.setArg( 6, lambda );
            _clwrof.setArg( 7, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clwrof.setArg( 8, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clwrof.setArg( 9, CLLocalSpace( sizeof( cl_float ) * 18 * 18 ) );
            _clwrof.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }
    }

    void PDOptCL::denoiseWeightedROF_PDD( Image& output, const Image& input, const Image& weight, const Vector4f& lambda, size_t iter ) const
    {
        if( input.memType() != IALLOCATOR_CL )
            throw CVTException( "Source must use IALLOCATOR_CL!" );

        output.assign( input, IALLOCATOR_CL );
        output = input;

        Image cltmp( input.width(), input.height(), input.format(), IALLOCATOR_CL );
        Image clp1( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clp2( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clq1( input.width(), input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clq2( input.width(), input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image* climg[ 2 ] = { &cltmp, &output };
        Image* clp[ 2 ] = { &clp1, &clp2 };
        Image* clq[ 2 ] = { &clq1, &clq2 };

        cl_float4 fillvalue = {{ 0.0f, 0.0f, 0.0f, 0.0f }};

        fill( *clp[ 1 ], fillvalue );
        fill( *clq[ 1 ], fillvalue );

        size_t swap = 0;
        for( size_t i = 0; i < iter; i++ ) {
            swap = i & 1;

            _clwrofpdd.setArg( 0, *climg[ swap ] );
            _clwrofpdd.setArg( 1, *clp[ swap ] );
            _clwrofpdd.setArg( 2, *clq[ swap ] );
            _clwrofpdd.setArg( 3, *climg[ 1 - swap ] );
            _clwrofpdd.setArg( 4, *clp[ 1 - swap ] );
            _clwrofpdd.setArg( 5, *clq[ 1 - swap ] );
            _clwrofpdd.setArg( 6, input );
            _clwrofpdd.setArg( 7, weight );
            _clwrofpdd.setArg( 8, lambda );
            _clwrofpdd.setArg( 9, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clwrofpdd.setArg( 10, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clwrofpdd.setArg( 11, CLLocalSpace( sizeof( cl_float ) * 18 * 18 ) );
            _clwrofpdd.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }

        if( !swap ) {
            _clwrofpdd.setArg( 0, *climg[ 1 ] );
            _clwrofpdd.setArg( 1, *clp[ 1 ] );
            _clwrofpdd.setArg( 2, *clq[ 1 ] );
            _clwrofpdd.setArg( 3, *climg[ 0 ] );
            _clwrofpdd.setArg( 4, *clp[ 0 ] );
            _clwrofpdd.setArg( 5, *clq[ 0 ] );
            _clwrofpdd.setArg( 6, input );
            _clwrofpdd.setArg( 7, weight );
            _clwrofpdd.setArg( 8, lambda );
            _clwrofpdd.setArg( 9, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clwrofpdd.setArg( 10, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clwrofpdd.setArg( 11, CLLocalSpace( sizeof( cl_float ) * 18 * 18 ) );
            _clwrofpdd.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }
    }

    void PDOptCL::denoiseWeightedHuber( Image& output, const Image& input, const Image& weight, float lambda, float hubereps, size_t iter ) const
    {
        if( input.memType() != IALLOCATOR_CL || weight.memType() != IALLOCATOR_CL )
            throw CVTException( "input/weight must use IALLOCATOR_CL!" );
        if( input.width() != weight.width() || input.height() != weight.height() )
            throw CVTException( "Source and weight-image must have the same size" );

        output.assign( input, IALLOCATOR_CL );

        Image cltmp( input.width(), input.height(), input.format(), IALLOCATOR_CL );
        Image clp1( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clp2( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image* climg[ 2 ] = { &cltmp, &output };
        Image* clp[ 2 ] = { &clp1, &clp2 };

        cl_float4 fillvalue = {{ 0.0f, 0.0f, 0.0f, 0.0f }};
        cl_float4 lambda4 = {{ lambda, lambda, lambda, lambda }};

        fill( *clp[ 1 ], fillvalue );

        size_t swap = 0;
        for( size_t i = 0; i < iter; i++ ) {
            swap = i & 1;

            _clwhuber.setArg( 0, *climg[ swap ] );
            _clwhuber.setArg( 1, *clp[ swap ] );
            _clwhuber.setArg( 2, *climg[ 1 - swap ] );
            _clwhuber.setArg( 3, *clp[ 1 - swap ] );
            _clwhuber.setArg( 4, input );
            _clwhuber.setArg( 5, weight );
            _clwhuber.setArg( 6, lambda4 );
            _clwhuber.setArg( 7, hubereps );
            _clwhuber.setArg( 8, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clwhuber.setArg( 9, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clwhuber.setArg( 10, CLLocalSpace( sizeof( cl_float ) * 18 * 18 ) );
            _clwhuber.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }

        if( !swap ) {
            _clwhuber.setArg( 0, *climg[ 1 ] );
            _clwhuber.setArg( 1, *clp[ 1 ] );
            _clwhuber.setArg( 2, *climg[ 0 ] );
            _clwhuber.setArg( 3, *clp[ 0 ] );
            _clwhuber.setArg( 4, input );
            _clwhuber.setArg( 5, weight );
            _clwhuber.setArg( 6, lambda4 );
            _clwhuber.setArg( 7, hubereps );
            _clwhuber.setArg( 8, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clwhuber.setArg( 9, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clwhuber.setArg( 10, CLLocalSpace( sizeof( cl_float ) * 18 * 18 ) );
            _clwhuber.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }
    }

    void PDOptCL::denoiseWeightedHuber_PDD( Image& output, const Image& input, const Image& weight, const Vector4f& lambda, float hubereps, size_t iter ) const
    {
        if( input.memType() != IALLOCATOR_CL )
            throw CVTException( "Source must use IALLOCATOR_CL!" );

        output.assign( input, IALLOCATOR_CL );

        Image cltmp( input.width(), input.height(), input.format(), IALLOCATOR_CL );
        Image clp1( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clp2( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clq1( input.width(), input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clq2( input.width(), input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image* climg[ 2 ] = { &cltmp, &output };
        Image* clp[ 2 ] = { &clp1, &clp2 };
        Image* clq[ 2 ] = { &clq1, &clq2 };

        cl_float4 fillvalue = {{ 0.0f, 0.0f, 0.0f, 0.0f }};

        fill( *clp[ 1 ], fillvalue );
        fill( *clq[ 1 ], fillvalue );

        size_t swap = 0;
        for( size_t i = 0; i < iter; i++ ) {
            swap = i & 1;

            _clwhuberpdd.setArg( 0, *climg[ swap ] );
            _clwhuberpdd.setArg( 1, *clp[ swap ] );
            _clwhuberpdd.setArg( 2, *clq[ swap ] );
            _clwhuberpdd.setArg( 3, *climg[ 1 - swap ] );
            _clwhuberpdd.setArg( 4, *clp[ 1 - swap ] );
            _clwhuberpdd.setArg( 5, *clq[ 1 - swap ] );
            _clwhuberpdd.setArg( 6, input );
            _clwhuberpdd.setArg( 7, weight );
            _clwhuberpdd.setArg( 8, lambda );
            _clwhuberpdd.setArg( 9, hubereps );
            _clwhuberpdd.setArg( 10, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clwhuberpdd.setArg( 11, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clwhuberpdd.setArg( 12, CLLocalSpace( sizeof( cl_float ) * 18 * 18 ) );
            _clwhuberpdd.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }

        if( !swap ) {
            _clwhuberpdd.setArg( 0, *climg[ 1 ] );
            _clwhuberpdd.setArg( 1, *clp[ 1 ] );
            _clwhuberpdd.setArg( 2, *clq[ 1 ] );
            _clwhuberpdd.setArg( 3, *climg[ 0 ] );
            _clwhuberpdd.setArg( 4, *clp[ 0 ] );
            _clwhuberpdd.setArg( 5, *clq[ 0 ] );
            _clwhuberpdd.setArg( 6, input );
            _clwhuberpdd.setArg( 7, weight );
            _clwhuberpdd.setArg( 8, lambda );
            _clwhuberpdd.setArg( 9, hubereps );
            _clwhuberpdd.setArg( 10, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clwhuberpdd.setArg( 11, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clwhuberpdd.setArg( 12, CLLocalSpace( sizeof( cl_float ) * 18 * 18 ) );
            _clwhuberpdd.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }
    }

    void PDOptCL::inpaintWeightedHuber( Image& output, const Image& input, const Image& weight, const Image& mask, float lambda, float hubereps, size_t iter ) const
    {
        if( input.memType() != IALLOCATOR_CL || weight.memType() != IALLOCATOR_CL || mask.memType() != IALLOCATOR_CL )
            throw CVTException( "input/weight/mask must use IALLOCATOR_CL!" );
        if( input.width() != weight.width() || input.height() != weight.height() ||
            input.width() != mask.width() || input.height() != mask.height() )
            throw CVTException( "Source and weight-image must have the same size" );


        Image cltmp( input, IALLOCATOR_CL );
        Image clp1( input.width()*2, input.height(), input.format(), IALLOCATOR_CL );
        Image clp2( input.width()*2, input.height(), input.format(), IALLOCATOR_CL );
        Image* climg[ 2 ] = { &cltmp, &output };
        Image* clp[ 2 ] = { &clp1, &clp2 };

        cl_float4 fillvalue = {{ 0.0f, 0.0f, 0.0f, 0.0f }};

        fill( *clp[ 1 ], fillvalue );

        size_t swap = 0;
        for( size_t i = 0; i < iter; i++ ) {
            swap = i & 1;

            _clinpaintwhuber.setArg( 0, *climg[ swap ] );
            _clinpaintwhuber.setArg( 1, *clp[ swap ] );
            _clinpaintwhuber.setArg( 2, *climg[ 1 - swap ] );
            _clinpaintwhuber.setArg( 3, *clp[ 1 - swap ] );
            _clinpaintwhuber.setArg( 4, input );
            _clinpaintwhuber.setArg( 5, weight );
            _clinpaintwhuber.setArg( 6, mask );
            _clinpaintwhuber.setArg( 7, lambda );
            _clinpaintwhuber.setArg( 8, hubereps );
            _clinpaintwhuber.setArg( 9, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clinpaintwhuber.setArg( 10, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clinpaintwhuber.setArg( 11, CLLocalSpace( sizeof( cl_float ) * 18 * 18 ) );
            _clinpaintwhuber.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }

        if( !swap ) {
            _clinpaintwhuber.setArg( 0, *climg[ 1 ] );
            _clinpaintwhuber.setArg( 1, *clp[ 1 ] );
            _clinpaintwhuber.setArg( 2, *climg[ 0 ] );
            _clinpaintwhuber.setArg( 3, *clp[ 0 ] );
            _clinpaintwhuber.setArg( 4, input );
            _clinpaintwhuber.setArg( 5, weight );
            _clinpaintwhuber.setArg( 6, mask );
            _clinpaintwhuber.setArg( 7, lambda );
            _clinpaintwhuber.setArg( 8, hubereps );
            _clinpaintwhuber.setArg( 9, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clinpaintwhuber.setArg( 10, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clinpaintwhuber.setArg( 11, CLLocalSpace( sizeof( cl_float ) * 18 * 18 ) );
            _clinpaintwhuber.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }
    }


    void PDOptCL::denoiseDiffusionTensorROF_PDD( Image& output, const Image& input, const Image& DT, const Vector4f& lambda, size_t iter ) const
    {
        if( input.memType() != IALLOCATOR_CL )
            throw CVTException( "Source must use IALLOCATOR_CL!" );

        output.reallocate( input, IALLOCATOR_CL );
        output = input;

        Image cltmp( input.width(), input.height(), input.format(), IALLOCATOR_CL );
        Image clp1( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clp2( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clq1( input.width(), input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clq2( input.width(), input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image* climg[ 2 ] = { &cltmp, &output };
        Image* clp[ 2 ] = { &clp1, &clp2 };
        Image* clq[ 2 ] = { &clq1, &clq2 };

        cl_float4 fillvalue = {{ 0.0f, 0.0f, 0.0f, 0.0f }};

        fill( *clp[ 1 ], fillvalue );
        fill( *clq[ 1 ], fillvalue );

        size_t swap = 0;
        for( size_t i = 0; i < iter; i++ ) {
            swap = i & 1;

            _cldtrofpdd.setArg( 0, *climg[ swap ] );
            _cldtrofpdd.setArg( 1, *clp[ swap ] );
            _cldtrofpdd.setArg( 2, *clq[ swap ] );
            _cldtrofpdd.setArg( 3, *climg[ 1 - swap ] );
            _cldtrofpdd.setArg( 4, *clp[ 1 - swap ] );
            _cldtrofpdd.setArg( 5, *clq[ 1 - swap ] );
            _cldtrofpdd.setArg( 6, input );
            _cldtrofpdd.setArg( 7, DT );
            _cldtrofpdd.setArg( 8, lambda );
            _cldtrofpdd.setArg( 9, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _cldtrofpdd.setArg( 10, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _cldtrofpdd.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }

        if( !swap ) {
            _cldtrofpdd.setArg( 0, *climg[ 1 ] );
            _cldtrofpdd.setArg( 1, *clp[ 1 ] );
            _cldtrofpdd.setArg( 2, *clq[ 1 ] );
            _cldtrofpdd.setArg( 3, *climg[ 0 ] );
            _cldtrofpdd.setArg( 4, *clp[ 0 ] );
            _cldtrofpdd.setArg( 5, *clq[ 0 ] );
            _cldtrofpdd.setArg( 6, input );
            _cldtrofpdd.setArg( 7, DT );
            _cldtrofpdd.setArg( 8, lambda );
            _cldtrofpdd.setArg( 9, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _cldtrofpdd.setArg( 10, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _cldtrofpdd.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }
    }

    void PDOptCL::denoiseDiffusionTensorHuber_PDD( Image& output, const Image& input, const Image& DT, const Vector4f& lambda, float hubereps, size_t iter ) const
    {
        if( input.memType() != IALLOCATOR_CL )
            throw CVTException( "Source must use IALLOCATOR_CL!" );

        output.reallocate( input, IALLOCATOR_CL );
        output = input;

        Image cltmp( input.width(), input.height(), input.format(), IALLOCATOR_CL );
        Image clp1( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clp2( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clq1( input.width(), input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clq2( input.width(), input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image* climg[ 2 ] = { &cltmp, &output };
        Image* clp[ 2 ] = { &clp1, &clp2 };
        Image* clq[ 2 ] = { &clq1, &clq2 };

        cl_float4 fillvalue = {{ 0.0f, 0.0f, 0.0f, 0.0f }};

        fill( *clp[ 1 ], fillvalue );
        fill( *clq[ 1 ], fillvalue );

        size_t swap = 0;
        for( size_t i = 0; i < iter; i++ ) {
            swap = i & 1;

            _cldthuberpdd.setArg( 0, *climg[ swap ] );
            _cldthuberpdd.setArg( 1, *clp[ swap ] );
            _cldthuberpdd.setArg( 2, *clq[ swap ] );
            _cldthuberpdd.setArg( 3, *climg[ 1 - swap ] );
            _cldthuberpdd.setArg( 4, *clp[ 1 - swap ] );
            _cldthuberpdd.setArg( 5, *clq[ 1 - swap ] );
            _cldthuberpdd.setArg( 6, input );
            _cldthuberpdd.setArg( 7, DT );
            _cldthuberpdd.setArg( 8, lambda );
            _cldthuberpdd.setArg( 9, hubereps );
            _cldthuberpdd.setArg( 10, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _cldthuberpdd.setArg( 11, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _cldthuberpdd.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }

        if( !swap ) {
            _cldthuberpdd.setArg( 0, *climg[ 1 ] );
            _cldthuberpdd.setArg( 1, *clp[ 1 ] );
            _cldthuberpdd.setArg( 2, *clq[ 1 ] );
            _cldthuberpdd.setArg( 3, *climg[ 0 ] );
            _cldthuberpdd.setArg( 4, *clp[ 0 ] );
            _cldthuberpdd.setArg( 5, *clq[ 0 ] );
            _cldthuberpdd.setArg( 6, input );
            _cldthuberpdd.setArg( 7, DT );
            _cldthuberpdd.setArg( 8, lambda );
            _cldthuberpdd.setArg( 9, hubereps );
            _cldthuberpdd.setArg( 10, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _cldthuberpdd.setArg( 11, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _cldthuberpdd.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }
    }

    void PDOptCL::denoiseNDDiffusionTensorROF_PDD( Image& output, const Image& input, const Matrix3f& mat, const Image& DT, const Vector4f& lambda, size_t iter ) const
    {
        if( input.memType() != IALLOCATOR_CL )
            throw CVTException( "Source must use IALLOCATOR_CL!" );

        output.reallocate( input, IALLOCATOR_CL );
        output = input;

        Image cltmp( input.width(), input.height(), input.format(), IALLOCATOR_CL );
        Image clp1( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clp2( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clq1( input.width(), input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clq2( input.width(), input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image* climg[ 2 ] = { &cltmp, &output };
        Image* clp[ 2 ] = { &clp1, &clp2 };
        Image* clq[ 2 ] = { &clq1, &clq2 };

        cl_float4 fillvalue = {{ 0.0f, 0.0f, 0.0f, 0.0f }};

        fill( *clp[ 1 ], fillvalue );
        fill( *clq[ 1 ], fillvalue );

        CLMatrix3f clmat3;
        clmat3.set( mat );

        size_t swap = 0;
        for( size_t i = 0; i < iter; i++ ) {
            swap = i & 1;

            _clnddtrofpdd.setArg( 0, *climg[ swap ] );
            _clnddtrofpdd.setArg( 1, *clp[ swap ] );
            _clnddtrofpdd.setArg( 2, *clq[ swap ] );
            _clnddtrofpdd.setArg( 3, *climg[ 1 - swap ] );
            _clnddtrofpdd.setArg( 4, *clp[ 1 - swap ] );
            _clnddtrofpdd.setArg( 5, *clq[ 1 - swap ] );
            _clnddtrofpdd.setArg( 6, input );
            _clnddtrofpdd.setArg( 7, DT );
            _clnddtrofpdd.setArg( 8, sizeof( CLMatrix3f ), &clmat3 );
            _clnddtrofpdd.setArg( 9, lambda );
            _clnddtrofpdd.setArg( 10, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clnddtrofpdd.setArg( 11, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clnddtrofpdd.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }

        if( !swap ) {
            _clnddtrofpdd.setArg( 0, *climg[ 1 ] );
            _clnddtrofpdd.setArg( 1, *clp[ 1 ] );
            _clnddtrofpdd.setArg( 2, *clq[ 1 ] );
            _clnddtrofpdd.setArg( 3, *climg[ 0 ] );
            _clnddtrofpdd.setArg( 4, *clp[ 0 ] );
            _clnddtrofpdd.setArg( 5, *clq[ 0 ] );
            _clnddtrofpdd.setArg( 6, input );
            _clnddtrofpdd.setArg( 7, DT );
            _clnddtrofpdd.setArg( 8, sizeof( CLMatrix3f ), &clmat3 );
            _clnddtrofpdd.setArg( 9, lambda );
            _clnddtrofpdd.setArg( 10, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clnddtrofpdd.setArg( 11, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clnddtrofpdd.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }
    }

    void PDOptCL::denoisePOSDiffusionTensorROF_PDD( Image& output, const Image& input, const Image& DT, const Vector4f& lambda, size_t iter ) const
    {
        if( input.memType() != IALLOCATOR_CL )
            throw CVTException( "Source must use IALLOCATOR_CL!" );

        output.reallocate( input, IALLOCATOR_CL );
        output = input;

        Image cltmp( input.width(), input.height(), input.format(), IALLOCATOR_CL );
        Image clp1( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clp2( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clq1( input.width(), input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clq2( input.width(), input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image* climg[ 2 ] = { &cltmp, &output };
        Image* clp[ 2 ] = { &clp1, &clp2 };
        Image* clq[ 2 ] = { &clq1, &clq2 };

        cl_float4 fillvalue = {{ 0.0f, 0.0f, 0.0f, 0.0f }};

        fill( *clp[ 1 ], fillvalue );
        fill( *clq[ 1 ], fillvalue );

        size_t swap = 0;
        for( size_t i = 0; i < iter; i++ ) {
            swap = i & 1;

            _clposdtrofpdd.setArg( 0, *climg[ swap ] );
            _clposdtrofpdd.setArg( 1, *clp[ swap ] );
            _clposdtrofpdd.setArg( 2, *clq[ swap ] );
            _clposdtrofpdd.setArg( 3, *climg[ 1 - swap ] );
            _clposdtrofpdd.setArg( 4, *clp[ 1 - swap ] );
            _clposdtrofpdd.setArg( 5, *clq[ 1 - swap ] );
            _clposdtrofpdd.setArg( 6, input );
            _clposdtrofpdd.setArg( 7, DT );
            _clposdtrofpdd.setArg( 8, lambda );
            _clposdtrofpdd.setArg( 9, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clposdtrofpdd.setArg( 10, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clposdtrofpdd.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }

        if( !swap ) {
            _clposdtrofpdd.setArg( 0, *climg[ 1 ] );
            _clposdtrofpdd.setArg( 1, *clp[ 1 ] );
            _clposdtrofpdd.setArg( 2, *clq[ 1 ] );
            _clposdtrofpdd.setArg( 3, *climg[ 0 ] );
            _clposdtrofpdd.setArg( 4, *clp[ 0 ] );
            _clposdtrofpdd.setArg( 5, *clq[ 0 ] );
            _clposdtrofpdd.setArg( 6, input );
            _clposdtrofpdd.setArg( 7, DT );
            _clposdtrofpdd.setArg( 8, lambda );
            _clposdtrofpdd.setArg( 9, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clposdtrofpdd.setArg( 10, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clposdtrofpdd.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }
    }

    void PDOptCL::inpaintDiffusionTensorHuber_PDD( Image& output, const Image& input, const Image& DT, const Image& mask, const Vector4f& lambda, float hubereps, size_t iter ) const
    {
        if( input.memType() != IALLOCATOR_CL )
            throw CVTException( "Source must use IALLOCATOR_CL!" );

        output.reallocate( input, IALLOCATOR_CL );
        output = input;

        Image cltmp( input.width(), input.height(), input.format(), IALLOCATOR_CL );
        Image clp1( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clp2( input.width()*2, input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clq1( input.width(), input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image clq2( input.width(), input.height(), IFormat::floatEquivalent( input.format() ), IALLOCATOR_CL );
        Image* climg[ 2 ] = { &cltmp, &output };
        Image* clp[ 2 ] = { &clp1, &clp2 };
        Image* clq[ 2 ] = { &clq1, &clq2 };

        cl_float4 fillvalue = {{ 0.0f, 0.0f, 0.0f, 0.0f }};

        fill( *clp[ 1 ], fillvalue );
        fill( *clq[ 1 ], fillvalue );

        size_t swap = 0;
        for( size_t i = 0; i < iter; i++ ) {
            swap = i & 1;

            _clinpaintdthuberpdd.setArg( 0, *climg[ swap ] );
            _clinpaintdthuberpdd.setArg( 1, *clp[ swap ] );
            _clinpaintdthuberpdd.setArg( 2, *clq[ swap ] );
            _clinpaintdthuberpdd.setArg( 3, *climg[ 1 - swap ] );
            _clinpaintdthuberpdd.setArg( 4, *clp[ 1 - swap ] );
            _clinpaintdthuberpdd.setArg( 5, *clq[ 1 - swap ] );
            _clinpaintdthuberpdd.setArg( 6, input );
            _clinpaintdthuberpdd.setArg( 7, DT );
            _clinpaintdthuberpdd.setArg( 8, mask );
            _clinpaintdthuberpdd.setArg( 9, lambda );
            _clinpaintdthuberpdd.setArg( 10, hubereps );
            _clinpaintdthuberpdd.setArg( 11, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clinpaintdthuberpdd.setArg( 12, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clinpaintdthuberpdd.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }

        if( !swap ) {
            _clinpaintdthuberpdd.setArg( 0, *climg[ 1 ] );
            _clinpaintdthuberpdd.setArg( 1, *clp[ 1 ] );
            _clinpaintdthuberpdd.setArg( 2, *clq[ 1 ] );
            _clinpaintdthuberpdd.setArg( 3, *climg[ 0 ] );
            _clinpaintdthuberpdd.setArg( 4, *clp[ 0 ] );
            _clinpaintdthuberpdd.setArg( 5, *clq[ 0 ] );
            _clinpaintdthuberpdd.setArg( 6, input );
            _clinpaintdthuberpdd.setArg( 7, DT );
            _clinpaintdthuberpdd.setArg( 8, mask );
            _clinpaintdthuberpdd.setArg( 9, lambda );
            _clinpaintdthuberpdd.setArg( 10, hubereps );
            _clinpaintdthuberpdd.setArg( 11, CLLocalSpace( sizeof( cl_float4 ) * 18 * 18 ) );
            _clinpaintdthuberpdd.setArg( 12, CLLocalSpace( sizeof( cl_float8 ) * 18 * 18 ) );
            _clinpaintdthuberpdd.run( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
        }
    }

    void PDOptCL::fill( Image& dst, const cl_float4& value ) const
    {
        _clfill.setArg( 0, dst );
        _clfill.setArg( 1, value );
        _clfill.run( CLNDRange( Math::pad( dst.width(), 16 ), Math::pad( dst.height(), 16 ) ), CLNDRange( 16, 16 ) );
    }

}
