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

/*
 * Author: Sebastian Klose
 *
 * Created on July 20, 2011, 10:05 AM
 */

#ifndef CVT_P3PSAC_H
#define	CVT_P3PSAC_H

#include <cvt/math/sac/SampleConsensusModel.h>
#include <cvt/math/Matrix.h>
#include <cvt/vision/Vision.h>
#include <cvt/vision/PointCorrespondences3d2d.h>
#include <cvt/math/LevenbergMarquard.h>
#include <cvt/util/EigenBridge.h>

namespace cvt
{
	template <class T> class P3PSac;

    template<>
	struct SACModelTraits<P3PSac<double> >
    {
        typedef Matrix4d   ResultType;
        typedef double     DistanceType;
    };

	template<>
	struct SACModelTraits<P3PSac<float> >
	{
		typedef Matrix4f	ResultType;
		typedef float		DistanceType;
	};

	template <class T>
	class P3PSac : public SampleConsensusModel<P3PSac<T> >
	{
		public:
			typedef typename SACModelTraits<P3PSac<T> >::ResultType       ResultType;
			typedef typename SACModelTraits<P3PSac<T> >::DistanceType     DistanceType;


		P3PSac( const PointSet<3, T> & p3d, const PointSet<2, T> & p2d, const Matrix3<T> & K, const Matrix3<T>& Kinv );

        size_t size() const { return _points3d.size(); }

        size_t minSampleSize() const
        {
            return 3;
        }

        ResultType estimate( const std::vector<size_t> & sampleIndices ) const;
        ResultType estimateWithInliers( const ResultType& prev, const std::vector<size_t> & sampleIndices ) const;

        ResultType refine( const ResultType& res, const std::vector<size_t> & inlierIndices ) const;

        void inliers( std::vector<size_t> & inlierIndices, const ResultType & estimate, const DistanceType maxDistance ) const;

      private:
		const PointSet<3, T> &    _points3d;
		const PointSet<2, T> &    _points2d;
		const Matrix3<T>	 &    _intrinsics;
		const Matrix3<T>	 &    _intrinsicsInv;
    };

        
	template <class T>
	inline P3PSac<T>::P3PSac( const PointSet<3, T> & p3d,
							  const PointSet<2, T> & p2d,
							  const Matrix3<T> & K,
							  const Matrix3<T>& Kinv ) :
		_points3d( p3d ),
		_points2d( p2d ),
		_intrinsics( K ),
		_intrinsicsInv( Kinv )
    {
    }

	template <class T>
	inline typename P3PSac<T>::ResultType P3PSac<T>::estimate( const std::vector<size_t> & sampleIndices ) const
    {
		PointSet<3, T> worldPts;
		PointSet<3, T> featureVecs;

		Vector3<T> tmp;
		Vector3<T> tmp2;
        for( size_t i = 0; i < sampleIndices.size(); i++ ){
            worldPts.add( _points3d[ sampleIndices[ i ] ] );
            tmp2.x = _points2d[ sampleIndices[ i ] ].x;
            tmp2.y = _points2d[ sampleIndices[ i ] ].y;
            tmp2.z = 1;
            tmp = _intrinsicsInv * tmp2;
			tmp.normalize();
            featureVecs.add( tmp );
        }
	
		std::vector<Matrix4<T> > results;
		Vision::p3p( results, ( const Vector3<T>* )featureVecs.ptr(), ( const Vector3<T>* )worldPts.ptr() );

		Vector3<T> pcam;
		Vector2<T> pp;

        double best = 10000;
        int bestIdx = -1;

        for( size_t i = 0; i < results.size(); i++ ){
            pcam = results[ i ] * worldPts[ 0 ];
            pp = _intrinsics * pcam;
            pp -= _points2d[ sampleIndices[ 0 ] ];
            double error = pp.lengthSqr();

            if( error < best ){
                best = error;
                bestIdx = i;
            }
        }

        if( bestIdx == -1 ){
			Matrix4<T> trans;
            trans.setIdentity();
            return trans;
        }

        return results[ bestIdx ];
    }

	template <class T>
	inline typename P3PSac<T>::ResultType P3PSac<T>::refine( const ResultType& res, const std::vector<size_t> & inlierIndices ) const
    {
		Eigen::Matrix<T, 3, 3> K;
		Eigen::Matrix<T, 4, 4> ext( Eigen::Matrix<T, 4, 4>::Identity() ), me;

		PointCorrespondences3d2d<T> pointCorresp( K, ext );
        EigenBridge::toEigen( me, res );
        pointCorresp.setPose( me );

		Eigen::Matrix<T, 3, 1> p3;
		Eigen::Matrix<T, 2, 1> p2;
        for( size_t i = 0; i < inlierIndices.size(); i++ ){
            size_t idx = inlierIndices[ i ];
           p3[ 0 ] = _points3d[ idx ].x;
           p3[ 1 ] = _points3d[ idx ].y;
           p3[ 2 ] = _points3d[ idx ].z;

           p2[ 0 ] = _points2d[ idx ].x;
           p2[ 1 ] = _points2d[ idx ].y;
           pointCorresp.add( p3, p2 );
        }

		RobustHuber<T, typename PointCorrespondences3d2d<T>::MeasType> costFunction( 1.0 );
		LevenbergMarquard<T> lm;
		TerminationCriteria<T> termCriteria( TERM_COSTS_THRESH | TERM_MAX_ITER );
		termCriteria.setCostThreshold( (T)0.1 );
        termCriteria.setMaxIterations( 20 );
        lm.optimize( pointCorresp, costFunction, termCriteria );

        me = pointCorresp.pose().transformation();

		Matrix4<T> refined;
        EigenBridge::toCVT( refined, me );
        return refined;
    }

	template <class T>
	inline void P3PSac<T>::inliers( std::vector<size_t> & inlierIndices,
									const ResultType & estimate,
									const DistanceType maxDistance ) const
    {
        // reproject all matches and compute the inliers
		Vector2<T> p2;
		Vector3<T> p3;

		/* invert the pose */
		Matrix3<T> R = _intrinsics * estimate.toMatrix3();
		Vector3<T> t( estimate[ 0 ][ 3 ], estimate[ 1 ][ 3 ], estimate[ 2 ][ 3 ] );
		// apply intrinsics
		t = _intrinsics * t;

        for( size_t i = 0; i < _points3d.size(); i++ ){
            // calc p' = estimate * p
            p3 = R * _points3d[ i ] + t;

			if( Math::abs( p3.z ) < (T)1e-6 )
				continue;

			p2.x = p3.x / p3.z;
			p2.y = p3.y / p3.z;

            if( ( p2 - _points2d[ i ] ).length() < maxDistance )
                inlierIndices.push_back( i );
        }
    }
}

#endif

