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

#include <cvt/vision/slam/stereo/KLTTracking.h>

namespace cvt
{
    KLTTracking::KLTTracking() :
        _ssdThreshold( Math::sqr( 30.0f ) ),
        _sadThreshold( 30 )
    {
    }

    KLTTracking::~KLTTracking()
    {
        clear();
    }

    void KLTTracking::trackFeatures( PointSet2d&                    trackedPositions,
                                     std::vector<size_t>&           trackedFeatureIds,
                                     const std::vector<Vector2f>&	predictedPositions,
                                     const std::vector<size_t>&		predictedIds,
                                     const ImagePyramid&            pyr )
    {
        SIMD* simd = SIMD::instance();
        const size_t nPixels = Math::sqr( PatchType::size() );
        const float  maxSSD = nPixels * _ssdThreshold;
        const size_t maxSAD = nPixels * _sadThreshold;

        Vector2f center;

        for( size_t i = 0; i < predictedPositions.size(); i++ ){
            const Vector2f& p = predictedPositions[ i ];

            size_t id = predictedIds[ i ];
            //  get the patch
            PatchType* patch = _patchForId[ id ];

            if( patch == 0 ){
                // this was a bad PATCH
                continue;
            }

            // start from predicted position
            patch->initPose( p );

            //  try to track the patch
            if( patch->align( pyr, 5 ) ){
                // successfully tracked: check SSD, SAD values
                float ssd = simd->SSD( patch->pixels(), patch->transformed(), nPixels );
                if( ssd < maxSSD ){
                    size_t sad = simd->SAD( patch->pixels(), patch->transformed(), nPixels );
                    if( sad < maxSAD ){
                        patch->currentCenter( center );
                        trackedPositions.add( Vector2d( center.x, center.y ) );
                        trackedFeatureIds.push_back( id );
                    }
                }
            }
        }
    }

    void KLTTracking::addFeatureToDatabase( const ImagePyramid& pyr,
                                            const ImagePyramid& pyrGradX,
                                            const ImagePyramid& pyrGradY,
                                            const Vector2f & f, size_t id )
    {
        std::vector<PatchType*> patches;
        std::vector<Vector2f>  feature;
        feature.push_back( f );

        PatchType::extractPatches( patches, feature, pyr, pyrGradX, pyrGradY );

        if( id != _patchForId.size() ){
            throw CVTException( "Patch IDs out of sync" );
        }

        // FIXME: shall we handle this differently?
        // Problem: Map has already added feature with id at this point
        if( patches.size() )
            _patchForId.push_back( patches[ 0 ] );
        else
            _patchForId.push_back( 0 );

    }


    void KLTTracking::clear()
    {
        for( size_t i = 0; i < _patchForId.size(); i++ ){
            delete _patchForId[ i ];
        }
        _patchForId.clear();
    }
}
