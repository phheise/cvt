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

#ifndef CVT_SLAMMAP_H
#define CVT_SLAMMAP_H

#include <cvt/vision/CameraCalibration.h>
#include <cvt/vision/slam/Keyframe.h>
#include <cvt/vision/slam/MapFeature.h>
#include <cvt/io/xml/XMLSerializable.h>

#include <Eigen/StdVector>

namespace cvt
{
   class SlamMap : public XMLSerializable
   {
      public:
         EIGEN_MAKE_ALIGNED_OPERATOR_NEW
         SlamMap();
         ~SlamMap();

         void clear();

        /**
         *	\brief		add a new keyframe to the map
         *	\param pose	the pose of the keyframe in the map: TODO: should be KF to world <- verify
         */
         size_t addKeyframe( const Eigen::Matrix4d& pose );

        /**
         *	\brief 		add a new 3D feature to the map
         *	\param world	the mapfeature to add
         *	\return 	the id of the newly added feature in the map
         */
         size_t addFeature( const MapFeature& world );

        /**
         *	\brief			add a new feature to a given keyframe (and the map)
         *	\param	world		the 3D MapFeature
         *	\param	feature		the 2D Measurement of the feature in the keyframe
         *	\param	keyframeId	the id of the keyframe
         *	\return 		the id of the newly added feature in the map
         */
         size_t addFeatureToKeyframe( const MapFeature& world,
                                      const MapMeasurement& feature,
                                      size_t keyframeId );

         /**
          *	\brief		adds a measurement to the point track of a given MapMeasurement and Keyframe
          *	\param	pointId	id of the 3D map measurement
          *	\param	keyframeId id of the keyframe
          *	\param	meas	the map measurement to add
          */
         void addMeasurement( size_t pointId,
                              size_t keyframeId,
                              const MapMeasurement& meas );


         int findClosestKeyframe( const Eigen::Matrix4d& worldT ) const;

         /**
          *	\brief predict features that project into the current frame
          *	\param	visibleFeatureIds	ids visible features
          *	\param	projections		    the predicted feature positions in the given frame
          *	\param	cameraPose		    pose of the camera
          *	\param	camCalib			calibration of the camera
          *	\param	maxDistance			maximum distance of keyframes that are taken into account for projection
          */
		 void   selectVisibleFeatures( std::vector<size_t>& visibleFeatureIds,
									   std::vector<Vector2f>& projections,
									   const Eigen::Matrix4d& cameraPose,
									   const CameraCalibration& camCalib,
									   double maxDistance = 3.0	) const;

         const MapFeature&		featureForId( size_t id ) const  { return _features[ id ];}
		 MapFeature&			featureForId( size_t id )		 { return _features[ id ];}
		 const Keyframe&		keyframeForId( size_t id ) const { return _keyframes[ id ];}
         Keyframe&				keyframeForId( size_t id )		 { return _keyframes[ id ];}
		 const Eigen::Matrix3d&	intrinsics() const { return _intrinsics; }
         void setIntrinsics( const Eigen::Matrix3d & K ) { _intrinsics = K; }

         size_t numFeatures()	  const { return _features.size(); }
         size_t numKeyframes()	  const { return _keyframes.size(); }
         size_t numMeasurements() const { return _numMeas; }

         void deserialize( XMLNode* node );
         XMLNode* serialize() const;

         void load( const cvt::String& filename );
         void save( const cvt::String& filename ) const;

         void loadBinary( const cvt::String& filename );
         void saveBinary( const cvt::String& filename ) const;

      private:
		 typedef std::vector<Keyframe, Eigen::aligned_allocator<Keyframe> > KeyframeVectorType;
         typedef std::vector<MapFeature, Eigen::aligned_allocator<MapFeature> > MapFeatureVectorType;

		 KeyframeVectorType		_keyframes;
		 MapFeatureVectorType	_features;
		 Eigen::Matrix3d		_intrinsics;
         size_t					_numMeas;
   };
}

#endif
