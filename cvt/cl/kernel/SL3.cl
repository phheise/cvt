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
#ifndef CL_SL3_CL
#define CL_SL3_CL

#import "cvt/cl/kernel/Matrix3.cl"

typedef float8 SL3;

inline SL3 SL3_identity()
{
    return ( float8 ) 0.0f;
}

inline SL3 SL3_translation( float2 t )
{
    // Verified by maxima
    return ( float8 ) ( t.x, t.y, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f );
}

inline SL3 SL3_compose( const SL3 h1, const SL3 h2 )
{
    return h1 + h2;
}

inline SL3 SL3_inverse( const SL3 h )
{
    return -h;
}

inline void SL3_jacobian( float8* dx, float8* dy, const Mat3f* h, const float2 pt )
{
#if 0
    float3 pp = mat3f_transform( h, ( float3 ) ( pt, 1.0f ) );
    pp.xy /= pp.z;

    *dx = ( ( float8 ) ( 1.0f, 0.0f, pt.y, 0.0f,  pt.x, -pp.x       , -pp.x * pt.x, -pp.x * pt.y ) ) / pp.z;
    *dy = ( ( float8 ) ( 0.0f, 1.0f, 0.0f, pt.x, -pt.y, -pt.y - pp.y, -pp.y * pt.y, -pp.y * pt.y ) ) / pp.z;
#else
    float2 pp = mat3f_transform_proj2( h, pt );

    *dx = ( ( float8 ) ( 1.0f, 0.0f, pp.y, 0.0f,  pp.x, -pp.x       , -pp.x * pp.x, -pp.x * pp.y ) );
    *dy = ( ( float8 ) ( 0.0f, 1.0f, 0.0f, pp.x, -pp.y, -2.0f * pp.y, -pp.y * pp.y, -pp.y * pp.y ) );

#endif
}

inline void SL3_to_matrix( Mat3f* mat, const SL3 sl )
{
    // Pade approximation
    Mat3f A;
    mat3f_set( &A, sl.s4, sl.s2, sl.s0,
                   sl.s3, - sl.s4 - sl.s5, sl.s1,
                   sl.s6, sl.s7, sl.s5 );

    const int padeApprox = 6;
    float infNorm = max( max( dot( A.m[ 0 ], ( float3 ) 1.0f ), 0.0f ), max( dot( A.m[ 1 ], ( float3 ) 1.0f ), dot( A.m[ 2 ], ( float3 ) 1.0f ) ) );
    int j = max( 0, 1 + ( int ) log2( infNorm ) );

    // tmpA = A * 2^j
    Mat3f tmpA, D, N, X, cX;

    mat3f_mul_scalar( &tmpA, 1.0f / ( float )( 1 << j ), &A );

    mat3f_set_identity( &D );
    mat3f_set_identity( &N );
    mat3f_set_identity( &X );

    float c = 1.0f;
    float s = -1.0f;
    int q = padeApprox;
    int twoq = ( padeApprox << 1 );

    for( int k = 1; k < padeApprox; k++ ){
        c *= ( float ) q / ( float )( twoq * k );
        mat3f_mul( &X, &tmpA, &X );
        mat3f_mul_scalar( &cX, c, &X );
        mat3f_add( &N, &N, &cX );
        mat3f_mad_scalar( &D, s, &cX );
        q--;
        twoq--;
        s *= -1.0f;
    }

    mat3f_inverse( &tmpA, &D );
//    mat3f_assign( mat, &tmpA );
    mat3f_mul( mat, &tmpA, &N );

    for( int k = 0; k < j; k++ ) {
        mat3f_mul( &tmpA, mat, mat );
        mat3f_assign( mat, &tmpA );
    }
}

kernel void SL3_warp_image( write_only image2d_t output, read_only image2d_t image, global SL3* h, global float* out )
{
    const float2 offsethalf = ( float2 ) ( 0.5f, 0.5f );
    const int width = get_image_width( image );
    const int height = get_image_height( image );
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
    const float2 coordf = ( float2 ) ( get_global_id( 0 ), get_global_id( 1 ) );

    if( gx >= width || gy >= height )
        return;

    SL3 sl = *h;
    Mat3f hh;
    SL3_to_matrix( &hh, sl );
    float2 pos2 = mat3f_transform_proj2( &hh, coordf );
    float4 val = read_imagef( image, SAMPLER_BILINEAR, pos2 + offsethalf );
    write_imagef( output, ( int2 ) ( gx, gy ), val );

    *out++ = hh.m[ 0 ].x;
    *out++ = hh.m[ 0 ].y;
    *out++ = hh.m[ 0 ].z;
    *out++ = hh.m[ 1 ].x;
    *out++ = hh.m[ 1 ].y;
    *out++ = hh.m[ 1 ].z;
    *out++ = hh.m[ 2 ].x;
    *out++ = hh.m[ 2 ].y;
    *out++ = hh.m[ 2 ].z;
}

#endif
