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
#ifndef CL_HOMOGRAPHY_CL
#define CL_HOMOGRAPHY_CL

#import "cvt/cl/kernel/Matrix3.cl"

typedef float8 Homographyf;

inline bool homographyf_is_good( const Homographyf h )
{
    if( !all( isfinite( h ) ) )
        return false;

    // check if perspective components are to strong
    if( dot( h.s67, h.s67 ) > 1e-3f )
        return false;

    // check if the upper 2 x 2 matrix is orientation preserving
    float det = ( h.s0 + 1.0f ) * ( h.s3 + 1.0f ) - h.s1 * h.s2;
    if( det < 0.0f )
        return false;

    // check the length of column 1 -> scale factor
    float2 c1 = ( float2 ) ( h.s0 + 1.0f, h.s1 );
    float len = length( c1 );
    if( len < 0.25f || len > 4.0f )
        return false;

    // check the length of column 2 -> scale factor
    float2 c2 = ( float2 ) ( h.s2, h.s3 + 1.0f );
    len = length( c2 );
    if( len < 0.25f || len > 4.0f )
        return false;

    return true;
}

inline Homographyf homographyf_identity()
{
    return ( float8 ) 0.0f;
}

inline Homographyf homographyf_translation( float2 t )
{
    return ( float8 ) ( 0.0f, 0.0f, 0.0f, 0.0f, t.x, t.y, 0.0f, 0.0f );
}

inline float2 homographyf_transform( Homographyf h, const float2 p )
{
    float p3 = 1.0f + h.s6 * p.x + h.s7 * p.y;
    float2 ret;

    ret.x = ( 1.0f + h.s0 ) * p.x + h.s2 * p.y + h.s4;
    ret.y = h.s1 * p.x + ( 1.0f + h.s3 ) * p.y + h.s5;
    return ret / p3;
}

inline void homographyf_to_matrix( Mat3f* mat, const Homographyf h )
{
    mat3f_set( mat, 1.0f + h.s0, h.s2       , h.s4,
                           h.s1, 1.0f + h.s3, h.s5,
                           h.s6, h.s7       , 1.0f  );
}

inline Homographyf homographyf_compose( const Homographyf h1, const Homographyf h2 )
{
    Homographyf ret;
    // float div = 1.0f + h1.s6 * h2.s4 + h1.s7 * h2.s5;

    // ret.s0 = h1.s0 + h2.s0 + h1.s0 * h2.s0 + h1.s2 * h2.s1 + h1.s4 * h2.s6 - h1.s6 * h2.s4 - h1.s7 * h2.s5;
    // ret.s1 = h1.s1 + h2.s1 + h1.s1 * h2.s0 + h1.s3 * h2.s1 + h1.s5 * h2.s6;
    // ret.s2 = h1.s2 + h2.s2 + h1.s0 * h2.s2 + h1.s2 * h2.s3 + h1.s4 * h2.s7;
    // ret.s3 = h1.s3 + h2.s3 + h1.s1 * h2.s2 + h1.s3 * h2.s3 + h1.s5 * h2.s7 - h1.s6 * h2.s4 - h1.s7 * h2.s5;
    // ret.s4 = h1.s4 + h2.s4 + h1.s0 * h2.s4 + h1.s2 * h2.s5;
    // ret.s5 = h1.s5 + h2.s5 + h1.s1 * h2.s4 + h1.s3 * h2.s5;
    // ret.s6 = h1.s6 + h2.s6 + h1.s6 * h2.s0 + h1.s7 * h2.s1;
    // ret.s7 = h1.s7 + h2.s7 + h1.s6 * h2.s2 + h1.s7 * h2.s3;

    ret.s0 = - h2.s5 * h1.s7 + h1.s4 * h2.s6 - h2.s4 * h1.s6 + h2.s1 * h1.s2 + h1.s0 * h2.s0 + h2.s0 + h1.s0;
    ret.s1 =   h1.s5 * h2.s6 + h2.s1 * h1.s3 + h2.s1 + h2.s0 * h1.s1 + h1.s1;
    ret.s2 =   h1.s4 * h2.s7 + h1.s2 * h2.s3 + h1.s0 * h2.s2 + h2.s2 + h1.s2;
    ret.s3 =   h1.s5 * h2.s7-h2.s5 * h1.s7-h2.s4 * h1.s6 + h1.s3 * h2.s3 + h2.s3 + h1.s3 + h1.s1 * h2.s2;
    ret.s4 =   h1.s2 * h2.s5 + h1.s0 * h2.s4 + h2.s4 + h1.s4;
    ret.s5 =   h1.s3 * h2.s5 + h2.s5 + h1.s5 + h1.s1 * h2.s4;
    ret.s6 =   h2.s1 * h1.s7 + h2.s6 + h2.s0 * h1.s6 + h1.s6;
    ret.s7 =   h2.s7 + h2.s3 * h1.s7 + h1.s7 + h2.s2 * h1.s6;
    return ret;
}

inline Homographyf homographyf_inverse( const Homographyf h )
{
    Homographyf ret;

    float mul = ( h.s0 + 1.0f  )* h.s3 - h.s1 * h.s2 + h.s0 + 1.0f;

    ret.s0 = 1.0f + h.s3 - h.s5 * h.s7 - mul;
    ret.s1 = -h.s1 + h.s5 * h.s6;
    ret.s2 = -h.s2 + h.s4 * h.s7;
    ret.s3 = 1.0f + h.s0 - h.s4 * h.s6 - mul;
    ret.s4 = -h.s4 - h.s3 * h.s4 + h.s2 * h.s5;
    ret.s5 = -h.s5 - h.s0 * h.s5 + h.s1 * h.s4;
    ret.s6 = -h.s6 - h.s3 * h.s6 + h.s1 * h.s7;
    ret.s7 = -h.s7 - h.s0 * h.s7 + h.s2 * h.s6;

    return ret / mul;

//  float3 tmp;
//
//  tmp.x = ( 1.0f + h.s3 ) * 1.0f - h.s7 * h.s5;
//  tmp.y = h.s6 * h.s5 - h.s1 * 1.0f;
//  tmp.z = h.s1 * h.s7 - h.s6 * ( 1.0f + h.s3 );
//
//  float det = dot( ( float3 ) ( 1.0f + h.s0, h.s2, h.s4 ), tmp );
//
//    ret.s0 = 1.0f + h.s3 - h.s5 * h.s7 - det * ( ( 1.0f + h.s0 ) * ( 1.0f + h.s3 ) - h.s1 * h.s2 );
//    ret.s1 = -h.s1 + h.s5 * h.s6;
//    ret.s2 = -h.s2 + h.s4 * h.s7;
//    ret.s3 = 1.0f + h.s0 - h.s4 * h.s6 - det * ( ( 1.0f + h.s0 ) * ( 1.0f + h.s3 ) - h.s1 * h.s2 );
//    ret.s4 = -h.s4 - h.s3 * h.s4 + h.s2 * h.s5;
//    ret.s5 = -h.s5 - h.s0 * h.s5 + h.s1 * h.s4;
//    ret.s6 = -h.s6 - h.s3 * h.s6 + h.s1 * h.s7;
//    ret.s7 = -h.s7 - h.s0 * h.s7 + h.s2 * h.s6;
//
//    return ret / ( det * ( ( 1.0f + h.s0 ) * ( 1.0f + h.s3 ) - h.s1 * h.s2 ) );
}

inline void homographyf_jacobian( float8* dx, float8* dy, const Homographyf h, const float2 pt )
{
    float pz = 1.0f + h.s6 * pt.x + h.s7 * pt.y;
    float2 pt2;

    pt2.x = ( ( 1.0f + h.s0 ) * pt.x + h.s2 * pt.y + h.s4 ) / pz;
    pt2.y = ( h.s1 * pt.x + ( 1.0f + h.s3 ) * pt.y + h.s5 ) / pz;

    *dx = ( ( float8 ) ( pt.x, 0.0f, pt.y, 0.0f, 1.0f, 0.0f, -pt.x * pt2.x, -pt.y * pt2.x ) ) / pz;
    *dy = ( ( float8 ) ( 0.0f, pt.x, 0.0f, pt.y, 0.0f, 1.0f, -pt.x * pt2.y, -pt.y * pt2.y ) ) / pz;
}

kernel void homographyf_warp_image( write_only image2d_t output, read_only image2d_t image, global Homographyf* h )
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

    Homographyf hh = *h;
    float2 pos2 = homographyf_transform( hh, coordf );
    float4 val = read_imagef( image, SAMPLER_BILINEAR, pos2 + offsethalf );
    write_imagef( output, ( int2 ) ( gx, gy ), val );
}

#endif
