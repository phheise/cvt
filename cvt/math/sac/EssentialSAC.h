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

#ifndef CVT_ESSENTIALSAC_H
#define	CVT_ESSENTIALSAC_H

#include <cvt/math/sac/SampleConsensusModel.h>
#include <cvt/math/Matrix.h>
#include <cvt/vision/features/FeatureMatch.h>
#include <cvt/geom/PointSet.h>
#include <cvt/math/Math.h>

namespace cvt
{
    class EssentialSAC;

    template<>
    struct SACModelTraits<EssentialSAC>
    {
        typedef Matrix3f   ResultType;
        typedef float      DistanceType;
    };

    class EssentialSAC : public SampleConsensusModel<EssentialSAC>
	{
		public:
			typedef SACModelTraits<EssentialSAC>::ResultType       ResultType;
			typedef SACModelTraits<EssentialSAC>::DistanceType     DistanceType;


			EssentialSAC( const std::vector<FeatureMatch>& matches, const Matrix3f & K ) : _matches( matches ), _K( K )
			{
				_Kinv = _K;
				if( !_Kinv.inverseSelf() ){
					throw CVTException( "Could not invert intrinsic matrix" );
				}
			}

			size_t size() const { return _matches.size(); }

			size_t minSampleSize() const
			{
				// minimum 8 correspondences to estimate
				// the essential matrix
				return 8;
			}

			ResultType estimate( const std::vector<size_t> & sampleIndices ) const;
            ResultType refine( const ResultType& res, const std::vector<size_t> & inlierIndices ) const;

			void inliers( std::vector<size_t> & inlierIndices, const ResultType & estimate, const DistanceType maxDistance ) const;

		private:
			const std::vector<FeatureMatch>&    _matches;
			Matrix3f							_K;
			Matrix3f							_Kinv;
	};

    inline EssentialSAC::ResultType EssentialSAC::estimate( const std::vector<size_t> & sampleIndices ) const
    {
        PointSet2f set0, set1;
        for( size_t i = 0; i < sampleIndices.size(); i++ ){
            set0.add( _matches[ sampleIndices[ i ] ].feature0->pt );
            set1.add( _matches[ sampleIndices[ i ] ].feature1->pt );
        }

        return set0.essentialMatrix( set1, _K );
    }

    inline EssentialSAC::ResultType EssentialSAC::refine( const ResultType&, const std::vector<size_t> & inlierIndices ) const
    {
        return estimate( inlierIndices );
    }

    inline void EssentialSAC::inliers( std::vector<size_t> & inlierIndices,
                                const ResultType & estimate,
                                const DistanceType maxDistance ) const
    {
		// compute the line for each point, and check the distance of the other point to it:
		Matrix3f funda = _Kinv.transpose() * estimate * _Kinv;
        Vector3f l, tmp;
        for( size_t i = 0; i < _matches.size(); i++ ){
            // calc p' = estimate * p
			tmp[ 0 ] = _matches[ i ].feature0->pt.x;
			tmp[ 1 ] = _matches[ i ].feature0->pt.y;
			tmp[ 2 ] = 1.0f;

			// get the line in the other image
			Line2Df line( funda *  tmp );

            if(  Math::abs( line.distance( _matches[ i ].feature1->pt ) ) < maxDistance )
                inlierIndices.push_back( i );
        }
    }
}

#endif

