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

#ifndef CVT_KLT_FEATURE_TRACKING_H
#define CVT_KLT_FEATURE_TRACKING_H

#include <cvt/vision/slam/stereo/FeatureTracking.h>
#include <cvt/vision/slam/stereo/DescriptorDatabase.h>
#include <cvt/vision/KLTPatch.h>
#include <cvt/math/GA2.h>


namespace cvt
{
    class KLTTracking : public FeatureTracking
    {
        public:
            KLTTracking();
            ~KLTTracking();

            /**
             * \brief track features in the current Image
             * \param trackedPositions		positions of the tracked 2D features
             * \param trackedFeatureIds		ids (of predictedIds) that have been tracked
             * \param predictedPositions	vector of predicted feature positions
             * \param img			the current image
             */
            void trackFeatures( PointSet2d&                     trackedPositions,
                                std::vector<size_t>&            trackedFeatureIds,
                                const std::vector<Vector2f>&	predictedPositions,
                                const std::vector<size_t>&      predictedIds,
                                const ImagePyramid &            pyr );

            void trackFeatures( PointSet2d&                    trackedPositions,
                                std::vector<size_t>&           trackedFeatureIds,
                                const std::vector<Vector2f>&	predictedPositions,
                                const std::vector<size_t>&		predictedIds,
                                const Image&                   img )
            {
                throw CVTException( "Not implemented" );
            }

            /**
             * \brief add a new feature to the database! (e.g. after triangulation)
             */
            void addFeatureToDatabase( const ImagePyramid &pyr,
                                       const ImagePyramid &pyrGradX,
                                       const ImagePyramid &pyrGradY,
                                       const Vector2f &f, size_t id );
            void clear();

            void addFeatureToDatabase( const Vector2f &, size_t )
            {
                throw CVTException( "NOT IMPLEMENTED" );
            }

        private:
            typedef GA2<float>          PoseType;
            static const size_t         PatchSize = 16;
            typedef KLTPatch<PatchSize, PoseType> PatchType;

            std::vector<PatchType*>     _patchForId;
            float                       _ssdThreshold;
            float                       _sadThreshold;
    };
}
#endif

