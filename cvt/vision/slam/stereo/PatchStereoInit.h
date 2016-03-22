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


#ifndef CVT_PATCHSTEREOINIT_H
#define CVT_PATCHSTEREOINIT_H

#include <cvt/vision/slam/stereo/DepthInitializer.h>
#include <cvt/vision/features/FAST.h>
#include <cvt/vision/ImagePyramid.h>
#include <cvt/vision/KLTPatch.h>
#include <cvt/math/GA2.h>
#include <cvt/math/Translation2D.h>

namespace cvt {

    class PatchStereoInit : public DepthInitializer
    {
        public:
            PatchStereoInit( const CameraCalibration& c0, const CameraCalibration& c1,
                             size_t w0, size_t h0 );
            ~PatchStereoInit();

            void        triangulateFeatures( std::vector<DepthInitResult> & triangulated,
                                             const std::vector<Vector2f>  & avoidPositionsImg0,
                                             const Image& view0, const Image& view1 );

            ParamSet&   parameters()    { return _pset; }

            struct Parameters
            {
                uint32_t maxNewFeatures;
                uint64_t maxSAD;
                float    maxEpilineDistance;
                float    maxReprojectionError;
                float    maxDepth;
                float    minDepth;
                uint8_t  fastThreshold;
                uint16_t gridSize;
                float    minInterFeatureDistance;
            };

        private:
            static const size_t PatchSize = 32;
            static const size_t NumPatchPixel = PatchSize * PatchSize;

            FAST            _detector;
            ImagePyramid    _pyramidView0;
            ImagePyramid    _pyramidView1;
            ImagePyramid    _pyrGradX;
            ImagePyramid    _pyrGradY;

            ParamSet        _pset;
            Parameters*     _params;

            SIMD*           _simd;

            // subpixel position refiner
            //typedef GA2<float> PoseT;
            typedef Translation2D<float> PoseT;
            typedef KLTPatch<PatchSize, PoseT> PatchType;

            void detectFeatures( std::vector<Feature2Df>& features, const ImagePyramid& pyramid );

            void filterFeatures( std::vector<Vector2f>& filtered,
                                 const std::vector<Feature2Df>& features,
                                 const std::vector<Vector2f>& avoid );

            void matchAndTriangulate( std::vector<DepthInitResult>& triangulated,
                                      const std::vector<Vector2f>& f0,
                                      const std::vector<Feature2Df>& f1 );

            size_t computePatchSAD( const float *p0, size_t s0,
                                    const float *p1, size_t s1 ) const;

            bool refinePositionSubPixel( PatchType& patch,
                                         IMapScoped<const float>& map );

            void updatePyramids( const Image& img0, const Image img1 );

    };

}

#endif
