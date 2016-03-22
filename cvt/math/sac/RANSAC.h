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
 * File:   RANSAC.h
 * Author: sebi, heise
 *
 * Created on July 19, 2011, 1:34 PM
 */

#ifndef CVT_RANSAC_H
#define	CVT_RANSAC_H

#include <cvt/math/Math.h>
#include <cvt/math/sac/SampleConsensusModel.h>

namespace cvt
{
    template <class Model>
    class RANSAC
    {
      public:
        typedef typename Model::ResultType   ResultType;
        typedef typename Model::DistanceType DistanceType;

        RANSAC( SampleConsensusModel<Model> & model,
                DistanceType maxDistance,
                float outlierProb = 0.05f ) :
            _model( model ), _maxDistance( maxDistance ), _outlierProb( outlierProb )
        {
        }

        ResultType estimate( size_t maxIter = 0 );

		const std::vector<size_t> &  inlierIndices() const { return _lastInliers; }

      private:
        SampleConsensusModel<Model>&  _model;

        DistanceType                  _maxDistance;
        float                         _outlierProb;
        std::vector<size_t>           _lastInliers;

        void randomSamples( std::vector<size_t> & indices );
    };

    template<class Model>
    inline typename RANSAC<Model>::ResultType RANSAC<Model>::estimate( size_t maxIter )
    {
        size_t n = ( size_t )-1;
        size_t samples = 0;

        ResultType result;

        std::vector<size_t> indices;        

        std::vector<size_t> bestIndices;
        size_t numBest = 0;

        if( maxIter )
            n = maxIter;

        while( n > samples ){
            randomSamples( indices );
            result = _model.estimate( indices );

            _model.inliers( _lastInliers, result, _maxDistance );

            if( _lastInliers.size() > numBest ){
                numBest = _lastInliers.size();
                bestIndices = indices;

                float epsilon = 1.0f - ( float )_lastInliers.size() / ( float )_model.size();

                size_t newn = Math::log( _outlierProb ) / Math::log( 1.0f - Math::pow( 1.0f - epsilon, ( float )_model.minSampleSize() ) );

                if( maxIter > 0 ){
					if( newn < maxIter ){
						n = newn;
					}
				} else {
                    n = newn;
				}
            }


            samples++;
        }

        result = _model.estimate( bestIndices );
        _model.inliers( _lastInliers, result, _maxDistance );

        return _model.refine( result, _lastInliers );
    }

    template<class Model>
    inline void RANSAC<Model>::randomSamples( std::vector<size_t> & indices )
	{
        indices.clear();

		size_t idx;
		while( indices.size() < _model.minSampleSize() ){
			idx = Math::rand( 0, _model.size() - 1 );

            bool insert = true;
            for( size_t i = 0; i < indices.size(); i++ ){
                if( idx == indices[ i ] ){
                    insert = false;
                    break;
                }
            }

            if( insert ){
                indices.push_back( idx );
			}
		}
	}
}

#endif	/* RANSAC_H */

