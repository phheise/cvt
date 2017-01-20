/*
   The MIT License (MIT)

   Copyright (c) 2011 - 2013, Philipp Heise and Sebastian Klose
   Copyright (c) 2016, BMW Car IT GmbH, Philipp Heise (philipp.heise@bmw.de)

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

#ifndef CVT_LINE_CL
#define CVT_LINE_CL

#import "Rect.cl"

typedef float3 Linef;

inline Linef linef_from_points( float2 p1, float2 p2 )
{
        float3 ret = ( float3 ) ( p1.x, p1.y, 1.0f );
        float3 tmp = ( float3 ) ( p2.x, p2.y, 1.0f );
        ret = cross( ret, tmp );
        ret /= length( ret.xy );
        return ret;
}

inline Linef linef_from_point_normal( float2 point, float2 normal )
{
        float3 ret = ( float3 ) ( normal.x, normal.y, 1.0f );
        ret.z = -dot( ret.xy, point );
        return ret;
}

inline bool line_clip( float2* pt1, float2* pt2, const Rectf rect )
{
    float xend, yend;
    float dx, dy, ox, oy;

    /* zero line */
    if( pt1->x == pt2->x && pt1->y == pt2->y )
        return false;

    /* both points inside */
    if( rectf_contains_point( rect, *pt1 ) && rectf_contains_point( rect, *pt2 ) )
        return true;

    xend = nextafter( rect.x + rectf_width( rect ), rect.x );
    yend = nextafter( rect.y + rectf_height( rect ), rect.y );

    /* both points outside and on the same side of one rect line */
    if( ( pt1->x < rect.x && pt2->x < rect.x ) ||
       ( pt1->x > xend && pt2->x > xend ) ||
       ( pt1->y < rect.y && pt2->y < rect.y ) ||
       ( pt1->y > yend && pt2->y > yend ) )
        return false;

    /* horizontal or vertical lines */
    if( pt1->y == pt2->y ) {
        pt1->x = clamp( pt1->x, rect.x, xend );
        pt2->x = clamp( pt2->x, rect.x, xend );
        return true;
    } else if( pt1->x == pt2->x ) {
        pt1->y = clamp( pt1->y, rect.y, yend );
        pt2->y = clamp( pt2->y, rect.y, yend );
        return true;
    }

    dx = pt2->x - pt1->x;
    dy = pt2->y - pt1->y;
    ox = pt1->x;
    oy = pt1->y;

    /* point 1*/
    if( pt1->x < rect.x ) { /* left of rect, clip to left*/
        pt1->y = pt2->y + ( dy * ( rect.x - pt2->x ) ) / dx;
        pt1->x = rect.x;
    } else if( pt1->x > xend ) { /* right of rect, clip to right*/
        pt1->y = pt2->y + ( dy * ( xend - pt2->x ) ) / dx;
        pt1->x = xend;
    }

    if( pt1->y < rect.y ) { /* above rect, clip to top */
        pt1->x = clamp( pt2->x + ( dx * ( rect.y - pt2->y ) ) / dy, rect.x, xend );
        pt1->y = rect.y;
    } else if( pt1->y > yend ) { /* below rect, clip to bottom */
        pt1->x = clamp( pt2->x + ( dx  * ( yend - pt2->y ) ) / dy, rect.x, xend );
        pt1->y = yend;
    }

    /* zero line */
    if( pt1->x == pt2->x && pt1->y == pt2->y )
        return false;

    /* point 2*/
    if( pt2->x < rect.x ) { /* left of rect, clip to left*/
        pt2->y = oy + ( dy * ( rect.x - ox ) ) / dx;
        pt2->x = rect.x;
    } else if( pt2->x > xend ) { /* right of rect, clip to right*/
        pt2->y = oy + ( dy * ( xend - ox ) ) / dx;
        pt2->x = xend;
    }

    if( pt2->y < rect.y ) { /* above rect, clip to top */
        pt2->x = clamp( ox + ( dx * ( rect.y - oy ) ) / dy, rect.x, xend );
        pt2->y = rect.y;
    } else if( pt2->y > yend ) { /* below rect, clip to bottom */
        pt2->x = clamp( ox + ( dx * ( yend - oy ) ) / dy, rect.x, xend );
        pt2->y = yend;
    }

    /* zero line */
    if( pt1->x == pt2->x && pt1->y == pt2->y )
        return false;

    return true;
}
#endif
