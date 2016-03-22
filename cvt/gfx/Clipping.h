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

#ifndef CVT_CLIPPING_H
#define CVT_CLIPPING_H

#include <cvt/math/Vector.h>
#include <cvt/geom/Line2D.h>

namespace cvt {
	class Clipping {
		public:
			static bool clip( const Recti& rect, Vector2i& pt1, Vector2i& pt2 );
			static bool clip( const Rectf& rect, Vector2f& pt1, Vector2f& pt2 );
			static bool clip( const Rectf& rect, const Line2Df& line, Vector2f& pt1, Vector2f& pt2 );

		private:
			Clipping();
			Clipping( const Clipping& );
	};

	inline bool Clipping::clip( const Recti& rect, Vector2i& pt1, Vector2i& pt2 )
	{
		int xend, yend;
		int dx, dy, ox, oy;

		/* zero line */
		if( pt1.x == pt2.x && pt1.y == pt2.y )
			return false;

		/* both points inside */
		if( rect.contains( pt1 ) && rect.contains( pt2 ) )
			return true;

		xend = rect.x + rect.width - 1;
		yend = rect.y + rect.height - 1;

		/* both points outside and on the same side of one rect line */
		if( ( pt1.x < rect.x && pt2.x < rect.x ) ||
			( pt1.x > xend && pt2.x > xend ) ||
			( pt1.y < rect.y && pt2.y < rect.y ) ||
			( pt1.y > yend && pt2.y > yend ) )
			return false;

		/* horizontal or vertical lines */
		if( pt1.y == pt2.y ) {
			pt1.x = Math::clamp( pt1.x, rect.x, xend );
			pt2.x = Math::clamp( pt2.x, rect.x, xend );
			return true;
		} else if( pt1.x == pt2.x ) {
			pt1.y = Math::clamp( pt1.y, rect.y, yend );
			pt2.y = Math::clamp( pt2.y, rect.y, yend );
			return true;
		}

		dx = pt2.x - pt1.x;
		dy = pt2.y - pt1.y;
		ox = pt1.x;
		oy = pt1.y;

		/* point 1*/
		if( pt1.x < rect.x ) { /* left of rect, clip to left*/
			pt1.y = pt2.y + ( dy * ( rect.x - pt2.x ) ) / dx;
			pt1.x = rect.x;
		} else if( pt1.x > xend ) { /* right of rect, clip to right*/
			pt1.y = pt2.y + ( dy * ( xend - pt2.x ) ) / dx;
			pt1.x = xend;
		}

		if( pt1.y < rect.y ) { /* above rect, clip to top */
			pt1.x = Math::clamp( pt2.x + ( dx * ( rect.y - pt2.y ) ) / dy, rect.x, xend );
			pt1.y = rect.y;
		} else if( pt1.y > yend ) { /* below rect, clip to bottom */
			pt1.x = Math::clamp( pt2.x + ( dx  * ( yend - pt2.y ) ) / dy, rect.x, xend );
			pt1.y = yend;
		}

		/* zero line */
		if( pt1.x == pt2.x && pt1.y == pt2.y )
			return false;

		/* point 2*/
		if( pt2.x < rect.x ) { /* left of rect, clip to left*/
			pt2.y = oy + ( dy * ( rect.x - ox ) ) / dx;
			pt2.x = rect.x;
		} else if( pt2.x > xend ) { /* right of rect, clip to right*/
			pt2.y = oy + ( dy * ( xend - ox ) ) / dx;
			pt2.x = xend;
		}

		if( pt2.y < rect.y ) { /* above rect, clip to top */
			pt2.x = Math::clamp( ox + ( dx * ( rect.y - oy ) ) / dy, rect.x, xend );
			pt2.y = rect.y;
		} else if( pt2.y > yend ) { /* below rect, clip to bottom */
			pt2.x = Math::clamp( ox + ( dx * ( yend - oy ) ) / dy, rect.x, xend );
			pt2.y = yend;
		}

		/* zero line */
		if( pt1.x == pt2.x && pt1.y == pt2.y )
			return false;

		return true;
	}

	inline bool Clipping::clip( const Rectf& rect, Vector2f& pt1, Vector2f& pt2 )
	{
		float xend, yend;
		float dx, dy, ox, oy;

		/* zero line */
		if( pt1.x == pt2.x && pt1.y == pt2.y )
			return false;

		/* both points inside */
		if( rect.contains( pt1 ) && rect.contains( pt2 ) )
			return true;

		xend = Math::floatPrev( rect.x + rect.width );
		yend = Math::floatPrev( rect.y + rect.height );

		/* both points outside and on the same side of one rect line */
		if( ( pt1.x < rect.x && pt2.x < rect.x ) ||
			( pt1.x > xend && pt2.x > xend ) ||
			( pt1.y < rect.y && pt2.y < rect.y ) ||
			( pt1.y > yend && pt2.y > yend ) )
			return false;

		/* horizontal or vertical lines */
		if( pt1.y == pt2.y ) {
			pt1.x = Math::clamp( pt1.x, rect.x, xend );
			pt2.x = Math::clamp( pt2.x, rect.x, xend );
			return true;
		} else if( pt1.x == pt2.x ) {
			pt1.y = Math::clamp( pt1.y, rect.y, yend );
			pt2.y = Math::clamp( pt2.y, rect.y, yend );
			return true;
		}

		dx = pt2.x - pt1.x;
		dy = pt2.y - pt1.y;
		ox = pt1.x;
		oy = pt1.y;

		/* point 1*/
		if( pt1.x < rect.x ) { /* left of rect, clip to left*/
			pt1.y = pt2.y + ( dy * ( rect.x - pt2.x ) ) / dx;
			pt1.x = rect.x;
		} else if( pt1.x > xend ) { /* right of rect, clip to right*/
			pt1.y = pt2.y + ( dy * ( xend - pt2.x ) ) / dx;
			pt1.x = xend;
		}

		if( pt1.y < rect.y ) { /* above rect, clip to top */
			pt1.x = Math::clamp( pt2.x + ( dx * ( rect.y - pt2.y ) ) / dy, rect.x, xend );
			pt1.y = rect.y;
		} else if( pt1.y > yend ) { /* below rect, clip to bottom */
			pt1.x = Math::clamp( pt2.x + ( dx  * ( yend - pt2.y ) ) / dy, rect.x, xend );
			pt1.y = yend;
		}

		/* zero line */
		if( pt1.x == pt2.x && pt1.y == pt2.y )
			return false;

		/* point 2*/
		if( pt2.x < rect.x ) { /* left of rect, clip to left*/
			pt2.y = oy + ( dy * ( rect.x - ox ) ) / dx;
			pt2.x = rect.x;
		} else if( pt2.x > xend ) { /* right of rect, clip to right*/
			pt2.y = oy + ( dy * ( xend - ox ) ) / dx;
			pt2.x = xend;
		}

		if( pt2.y < rect.y ) { /* above rect, clip to top */
			pt2.x = Math::clamp( ox + ( dx * ( rect.y - oy ) ) / dy, rect.x, xend );
			pt2.y = rect.y;
		} else if( pt2.y > yend ) { /* below rect, clip to bottom */
			pt2.x = Math::clamp( ox + ( dx * ( yend - oy ) ) / dy, rect.x, xend );
			pt2.y = yend;
		}

		/* zero line */
		if( pt1.x == pt2.x && pt1.y == pt2.y )
			return false;

		return true;
	}


	inline bool Clipping::clip( const Rectf& rect, const Line2Df& line, Vector2f& pt1, Vector2f& pt2 )
	{
		/* Vaclav Skala -  A new approach to line and line segment clipping in homogeneous coordinates*/
		int _table[ 16 ][ 2 ] = {
			{ -1, - 1 },
			{  0,   3 },
			{  0,   1 },
			{  1,   3 },
			{  1,   2 },
			{ -1, - 1 },
			{  0,   2 },
			{  2,   3 },
			{  2,   3 },
			{  0,   2 },
			{ -1, - 1 },
			{  1,   2 },
			{  1,   3 },
			{  0,   1 },
			{  0,   3 },
			{ -1, - 1 }
		};
		unsigned int mask = 0;

		mask |= ( line.distance( rect.x, rect.y ) >= 0.0f );
		mask <<= 1;
		mask |= ( line.distance( rect.x + rect.width, rect.y ) >= 0.0f );
		mask <<= 1;
		mask |= ( line.distance( rect.x + rect.width, rect.y + rect.height ) >= 0.0f );
		mask <<= 1;
		mask |= ( line.distance( rect.x, rect.y + rect.height ) >= 0.0f );

		if( mask == 0x0 || mask == 0xf )
			return false;

		int l0 = _table[ mask ][ 0 ];
		int l1 = _table[ mask ][ 1 ];

		switch( l0 ) {
			case 0:
				{
					Line2Df line2( Vector3f( 0, 1, -( rect.y + rect.height ) ) );
					line2.intersect( line, pt1 );
					break;
				}
			case 1:
				{
					Line2Df line2( Vector3f( 1, 0, -( rect.x + rect.width ) ) );
					line2.intersect( line, pt1 );
					break;
				}
			case 2:
				{
					Line2Df line2( Vector3f( 0, 1, -rect.y ) );
					line2.intersect( line, pt1 );
					break;
				}
			case 3:
				{
					Line2Df line2( Vector3f( 1, 0, -( rect.x ) ) );
					line2.intersect( line, pt1 );
					break;
				}
			default:
				break;
		}


		switch( l1 ) {
			case 0:
				{
					Line2Df line2( Vector3f( 0, 1, -( rect.y + rect.height ) ) );
					line2.intersect( line, pt2 );
					break;
				}
			case 1:
				{
					Line2Df line2( Vector3f( 1, 0, -( rect.x + rect.width ) ) );
					line2.intersect( line, pt2 );
					break;
				}
			case 2:
				{
					Line2Df line2( Vector3f( 0, 1, -rect.y ) );
					line2.intersect( line, pt2 );
					break;
				}
			case 3:
				{
					Line2Df line2( Vector3f( 1, 0, -( rect.x ) ) );
					line2.intersect( line, pt2 );
					break;
				}
			default:
				break;
		}
		return true;
	}
}

#endif
