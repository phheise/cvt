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

#ifndef CVT_DEPTHINITIALIZER_H
#define CVT_DEPTHINITIALIZER_H

#include <cvt/vision/StereoCameraCalibration.h>
#include <cvt/vision/Vision.h>
#include <cvt/gfx/Image.h>
#include <cvt/math/Vector.h>
#include <cvt/util/Range.h>
#include <cvt/util/ParamSet.h>

#include <vector>

namespace cvt
{
    class DepthInitializer
    {
        public:
            struct DepthInitResult
            {
                Vector4f    point3d;
                Vector2f    meas0;
                Vector2f    meas1;
                float       reprojectionError;
            };

            DepthInitializer( const CameraCalibration& c0, const CameraCalibration& c1, size_t w, size_t h );
            virtual ~DepthInitializer();

            /**
             *	\brief triangulate features from two (undistorted) views
             */
            virtual void triangulateFeatures( std::vector<DepthInitResult> & triangulated,
                                              const std::vector<Vector2f> & avoidPositionsImg0,
                                              const Image& view0, const Image& view1 ) = 0;

            virtual ParamSet&                parameters() = 0;

            const CameraCalibration& calibration0() const { return _stereoCalib.firstCamera(); }
            const CameraCalibration& calibration1() const { return _stereoCalib.secondCamera(); }

            const Image&    undistortionMap0() const { return _undistortionMaps[ 0 ]; }
            const Image&    undistortionMap1() const { return _undistortionMaps[ 1 ]; }

        protected:
            StereoCameraCalibration     _stereoCalib;
            std::vector<Image>          _undistortionMaps;

            /* triangulate a new point */
            void triangulateSinglePoint( DepthInitResult&   result,
                                         const Matrix4f&    projMat0,
                                         const Matrix4f&    projMat1,
                                         const Rangef&      depthRange ) const;

    };

    inline DepthInitializer::DepthInitializer( const CameraCalibration& c0, const CameraCalibration& c1, size_t w, size_t h )
    {
        StereoCameraCalibration scam( c0, c1 );
        _undistortionMaps.resize( 2 );
        _undistortionMaps[ 0 ].reallocate( w, h, IFormat::GRAYALPHA_FLOAT );
        _undistortionMaps[ 1 ].reallocate( w, h, IFormat::GRAYALPHA_FLOAT );
        scam.undistortRectify( _stereoCalib, _undistortionMaps[ 0 ], _undistortionMaps[ 1 ], w, h, false );
    }

   inline DepthInitializer::~DepthInitializer()
   {
   }

   inline void DepthInitializer::triangulateSinglePoint( DepthInitResult& result,
                                                         const Matrix4f&  projMat0,
                                                         const Matrix4f&  projMat1,
                                                         const Rangef&    depthRange ) const
   {
       Vector4f repr;
       Vector2f repr2;

       Vision::triangulate( result.point3d, projMat0, projMat1, result.meas0, result.meas1 );

       // normalize 4th coord;
       result.point3d /= result.point3d.w;
       result.reprojectionError = Math::MAXF;
       if( result.point3d.z > depthRange.min && result.point3d.z < depthRange.max ){
           repr = projMat0 * result.point3d;
           repr2.x = repr.x / repr.z;
           repr2.y = repr.y / repr.z;
           result.reprojectionError = ( result.meas0 - repr2 ).length();

           repr = projMat1 * result.point3d;
           repr2.x = repr.x / repr.z;
           repr2.y = repr.y / repr.z;
           result.reprojectionError += ( result.meas1 - repr2 ).length();

           result.reprojectionError /= 2.0f;
       }
   }
}

#endif
