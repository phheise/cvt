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

#include <cvt/vision/PMHuberStereo.h>
#include <cvt/util/Exception.h>
#include <cvt/cl/CLBuffer.h>
#include <cvt/cl/kernel/pmhstereo.h>
#include <cvt/cl/kernel/pyr/pyrupmul.h>
#include <cvt/cl/CLPyramid.h>

namespace cvt {

#define KX 16
#define KY 16
#define VIEWSAMPLES 2
#define PMDEBUG 0

    struct PMHVIEWPROP {
        cl_int n;
        cl_float4 samples[ VIEWSAMPLES ];
    };


    PMHuberStereo::PMHuberStereo() :
        _clpmh_init( _pmhstereo_source, "pmhstereo_init" ),
        _clpmh_init_disparity( _pmhstereo_source, "pmhstereo_init_disparity" ),
        _clpmh_init_disparity_normal( _pmhstereo_source, "pmhstereo_init_disparity_normal" ),
        _clpmh_propagate( _pmhstereo_source, "pmhstereo_propagate_view" ),
        _clpmh_depthmap( _pmhstereo_source, "pmhstereo_depthmap" ),
        _clpmh_viewbufclear( _pmhstereo_source, "pmhstereo_viewbuf_clear" ),
        _clpmh_fill( _pmhstereo_source, "pmhstereo_fill_state" ),
        _clpmh_consistency( _pmhstereo_source, "pmhstereo_consistency" ),
        _clpmh_filldepthmap( _pmhstereo_source, "pmhstereo_fill_depthmap" ),
        _clpmh_todisparity( _pmhstereo_source, "pmhstereo_to_disparity" ),
        _clpmh_fillnormalmap( _pmhstereo_source, "pmhstereo_fill_normalmap" ),
        _clpmh_normaldepth( _pmhstereo_source, "pmhstereo_normal_depth" ),
        _clpmh_clear( _pmhstereo_source, "pmhstereo_clear" ),
        _clpmh_occmap( _pmhstereo_source, "pmhstereo_occmap" ),
        _clpmh_gradxy( _pmhstereo_source, "pmhstereo_gradxy" ),
        _clpmh_weight( _pmhstereo_source, "pmhstereo_weight" ),
        _clpmh_bilateralweight( _pmhstereo_source, "pmhstereo_bilateral_weight_to_alpha" ),
        _clpmh_visualize_depth_normal( _pmhstereo_source, "pmhstereo_visualize_depth_normal" ),
        _clpmh_disparitytonormal( _pmhstereo_source, "pmhstereo_disparity_to_normal" ),
        _clpyrupmul( _pyrupmul_source, "pyrup_mul4f" )
    {

    }

    PMHuberStereo::~PMHuberStereo()
    {
    }

    void PMHuberStereo::disparityMap( Image& dmap, const Image& left, const Image& right, size_t patchsize, float disparitymax, size_t iterations,
                               DisparityPostProcessing pp, Image* normalmap, const Image* initdisparity, const Image* initnormalmap )
    {
        const int levels = 1;
        const float maxdispdiff     = 1.0f;
        const float maxanglediff    = 5.0f;
        float curDisparityMax;

        CLPyramid pyrleft( 0.35f, levels );
        CLPyramid pyrright( 0.35f, levels );

        pyrleft.update( left );
        pyrright.update( right );

        Image matchleft, matchright;

        curDisparityMax = disparitymax * ( ( ( float ) pyrleft[ levels - 1 ].width() ) / ( ( float ) pyrleft[ 0 ].width() ) );
        disparityMapAll( matchleft, matchright, pyrleft[ levels - 1 ], pyrright[ levels - 1 ], patchsize, curDisparityMax, iterations, 1e-4f,
                         initdisparity, NULL, initnormalmap, NULL );

        for( int l = levels - 2; l >= 0; l-- ) {
            Image tmp, dleft, dright, nleft, nright;

            convertDisparity( tmp, matchleft, 1.0f, true );
            dleft.reallocate( pyrleft[ l ].width(), pyrleft[ l ].height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );
            pyrUpMul( dleft, tmp, Vector4f( ( ( float ) pyrleft[ l ].width() ) / ( ( float ) pyrleft[ l + 1 ].width() ) ) );

            convertDisparity( tmp, matchright, 1.0f, false );
            dright.reallocate( pyrright[ l ].width(), pyrright[ l ].height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );
            pyrUpMul( dright, tmp, Vector4f( ( ( float ) pyrright[ l ].width() ) / ( ( float ) pyrright[ l + 1 ].width() ) ) );

            convertFillNormal( tmp, matchleft, true);
            nleft.reallocate( pyrleft[ l ].width(), pyrleft[ l ].height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
            pyrUpMul( nleft, tmp, Vector4f( 1.0f ) );

            convertFillNormal( tmp, matchright, false );
            nright.reallocate( pyrright[ l ].width(), pyrright[ l ].height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
            pyrUpMul( nright, tmp, Vector4f( 1.0f ) );

            float mintheta = ( ( ( float ) ( levels - l ) ) / ( ( float ) levels ) );
            curDisparityMax = disparitymax * ( ( ( float ) pyrleft[ l ].width() ) / ( ( float ) pyrleft[ 0 ].width() ) );
            std::cout << mintheta << std::endl;
            disparityMapAll( matchleft, matchright, pyrleft[ l ], pyrright[ l ], patchsize, curDisparityMax, iterations, 1e-1f * mintheta + 1e-4f,
                             &dleft, &dright, &nleft, &nright );

        }


        if( pp == DISPARITY_PP_NONE ) {
            convertDisparity( dmap, matchleft, 1.0f, true );

            if( normalmap != NULL ) {
                normalmap->reallocate( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
                convertFillNormal( *normalmap,  matchleft, true );
            }
        } else if( pp == DISPARITY_PP_LRCHECK || pp == DISPARITY_PP_LRCHECK_FILL ) {
            Image tmp;
            checkConsistencyLR( tmp, matchleft, matchright, maxdispdiff, maxanglediff, true );
            if( pp == DISPARITY_PP_LRCHECK )
                convertDisparity( dmap, tmp, 1.0f, true );
            else
                convertFillDisparity( dmap, tmp, 1.0f );

            if( normalmap != NULL ) {
                normalmap->reallocate( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
                convertFillNormal( *normalmap, tmp, true );
            }
        }
    }



    void PMHuberStereo::depthMap( Image& dmap, const Image& left, const Image& right, size_t patchsize, float disparitymax, size_t iterations, float dscale, DisparityPostProcessing pp,
                                  Image* normalmap, const Image* initdisparity, const Image* initnormalmap )
    {
        if( left.width() != right.width() || left.height() != right.height() ||
            left.memType() != IALLOCATOR_CL || right.memType() != IALLOCATOR_CL )
            throw CVTException( "Left/Right stereo images inconsistent or incompatible memory type" );

        const float alpha = 12.0f;
        const float beta  = 1.0f;
        const float HUBEREPS        = 0.0001f;
        const float maxdispdiff     = 1.0f;
        const float maxanglediff    = 5.0f;
        const float THETASCALE_D    = 15.0f;
        const float THETASCALE_NORM = 15.0f;
        const float THETABIAS_D     = 2.0f;
        const float THETABIAS_NORM  = 2.0f;
        const float THETASCALE      = 10.0f;
        const float thetamul        = 1.2f;
        const size_t PDOPTITER      = 40;
        float theta = 1e-3f;//1.0f / ( Math::pow( thetamul, ( float ) ( iterations - 1 ) ) );
//      float theta = 1.0f / ( Math::pow( thetamul, ( float ) ( iterations - 1 ) ) );

        if( dscale <= 0.0f )
            dscale = 1.0f / disparitymax;

        CLBuffer viewbuf1( sizeof( PMHVIEWPROP ) * left.width() * left.height() );
        CLBuffer viewbuf2( sizeof( PMHVIEWPROP ) * right.width() * right.height() );

        Image leftgrad( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image rightgrad( right.width(), right.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image leftDT( left.width(), left.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );
        Image rightDT( right.width(), right.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );
        Image leftsmooth( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image rightsmooth( right.width(), right.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image clsmoothtmp( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image clsmoothtmp2( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image cloccimg( left.width(), left.height(), IFormat::GRAY_UINT8, IALLOCATOR_CL );

        /* calculate gradient image */
        gradient( leftgrad, left );
        gradient( rightgrad, right );

        /* calculate huber weights */
        _pdopt.diffusionTensor( leftDT, left, alpha, beta );
        _pdopt.diffusionTensor( rightDT, right, alpha, beta );

        /* clear the smoothed images */
        clear( leftsmooth );
        clear( rightsmooth );

        Image clmatches1_1( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image clmatches1_2( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image* clmatches1[ 2 ] = { &clmatches1_1, &clmatches1_2 };

        Image clmatches2_1( right.width(), right.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image clmatches2_2( right.width(), right.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image* clmatches2[ 2 ] = { &clmatches2_1, &clmatches2_2 };

        /* PMH init */
        init( *clmatches1[ 0 ], left, right, leftgrad, rightgrad, patchsize, disparitymax, true, initdisparity, initnormalmap );
        init( *clmatches2[ 0 ], right, left, rightgrad, leftgrad, patchsize, disparitymax, false, NULL, NULL );
//        init( *clmatches2[ 0 ], right, left, rightgrad, leftgrad, patchsize, disparitymax, false, initdisparity, initnormalmap );

        /* Clear view propagation buffer from the right view - there is nothing to propagate atm*/
        clearViewBuffer( viewbuf2, right.width(), right.height() );

        int swap = 0;
        for( size_t iter = 0; iter < iterations; iter++ ) {

            std::cout << iter << " / " << iterations << std::endl;

#ifdef PMDEBUG
            std::cout << "Theta: " << theta << std::endl;
            _clpmh_depthmap.setArg( 0, clsmoothtmp );
            _clpmh_depthmap.setArg( 1, *clmatches1[ swap ] );
            _clpmh_depthmap.setArg( 2, 1.0f / disparitymax );
            _clpmh_depthmap.runWait( CLNDRange( Math::pad( left.width(), KX ), Math::pad( left.height(), KY ) ), CLNDRange( KX, KY ) );
            clsmoothtmp.save("stereo1.png");
            std::cout << "Wrote stereo1.png" << std::endl;

#if 1
            _clpmh_depthmap.setArg( 0, clsmoothtmp );
            _clpmh_depthmap.setArg( 1, *clmatches2[ swap ]  );
            _clpmh_depthmap.setArg( 2, 1.0f / disparitymax );
            _clpmh_depthmap.runWait( CLNDRange( Math::pad( right.width(), KX ), Math::pad( right.height(), KY ) ), CLNDRange( KX, KY ) );
            clsmoothtmp.save("stereo2.png");
            std::cout << "Wrote stereo2.png" << std::endl;
#endif

/*          clconsistency.setArg( 0, cloutput1 );
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

            //Vector4f LAMBDA( theta * THETASCALE_NORM + THETABIAS_NORM, theta * THETASCALE_NORM + THETABIAS_NORM, theta * THETASCALE_D + THETABIAS_D, 1.0f );
            Vector4f LAMBDA( theta * THETASCALE_D + THETABIAS_D );

            clearViewBuffer( viewbuf1, left.width(), left.height() );

            propagate( *clmatches1[ 1 - swap ], *clmatches1[ swap ], left, right, leftgrad, rightgrad, leftsmooth, iter < 0 ? 0 : theta * THETASCALE,
                       patchsize, disparitymax, true, iter, viewbuf2, viewbuf1 );

            clearViewBuffer( viewbuf2, right.width(), right.height() );

            propagate( *clmatches2[ 1 - swap ], *clmatches2[ swap ], right, left, rightgrad, leftgrad, rightsmooth, iter < 0 ? 0 : theta * THETASCALE,
                       patchsize, disparitymax, false, iter, viewbuf1, viewbuf2 );

            occlusionMap( cloccimg, *clmatches1[ 1 - swap ], *clmatches2[ 1 - swap ], maxdispdiff, maxanglediff, true );
#ifdef PMDEBUG
            cloccimg.save("stereo1occ.png");
#endif

//            checkConsistencyLR( clsmoothtmp, *clmatches1[ 1 - swap ], *clmatches2[ 1 - swap ], maxdispdiff, maxanglediff, true );
            convertFillNormalDisparity( clsmoothtmp2, *clmatches1[ 1 - swap ], left, 1.0f / disparitymax, true );
            _pdopt.denoiseDiffusionTensorHuber_PDD( leftsmooth, clsmoothtmp2, leftDT, LAMBDA, HUBEREPS, PDOPTITER );
            //_pdopt.denoiseDiffusionTensorROF_PDD( leftsmooth, clsmoothtmp2, leftDT, LAMBDA, PDOPTITER );
            //_pdopt.denoisePOSDiffusionTensorROF_PDD( leftsmooth, clsmoothtmp2, leftDT, LAMBDA, PDOPTITER );
            //normalDepth( clsmoothtmp2, *clmatches1[ 1 - swap ], disparitymax, true );
            //_pdopt.inpaintDiffusionTensorHuber_PDD( leftsmooth, clsmoothtmp2, leftDT, cloccimg, LAMBDA, HUBEREPS, PDOPTITER );
            //_pdopt.denoiseHL_cwise( leftsmooth, clsmoothtmp2, 1.0f / LAMBDA.x, 0.56f, PDOPTITER );
            //_pdopt.denoiseLOG1( leftsmooth, clsmoothtmp2, 1.0f / LAMBDA.x, 1.0f, PDOPTITER );
            //_pdopt.denoiseHuber_PDD( leftsmooth, clsmoothtmp2, LAMBDA.x, HUBEREPS, PDOPTITER );
            //_pdopt.mumfordShahConst( leftsmooth, *clmatches1[ 1 - swap ], 1.0f / LAMBDA.x, PDOPTITER );

#ifdef PMDEBUG
            visualize( "pmh1", clsmoothtmp2, 1.0f );
            visualize( "pmh1_smooth", leftsmooth, 1.0f );
#endif

            occlusionMap( cloccimg, *clmatches2[ 1 - swap ], *clmatches1[ 1 - swap ], maxdispdiff, maxanglediff, true );
#ifdef PMDEBUG
            cloccimg.save("stereo2occ.png");
#endif

//            checkConsistencyLR( clsmoothtmp, *clmatches2[ 1 - swap ], *clmatches1[ 1 - swap ], maxdispdiff, maxanglediff, false );
            convertFillNormalDisparity( clsmoothtmp2, *clmatches2[ 1 - swap ], right, 1.0f / disparitymax, false );
            _pdopt.denoiseDiffusionTensorHuber_PDD( rightsmooth, clsmoothtmp2, rightDT, LAMBDA, HUBEREPS, PDOPTITER );
            //_pdopt.denoiseDiffusionTensorROF_PDD( rightsmooth, clsmoothtmp2, rightDT, LAMBDA, PDOPTITER );
            //_pdopt.denoisePOSDiffusionTensorROF_PDD( rightsmooth, clsmoothtmp2, rightDT, LAMBDA, PDOPTITER );
            //normalDepth( clsmoothtmp2, *clmatches2[ 1 - swap ], disparitymax, false );
            //_pdopt.inpaintDiffusionTensorHuber_PDD( rightsmooth, clsmoothtmp2, rightDT, cloccimg, LAMBDA, HUBEREPS, PDOPTITER );
            //_pdopt.denoiseDiffusionTensorHuber_PDD( rightsmooth, clsmoothtmp2, rightDT, LAMBDA, HUBEREPS, PDOPTITER );
            //_pdopt.denoiseHL_cwise( rightsmooth, clsmoothtmp2, 1.0f / LAMBDA.x, 0.56f, PDOPTITER );
            //_pdopt.denoiseLOG1( rightsmooth, clsmoothtmp2, 1.0f / LAMBDA.x, 1.0f, PDOPTITER );
            //_pdopt.denoiseHuber_PDD( rightsmooth, clsmoothtmp2, LAMBDA.x, HUBEREPS, PDOPTITER );
            //_pdopt.mumfordShahConst( rightsmooth, *clmatches2[ 1 - swap ], 1.0f / LAMBDA.x, PDOPTITER );

#ifdef PMDEBUG
            visualize( "pmh2", clsmoothtmp2, 1.0f );
            visualize( "pmh2_smooth", rightsmooth, 1.0f );
#endif

            //theta *= thetamul;

            if( iter >= 3 ) {
                theta = ( ( iter - 3.0f ) / ( ( float ) iterations - 4.0f ) );
                theta = theta * theta * theta + 1e-3f;
            }

            //if( iter >= 5 )
            //    theta = Math::smoothstep<float>( ( ( iter - 5.0f ) / ( ( float ) iterations - 5.0f ) )  ) * 1.0f;

            swap = 1 - swap;
        }

        dmap.reallocate( left.width(), left.height(), ( dmap.channels() != 1 ) ? IFormat::GRAY_FLOAT : dmap.format(), IALLOCATOR_CL );

        if( pp == DISPARITY_PP_NONE ) {
            convertDisparity( dmap, *clmatches1[ swap ], dscale, true );

            if( normalmap != NULL ) {
                normalmap->reallocate( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
                convertFillNormal( *normalmap,  *clmatches1[ swap ], true );
            }
        } else if( pp == DISPARITY_PP_LRCHECK || pp == DISPARITY_PP_LRCHECK_FILL ) {
            checkConsistencyLR( clsmoothtmp, *clmatches1[ swap ], *clmatches2[ swap ], maxdispdiff, maxanglediff, true );
            if( pp == DISPARITY_PP_LRCHECK )
                convertDisparity( dmap, clsmoothtmp, dscale, true );
            else
                convertFillDisparity( dmap, clsmoothtmp, dscale );

            if( normalmap != NULL ) {
                normalmap->reallocate( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
                convertFillNormal( *normalmap, clsmoothtmp, true );
            }
        }

    }

    void PMHuberStereo::init( Image& output, const Image& first, const Image& second, const Image& firstgrad, const Image& secondgrad, int patchsize, float disparitymax, bool LR, const Image* initdisparity, const Image* initnormalmap )
    {
        if( !initdisparity ) {
            _clpmh_init.setArg( 0, output );
            _clpmh_init.setArg( 1, first );
            _clpmh_init.setArg( 2, second );
            _clpmh_init.setArg( 3, firstgrad );
            _clpmh_init.setArg( 4, secondgrad );
            _clpmh_init.setArg( 5, ( int ) patchsize );
            _clpmh_init.setArg( 6, disparitymax );
            _clpmh_init.setArg<int>( 7, ( int ) LR );
            _clpmh_init.run( CLNDRange( Math::pad( first.width(), KX ), Math::pad( first.height(), KY ) ), CLNDRange( KX, KY ) );
        } else {
            Image normal;

            if( !initnormalmap )
                disparityToNormal( normal, *initdisparity );

            _clpmh_init_disparity_normal.setArg( 0, output );
            _clpmh_init_disparity_normal.setArg( 1, first );
            _clpmh_init_disparity_normal.setArg( 2, second );
            _clpmh_init_disparity_normal.setArg( 3, firstgrad );
            _clpmh_init_disparity_normal.setArg( 4, secondgrad );
            _clpmh_init_disparity_normal.setArg( 5, ( int ) patchsize );
            _clpmh_init_disparity_normal.setArg( 6, disparitymax );
            _clpmh_init_disparity_normal.setArg<int>( 7, ( int ) LR );
            _clpmh_init_disparity_normal.setArg( 8, *initdisparity );
            _clpmh_init_disparity_normal.setArg( 9, initnormalmap?*initnormalmap:normal );
            _clpmh_init_disparity_normal.run( CLNDRange( Math::pad( first.width(), KX ), Math::pad( first.height(), KY ) ), CLNDRange( KX, KY ) );
        }
    }

    void PMHuberStereo::propagate( Image& output, const Image& old, const Image& first, const Image& second, const Image& firstgrad, const Image& secondgrad, const Image& smooth,
                                  float theta, int patchsize, float disparitymax, bool LR, int iteration, CLBuffer& viewbufin, CLBuffer& viewbufout  )
    {

            //Image klt( output.width(), output.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );

            _clpmh_propagate.setArg( 0, output );
            _clpmh_propagate.setArg( 1, old );
            _clpmh_propagate.setArg( 2, first );
            _clpmh_propagate.setArg( 3, second );
            _clpmh_propagate.setArg( 4, firstgrad );
            _clpmh_propagate.setArg( 5, secondgrad );
            _clpmh_propagate.setArg( 6, smooth );
            _clpmh_propagate.setArg( 7, theta );
            _clpmh_propagate.setArg( 8, ( int ) patchsize );
            _clpmh_propagate.setArg( 9, disparitymax );
            _clpmh_propagate.setArg<int>( 10, ( int ) LR );
            _clpmh_propagate.setArg( 11, iteration );
            _clpmh_propagate.setArg( 12, viewbufin );
            _clpmh_propagate.setArg( 13, viewbufout );
            //_clpmh_propagate.setArg( 14, klt );
            _clpmh_propagate.runWait( CLNDRange( Math::pad( first.width(), KX ), Math::pad( first.height(), KY ) ), CLNDRange( KX, KY ) );
            //klt.save("pmh_klt.png");
    }

    void PMHuberStereo::disparityMapAll( Image& matchesleft, Image& matchesright,
                                    const Image& left, const Image& right,
                                    int patchsize, float disparitymax, size_t iterations, float thetamin,
                                    const Image* initdisparityleft, const Image* initdisparityright,
                                    const Image* initnormalleft, const Image* initnormalright )
    {
        if( left.width() != right.width() || left.height() != right.height() ||
            left.memType() != IALLOCATOR_CL || right.memType() != IALLOCATOR_CL )
            throw CVTException( "Left/Right stereo images inconsistent or incompatible memory type" );

        const float alpha = 24.0f;
        const float beta  = 1.0f;
        const float HUBEREPS        = 0.0001f;
        const float maxdispdiff     = 1.0f;
        const float maxanglediff    = 8.0f;
        const float THETASCALE_D    = 10.0f;
        const float THETASCALE_NORM = 10.0f;
        const float THETABIAS_D     = 1.0f;
        const float THETABIAS_NORM  = 1.0f;
        const float THETASCALE      = 50.0f;
        const float thetamul        = 1.2f;
        const size_t PDOPTITER      = 40;
        float theta = thetamin;

        CLBuffer viewbuf1( sizeof( PMHVIEWPROP ) * left.width() * left.height() );
        CLBuffer viewbuf2( sizeof( PMHVIEWPROP ) * right.width() * right.height() );

        Image leftbw( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image rightbw( right.width(), right.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image leftgrad( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image rightgrad( right.width(), right.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image leftDT( left.width(), left.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );
        Image rightDT( right.width(), right.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );
        Image leftsmooth( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image rightsmooth( right.width(), right.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image clsmoothtmp( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image clsmoothtmp2( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image cloccimg( left.width(), left.height(), IFormat::GRAY_UINT8, IALLOCATOR_CL );

        bilateralWeightToAlpha( leftbw, left, patchsize );
        bilateralWeightToAlpha( rightbw, right, patchsize );

        /* calculate gradient image */
        gradient( leftgrad, left );
        gradient( rightgrad, right );

        /* calculate huber weights */
        _pdopt.diffusionTensor( leftDT, left, alpha, beta );
        _pdopt.diffusionTensor( rightDT, right, alpha, beta );

        /* clear the smoothed images */
        clear( leftsmooth );
        clear( rightsmooth );

        Image matcheslefttmp( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        matchesleft.reallocate( left.width(), left.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image* clmatches1[ 2 ] = { &matcheslefttmp, &matchesleft };

        Image matchesrighttmp( right.width(), right.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        matchesright.reallocate( right.width(), right.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        Image* clmatches2[ 2 ] = { &matchesrighttmp, &matchesright };

        /* PMH init */
        init( *clmatches1[ 0 ], left, right, leftgrad, rightgrad, patchsize, disparitymax, true, initdisparityleft, initnormalleft);
        init( *clmatches2[ 0 ], right, left, rightgrad, leftgrad, patchsize, disparitymax, false, initdisparityright, initnormalright );


        if( initdisparityleft ) {
            Vector4f LAMBDA( theta * THETASCALE_NORM + THETABIAS_NORM, theta * THETASCALE_NORM + THETABIAS_NORM, theta * THETASCALE_D + THETABIAS_D, 1.0f );
            normalDepth( clsmoothtmp2, *clmatches1[ 0 ], 1.0f / disparitymax, true );
            _pdopt.denoiseDiffusionTensorROF_PDD( leftsmooth, clsmoothtmp2, leftDT, LAMBDA, PDOPTITER );
        }

        if( initdisparityright ) {
            Vector4f LAMBDA( theta * THETASCALE_NORM + THETABIAS_NORM, theta * THETASCALE_NORM + THETABIAS_NORM, theta * THETASCALE_D + THETABIAS_D, 1.0f );
            normalDepth( clsmoothtmp2, *clmatches2[ 0 ], 1.0f / disparitymax, false );
            _pdopt.denoiseDiffusionTensorROF_PDD( rightsmooth, clsmoothtmp2, rightDT, LAMBDA, PDOPTITER );
        }


        /* Clear view propagation buffer from the right view - there is nothing to propagate atm*/
        clearViewBuffer( viewbuf2, right.width(), right.height() );

        int swap = 0;
        iterations |= 1; // make the number of iterations odd to fill the output buffers matchesleft, matchesright
        for( size_t iter = 0; iter < iterations; iter++ ) {

            std::cout << iter << " / " << iterations << std::endl;

#ifdef PMDEBUG
            std::cout << "Theta: " << theta << std::endl;
            _clpmh_depthmap.setArg( 0, clsmoothtmp );
            _clpmh_depthmap.setArg( 1, *clmatches1[ swap ] );
            _clpmh_depthmap.setArg( 2, 1.0f / disparitymax );
            _clpmh_depthmap.runWait( CLNDRange( Math::pad( left.width(), KX ), Math::pad( left.height(), KY ) ), CLNDRange( KX, KY ) );
            clsmoothtmp.save("stereo1.png");
            std::cout << "Wrote stereo1.png" << std::endl;

#if 1
            _clpmh_depthmap.setArg( 0, clsmoothtmp );
            _clpmh_depthmap.setArg( 1, *clmatches2[ swap ]  );
            _clpmh_depthmap.setArg( 2, 1.0f / disparitymax );
            _clpmh_depthmap.runWait( CLNDRange( Math::pad( right.width(), KX ), Math::pad( right.height(), KY ) ), CLNDRange( KX, KY ) );
            clsmoothtmp.save("stereo2.png");
            std::cout << "Wrote stereo2.png" << std::endl;
#endif

/*          clconsistency.setArg( 0, cloutput1 );
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


            Vector4f LAMBDA( theta * THETASCALE_NORM + THETABIAS_NORM, theta * THETASCALE_NORM + THETABIAS_NORM, theta * THETASCALE_D + THETABIAS_D, 1.0f );

            clearViewBuffer( viewbuf1, left.width(), left.height() );

            propagate( *clmatches1[ 1 - swap ], *clmatches1[ swap ], leftbw, rightbw, leftgrad, rightgrad, leftsmooth, theta * THETASCALE,
                       patchsize, disparitymax, true, iter, viewbuf2, viewbuf1 );


            clearViewBuffer( viewbuf2, right.width(), right.height() );

            propagate( *clmatches2[ 1 - swap ], *clmatches2[ swap ], rightbw, leftbw, rightgrad, leftgrad, rightsmooth, theta * THETASCALE,
                       patchsize, disparitymax, false, iter, viewbuf1, viewbuf2 );


#ifdef PMDEBUG
            occlusionMap( cloccimg, *clmatches1[ 1 - swap ], *clmatches2[ 1 - swap ], maxdispdiff, maxanglediff, true );
            cloccimg.save("stereo1occ.png");
#endif

            //convertFillNormalDisparity( clsmoothtmp2, *clmatches1[ 1 - swap ], left, 1.0f / disparitymax, true );
            normalDepth( clsmoothtmp2, *clmatches1[ 1 - swap ], 1.0f / disparitymax, true );
            _pdopt.denoiseDiffusionTensorROF_PDD( leftsmooth, clsmoothtmp2, leftDT, LAMBDA, PDOPTITER );

#ifdef PMDEBUG
            visualize( "pmh1", clsmoothtmp2, 1.0f );
            visualize( "pmh1_smooth", leftsmooth, 1.0f );
#endif

#ifdef PMDEBUG
            occlusionMap( cloccimg, *clmatches2[ 1 - swap ], *clmatches1[ 1 - swap ], maxdispdiff, maxanglediff, true );
            cloccimg.save("stereo2occ.png");
#endif

            //convertFillNormalDisparity( clsmoothtmp2, *clmatches2[ 1 - swap ], right, 1.0f / disparitymax, false );
            normalDepth( clsmoothtmp2, *clmatches2[ 1 - swap ], 1.0f / disparitymax, false );
            _pdopt.denoiseDiffusionTensorROF_PDD( rightsmooth, clsmoothtmp2, rightDT, LAMBDA, PDOPTITER );

#ifdef PMDEBUG
            visualize( "pmh2", clsmoothtmp2, 1.0f );
            visualize( "pmh2_smooth", rightsmooth, 1.0f );
#endif

            //if( iter >= 2 ) {
                theta = ( ( ( ( float ) iter ) - 0.0f ) / ( ( float ) iterations - 0.0f ) );
                theta = Math::pow( theta, 4.0f ) + thetamin;
            //}

            swap = 1 - swap;
        }
    }

    void PMHuberStereo::occlusionMap( Image& occ, const Image& first, const Image& second, float maxdispdiff, float maxanglediff, bool LR ) const
    {
        _clpmh_occmap.setArg( 0, occ );
        _clpmh_occmap.setArg( 1, first );
        _clpmh_occmap.setArg( 2, second );
        _clpmh_occmap.setArg( 3, maxdispdiff );
        _clpmh_occmap.setArg( 4, maxanglediff );
        _clpmh_occmap.setArg<int>( 5, ( int ) LR );
        _clpmh_occmap.runWait( CLNDRange( Math::pad( first.width(), KX ), Math::pad( first.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void PMHuberStereo::checkConsistencyLR( Image& output, const Image& first, const Image& second, float maxdispdiff, float maxanglediff, bool LR ) const
    {
        output.reallocate( first.width(), first.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        _clpmh_consistency.setArg( 0, output );
        _clpmh_consistency.setArg( 1, first );
        _clpmh_consistency.setArg( 2, second );
        _clpmh_consistency.setArg( 3, maxdispdiff );
        _clpmh_consistency.setArg( 4, maxanglediff );
        _clpmh_consistency.setArg<int>( 5, ( int ) LR );
        _clpmh_consistency.runWait( CLNDRange( Math::pad( first.width(), KX ), Math::pad( first.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void PMHuberStereo::convertFillNormalDisparity( Image& output, const Image& input, const Image& other, float dispscale, bool LR )
    {
        _clpmh_fill.setArg( 0, output );
        _clpmh_fill.setArg( 1, input );
        _clpmh_fill.setArg( 2, other );
        _clpmh_fill.setArg( 3, dispscale );
        _clpmh_fill.setArg<int>( 4, ( int ) LR );
        _clpmh_fill.runWait( CLNDRange( Math::pad( input.width(), KX ), Math::pad( input.height(), KY ) ), CLNDRange( KX, KY ) );
    }


    void PMHuberStereo::convertFillDisparity( Image& output, const Image& input, float dispscale )
    {
        _clpmh_filldepthmap.setArg( 0, output );
        _clpmh_filldepthmap.setArg( 1, input );
        _clpmh_filldepthmap.setArg( 2, dispscale );
        _clpmh_filldepthmap.runWait( CLNDRange( Math::pad( input.width(), KX ), Math::pad( input.height(), KY ) ), CLNDRange( KX, KY ) );
    }


    void PMHuberStereo::convertDisparity( Image& output, const Image& input, float dispscale, bool LR )
    {
        output.reallocate( input.width(), input.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );
        _clpmh_todisparity.setArg( 0, output );
        _clpmh_todisparity.setArg( 1, input );
        _clpmh_todisparity.setArg( 2, dispscale );
        _clpmh_todisparity.setArg<int>( 3, ( int ) LR );
        _clpmh_todisparity.runWait( CLNDRange( Math::pad( input.width(), KX ), Math::pad( input.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void PMHuberStereo::convertFillNormal( Image& output, const Image& input, bool LR )
    {
        output.reallocate( input.width(), input.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        _clpmh_fillnormalmap.setArg( 0, output );
        _clpmh_fillnormalmap.setArg( 1, input );
        _clpmh_fillnormalmap.setArg<int>( 2, ( int ) LR );
        _clpmh_fillnormalmap.runWait( CLNDRange( Math::pad( input.width(), KX ), Math::pad( input.height(), KY ) ), CLNDRange( KX, KY ) );
    }


    void PMHuberStereo::gradient( Image& output, const Image& input )
    {
        _clpmh_gradxy.setArg( 0, output );
        _clpmh_gradxy.setArg( 1, input );
        _clpmh_gradxy.run( CLNDRange( Math::pad( input.width(), KX ), Math::pad( input.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void PMHuberStereo::clear( Image& output )
    {
        _clpmh_clear.setArg( 0, output );
        _clpmh_clear.run( CLNDRange( Math::pad(  output.width(), KX ), Math::pad(  output.height(), KY ) ), CLNDRange( KX, KY ) );
    }


    void PMHuberStereo::clearViewBuffer( CLBuffer& buf, int width, int height )
    {
        _clpmh_viewbufclear.setArg( 0, buf );
        _clpmh_viewbufclear.setArg( 1, width );
        _clpmh_viewbufclear.setArg( 2, height );
        _clpmh_viewbufclear.run( CLNDRange( Math::pad( width, KX ), Math::pad( height, KY ) ), CLNDRange( KX, KY ) );
    }

    void PMHuberStereo::normalDepth( Image& output, const Image& input, float dispscale, bool LR  )
    {
        _clpmh_normaldepth.setArg( 0, output );
        _clpmh_normaldepth.setArg( 1, input );
        _clpmh_normaldepth.setArg( 2, dispscale );
        _clpmh_normaldepth.setArg<cl_int>( 3, ( cl_int ) LR ); // right to left
        _clpmh_normaldepth.runWait( CLNDRange( Math::pad( input.width(), KX ), Math::pad( input.height(), KY ) ), CLNDRange( KX, KY ) );
    }


    void PMHuberStereo::bilateralWeightToAlpha( Image& output, const Image& input, int patchsize ) const
    {
        _clpmh_bilateralweight.setArg( 0, output );
        _clpmh_bilateralweight.setArg( 1, input );
        _clpmh_bilateralweight.setArg<cl_int>( 2, patchsize );
        _clpmh_bilateralweight.runWait( CLNDRange( Math::pad( input.width(), KX ), Math::pad( input.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void PMHuberStereo::visualize( const String& name, const Image& input, float dscale ) const
    {
        Image d( input.width(), input.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );
        Image n( input.width(), input.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );

        _clpmh_visualize_depth_normal.setArg( 0, d );
        _clpmh_visualize_depth_normal.setArg( 1, n );
        _clpmh_visualize_depth_normal.setArg( 2, input );
        _clpmh_visualize_depth_normal.setArg( 3, dscale );
        _clpmh_visualize_depth_normal.run( CLNDRange( Math::pad( input.width(), KX ), Math::pad( input.height(), KY ) ), CLNDRange( KX, KY ) );

        d.save( name + "_depth.png");
        n.save( name + "_normal.png");
    }

    void PMHuberStereo::disparityToNormal( Image& output, const Image& input ) const
    {
        output.reallocate( input.width(), input.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        _clpmh_disparitytonormal.setArg( 0, output );
        _clpmh_disparitytonormal.setArg( 1, input );
        _clpmh_disparitytonormal.run( CLNDRange( Math::pad( input.width(), KX ), Math::pad( input.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void PMHuberStereo::pyrUpMul( Image& output, const Image& input, const Vector4f& mul )
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
}
