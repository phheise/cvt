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
#ifndef CL_SE3_CL
#define CL_SE3_CL

#import "Matrix3.cl"
#import "Matrix6.cl"

typedef struct {
    float3 rot;
    float3 trans;
} SE3;

inline SE3 se3_identity()
{
    SE3 ret;
    ret.rot = ( float3 ) 0.0f;
    ret.trans = ( float3 ) 0.0f;
    return ret;
}

/* only the first six entries of dx,dy are valid */
inline void SE3_screen_jacobian( float8* dx, float8* dy, const Mat3f* K, const float3 pt )
{
    float zz = 1.0f / ( pt.z * pt.z );
    float z  = 1.0f / pt.z;
    float xx = pt.x * pt.x;
    float yy = pt.y * pt.y;

    dx->s0 = -K->m[ 0 ].x * pt.x * pt.y * zz;
    dx->s1 =  K->m[ 0 ].x * xx * zz + K->m[ 0 ].x;
    dx->s2 = -K->m[ 0 ].x * pt.y * z;
    dx->s3 =  K->m[ 0 ].x * z;
    dx->s4 =  0.0f;
    dx->s5 = -K->m[ 0 ].x * pt.x * zz;

    dy->s0 = -K->m[ 1 ].y * yy * zz - K->m[ 1 ].y;
    dy->s1 =  K->m[ 1 ].y * pt.x * pt.y * zz;
    dy->s2 =  K->m[ 1 ].y * pt.x * z;
    dy->s3 =  0.0f;
    dy->s4 =  K->m[ 1 ].y * z;
    dy->s5 = -K->m[ 1 ].y * pt.y * zz;
}

inline void SE3_screen_hessian( Mat6f* wx, Mat6f* wy,
                                const float3 pt,
                                const Mat3f* K )
{
    float x = pt.x;
    float y = pt.y;
    float xx = pt.x * pt.x;
    float xxy = xx * y;
    float xy = x * y;
    float yy = pt.y * pt.y;
    float iz = 1.0 / pt.z;
    float izz = 1.0 / ( pt.z * pt.z );
    float izzz = 1.0 /( pt.z * pt.z * pt.z );
    float fx = K->m[ 0 ].x;
    float fy = K->m[ 1 ].y;

    wx->m[ 0 ][ 0 ] = fx*x*iz+2*fx*x*yy*izzz;
    wx->m[ 0 ][ 1 ] = y*(-fx*iz-(2*fx*xx)*izzz)+(0.5*fx*y)*iz;
    wx->m[ 0 ][ 2 ] = fx*yy*izz-(0.5*fx*xx)*izz+0.5*fx;
    wx->m[ 0 ][ 3 ] = -fx*y*izz;
    wx->m[ 0 ][ 4 ] = -0.5*fx*x*izz;
    wx->m[ 0 ][ 5 ] = 2*fx*xy*izzz;

    wx->m[ 1 ][ 0 ] = -0.5*fx*y*iz-2*fx*xxy*izzz;
    wx->m[ 1 ][ 1 ] = fx*x*iz-x*(-fx*iz-(2*fx*xx)*izzz);
    wx->m[ 1 ][ 2 ] = -1.5*fx*xy*izz;
    wx->m[ 1 ][ 3 ] = 1.5*fx*x*izz;
    wx->m[ 1 ][ 4 ] = 0;
    wx->m[ 1 ][ 5 ] = -0.5*fx*iz-2*fx*xx*izzz;

    wx->m[ 2 ][ 0 ] = fx*yy*izz-0.5*fx*xx*izz+0.5*fx;
    wx->m[ 2 ][ 1 ] = -1.5*fx*xy*izz;
    wx->m[ 2 ][ 2 ] = -fx*x*iz;
    wx->m[ 2 ][ 3 ] = 0;
    wx->m[ 2 ][ 4 ] = -0.5*fx*iz;
    wx->m[ 2 ][ 5 ] = fx*y*izz;

    wx->m[ 3 ][ 0 ] = -fx*y*izz;
    wx->m[ 3 ][ 1 ] = 1.5*fx*x*izz;
    wx->m[ 3 ][ 2 ] = 0;
    wx->m[ 3 ][ 3 ] = 0;
    wx->m[ 3 ][ 4 ] = 0;
    wx->m[ 3 ][ 5 ] = -fx*izz;

    wx->m[ 4 ][ 0 ] = -0.5*fx*x*izz;
    wx->m[ 4 ][ 1 ] = 0;
    wx->m[ 4 ][ 2 ] = -0.5*fx*iz;
    wx->m[ 4 ][ 3 ] = 0;
    wx->m[ 4 ][ 4 ] = 0;
    wx->m[ 4 ][ 5 ] = 0;

    wx->m[ 5 ][ 0 ] = 2*fx*xy*izzz;
    wx->m[ 5 ][ 1 ] = -0.5*fx*iz-2*fx*xx*izzz;
    wx->m[ 5 ][ 2 ] = fx*y*izz;
    wx->m[ 5 ][ 3 ] = -fx*izz;
    wx->m[ 5 ][ 4 ] = 0;
    wx->m[ 5 ][ 5 ] = 2*fx*x*izzz;

    wy->m[ 0 ][ 0 ] = y*(fy*iz+(2*fy*yy)*izzz)+fy*y*iz;
    wy->m[ 0 ][ 1 ] = -0.5*fy*x*iz-2*fy*x*yy*izzz;
    wy->m[ 0 ][ 2 ] = -1.5*fy*xy*izz;
    wy->m[ 0 ][ 3 ] = 0;
    wy->m[ 0 ][ 4 ] = -1.5*fy*y*izz;
    wy->m[ 0 ][ 5 ] = 0.5*fy*iz+2*fy*yy*izzz;

    wy->m[ 1 ][ 0 ] = 0.5*fy*x*iz-x*(fy*iz+(2*fy*yy)*izzz);
    wy->m[ 1 ][ 1 ] = fy*y*iz+2*fy*xxy*izzz;
    wy->m[ 1 ][ 2 ] = -0.5*fy*yy*izz+fy*xx*izz+0.5*fy;
    wy->m[ 1 ][ 3 ] = 0.5*fy*y*izz;
    wy->m[ 1 ][ 4 ] = fy*x*izz;
    wy->m[ 1 ][ 5 ] = -2*fy*xy*izzz;

    wy->m[ 2 ][ 0 ] = -1.5*fy*xy*izz;
    wy->m[ 2 ][ 1 ] = -0.5*fy*yy*izz+fy*xx*izz+0.5*fy;
    wy->m[ 2 ][ 2 ] = -fy*y*iz;
    wy->m[ 2 ][ 3 ] = 0.5*fy*iz;
    wy->m[ 2 ][ 4 ] = 0;
    wy->m[ 2 ][ 5 ] = -fy*x*izz;

    wy->m[ 3 ][ 0 ] = 0;
    wy->m[ 3 ][ 1 ] = 0.5*fy*y*izz;
    wy->m[ 3 ][ 2 ] = 0.5*fy*iz;
    wy->m[ 3 ][ 3 ] = 0;
    wy->m[ 3 ][ 4 ] = 0;
    wy->m[ 3 ][ 5 ] = 0;

    wy->m[ 4 ][ 0 ] = -1.5*fy*y*izz;
    wy->m[ 4 ][ 1 ] = fy*x*izz;
    wy->m[ 4 ][ 2 ] = 0;
    wy->m[ 4 ][ 3 ] = 0;
    wy->m[ 4 ][ 4 ] = 0;
    wy->m[ 4 ][ 5 ] = -fy*izz;

    wy->m[ 5 ][ 0 ] = 0.5*fy*iz+2*fy*yy*izzz;
    wy->m[ 5 ][ 1 ] = -2*fy*xy*izzz;
    wy->m[ 5 ][ 2 ] = -fy*x*izz;
    wy->m[ 5 ][ 3 ] = 0;
    wy->m[ 5 ][ 4 ] = -fy*izz;
    wy->m[ 5 ][ 5 ] = 2*fy*y*izzz;
}
#endif
