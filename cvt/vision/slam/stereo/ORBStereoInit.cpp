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

#include <cvt/vision/slam/stereo/ORBStereoInit.h>

namespace cvt
{
    static ParamInfo* _pinfos[] = {
        new ParamInfoTyped<float>( "maxEpilineDistance", 0.0f, 10.0f, 1.0f, true, 1, offsetof( ORBStereoInit::Parameters, maxEpilineDistance ) ),
        new ParamInfoTyped<float>( "maxDescriptorDistance", 0.0f, 255.0f, 80.0f, true, 1, offsetof( ORBStereoInit::Parameters, maxDescriptorDistance ) ),
        new ParamInfoTyped<float>( "maxReprojectionError", 0.0f, 20.0f, 2.0f, true, 1, offsetof( ORBStereoInit::Parameters, maxReprojectionError ) ),
        new ParamInfoTyped<float>( "minDepth", 0.1f, 5.0f, 0.5f, true, 1, offsetof( ORBStereoInit::Parameters, minDepth ) ),
        new ParamInfoTyped<float>( "maxDepth", 3.0f, 100.0f, 30.0f, true, 1, offsetof( ORBStereoInit::Parameters, maxDepth ) ),
        new ParamInfoTyped<uint8_t>( "fastThreshold", 10, 200, 20, true, 1, offsetof( ORBStereoInit::Parameters, fastThreshold ) ),
        new ParamInfoTyped<uint32_t>( "orbMaxFeatures", 10, 10000, 2000, true, 1, offsetof( ORBStereoInit::Parameters, orbMaxFeatures ) )
    };

   ORBStereoInit::ORBStereoInit( const CameraCalibration& c0, const CameraCalibration& c1,
                                 size_t w0, size_t h0 ) :
       DepthInitializer( c0, c1, w0, h0 ),
       _matcher( 1.0f, 80, _stereoCalib.firstCamera(), _stereoCalib.secondCamera() ),
       _pset( _pinfos, 7, false )
   {
   }

   void ORBStereoInit::triangulateFeatures( std::vector<DepthInitResult> & triangulated,
                                            const std::vector<Vector2f> & avoidPositionsImg0,
                                            const Image& view0, const Image& view1 )
   {
       Parameters* params = _pset.ptr<Parameters>();

       _matcher.setMaxDescDist( params->maxDescriptorDistance );
       _matcher.setMaxLineDist( params->maxEpilineDistance );
       Rangef depthRange( params->minDepth, params->maxDepth );

       ORB orb0( view0, 2, 0.5f, params->fastThreshold, params->orbMaxFeatures, true );
       ORB orb1( view1, 2, 0.5f, params->fastThreshold, params->orbMaxFeatures, true );

       // find stereoMatches by avoiding already found matches
       std::set<size_t> doNotUse;
       for( size_t i = 0; i < orb0.size(); i++ ){
           const Vector2f& po = orb0[ i ].pt;
           for( size_t k = 0; k < avoidPositionsImg0.size(); k++ ){
               if( ( po - avoidPositionsImg0[ k ] ).lengthSqr() < Math::sqr( 16.0f ) ){
                   doNotUse.insert( i );
                   break;
               }
           }
       }

       FeatureMatch match;
       DepthInitResult result;

       for( size_t i = 0; i < orb0.size(); ++i ){
           match.feature0 = &orb0[ i ];
           size_t idx = _matcher.matchEpipolar( match, orb1, doNotUse );
           if( match.feature1 ){
                doNotUse.insert( idx );

                // TODO: check the SAD between the two features for savety?
                //if( checkFeatureSAD( match.feature0->pt, match.feature1->pt, first, second ) )

                // correct the correspondences
                result.meas0 = match.feature0->pt;
                result.meas1 = match.feature1->pt;
                Vision::correctCorrespondencesSampson( result.meas0, result.meas1, _matcher.fundamental() );

                triangulateSinglePoint( result,
                                        _stereoCalib.firstCamera().projectionMatrix(),
                                        _stereoCalib.secondCamera().projectionMatrix(),
                                        depthRange );

                if( result.reprojectionError < params->maxReprojectionError ){
                    triangulated.push_back( result );
                }
             }
        }
    }



}
