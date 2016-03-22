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

#ifndef ORB_TRACKING_H
#define ORB_TRACKING_H

#include <cvt/vision/Vision.h>
#include <cvt/vision/features/ORB.h>
#include <cvt/vision/slam/MapFeature.h>
#include <cvt/vision/slam/stereo/DescriptorDatabase.h>
#include <cvt/vision/slam/stereo/FeatureTracking.h>
#include <cvt/vision/features/FeatureMatch.h>

namespace cvt
{

   /**
    *	Feature Tracking using ORB Matching
    */
   class ORBTracking : public FeatureTracking
   {
      public:
         ORBTracking();
         ~ORBTracking();

         void trackFeatures( PointSet2d&                    trackedPositions,
                             std::vector<size_t>&           trackedFeatureIds,
                             const std::vector<Vector2f>&   predictedPositions,
                             const std::vector<size_t>&     predictedIds,
                             const Image&                   img );

		 void addFeatureToDatabase( const Vector2f& f, size_t id );

         void clear();

      private:
		 DescriptorDatabase<FeatureDescriptor> _descriptors;
		 size_t								   _maxDescDistance;
		 float								   _windowRadius;

         /* orb parameters */
         size_t                         _orbOctaves;
         float							_orbScaleFactor;
         uint8_t						_orbCornerThreshold;
         size_t                         _orbMaxFeatures;
         bool							_orbNonMaxSuppression;
         ORB							_orb0;
         std::set<size_t>               _orb0MatchedIds;

         int matchInWindow( FeatureMatch& match, const Vector2f & p, const ORB & orb, const std::set<size_t> & used ) const;

   };
}

#endif

