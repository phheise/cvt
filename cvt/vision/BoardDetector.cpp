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

#include "BoardDetector.h"

namespace cvt {

struct ClosestPointCmp
{
	ClosestPointCmp( const Vector2f& pt ) : _pt( pt )
	{
	}

	bool operator()( const Ellipsef& a, const Ellipsef& b )
	{
		Vector2f ca = a.center() - _pt;
		Vector2f cb = b.center() - _pt;

		return ca.lengthSqr() < cb.lengthSqr();
	}

	Vector2f _pt;
};

struct AngularPointCmp
{
	AngularPointCmp( const Vector2f& pt ) : _pt( pt )
	{
	}

	bool operator()( const Ellipsef& a, const Ellipsef& b )
	{
		Vector2f ca = a.center() - _pt;
		Vector2f cb = b.center() - _pt;

		return Math::atan2( ca.y, ca.x ) < Math::atan2( cb.y, cb.x );
	}

	Vector2f _pt;
};

struct DirectionPointCmp
{
	DirectionPointCmp( const Vector2f& pt ) : _pt( pt )
	{
	}

	bool operator()( const Ellipsef& a, const Ellipsef& b )
	{
		Vector2f ca = a.center();
		Vector2f cb = b.center();

		return ca * _pt < cb * _pt;
	}

	Vector2f _pt;
};

struct DirectionLineCmp
{
	DirectionLineCmp( const Vector2f& pt ) : _pt( pt )
	{
	}

	bool operator()( const std::vector<Ellipsef> &a, const std::vector<Ellipsef> &b )
	{
		Vector2f ca = a[0].center();
		Vector2f cb = b[0].center();

		return ca * _pt < cb * _pt;
	}

	Vector2f _pt;
};



	bool BoardDetector::detectACirclePattern( PointSet2f& boardPattern, const Image& im, size_t dotCount1, size_t dotCount2 )
	{
		Image in1, in2;
		std::vector<Ellipsef> ellipses;
		std::vector< std::vector<Ellipsef> > lines;

		im.convert( in1, IFormat::GRAY_FLOAT );
		in1.canny( in2, 0.05f, 0.3f );

		IComponents<float> comps( in2 );

		for( size_t i = 0; i < comps.size(); i++ ) {
			Ellipsef ellipse;
			float fit;
			fit = comps[ i ].fitEllipse( ellipse );
			if( !Math::isNaN( fit ) && fit < 0.1f && ellipse.semiMajor() / ellipse.semiMinor() < 1.5f && ellipse.area() > 5.0f && ellipse.area() < 2500 ) {
				ellipses.push_back( ellipse );
			}
		}

		if ( ellipses.size() < dotCount1 * dotCount2 )
			return false;

		std::vector<Ellipsef> set1;
		std::vector<Ellipsef> set2;
		std::vector<Ellipsef> set3;
		std::vector<bool>     added;


		for( size_t i = 0; i < ellipses.size(); i++ )
		{
			set1.push_back( ellipses[ i ] );
			set2.push_back( ellipses[ i ] );
			set3.push_back( ellipses[ i ] );
			added.push_back( false );
		}

		PointSet2f model;
		PointSet2f data;

		model.add( Vector2f(  0.5,  0.5 ) );
		model.add( Vector2f( -0.5,  0.5 ) );
		model.add( Vector2f( -0.5, -0.5 ) );
		model.add( Vector2f(  0.5, -0.5 ) );

		for ( size_t k = 0; k < set1.size(); ++k ) {
			size_t size = 5;

			partial_sort( set2.begin(), set2.begin() + size, set2.end(), ClosestPointCmp( set1[ k ].center() ) );
			sort( set2.begin() + 1, set2.begin() + size, AngularPointCmp( set1[ k ].center() ) );

			for ( size_t i = 1; i < 5; ++i )
			{
				Vector2f center = set2[i].center();
				data.add( center );
			}

			Matrix3<float> homography = model.alignPerspective( data );

			Vector2f point = homography * Vector2f( 0, 0 );
			float error = ( point - set2[0].center() ).length();
			float distance = ( data[0] - data[1] ).length();
			float maxError = 0.1f * distance;

			if ( error < maxError ) {
				for ( size_t i = 0; i < 4; ++i ) {
					lines.push_back( std::vector< Ellipsef>() );
					Line2Df line( data[i], data[ (i+1) % 4 ] );

					std::vector<size_t> indices;
					bool found = true;

					for ( size_t j = 0; j < set3.size() && found; ++j ) {
						float distance = fabs( line.distance( set3[j].center() ) );

						if ( distance < maxError ) {
							found = found && !added[ j ];
							indices.push_back( j );
							lines.back().push_back( set3[ j ] );
						}
					}

					if ( lines.back().size() == dotCount1 && found ) {
						for( size_t i = 0; i < indices.size(); i++ ) {
							added[ indices[i] ] = true;
						}
					} else {
						lines.pop_back();
					}
				}
			}

			data.clear();
		}

		if ( lines.size() > 2 )	{
			Vector2f dir( 0, 0 );
			for( size_t i = 0; i < lines[0].size(); i++ )
				dir += lines[2][i].center() - lines[0][i].center();
			dir.normalize();

			dir = Vector2f( -dir.y, dir.x );

			for( size_t i = 0; i < lines.size(); i++ )
				sort( lines[i].begin(), lines[i].end(), DirectionPointCmp( dir ) );

			dir = Vector2f( dir.y, -dir.x );

			sort( lines.begin(), lines.end(), DirectionLineCmp( dir ) );

			std::vector< Ellipsef > data1;

			data1.push_back( lines[0][0] );
			data1.push_back( lines[0][1] );
			data1.push_back( lines[2][0] );
			data1.push_back( lines[2][1] );

			sort( data1.begin(), data1.end(), AngularPointCmp( lines[ 1 ][ 0 ].center() ) );

			data.add( data1[0].center() );
			data.add( data1[1].center() );
			data.add( data1[2].center() );
			data.add( data1[3].center() );

			Matrix3<float> homography = model.alignPerspective( data );
			Vector2f point = homography * Vector2f( 0, 0 );
			float error = ( point - lines[1][0].center() ).length();
			float distance = (data[0] - data[1]).length();
			float maxError = 0.1f * distance;

			if ( error > maxError )	{
				for( size_t i = 0; i < lines.size(); i++ ) {
					reverse( lines[i].begin(), lines[i].end() );
				}
			}

			Vector2f a = lines[0][2].center() - lines[0][0].center();
			Vector2f b = lines[2][0].center() - lines[0][0].center();

			if ( a.x * b.y - b.x * a.y < 0 )
				reverse( lines.begin(), lines.end() );
		}

		boardPattern.clear();
		size_t rows = lines.size();
		if( rows != dotCount2 )
			return false;

		for( size_t i = 0; i < lines.size(); i++ ) {
			int cols = lines[i].size();
			for ( int j = 0; j < cols; j++ ) {
				boardPattern.add( lines[i][j].center() );
			}
		}
		return true;
	}


}
