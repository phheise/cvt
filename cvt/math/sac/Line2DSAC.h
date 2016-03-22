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
 * File:   Line2DSAC.h
 * Author: sebi, heise
 *
 * Created on July 19, 2011, 1:34 PM
 */

#ifndef CVT_LINE2DSAC_H
#define	CVT_LINE2DSAC_H

#include <cvt/math/sac/SampleConsensusModel.h>
#include <cvt/geom/Line2D.h>
#include <Eigen/SVD>

namespace cvt
{
    class Line2DSAC;

    template<>
    struct SACModelTraits<Line2DSAC>
    {
        typedef Line2Df    ResultType;
        typedef float      DistanceType;
    };

    class Line2DSAC : public SampleConsensusModel<Line2DSAC>
{
      public:
        typedef SACModelTraits<Line2DSAC>::ResultType       ResultType;
        typedef SACModelTraits<Line2DSAC>::DistanceType     DistanceType;


        Line2DSAC( const std::vector<Vector2f>& pts ) : _points( pts )
        {
        }

        size_t size() const { return _points.size(); }

        size_t minSampleSize() const { return 2; }

        ResultType estimate( const std::vector<size_t> & sampleIndices ) const;        
        ResultType refine( const ResultType& res, const std::vector<size_t> & inliers  ) const;

        void inliers( std::vector<size_t> & inlierIndices, const ResultType & estimate, const DistanceType maxDistance ) const;

      private:
        const std::vector<Vector2f>&    _points;
    };

    inline Line2DSAC::ResultType Line2DSAC::estimate( const std::vector<size_t>& sampleIndices ) const
    {
        const Vector2f & p0 = _points[ sampleIndices[ 0 ] ];
        const Vector2f & p1 = _points[ sampleIndices[ 1 ] ];

        return Line2Df( p0, p1 );
    }

    inline Line2DSAC::ResultType Line2DSAC::refine( const ResultType& res, const std::vector<size_t> & inliers  ) const
    {
        Eigen::Matrix3f cov( Eigen::Matrix3f::Zero() );

        for( size_t i = 0; i < inliers.size(); i++ ){
            const Vector2f & p = _points[ inliers[ i ] ];
            cov( 0, 0 ) += Math::sqr( p.x );
            cov( 1, 1 ) += Math::sqr( p.y );
            cov( 0, 1 ) += p.x * p.y;
            cov( 0, 2 ) += p.x;
            cov( 1, 2 ) += p.y;
        }

        cov( 1, 0 ) = cov( 0, 1 );
        cov( 2, 0 ) = cov( 0, 2 );
        cov( 2, 1 ) = cov( 1, 2 );
        cov( 2, 2 ) = inliers.size();

		Eigen::JacobiSVD<Eigen::Matrix3f> svd( cov, Eigen::ComputeFullU | Eigen::ComputeFullV );
        Eigen::Vector3f l = svd.matrixU().col( 2 );

        return Line2Df( Vector3f( l[ 0 ], l[ 1 ], l[ 2 ] ) );
    }

    void Line2DSAC::inliers( std::vector<size_t> & inlierIndices,
                             const Line2DSAC::ResultType & estimate,
                             const Line2DSAC::DistanceType maxDistance ) const
    {
        for( size_t i = 0; i < _points.size(); i++ ){
            if( Math::abs( estimate.distance( _points[ i ] ) ) < maxDistance )
                inlierIndices.push_back( i );
        }
    }



}


#endif

