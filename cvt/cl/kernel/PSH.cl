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
#ifndef CVT_PSH_CL
#define CVT_PSH_CL

#import "Matrix3.cl"
#import "Matrix4.cl"

/* Plane sweeping homography */

typedef struct {
    Mat3f HR;
    Mat3f HTN;
} PSH;

inline void psh_init( PSH* psh, const Mat3f* K, const Mat3f* Kinv, const Mat4f* RT, float3 normal )
{
    Mat3f tmp, tmp2, R;

    R.m[ 0 ]  = RT->m[ 0 ].xyz;
    R.m[ 1 ]  = RT->m[ 1 ].xyz;
    R.m[ 2 ]  = RT->m[ 2 ].xyz;

    float3 t = ( float3 ) ( RT->m[ 0 ].w, RT->m[ 1 ].w, RT->m[ 2 ].w );

    // Plane induced homography
    // K R K^-1
    mat3f_mul( &tmp, K, &R );
    mat3f_mul( &psh->HR, &tmp, Kinv );

    // K ( t n^T ) K^-1
    mat3f_outer( &tmp2, t, normal );
    mat3f_mul( &tmp, K, &tmp2);
    mat3f_mul( &psh->HTN, &tmp, Kinv );
}

inline void psh_init_transpose_inverse( PSH* psh, const Mat3f* K, const Mat3f* Kinv, const Mat4f* RT, float3 normal )
{
    // To transform lines we need H^-T, we use the Sherman-Morrison formula
    // ( A + uv^T )^-1 = A^-1 - ( A^-1 u v^T A^-1 ) / ( 1 + v^T A^-1 u )
    // H = K ( R + d t n^T ) K^-1
    // H^-T =
    //      = ( K R K^-1 )^T -  d / ( 1 + d * (Rn)^T t ) ( K ( R^T t (Rn)^T ) K^-1 )^T

    Mat3f tmp, tmp2, R;

    R.m[ 0 ]  = RT->m[ 0 ].xyz;
    R.m[ 1 ]  = RT->m[ 1 ].xyz;
    R.m[ 2 ]  = RT->m[ 2 ].xyz;

    float3 t = ( float3 ) ( RT->m[ 0 ].w, RT->m[ 1 ].w, RT->m[ 2 ].w );

    mat3f_mul_transpose( &tmp, K, &R );
    mat3f_mul( &tmp2, &tmp, Kinv ); // K^-1 R K
    mat3f_transpose( &psh->HR, &tmp2 ); // K^T R^T K^-T

    mat3f_outer( &tmp2, mat3f_transpose_transform( &R, t ), mat3f_transform( &R, normal ) );
    mat3f_mul( &tmp, K, &tmp2);
    mat3f_mul( &tmp2, &tmp, Kinv ); // K ( t n^T ) K^-1
    mat3f_transpose( &psh->HTN, &tmp2 );
}

inline void psh_homography( Mat3f* dst, const PSH* psh, float planedepth )
{
    mat3f_assign( dst, &psh->HR );
    mat3f_mad_scalar( dst, -planedepth, &psh->HTN );
}
#endif
