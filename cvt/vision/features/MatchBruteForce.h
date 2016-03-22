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

#ifndef CVT_FEATURE_MATCHER_INL
#define CVT_FEATURE_MATCHER_INL

#include <cvt/vision/features/RowLookupTable.h>

namespace cvt {

	namespace FeatureMatcher {
		template<typename T, typename DFUNC>
		static inline void matchBruteForce( std::vector<FeatureMatch>& matches, const std::vector<T>& seta, const std::vector<T>& setb, DFUNC dfunc, float distThreshold )
		{
			matches.reserve( seta.size() );
			for( size_t i = 0; i < seta.size(); i++ ) {
				FeatureMatch m;
				const T& d0 = seta[ i ];

				m.feature0 = &d0;
				m.feature1 = 0;
				m.distance = distThreshold;
				for( size_t k = 0; k < setb.size(); k++ ) {
					const T& d1 = setb[ k ];
					float distance = dfunc( d0, d1 );

					if( distance < m.distance ) {
						m.feature1 = &d1;
						m.distance = distance;
					}
				}

				if( m.feature1 )
					matches.push_back( m );
			}
		}

		template<typename T, typename DFUNC>
		static inline void matchInWindow( std::vector<MatchingIndices>& matches,
										  const std::vector<FeatureDescriptor*>& setA,
										  const std::vector<T>& setB,
										  DFUNC dfunc,
										  float maxFeatureDist,
										  float maxDescDistance )
		{
			matches.reserve( setA.size() );
			MatchingIndices m;
			float distanceSquare = Math::sqr( maxFeatureDist );
			for( size_t i = 0; i < setA.size(); ++i ) {
				const T& d0 = *( ( const T* )setA[ i ] );
				m.srcIdx = i;
				m.dstIdx = 0;
				m.distance = maxDescDistance;
				for( size_t k = 0; k < setB.size(); ++k ) {
					const T& d1 = setB[ k ];
					// euclidean distance
					float ptDist = ( d1.pt - d0.pt ).lengthSqr();
					if( ptDist > distanceSquare )
						continue;
					// descriptor distance
					float distance = dfunc( d0, d1 );

					if( distance < m.distance ) {
						m.dstIdx = k;
						m.distance = distance;
					}
				}

				if( m.distance < maxDescDistance ){					

					matches.push_back( m );
				}
			}
		}

		template<typename T, typename DFUNC>
		static inline void matchInWindow( std::vector<MatchingIndices>& matches,
										  const RowLookupTable& rlt,
										  const std::vector<FeatureDescriptor*>& setA,
										  const std::vector<T>& setB,
										  DFUNC dfunc,
										  float maxFeatureDist,
										  float maxDescDistance )
		{
			matches.reserve( setA.size() );
			MatchingIndices m;
			for( size_t i = 0; i < setA.size(); ++i ) {
				const T& d0 = *( ( const T* )setA[ i ] );
				m.srcIdx = i;
				m.dstIdx = 0;
				m.distance = maxDescDistance;

                float minX = d0.pt.x - maxFeatureDist;
                float maxX = d0.pt.x + maxFeatureDist;
                float minY = d0.pt.y - maxFeatureDist;
				float maxY = d0.pt.y + maxFeatureDist;

				for( int y = minY; y < maxY; ++y ){
					if( rlt.isValidRow( y ) ){
						// match all features of this row
						const RowLookupTable::Row& row = rlt.row( y );
						int k = row.start;
						size_t rEnd = k + row.len;
						for( size_t k = row.start; k < rEnd; ++k ){
							const T& d1 = setB[ k ];
							if( d1.pt.x < minX )
								continue;
							if( d1.pt.x > maxX )
								break;
							// Match
							float distance = dfunc( d0, d1 );
							if( distance < m.distance ) {
								m.dstIdx = k;
								m.distance = distance;
							}
						}
					}
				}

				if( m.distance < maxDescDistance ){
					matches.push_back( m );
				}
			}
		}

		template<typename T, typename DFUNC>
		static inline void scanLineMatch( std::vector<FeatureMatch>& matches,
										  const std::vector<const FeatureDescriptor*>& left,
										  const std::vector<T>& right,
										  DFUNC dfunc,
										  float minDisp,
										  float maxDisp,
										  float maxDescDist,
										  float maxLineDist )
		{
			matches.reserve( left.size() );
			FeatureMatch m;
			for( size_t i = 0; i < left.size(); ++i ){
				const T* d = ( const T* )left[ i ];
				m.distance = maxDescDist;
				m.feature0 = d;
				m.feature1 = 0;

				for( size_t k = 0; k < right.size(); ++k ){
					const T& dr = right[ k ];
					float yDist = Math::abs( d->pt[ 1 ] - dr.pt[ 1 ] );
					if( yDist < maxLineDist &&
                        d->octave == dr.octave /*&&
                        Math::abs( d->angle - dr.angle ) < 0.1f*/ ){
						float disp = d->pt[ 0 ] - dr.pt[ 0 ];
						if( disp > minDisp && disp < maxDisp ){
							float descDist = dfunc( *d, dr );
							if( descDist < m.distance ){
								m.distance = descDist;
								m.feature1 = &dr;
							}
						}
                    }
				}
				if( m.distance < maxDescDist ){
					matches.push_back( m );
				}
			}
		}

        template<typename T, typename DFUNC>
        static inline void scanLineMatch( std::vector<FeatureMatch>& matches,
                                          const RowLookupTable& rlt,
                                          const std::vector<const FeatureDescriptor*>& left,
                                          const std::vector<T>& right,
                                          DFUNC dfunc,
                                          float minDisp,
                                          float maxDisp,
                                          float maxDescDist,
                                          float maxLineDist )
        {
            matches.reserve( left.size() );
            FeatureMatch m;
            for( size_t i = 0; i < left.size(); ++i ){
                const T* d = ( const T* )left[ i ];
                m.distance = maxDescDist;
                m.feature0 = d;
                m.feature1 = 0;

                float minX = d->pt.x - maxDisp;
                float maxX = d->pt.x - minDisp;
                float minY = d->pt.y - maxLineDist;
                float maxY = d->pt.y + maxLineDist;

                for( int y = minY; y < maxY; ++y ){
                    if( rlt.isValidRow( y ) ){
                        const RowLookupTable::Row& row = rlt.row( y );
                        int k = row.start;
                        size_t rEnd = k + row.len;
                        for( size_t k = row.start; k < rEnd; ++k ){
                            const T& dr = right[ k ];
                            if( dr.pt.x < minX )
                                continue;
                            if( dr.pt.x > maxX )
                                break;

                            // Match
                            float distance = dfunc( *d, dr );
                            if( distance < m.distance ) {
                                m.feature1 = &dr;
                                m.distance = distance;
                            }
                        }
                    }
                }
                if( m.distance < maxDescDist ){
                    matches.push_back( m );
                }
            }
        }
	}
}

#endif
