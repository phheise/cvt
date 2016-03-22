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

#ifndef CVT_FEATURE2DROC_H
#define CVT_FEATURE2DROC_H

#include <cvt/vision/features/FeatureMatch.h>
#include <cvt/math/Vector.h>
#include <cvt/math/Matrix.h>
#include <cvt/util/String.h>

namespace cvt {
	class Feature2DROC {
		public:
			Feature2DROC( const std::vector<FeatureMatch>& matches, const Matrix3f& gthomography, float threshold = 5.0f );

			/* compute averave roc of several input matches */
			Feature2DROC( float threshold = 5.0f );

			void addData( const std::vector<FeatureMatch> & matches, const Matrix3f& homography );

			float AUC() const;
			void toFile( const String& path ) const;
			void clear();
			
			void calcRocPoints(); 

		private:

			float				 _gtthres; // Max distance for match || H * x - x' || < _gtthres -> match
			float				 _maxdistance; // max distance
			std::vector<Point2f> _rocpts;

			std::vector<bool>	 _isMatch;
			std::vector<float>	 _distances;
	};


	inline Feature2DROC::Feature2DROC( const std::vector<FeatureMatch>& matches, const Matrix3f& gth, float threshold ) :
		_gtthres( threshold ),
		_maxdistance( 0 )
	{
		addData( matches, gth );
		calcRocPoints();
	}


	inline Feature2DROC::Feature2DROC( float threshold ):
		_gtthres( threshold ),
		_maxdistance( 0 )
	{
	}

	inline void Feature2DROC::addData( const std::vector<FeatureMatch> & matches, const Matrix3f& homography )
	{
		std::vector<FeatureMatch>::const_iterator it = matches.begin();
		std::vector<FeatureMatch>::const_iterator itEnd = matches.end();

		while( it != itEnd ){
			Vector2f gt = homography * it->feature0->pt;

			float dist = ( it->feature1->pt - gt ).length();
			if( dist < _gtthres )
				_isMatch.push_back( true );
			else
				_isMatch.push_back( false );

			_distances.push_back( it->distance );

			if( it->distance > _maxdistance )
				_maxdistance = it->distance;
			it++;
		}
	}

	inline void Feature2DROC::clear()
	{
		_maxdistance = 0;
		_rocpts.clear();
		_isMatch.clear();
		_distances.clear();
	}

	inline void Feature2DROC::calcRocPoints()
	{
		float threshold;
		for( size_t i = 0; i < 102; i++ ) {
			threshold = ( _maxdistance / 100.0f ) * ( float ) i;

			size_t tp = 0;
			size_t actualCorrect = 0;
			size_t fp = 0;
			size_t actualError = 0;

			for( size_t k = 0; k < _distances.size(); k++ ) {
				if( _isMatch[ k ] ) {
					actualCorrect++;
					if( _distances[ k ] < threshold )
						tp++;
				} else {
					actualError++;
					if( _distances[ k ] < threshold )
						fp++;
				}
			}
			Vector2f rocpt( ( float ) tp / ( float ) actualCorrect, ( float ) fp / ( float ) actualError );
			_rocpts.push_back( rocpt );
		}
	}

	inline void Feature2DROC::toFile( const String& path ) const
	{
		FILE* f = fopen( path.c_str(), "w" );
		if( !f )
			return;
		for( size_t i = 0; i < _rocpts.size(); i++ )
			fprintf( f, "%.10f %.10f\n", _rocpts[ i ].y, _rocpts[ i ].x );
		fclose( f );
	}

	inline float Feature2DROC::AUC() const
	{
		float auc=0;
		Point2f diff;
		for( size_t i = 1; i < _rocpts.size(); i++ ) {
			diff = _rocpts[ i ] - _rocpts[ i - 1 ];
			auc = auc + diff.y * _rocpts[ i - 1 ].x + diff.x * diff.y / 2.0f;
		}
		return auc;
	}
}

#endif
