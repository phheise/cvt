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

#include <cvt/vision/slam/stereo/ORBTracking.h>

namespace cvt
{
   ORBTracking::ORBTracking() :
      _maxDescDistance( 80 ),
      _windowRadius( 100 ),
      _orbOctaves( 3 ),
      _orbScaleFactor( 0.5f ),
      _orbCornerThreshold( 15 ),
      _orbMaxFeatures( 2000 ),
	  _orbNonMaxSuppression( true )
   {
   }

   ORBTracking::~ORBTracking()
   {
   }

   void ORBTracking::trackFeatures( PointSet2d&                         trackedPositions,
                                    std::vector<size_t>&		trackedFeatureIds,
                                    const std::vector<Vector2f>&	predictedPositions,
                                    const std::vector<size_t>&		predictedIds,
                                    const Image&			img )
   {
      // create the ORB
	  //_orb0.update( img );

      // we want to find the best matching orb feature from current, that lies
      // within a certain distance from the "predicted" position
      std::vector<size_t>::const_iterator currentId = predictedIds.begin();
      std::vector<size_t>::const_iterator tEnd = predictedIds.end();
      std::vector<Vector2f>::const_iterator pred = predictedPositions.begin();

      // keep track of already assigned indices to avoid double associations
      _orb0MatchedIds.clear();
      while( currentId != tEnd ){
         FeatureMatch m;
         const ORBFeature & desc = _descriptors.descriptor( *currentId );
         m.feature0 = &desc;
         int matchedIdx = matchInWindow( m, *pred, _orb0, _orb0MatchedIds );
         if( matchedIdx != -1 ){
            _orb0MatchedIds.insert( ( size_t )matchedIdx );
            trackedPositions.add( Vector2d( m.feature1->pt.x, m.feature1->pt.y ) );
            trackedFeatureIds.push_back( *currentId );
         }
         ++currentId;
         ++pred;
      }
   }

   void ORBTracking::addFeatureToDatabase( const Vector2f & f, size_t id )
   {
       // this is odd, we need to search the closest feature in orb0
       size_t closestIdx = 0;
       float  distsqr = Math::MAXF;
       for( size_t i = 0; i < _orb0.size(); ++i ){
           float currd = ( _orb0[ i ].pt - f ).lengthSqr();
           if( currd < distsqr ){
               closestIdx = i;
               distsqr = currd;
           }
       }

       _descriptors.addDescriptor( _orb0[ closestIdx ], id );
   }

   int ORBTracking::matchInWindow( FeatureMatch& match, const Vector2f & p, const ORB & orb, const std::set<size_t> & used ) const
   {
      const ORBFeature * f = (ORBFeature*)match.feature0;
      match.feature1 = 0;
      match.distance = _maxDescDistance;
      size_t currDist;
      const std::set<size_t>::const_iterator usedEnd = used.end();
      size_t matchedId = 0;
      for( size_t i = 0; i < orb.size(); i++ ){
         if( used.find( i ) == usedEnd ){
            if( ( p - orb[ i ].pt ).length() < _windowRadius ){
               // try to match
               currDist = f->distance( orb[ i ] );
               if( currDist < match.distance ){
                  match.feature1 = &orb[ i ];
                  match.distance = currDist;
                  matchedId = i;
               }
            }
         }
      }

      // to ensure unique matches
      if( match.distance < _maxDescDistance ){
         return matchedId;
      }
      return -1;
   }

   void ORBTracking::clear()
   {
      _descriptors.clear();
      _orb0MatchedIds.clear();
   }
}

