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

#ifndef CVT_QBEZIER_H
#define CVT_QBEZIER_H

#include <cvt/math/Vector.h>
#include <cvt/math/Math.h>
#include <cvt/geom/Polygon.h>

namespace cvt {
	template<typename T> class Polygon;

	template<typename T>
	class QBezier2 {
		public:
			QBezier2();
			QBezier2( const QBezier2<T>& spl );
			QBezier2( T x1, T y1, T dx, T dy, T x2, T y2  );
			QBezier2( const Vector2<T>& p1, const Vector2<T>& d, const Vector2<T>& p2 );

			QBezier2<T>& operator=( const QBezier2<T>& spl );

			void set( T x1, T y1, T dx, T dy, T x2, T y2  );
			void set( const Vector2<T>& p1, const Vector2<T>& d, const Vector2<T>& p2 );

			Vector2<T>& operator[]( int x );
			const Vector2<T>& operator[]( int x ) const;

			void samplePoint( Vector2<T>& pt, T t );
			void sampleDerivative( Vector2<T>& pt, T t );

			void addToPolygon( Polygon<T>& poly, T tolerance = ( T ) 1 / ( T ) 2 );

			void remove( T t );
			void split( QBezier2<T>& out1, QBezier2<T>& out2, T t ) const;
			void splitHalf( QBezier2<T>& out1, QBezier2<T>& out2 ) const;

		private:
			Vector2<T> _pts[ 3 ];
	};

	template<typename T>
	inline QBezier2<T>::QBezier2()
	{
	}

	template<typename T>
	inline QBezier2<T>::QBezier2( const QBezier2<T>& spl )
	{
		_pts[ 0 ] = spl._pts[ 0 ];
		_pts[ 1 ] = spl._pts[ 1 ];
		_pts[ 2 ] = spl._pts[ 2 ];
	}

	template<typename T>
	QBezier2<T>::QBezier2( T x1, T y1, T dx1, T dy1, T x2, T y2  )
	{
		_pts[ 0 ].x = x1;
		_pts[ 0 ].y = y1;
		_pts[ 1 ].x = dx1;
		_pts[ 1 ].y = dy1;
		_pts[ 2 ].x = x2;
		_pts[ 2 ].y = y2;
	}

	template<typename T>
	QBezier2<T>::QBezier2( const Vector2<T>& p1, const Vector2<T>& d1, const Vector2<T>& p2 )
	{
		_pts[ 0 ] = p1;
		_pts[ 1 ] = d1;
		_pts[ 2 ] = p2;
	}

	template<typename T>
	inline QBezier2<T>& QBezier2<T>::operator=( const QBezier2<T>& spl )
	{
		_pts[ 0 ] = spl._pts[ 0 ];
		_pts[ 1 ] = spl._pts[ 1 ];
		_pts[ 2 ] = spl._pts[ 2 ];
		return *this;
	}

	template<typename T>
	void QBezier2<T>::set( T x1, T y1, T dx1, T dy1, T x2, T y2  )
	{
		_pts[ 0 ].x = x1;
		_pts[ 0 ].y = y1;
		_pts[ 1 ].x = dx1;
		_pts[ 1 ].y = dy1;
		_pts[ 2 ].x = x2;
		_pts[ 2 ].y = y2;
	}

	template<typename T>
	void QBezier2<T>::set( const Vector2<T>& p1, const Vector2<T>& d1, const Vector2<T>& p2 )
	{
		_pts[ 0 ] = p1;
		_pts[ 1 ] = d1;
		_pts[ 2 ] = p2;
	}

	template<typename T>
	const Vector2<T>& QBezier2<T>::operator[]( int x ) const
	{
		return _pts[ x ];
	}

	template<typename T>
	Vector2<T>& QBezier2<T>::operator[]( int x )
	{
		return _pts[ x ];
	}

	template<typename T>
	void QBezier2<T>::samplePoint( Vector2<T>& pt, T t )
	{
		/* t * ( ( p2 - 2 * p1 + p0 ) * t + 2 * ( p1 - p0 ) ) + p0 */
		pt = _pts[ 2 ] - 2 * _pts[ 1 ] + _pts[ 0 ];
		pt *= t;
		pt += 2 * ( _pts[ 1 ] - _pts[ 0 ] );
		pt *= t;
		pt += _pts[ 0 ];
	}

	template<typename T>
	void QBezier2<T>::sampleDerivative( Vector2<T>& pt, T t )
	{
	    /* ( ( p2 - 2 *p1 + p0 ) * t + ( p1- p0 ) ) * 2 */
		pt = _pts[ 2 ] - 2 * _pts[ 1 ] + _pts[ 0 ];
		pt *= t;
		pt += ( _pts[ 1 ] - _pts[ 0 ] );
		pt *= 2;
	}

	template<typename T>
	void QBezier2<T>::remove( T t )
	{
		_pts[ 0 ] = _pts[ 0 ] + ( _pts[ 1 ] - _pts[ 0 ] ) * t;
		_pts[ 1 ] = _pts[ 1 ] + ( _pts[ 2 ] - _pts[ 1 ] ) * t;
		_pts[ 0 ] = _pts[ 0 ] + ( _pts[ 1 ] - _pts[ 0 ] ) * t;
	}

	template<typename T>
	void QBezier2<T>::split( QBezier2<T>& out1, QBezier2<T>& out2, T t ) const
	{
		out1._pts[ 0 ] = _pts[ 0 ]; /*first point from spl to out1*/
		out2._pts[ 2 ] = _pts[ 2 ]; /*last point from spl to out2*/
		out1._pts[ 1 ] = _pts[ 0 ] + ( _pts[ 1 ] - _pts[ 0 ] ) * t;
		out2._pts[ 1 ] = _pts[ 1 ] + ( _pts[ 2 ] - _pts[ 1 ] ) * t;

		out1._pts[ 2 ] = out2._pts[ 0 ] = out1._pts[ 1 ] + ( out2._pts[ 1 ] - out1._pts[ 1 ] ) * t;
	}

	template<typename T>
	void QBezier2<T>::splitHalf( QBezier2<T>& out1, QBezier2<T>& out2 ) const
	{
		T half = ( T ) 1 / ( T ) 2;

		out1._pts[ 0 ] = _pts[ 0 ]; /*first point from spl to out1*/
		out2._pts[ 2 ] = _pts[ 2 ]; /*last point from spl to out2*/
		out1._pts[ 1 ] = _pts[ 0 ] + ( _pts[ 1 ] - _pts[ 0 ] ) * half;
		out2._pts[ 1 ] = _pts[ 1 ] + ( _pts[ 2 ] - _pts[ 1 ] ) * half;

		out1._pts[ 2 ] = out2._pts[ 0 ] = out1._pts[ 1 ] + ( out2._pts[ 1 ] - out1._pts[ 1 ] ) * half;
	}

	template<typename T>
	inline void QBezier2<T>::addToPolygon( Polygon<T>& poly, T tolerance )
	{
		QBezier2<T> stack[ 32 ];
		QBezier2<T>* spl = stack;
		stack[ 0 ] = *this;

		poly.addPoint( _pts[ 0 ] );

		while( spl >= stack ) {
			Vector2<T> mid = ( ( *spl )[ 2 ] + ( *spl )[ 0 ] ) * ( ( T ) 1 / ( T ) 2 );
			T dist = ( ( *spl )[ 1 ] - mid ).lengthSqr();

			if ( dist < tolerance || spl == stack + 31 ) {
				poly.addPoint( ( *spl )[ 2 ] );
				spl--;
			} else {
				spl->splitHalf( *( spl + 1 ), *spl );
				spl++;
			}
		}
	}

	typedef QBezier2<float> QBezier2f;
}

#endif
