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

#ifndef CVT_STEREO_SLAM_H
#define CVT_STEREO_SLAM_H

#include <cvt/gfx/Image.h>
#include <cvt/math/SE3.h>
#include <cvt/util/Signal.h>
#include <cvt/vision/slam/SlamMap.h>
#include <cvt/vision/slam/Keyframe.h>
#include <cvt/vision/slam/stereo/DescriptorDatabase.h>
#include <cvt/vision/features/FeatureMatch.h>
#include <cvt/vision/CameraCalibration.h>
#include <cvt/vision/StereoCameraCalibration.h>
#include <cvt/vision/slam/stereo/FeatureTracking.h>
#include <cvt/vision/slam/stereo/MapOptimizer.h>
#include <set>

namespace cvt
{
   // Managing class for stereo SLAM
   class StereoSLAM
   {
	   public:
		   struct Params {
				Params():
                   keyframeSelectionRadius( 8.0f ),
				   pyramidOctaves( 4 ),
				   pyramidScaleFactor( 0.6f ),
                   matchingWindow( 70.0f ),
				   matchingMaxDescDistance( 80.0f ),
                   minTrackedFeatures( 100 ),
                   maxKeyframeDistance( 4.0f ),
                   minFeaturesForKeyframe( 50 ),
                   minDisparity( 13.0f ),
                   maxDisparity( 150.0f ),
                   maxEpilineDistance( 1.5f ),
                   stereoMaxDescDistance( 70.0f ),
                   useGridFiltering( true ),
                   maxFeaturesPerCell( 100 ),
                   gridFilteringCellsX( 10 ),
                   gridFilteringCellsY( 4 ),
                   nonMaximumSuppressionRadius( 3 ),
                   bestFeaturesCount( 4000 ),
                   kltTrackingIters( 2 ),
                   kltAvgSAD( 0.25f ),
				   kltStereoIters( 2 ),
                   useSBA( false ),
                   sbaIterations( 5 ),
                   sbaDeltaKeyframes( 1 ),
				   dbgShowFeatures( false ),
				   dbgShowNMSFilteredFeatures( false ),
				   dbgShowBest3kFeatures( false ),
				   dbgShowStereoMatches( false )
				{
				}

				float  keyframeSelectionRadius;
				size_t pyramidOctaves;
				float  pyramidScaleFactor;

				/* radius matching*/
				float  matchingWindow;
				float  matchingMaxDescDistance;

				/* keyframe recreation */
				size_t minTrackedFeatures;
				float  maxKeyframeDistance;
				size_t minFeaturesForKeyframe;

				/* stereo matching params*/
				float minDisparity;
				float maxDisparity;
				float maxEpilineDistance;
				float stereoMaxDescDistance;

                /* Filtering */
                bool    useGridFiltering;
                int     maxFeaturesPerCell;
                int     gridFilteringCellsX;
                int     gridFilteringCellsY;
                int     nonMaximumSuppressionRadius;
                int     bestFeaturesCount;

				/* klt params */
                size_t kltTrackingIters;
                float  kltAvgSAD;
				size_t kltStereoIters;

                /* use SBA */
                bool    useSBA;
                size_t  sbaIterations;

                /* trigger sba when sbaDeltaKeyframes have been
                 * added since last sba run */
                size_t  sbaDeltaKeyframes;

				/* debug params */
				bool dbgShowFeatures;
				bool dbgShowNMSFilteredFeatures;
				bool dbgShowBest3kFeatures;
				bool dbgShowStereoMatches;
		   };

		   StereoSLAM( FeatureDetector* detector,
					   FeatureDescriptorExtractor* descExtractor,
					   const StereoCameraCalibration& calib,
					   const Params& params=Params());


		 /**
		  * @brief newImages
		  * @param imgLeft	undistorted-rectified left frame
		  * @param imgRight undistorted-rectified right frame
		  */
		 void				newImages( const Image& imgLeft,
									   const Image& imgRight );

		 const SlamMap&		map() const { return _map; }

		 void				clear();
		 void				setPose( const Matrix4f& pose );
		 const SE3<float>&	pose() const { return _pose; }

         void               setConfig( const Params& configParams );
         const Params&      config() const { return _params; }

		 Signal<const Image&>       newStereoView;
		 Signal<const Image&>       trackedFeatureImage;
		 Signal<void>               keyframeAdded;
		 Signal<const SlamMap&>     mapChanged;
		 Signal<const Matrix4f&>    newCameraPose;
		 Signal<size_t>             numTrackedPoints;
		 Signal<const Image&>       newStereoMatches;
		 Signal<const PointSet3f&>  triangulatedPoints;

		 EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	  private:
		 struct TrackedFeatures {
			PointSet2f			points2d;
			PointSet3f			points3d;

			/* ids of the 3d points within the map */
			std::vector<size_t> mapFeatureIds;

			void reserve( size_t n )
			{
				points2d.reserve( n );
				points3d.reserve( n );
				mapFeatureIds.reserve( n );
			}

			size_t size() const { return points3d.size(); }
		 };

		 typedef DescriptorDatabase::PatchType	PatchType;
		 Params						 _params;
		 FeatureDetector*			 _detector;
		 FeatureDescriptorExtractor* _descExtractorLeft;
		 FeatureDescriptorExtractor* _descExtractorRight;
		 DescriptorDatabase			 _descriptorDatabase;
		 ImagePyramid				 _pyrLeft;
		 ImagePyramid				 _pyrRight;

		 /* float versions for KLT */
		 ImagePyramid				 _pyrLeftf;
		 ImagePyramid				 _pyrRightf;
		 ImagePyramid				 _gradXl;
		 ImagePyramid				 _gradYl;
		 IKernel					 _kernelGx;
		 IKernel					 _kernelGy;


		 StereoCameraCalibration	 _calib;

		 /* the current pose of the camera rig */
		 SE3<float>					 _pose;
		 int						 _activeKF;
         Eigen::Matrix4d             _keyframeRelativePose;
		 SlamMap					 _map;
		 MapOptimizer				 _bundler;
		 Image						 _lastImage;
		 Image						 _debugMono;

		 void extractFeatures( const Image& left, const Image& right );

		 void predictVisibleFeatures( std::vector<Vector2f>& imgPositions,
									  std::vector<size_t>& ids,
									  std::vector<FeatureDescriptor*>& descriptors,
									  std::vector<PatchType*>& patches,
									  const Eigen::Matrix4d& cameraPose );

		 void trackPredictedFeatures( TrackedFeatures& tracked,
									  std::vector<MatchingIndices> &matchedIndices,
									  const std::vector<FeatureDescriptor*>& predictedDescriptors,
									  std::vector<StereoSLAM::PatchType*>& predictedPatches,
									  const std::vector<size_t>& mapIds );

		 void createCorrespondences( PointSet2f& points2d,
									 PointSet3f& points3d,
									 std::vector<size_t>& mapIndices,
									 const std::vector<MatchingIndices>& matchedIndices,
									 const std::vector<size_t>& ids ) const;

		 void estimateCameraPose( std::vector<size_t>& inlierIndices,
								  const PointSet3f & p3d,
								  const PointSet2f & p2d );

		 bool newKeyframeNeeded( size_t numTrackedFeatures ) const;

		 void initNewStereoFeatures( PointSet3f& newPts3d,
									 std::vector<const FeatureDescriptor*>& newDescriptors,
									 std::vector<PatchType*>& newPatches,
									 const std::vector<size_t> &trackingInliers,
									 const std::vector<MatchingIndices>& matchedIndices );

		 void sortOutFreeFeatures( std::vector<const FeatureDescriptor*>& freeFeatures,
								   const FeatureDescriptorExtractor* extractor,
								   const std::vector<size_t>& inliers,
								   const std::vector<MatchingIndices>& matches ) const;

		 void addNewKeyframe( const std::vector<const FeatureDescriptor*>& newDescriptors,
							  std::vector<PatchType *>& newPatches,
							  const PointSet3f& newPoints3d,
							  const PointSet2f& trackedMapPoints,
							  const std::vector<size_t>& trackedMapIds,
							  const std::vector<size_t>& inliers );

		 void debugPatchWorkImage( const std::set<size_t>&          indices,
								   const std::vector<size_t>&       featureIds,
								   const std::vector<FeatureMatch>& matches );

		 void createDebugImageMono( Image & debugImage,
									const PointSet2f & tracked,
									const std::vector<Vector2f> & predPos ) const;

		 void createDebugImageMono1( Image & debugImage,
									 const TrackedFeatures& tracked,
									 const std::vector<Vector2f>& predPos,
									 const std::vector<MatchingIndices>& matchedIndices,
									 const std::vector<size_t>& predictedFeatureIds) const;

		 void debugImageDrawFeatures( Image & debugImage,
									  const FeatureSet& featureSet,
									  const Color& color=Color::BLUE,
									  const size_t rectSize=5 ) const;

		 void createStereoMatchingDebugImage( Image & debugImage,
											  std::vector<FeatureMatch> stereoMatches );

   };
}

#endif
