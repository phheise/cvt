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


#include <cvt/vision/slam/stereo/PatchStereoInit.h>
#include <cvt/vision/FeatureFilter.h>
#include <cvt/gfx/IMapScoped.h>
#include <cvt/geom/Line2D.h>

#include <cvt/util/ParamInfo.h>

#include <set>

namespace cvt {

    static ParamInfo* _pinfos[] = {
        new ParamInfoTyped<uint32_t>( "maxNewFeatures", 0, 5000, 800, true, 1, offsetof( PatchStereoInit::Parameters, maxNewFeatures ) ),
        new ParamInfoTyped<uint64_t>( "maxSAD", 0, 255, 30, true, 1, offsetof( PatchStereoInit::Parameters, maxSAD ) ),
        new ParamInfoTyped<float>( "maxEpilineDistance", 0.0f, 10.0f, 1.0f, true, 1, offsetof( PatchStereoInit::Parameters, maxEpilineDistance ) ),
        new ParamInfoTyped<float>( "maxReprojectionError", 0.0f, 10.0f, 1.0f, true, 1, offsetof( PatchStereoInit::Parameters, maxReprojectionError ) ),
        new ParamInfoTyped<float>( "minDepth", 0.1f, 5.0f, 3.0f, true, 1, offsetof( PatchStereoInit::Parameters, minDepth ) ),
        new ParamInfoTyped<float>( "maxDepth", 3.0f, 100.0f, 50.0f, true, 1, offsetof( PatchStereoInit::Parameters, maxDepth ) ),
        new ParamInfoTyped<uint8_t>( "fastThreshold", 1, 255, 30, true, 1, offsetof( PatchStereoInit::Parameters, fastThreshold ) ),
        new ParamInfoTyped<uint16_t>( "gridSize", 1, 1000, 30, true, 1, offsetof( PatchStereoInit::Parameters, gridSize ) ),
        new ParamInfoTyped<float>( "minInterFeatureDistance", 2.0f, 200.0f, 20.0f, true, 1, offsetof( PatchStereoInit::Parameters, minInterFeatureDistance ) )
    };

    PatchStereoInit::PatchStereoInit( const CameraCalibration& c0, const CameraCalibration& c1, size_t w0, size_t h0 ):
        DepthInitializer( c0, c1, w0, h0 ),
        _pyramidView0( 3, 0.5f ),
        _pyramidView1( 3, 0.5f ),
        _pyrGradX( 3, 0.5f ),
        _pyrGradY( 3, 0.5f ),
        _pset( _pinfos, 9, false )
    {
        _params = _pset.ptr<Parameters>();
        _detector.setBorder( 16 );
        _detector.setThreshold( _params->fastThreshold );
        _detector.setNonMaxSuppress( true );

        _simd = SIMD::instance();
    }

    PatchStereoInit::~PatchStereoInit()
    {
        _simd = 0;
    }

    void PatchStereoInit::updatePyramids( const Image& img0, const Image img1 )
    {
        _pyramidView0.update( img0 );
        _pyramidView1.update( img1 );

        _pyramidView0.convolve( _pyrGradX, IKernel::HAAR_HORIZONTAL_3 );
        _pyramidView0.convolve( _pyrGradY, IKernel::HAAR_VERTICAL_3 );
    }

    void PatchStereoInit::triangulateFeatures( std::vector<DepthInitResult> & triangulated,
                                               const std::vector<Vector2f> & avoidPositionsImg0,
                                               const Image& view0, const Image& view1 )
    {
        updatePyramids( view0, view1 );

        std::vector<Feature2Df> features0, features1;

        // first, detect the features in both views over multiple scales
        _detector.setNonMaxSuppress( true );
        detectFeatures( features0, _pyramidView0 );
        _detector.setNonMaxSuppress( false );
        detectFeatures( features1, _pyramidView1 );

        std::vector<Vector2f> filtered0;
        filterFeatures( filtered0, features0, avoidPositionsImg0 );

        // try to find matches using the epipolar constraint
        // and SAD matching based on the image data
        matchAndTriangulate( triangulated, filtered0, features1 );
    }

    void PatchStereoInit::detectFeatures( std::vector<Feature2Df>& features, const ImagePyramid& pyramid )
    {
        // detect for each scale
        ScaleSpaceFeatureInserterf inserter( features );
        inserter.scale  = 1.0f;
        for( size_t i = 0; i < pyramid.octaves(); i++ ){
            inserter.octave = i;
            _detector.extract( pyramid[ i ], inserter );
            inserter.scale /= pyramid.scaleFactor();
        }
    }

    void PatchStereoInit::filterFeatures( std::vector<Vector2f>& filtered,
                                          const std::vector<Feature2Df>& features,
                                          const std::vector<Vector2f>& avoid )
    {
        // perform a grid filtering on the first view to have better distribution
        FeatureFilter gridFilter( _params->gridSize, _pyramidView0[ 0 ].width(), _pyramidView0[ 0 ].height() );
        float minDistSqr = Math::sqr( _params->minInterFeatureDistance );
        for( size_t i = 0; i < features.size(); i++ ){
            // check if there is a too close feature in the avoid set
            const Feature2Df & f = features[ i ];
            bool take = true;
            for( size_t k = 0; k < avoid.size(); k++ ){
                if( ( f.pt - avoid[ k ] ).lengthSqr() < minDistSqr ){
                    take = false;
                    break;
                }
            }

            if( take ){
                gridFilter.addFeature( &features[ i ] );
            }
        }

        // now filter the features
        gridFilter.gridFilteredFeatures( filtered, _params->maxNewFeatures );
    }

    void PatchStereoInit::matchAndTriangulate( std::vector<DepthInitResult>& triangulated,
                                               const std::vector<Vector2f>& f0,
                                               const std::vector<Feature2Df>& f1 )
    {
        Vector3f p0;
        p0.z = 1.0f;

        std::set<size_t> assigned;

        CVT_ASSERT( _pyramidView0[ 0 ].format() == IFormat::GRAY_FLOAT, "Pyramid 0 has to be GRAY_FLOAT" );
        CVT_ASSERT( _pyramidView1[ 0 ].format() == IFormat::GRAY_FLOAT, "Pyramid 1 has to be GRAY_FLOAT" );


        IMapScoped<const float> map0( _pyramidView0[ 0 ] );
        IMapScoped<const float> gxMap( _pyrGradX[ 0 ] );
        IMapScoped<const float> gyMap( _pyrGradY[ 0 ] );
        IMapScoped<const float> map1( _pyramidView1[ 0 ] );
        const size_t PatchHalf = PatchSize >> 1;
        size_t maxSAD = _params->maxSAD * Math::sqr( PatchSize );

        Rangef depthRange( _params->minDepth, _params->maxDepth );

        size_t w0 = _pyramidView0[ 0 ].width();
        size_t h0 = _pyramidView0[ 0 ].height();
        size_t w1 = _pyramidView1[ 0 ].width();
        size_t h1 = _pyramidView1[ 0 ].height();

        PatchType patch( 1 );

        float baseLine = _stereoCalib.baseLine();
        const Matrix3f& K = _stereoCalib.firstCamera().intrinsics();
        float focalLen = K[ 0 ][ 0 ];
        float cx = K[ 0 ][ 2 ];
        float cy = K[ 1 ][ 2 ];

        float minDisp = focalLen * baseLine / depthRange.max;
        float maxDisp = focalLen * baseLine / depthRange.min;

        size_t f0Stride = map0.stride() / sizeof( float );
        size_t f1Stride = map1.stride() / sizeof( float );

        for( size_t i = 0; i < f0.size(); i++ ){
            const Vector2f& pos0 = f0[ i ];

            p0.x = pos0.x;
            p0.y = pos0.y;

            if( p0.x < PatchHalf || ( p0.x + PatchHalf ) >= w0 ||
                p0.y < PatchHalf || ( p0.y + PatchHalf ) >= h0 )
                continue;

            const float* ptr0 = map0.ptr() + (int)( p0.y - PatchHalf ) * f0Stride + (int)p0.x - PatchHalf;

            const std::set<size_t>::const_iterator assignedEnd = assigned.end();
            size_t bestSAD = maxSAD;
            size_t bestIdx =  0;

            for( size_t k = 0; k < f1.size(); k++ ){
                if( assigned.find( k ) == assignedEnd ){
                    const Vector2f& p1 = f1[ k ].pt;

                    if( p1.x < PatchHalf || ( p1.x + PatchHalf ) >= w1 ||
                        p1.y < PatchHalf || ( p1.y + PatchHalf ) >= h1 )
                        continue;

                    float d = Math::abs( p0.y - p1.y );

                    if( d < _params->maxEpilineDistance && p1.x < p0.x ){
                        const float* ptr1 = map1.ptr() + (int)( p1.y - PatchHalf ) * f1Stride + (int)p1.x - PatchHalf;
                        // check if SAD is smaller than current best
                        size_t sad = computePatchSAD( ptr0, f0Stride, ptr1, f1Stride );

                        if( sad < bestSAD ){
                            bestSAD = sad;
                            bestIdx = k;
                        }
                    }
                }
            }

            if( bestSAD < maxSAD ){
                // triangulate
                DepthInitResult result;
                result.meas0 = pos0;
                result.meas1 = f1[ bestIdx ].pt;

                patch.update( map0, gxMap, gyMap, pos0, w0, h0, 0 );
                patch.initPose( result.meas1 );

                if( refinePositionSubPixel( patch, map1 ) ){

                    // refined:
                    patch.currentCenter( result.meas1 );

                    float disparity = ( -result.meas1.x + result.meas0.x );

                    if( disparity > minDisp && disparity < maxDisp ){
                        float depth = focalLen * baseLine / disparity;

                        result.point3d.x = ( result.meas0.x - cx ) * depth / focalLen;
                        result.point3d.y = ( result.meas0.y - cy ) * depth / focalLen;
                        result.point3d.z = depth;
                        result.point3d.w = 1.0f;

                        triangulated.push_back( result );
                        assigned.insert( bestIdx );

                    }
                }
            }
        }
    }

    size_t PatchStereoInit::computePatchSAD( const float* p0, size_t s0,
                                             const float* p1, size_t s1 ) const
    {
        size_t n = PatchSize;
        size_t ret = 0;

        while( n-- ){
            ret += _simd->SAD( p0, p1, PatchSize );
            p0 += s0;
            p1 += s1;
        }

        return ret;
    }

    bool PatchStereoInit::refinePositionSubPixel( PatchType & patch,
                                                  IMapScoped<const float>& map )
    {
        if( patch.align( map.ptr(), map.stride(), _pyramidView1[ 0 ].width(), _pyramidView1[ 0 ].height() ) ){
            // check the SAD
            float sad = _simd->SAD( patch.pixels(), patch.transformed(), NumPatchPixel ) / float( NumPatchPixel );
            if( sad < _params->maxSAD / 255.0f )
                return true;
        }

        return false;
    }

}
