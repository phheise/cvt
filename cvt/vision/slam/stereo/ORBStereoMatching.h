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

#ifndef ORB_STEREO_MATCHING_H
#define ORB_STEREO_MATCHING_H

#include <cvt/vision/ORB.h>
#include <cvt/vision/CameraCalibration.h>
#include <cvt/vision/Vision.h>
#include <cvt/vision/FeatureMatch.h>
#include <cvt/geom/Line2D.h>

#include <set>

namespace cvt
{
   class ORBStereoMatching
   {
      public:
         ORBStereoMatching( float maxLineDistance,
                            float maxDescriptorDistance,
                            const CameraCalibration & c0,
                            const CameraCalibration & c1 );

         /**
          * \brief  find matches between the two orbs along the epipolar line
          * \param  matches container to return the matches within
          * \param  orb0    ORB object for the first view
          * \param  orb1    ORB object for the second view
          * \param  usedIndices those indices of orb0, that are already in use
          */
         void matchEpipolar( std::vector<FeatureMatch>& matches, const ORB& orb0, const ORB& orb1, const std::set<size_t> & usedIndices ) const;

         /**
          * \brief  find best matching ORB feature in orb1 for m.feature0
          * \param  m       the feature match (in/out), m.feature0 has to be initialized
          * \param  orb1    the ORBs to search within
          * \param  usedIn1 those feature indices in orb1, that have already been assigned!
          */
         size_t matchEpipolar( FeatureMatch& m, const ORB& orb1, const std::set<size_t>& usedIn1 ) const;

         /**
          * \return the fundamental matrix mapping points from camera c0, to lines in c1
          */
         const Matrix3f & fundamental() const { return _fundamental; }

         /**
          * \brief set the maximum allowed distance to the epipolar line
          * \param  dist    the new maximum distance
          */
         void setMaxLineDist( float dist ) { _maxLineDist = dist; }

         /**
          * \returns the maximum allowed distance to the epipolar line
          */
         float maxLineDistance() const { return _maxLineDist; }

         /**
          * \returns the maximum allowed descriptor distance
          */
         float maxDescDist() const { return _maxDescDist; }

         /**
          * \brief set the maximum descriptor distance
          * \param  dist    the new maximum distance
          */
         void setMaxDescDist( float dist ) { _maxDescDist = dist; }

      private:
         Matrix3f	_fundamental;
         float		_maxLineDist;
         float		_maxDescDist;
   };

   inline ORBStereoMatching::ORBStereoMatching( float maxLineDistance,
                                     float maxDescriptorDistance,
                                      const CameraCalibration& c0,
                                     const CameraCalibration& c1 ) :
      _maxLineDist( maxLineDistance ),
      _maxDescDist( maxDescriptorDistance )
   {
      // calc the essential matrix from the calibration data
      Vision::composeFundamental( _fundamental,
                            c0.intrinsics(),
                            c0.extrinsics(),
                            c1.intrinsics(),
                              c1.extrinsics() );
   }

   inline size_t ORBStereoMatching::matchEpipolar( FeatureMatch& m, const ORB& orb1, const std::set<size_t>& usedIn1 ) const
   {
      Vector3f point;
      point.z = 1.0f;

      m.distance = _maxDescDist;
      m.feature1 = 0;
      size_t distance;


      point.x = m.feature0->pt.x;
      point.y = m.feature0->pt.y;
      Line2Df l( _fundamental * point );

      size_t currAssigned = 0;
      const std::set<size_t>::const_iterator usedEnd = usedIn1.end();
      const ORBFeature& f = ( const ORBFeature& )*m.feature0;
      for( size_t k = 0; k < orb1.size(); k++ ){
         if( usedIn1.find( k ) == usedEnd ) {
            float lDist = l.distance( orb1[ k ].pt );
            if( Math::abs( lDist ) < _maxLineDist ){
               // check descriptor distance
               distance = f.distance( orb1[ k ] );
               if( distance < m.distance ){
                  m.feature1 = &orb1[ k ];
                  m.distance = distance;
                  currAssigned = k;
               }
            }
         }
      }
      return currAssigned;
   }

   inline void ORBStereoMatching::matchEpipolar( std::vector<FeatureMatch>& matches, const ORB& orb0, const ORB& orb1, const std::set<size_t> & used ) const
   {
      // with a row lookup table, we could probably speed up the search
      Vector3f line, point;
      point.z = 1.0f;
      FeatureMatch match;
      size_t distance;

      std::set<size_t>::const_iterator usedEnd = used.end();

      // ids from orb1 that have been assigned to a feature of orb1
      std::set<size_t> assigned;

      for( size_t i = 0; i < orb0.size(); i++ ){
         if( used.find( i ) != usedEnd )
            continue;

         const ORBFeature & f = orb0[ i ];
         point.x = f.pt.x;
         point.y = f.pt.y;
         line = _fundamental * point;
         Line2Df l( line );

         match.distance = _maxDescDist;
         match.feature0 = &orb0[ i ];

         size_t currAssigned = 0;
         const std::set<size_t>::const_iterator assignedEnd = assigned.end();
         for( size_t k = 0; k < orb1.size(); k++ ){
            if( assigned.find( k ) == assignedEnd ) {
               float lDist = l.distance( orb1[ k ].pt );
               if( Math::abs( lDist ) < _maxLineDist ){
                  // check descriptor distance
                  distance = f.distance( orb1[ k ] );
                  if( distance < match.distance ){
                     match.feature1 = &orb1[ k ];
                     match.distance = distance;
                     currAssigned = k;
                  }
               }
            }
         }

         if( match.distance < _maxDescDist ){
            matches.push_back( match );
            assigned.insert( currAssigned );
         }
      }
   }
}

#endif
