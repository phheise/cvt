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
#import "cvt/cl/kernel/Matrix5.cl"

#define COLORWEIGHTKLT 18.0f

inline float gradientx( read_only image2d_t img, const float2 pt )
{
    const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
    const float4 grayWeight = ( float4 ) ( 0.2126f, 0.7152f, 0.0722f, 0.0f );
    const float2 offsethalf = ( float2 ) ( 0.5f, 0.5f );
    const float stepsize = 0.25f;
    float2 posx1 = pt + ( float2 ) ( -stepsize, 0.0f );
    float2 posx2 = pt + ( float2 ) (  stepsize, 0.0f );

    float valx1 = dot( read_imagef( img, SAMPLER_BILINEAR, posx1 + offsethalf ), grayWeight );
    float valx2 = dot( read_imagef( img, SAMPLER_BILINEAR, posx2 + offsethalf ), grayWeight );

    return ( valx1 - valx2 ) / ( 2.0f * stepsize);
}

float klt_stereo_ai_iteration( float3* delta, float2* deltaai, read_only image2d_t img1, read_only image2d_t img2, const float2 pt, const float3 warp, const float2 ai, const int patchsize )
{
    const float4 grayWeight =  ( float4 ) ( 0.2126f, 0.7152f, 0.0722f, 0.0f );
    const float2 offsethalf = ( float2 ) ( 0.5f, 0.5f );
    const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;
    float3 jac1, b1;
    float2 jac2, b2;
    Mat5f A, mattmp;
    float ssd = 0;
    float8 delta8;

    mat5f_set_zero( &A );
    b1 = ( float3 ) 0.0f;
    b2 = ( float2 ) 0.0f;

    float4 valcenter = read_imagef( img1, SAMPLER_BILINEAR, pt + offsethalf );
    for( float dy = -patchsize; dy <= patchsize; dy+= 1.0f ) {
        for( float dx = -patchsize; dx <= patchsize; dx+= 1.0f ) {
            float2 displace = ( float2 ) ( dx * 1.0f, dy * 1.0f );
            float2 pos1 = pt + displace;
            float2 pos2 = pt;
            pos2.x += ( 1.0f + warp.x ) * ( displace.x ) + warp.y * ( displace.y ) + warp.z;
            pos2.y += displace.y;

            float4 val1c = read_imagef( img1, SAMPLER_BILINEAR, pos1 + offsethalf );
            float4 val2c = read_imagef( img2, SAMPLER_BILINEAR, pos2 + offsethalf );
            float val1 = dot( val1c, grayWeight );
            float val2 = dot( val2c, grayWeight );
            //float val2ai = dot( val2c * ( 1.0f + ai.x ) + ai.y, grayWeight );
            float val2ai = val2 * ( 1.0f + ai.x ) + ai.y;

            float weight = native_exp( -dot( fabs( valcenter.xyz - val1c.xyz ), ( float3 ) 1.0f ) * COLORWEIGHTKLT + 0.0f );

            float diff = val2ai - val1;

            ssd += diff * diff;

            float gradx = /* gradientx( img1, pos1 ) * 0.5f + 0.5f */ gradientx( img2, pos2 ) * ( 1.0f + ai.x );

            jac1  = gradx * ( float3 ) ( displace.x, displace.y, 1.0f );
            jac2 =  ( float2 ) ( val2ai, 1.0f );

            mat5f_outer( &mattmp, jac1, jac2, weight * jac1, weight * jac2 );
            mat5f_add( &A, &A, &mattmp );

            b1 += jac1 * weight * diff;
            b2 += jac2 * weight * diff;
        }
    }

//    A.m[ 0 ][ 0 ] *= 1.0;
//    A.m[ 1 ][ 1 ] *= 1.0;
//    A.m[ 2 ][ 2 ] *= 1.0;
    A.m[ 3 ][ 3 ] *= 1.05;
    A.m[ 4 ][ 4 ] *= 1.05;
    delta8   = mat5f_lusolve( &A, b1, b2 );
    *delta   = delta8.s012;
    *deltaai = delta8.s34;
    return ssd;
}

const sampler_t SAMPLER_NN       = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

inline float3 warp_invert( float3 a )
{
    return ( float3 ) ( ( 1.0f / ( a.x + 1.0f ) ) - 1.0f, -a.y / (a.x + 1.0f ), -a.z / ( a.x + 1.0f ) );
}

inline float3 warp_compose( float3 a, float3 b )
{
    return ( float3 ) ( a.x * b.x + a.x + b.x, a.x * b.y + a.y + b.y, a.x * b.z + a.z + b.z );
}

kernel void klt_stereo_ai( write_only image2d_t disparity, read_only image2d_t left, read_only image2d_t right, read_only image2d_t dispin, int patchsize, int itermax )
{
    const int iwidth = get_image_width( dispin );
    const int iheight = get_image_height( dispin );
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int2 coord = ( int2 ) ( gx, gy );
    const float2 coordf = ( float2 ) ( gx, gy );

    if( gx >= iwidth || gy >= iheight )
        return;

    float ssdprev = 1e10f;
    float disp = read_imagef( dispin, SAMPLER_NN, ( int2 ) ( gx, gy ) ).x;
    float3 warp = ( float3 ) ( 0.0f, 0.0f, -disp );
    float2 ai = ( float2 ) 0;

    float3 delta;
    float2 deltaai;

    for( int i = 0; i < itermax; i++ ) {
        float ssd = klt_stereo_ai_iteration( &delta, &deltaai, left, right, coordf, warp, ai, patchsize );
        if( ssd  > ssdprev || !all( isfinite( delta ) ) || !all( isfinite( deltaai ) ) )
            break;
        ssdprev = ssd;
//        warp += delta;
        warp = warp_compose( warp, delta );
        ai.x = ( ai.x - deltaai.x ) / ( 1.0f + deltaai.x );
        ai.y = ( ai.y - deltaai.y ) / ( 1.0f + deltaai.x );
    }

    float diff = fabs( disp + warp.z );
    write_imagef( disparity, coord, diff > 2.0f ? disp : -warp.z );
//    write_imagef( disparity, coord, -warp.z );
}
