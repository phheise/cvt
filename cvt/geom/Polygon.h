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

#ifndef CVT_POLYGON2D_H
#define CVT_POLYGON2D_H

#include <cvt/math/Math.h>
#include <cvt/math/Vector.h>
#include <cvt/gfx/Path.h>
#include <cvt/geom/CBezier.h>
#include <cvt/geom/QBezier.h>
#include <cvt/geom/Rect.h>
#include <vector>

namespace cvt {
	template<typename T> class CBezier2;
	template<typename T> class QBezier2;

	template<typename T>
	class Polygon {
		public:
			Polygon() {}
			~Polygon() {}
			//FIXME: add copy constructor and assignemnt operator

			const Vector2<T>& operator[]( int index ) const;
			Vector2<T>& operator[]( int index );
			size_t size() const;
			void addPoint( const Vector2<T>& pt );
			void reset();
			Rect<T> bbox() const;
			T area() const;
			Vector2<T> centroid() const;

		private:
			std::vector< Vector2<T> > _pts;
	};

	typedef Polygon<float> Polygonf;
	typedef Polygon<double> Polygond;

	template<typename T>
	class PolygonSet {
		public:
			PolygonSet() {};
			~PolygonSet() {};
			PolygonSet( const Path<T>& path, T tolerance = ( T ) 1  );

			const Polygon<T> operator[]( int index ) const;
			Polygon<T>& operator[]( int index );
			size_t size() const;
			void addPolygon( const Polygon<T>& poly );
		private:
			std::vector< Polygon<T> > _polys;
	};

	typedef PolygonSet<float> PolygonSetf;
	typedef PolygonSet<double> PolygonSetd;

	template<typename T>
	inline size_t Polygon<T>::size() const
	{
		return _pts.size();
	}

	template<typename T>
	inline const Vector2<T>& Polygon<T>::operator[]( int i ) const
	{
		return _pts[ i ];
	}


	template<typename T>
	inline Vector2<T>& Polygon<T>::operator[]( int i )
	{
		return _pts[ i ];
	}

	template<typename T>
	inline void Polygon<T>::addPoint( const Vector2<T>& pt )
	{
		_pts.push_back( pt );
	}

	template<typename T>
	inline void Polygon<T>::reset()
	{
		if( _pts.size() )
			_pts.clear();
	}

	template<typename T>
	inline Rect<T> Polygon<T>::bbox() const
	{
		size_t n = size() - 1;
		Rect<T> rect;

		const Vector2<T>* pt = &_pts[ 0 ];
		rect.set( pt->x, pt->y, 0, 0 );
		pt++;
		while( n-- ) {
			rect.join( *pt++ );
		}

		return rect;
	}

	template<typename T>
	inline T Polygon<T>::area() const
	{
		size_t n = size() - 1;
		T area = 0;

		const Vector2<T>* pt = &_pts[ 0 ];
		while( n-- ) {
			area += pt->x * ( pt + 1 )->y - pt->y * ( pt + 1 )->x;
			pt++;
		}
		area += pt->x * _pts[ 0 ].y - pt->y * _pts[ 0 ].x;
		return area / 2;
	}

	template<typename T>
	inline Vector2<T> Polygon<T>::centroid() const
	{
		size_t n = size() - 1;
		Vector2<T> c( 0, 0 );
		T det;
		T area2 = 0;

		const Vector2<T>* pt = &_pts[ 0 ];
		while( n-- ) {
			det = pt->x * ( pt + 1 )->y - pt->y * ( pt + 1 )->x;
			area2 += det;
			c.x += ( pt->x + ( pt + 1 )->x ) * det;
			c.y += ( pt->y + ( pt + 1 )->y ) * det;
			pt++;
		}
		det = pt->x * _pts[ 0 ].y - pt->y * _pts[ 0 ].x;
		c.x += ( pt->x + _pts[ 0 ].x ) * det;
		c.y += ( pt->y + _pts[ 0 ].y ) * det;

		c /= 3 * area2;
		return c;
	}

	template<typename T>
	inline const Polygon<T> PolygonSet<T>::operator[]( int index ) const
	{
		return _polys[ index ];
	}

	template<typename T>
	inline Polygon<T>& PolygonSet<T>::operator[]( int index )
	{
		return _polys[ index ];
	}

	template<typename T>
	inline size_t PolygonSet<T>::size() const
	{
		return _polys.size();
	}

	template<typename T>
	inline void PolygonSet<T>::addPolygon( const Polygon<T>& poly )
	{
		_polys.push_back( poly );
	}

	template<typename T>
	inline PolygonSet<T>::PolygonSet( const Path<T>& path, T tolerance )
	{
		Vector2<T> current( 0, 0 ); //	needed for relative move, line, curve and the general curve
		typedef typename Path<T>::PathNode PathNode;
		Polygon<T> poly;

		for( int i = 0, end = path.size(); i < end; i++ ) {
			const PathNode& node = path[ i ];
			switch( node.type ) {
				case Path<T>::PATHNODE_LINE:
						poly.addPoint( node.pt[ 0 ] );
						current = node.pt[ 0 ];
						break;
				case Path<T>::PATHNODE_MOVE:
						if( poly.size() > 1 ) {
							addPolygon( poly );
						}
						poly.reset();
						poly.addPoint( node.pt[ 0 ] );
						current = node.pt[ 0 ];
						break;
				case Path<T>::PATHNODE_CLOSE:
						if( poly.size() > 1 ) {
							poly.addPoint( poly[ 0 ] );
							addPolygon( poly );
							current = poly[ 0 ];
						} else
							current.set( 0, 0 );
						poly.reset();
						break;
				case Path<T>::PATHNODE_CURVE:
						if( poly.size() ) {
							CBezier2<T> spline( current, node.pt[ 0 ], node.pt[ 1 ], node.pt[ 2 ] );
							spline.addToPolygon( poly, tolerance );
#if 0
							/* flatten the spline */
							Bezier<T> spline( current, node.pt[ 0 ], node.pt[ 1 ], node.pt[ 2 ] );
							Bezier<T> a, b, c;
							size_t n;
							T t[ 2 ], alpha;

							n = spline.inflectionPoints( t );
							if( n )
								spline.split( a, b, t[ 0 ] );
							else
								a = spline;

							poly.addPoint( a[ 0 ] );
							alpha = a.flattenFirst( tolerance );
							alpha = Math::min( a.flatten( tolerance ), alpha );
							a.remove( alpha );
							poly.addPoint( a[ 0 ] );
							while( 1 ) {
								alpha =  a.flatten( tolerance );
								if( alpha < 1 ) {
									a.remove( alpha );
									poly.addPoint( a[ 0 ] );
								} else {
									poly.addPoint( a[ 3 ] );
									break;
								}
							}

							if( n ) {
								if( n == 2 ) {
									a = b;
									a.split( b, c,  ( t[ 1 ] - t[ 0 ] ) / ( 1.0f - t[ 0 ] ) );
								}
								poly.addPoint( b[ 0 ] );
								alpha =  b.flattenFirst( tolerance );
								b.remove( alpha );
								poly.addPoint( b[ 0 ] );
								while( 1 ) {
									alpha =  b.flatten( tolerance );
									if( alpha < 1 ) {
										b.remove( alpha );
										poly.addPoint( b[ 0 ] );
									} else {
										poly.addPoint( b[ 3 ] );
										break;
									}
								}

								if( n == 2 ) {
									poly.addPoint( c[ 0 ] );
									alpha =  c.flattenFirst( tolerance );
									c.remove( alpha );
									poly.addPoint( c[ 0 ] );
									while( 1 ) {
										alpha =  c.flatten( tolerance );
										if( alpha < 1 ) {
											c.remove( alpha );
											poly.addPoint( c[ 0 ] );
										} else {
											poly.addPoint( c[ 3 ] );
											break;
										}
									}
								}
							}
#endif
						} else {
							poly.addPoint( node.pt[ 2 ] );
						}
						current = node.pt[ 2 ];
						break;
			}
		}
		if( poly.size() > 1 ) {
			addPolygon( poly );
		}

	}
}

#endif
