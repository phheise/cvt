/*
   The MIT License (MIT)

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
#import "Matrix3.cl"
#import "Matrix4.cl"
#import "SE3.cl"
#import "Feature.cl"
#import "RGBDWarpReduce.cl"
#import "PatchCostFunctions.cl"

#define PATCHRADIUS 2
#define PATCHRADIUS_STEREO 2
#define DISPARITY_MAX 160.0f
#define DISPARITY_MIN 1.0f
#define DISPARITY_KLT 1.0f
#define KLT_STEPS 5
#define KLT_FACTOR 1.0f

typedef struct {
    int    valid;
    float2 pt;
    float  depth;
    float  weight;
} RGBDEntry;

typedef struct {
    int    valid;
    float  cost;
    float  weight;
    float  jac[ 6 ];
    float  hess[ 6 + 5 + 4 + 3 + 2 + 1 ];
} RGBDJacobianHess;

const sampler_t SAMPLER_NN       = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
const sampler_t SAMPLER_NN_ZERO  = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

#if 0
// Perform evaluation directly on the 4 neighbours as well in order to calculate the gradients efficiently
inline float census_cross( float4* retneighbours, read_only image2d_t colimg1, read_only image2d_t colimg2,
                           const float2 coord, const float2 coord2 )
{
    uint reti = 0;
    uint4 ret4i = ( uint4 ) 0;
    float ret = 0;
    float4 ret4 = ( float4 ) 0;
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );
    const float CENSUS_EPSILON = 0.05f;
    float3 values[ 2 * ( PATCHRADIUS + 1 ) + 1 ][ 2 * ( PATCHRADIUS + 1 ) + 1 ];

    float3 valc1 = read_imagef( colimg1, SAMPLER_BILINEAR, coord + OFFSETHALF ).xyz;

    for( int dy = -( PATCHRADIUS + 1 ); dy <= ( PATCHRADIUS + 1 ); dy++ ) {
#pragma unroll
        for( int dx = -( PATCHRADIUS + 1 ); dx <= ( PATCHRADIUS + 1 ); dx++ ) {
            float2 pos2 = coord2 + ( float2 ) ( dx, dy );
            values[ PATCHRADIUS + 1 + dy ][ PATCHRADIUS + 1 + dx ] = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF ).xyz;
        }
    }

    float3 valc2   = values[ PATCHRADIUS + 1 + 0 ][ PATCHRADIUS + 1 + 0 ];
    float3 valc2xp = values[ PATCHRADIUS + 1 + 0 ][ PATCHRADIUS + 1 + 1 ];
    float3 valc2xm = values[ PATCHRADIUS + 1 + 0 ][ PATCHRADIUS + 1 - 1 ];
    float3 valc2yp = values[ PATCHRADIUS + 1 + 1 ][ PATCHRADIUS + 1 + 0 ];
    float3 valc2ym = values[ PATCHRADIUS + 1 - 1 ][ PATCHRADIUS + 1 + 0 ];

    for( int dy = -PATCHRADIUS; dy <= PATCHRADIUS; dy++ ) {
#pragma unroll
        for( int dx = -PATCHRADIUS; dx <= PATCHRADIUS; dx++ ) {
            float2 pos = coord + ( float2 ) ( dx, dy );
            float3 val1   = read_imagef( colimg1, SAMPLER_BILINEAR, pos + OFFSETHALF ).xyz - valc1;
            float3 val2   = values[ PATCHRADIUS + 1 + dy + 0 ][ PATCHRADIUS + 1 + dx + 0 ] - valc2;
            float3 val2xp = values[ PATCHRADIUS + 1 + dy + 0 ][ PATCHRADIUS + 1 + dx + 1 ] - valc2xp;
            float3 val2xm = values[ PATCHRADIUS + 1 + dy + 0 ][ PATCHRADIUS + 1 + dx - 1 ] - valc2xm;
            float3 val2yp = values[ PATCHRADIUS + 1 + dy + 1 ][ PATCHRADIUS + 1 + dx + 0 ] - valc2yp;
            float3 val2ym = values[ PATCHRADIUS + 1 + dy - 1 ][ PATCHRADIUS + 1 + dx + 0 ] - valc2ym;

#if 0
#if 0
            val1   = select( sign( val1 ), ( float3 ) 0.0f, fabs( val1 ) < CENSUS_EPSILON );
            val2   = select( sign( val2 ), ( float3 ) 0.0f, fabs( val2 ) < CENSUS_EPSILON );
            val2xp = select( sign( val2xp ), ( float3 ) 0.0f, fabs( val2xp ) < CENSUS_EPSILON );
            val2xm = select( sign( val2xm ), ( float3 ) 0.0f, fabs( val2xm ) < CENSUS_EPSILON );
            val2yp = select( sign( val2yp ), ( float3 ) 0.0f, fabs( val2yp ) < CENSUS_EPSILON );
            val2ym = select( sign( val2ym ), ( float3 ) 0.0f, fabs( val2ym ) < CENSUS_EPSILON );
#endif
            val1   = as_float3( as_uint3( isgreaterequal( fabs( val1 ), CENSUS_EPSILON ) ) & as_uint3( copysign( ( float3) 1.0, val1 ) ) );
            val2   = as_float3( as_uint3( isgreaterequal( fabs( val2 ), CENSUS_EPSILON ) ) & as_uint3( copysign( ( float3) 1.0, val2 ) ) );
            val2xp = as_float3( as_uint3( isgreaterequal( fabs( val2xp ), CENSUS_EPSILON ) ) & as_uint3( copysign( ( float3) 1.0, val2xp ) ) );
            val2xm = as_float3( as_uint3( isgreaterequal( fabs( val2xm ), CENSUS_EPSILON ) ) & as_uint3( copysign( ( float3) 1.0, val2xm ) ) );
            val2yp = as_float3( as_uint3( isgreaterequal( fabs( val2yp ), CENSUS_EPSILON ) ) & as_uint3( copysign( ( float3) 1.0, val2yp ) ) );
            val2ym = as_float3( as_uint3( isgreaterequal( fabs( val2ym ), CENSUS_EPSILON ) ) & as_uint3( copysign( ( float3) 1.0, val2ym ) ) );

            //ret += dot( fabs( val2 - val1 ), ( float3 ) 0.333f );

            //ret4 += ( float4 ) ( dot( fabs( val2xp - val1 ), ( float3 ) 0.333f ),
            //                     dot( fabs( val2xm - val1 ), ( float3 ) 0.333f ),
            //                     dot( fabs( val2yp - val1 ), ( float3 ) 0.333f ),
            //                     dot( fabs( val2ym - val1 ), ( float3 ) 0.333f ) );
#endif

            int3 a = as_int3( as_uint3( isgreaterequal( fabs( val1 ), CENSUS_EPSILON ) ) & as_uint3( ( int3 ) 1 - as_int3( ( as_uint3( val1 ) >> 31 ) << 1 )  ) );
            int3 b = as_int3( as_uint3( isgreaterequal( fabs( val2 ), CENSUS_EPSILON ) ) & as_uint3( ( int3 ) 1 - as_int3( ( as_uint3( val2 ) >> 31 ) << 1 )  ) );
            uint3 c = abs( b - a );
            reti += c.x + c.y + c.z ;

            b = as_int3( as_uint3( isgreaterequal( fabs( val2xp ), CENSUS_EPSILON ) ) & as_uint3( ( int3 ) 1 - as_int3( ( as_uint3( val2xp ) >> 31 ) << 1 )  ) );
            c = abs( b - a );
            ret4i.x += c.x + c.y + c.z ;

            b = as_int3( as_uint3( isgreaterequal( fabs( val2xm ), CENSUS_EPSILON ) ) & as_uint3( ( int3 ) 1 - as_int3( ( as_uint3( val2xm ) >> 31 ) << 1 )  ) );
            c = abs( b - a );
            ret4i.y += c.x + c.y + c.z ;

            b = as_int3( as_uint3( isgreaterequal( fabs( val2yp ), CENSUS_EPSILON ) ) & as_uint3( ( int3 ) 1 - as_int3( ( as_uint3( val2yp ) >> 31 ) << 1 )  ) );
            c = abs( b - a );
            ret4i.z += c.x + c.y + c.z ;

            b = as_int3( as_uint3( isgreaterequal( fabs( val2ym ), CENSUS_EPSILON ) ) & as_uint3( ( int3 ) 1 - as_int3( ( as_uint3( val2ym ) >> 31 ) << 1 )  ) );
            c = abs( b - a );
            ret4i.w += c.x + c.y + c.z ;
        }
    }

    const float scale = 1.0f / ( ( ( float ) ( PATCHRADIUS * 2 + 1 ) * ( PATCHRADIUS * 2 + 1 ) ) * 3.0f );
    ret = scale * reti;
    *retneighbours = scale * convert_float4( ret4i );
    return ret;
}
#endif

#define eval( i0, i1, p0, p1, s ) eval_ncc( i0, i1, p0, p1, s )
#define eval_stereo( i0, i1, p0, p1, s ) eval_ncc( i0, i1, p0, p1, s )

kernel void ssdvo_warp( global RGBDJacobianHess* output, global RGBDEntry* entries, const unsigned int size,
                            const Mat3f K, const Mat4f RTKinv, float pyrscale,
                            read_only image2d_t image0, read_only image2d_t image1, float threshold, int remove )
{
    const unsigned int tid = get_global_id( 0 );
    const float xend = ( ( float ) get_image_width( image1 ) ) - 0.5f; // width - 1 + 0.5f
    const float yend = ( ( float ) get_image_height( image1 ) ) - 0.5f; // height - 1 + 0.5f
    const int patchsize = PATCHRADIUS;

    if( tid >= size )
        return;

    float8 dx, dy;

    float4 ptt = mat4f_transform( &RTKinv, ( float4 ) ( entries[ tid ].pt * entries[ tid ].depth, entries[ tid ].depth, 1.0f ) );
    float3 ptt2 = mat3f_transform( &K, ptt.xyz );
    ptt2.xy /= ptt2.z;

    // check if we are inside the other image namely image1, finite and in front of the camera
    if( ptt.z > 0.5f && ptt2.x > 0.0f && ptt2.x < xend - 0.5f && ptt2.y > 0.0f && ptt2.y < yend - 0.5f && all( isfinite( ptt2 ) ) && entries[ tid ].valid ) {

        float2 ptimg0 = entries[ tid ].pt * pyrscale;
#if 1
        float f0  = eval( image0, image1, ptimg0, ptt2.xy, patchsize );
        if( f0 >= threshold ) {
            entries[ tid ].valid = 0;
            output[ tid ].valid = 0;
            return;
        }

        //float2 hessdiag;

        float fxp = eval( image0, image1, ptimg0, ptt2.xy + ( float2 ) (  1.0f,  0.0f ), patchsize );
        float fxm = eval( image0, image1, ptimg0, ptt2.xy + ( float2 ) ( -1.0f,  0.0f ), patchsize );
        //hessdiag.x = -2.0f * f0 + fxp + fxm;
        //if( hessdiag.x < 0 ) {
        //    output[ tid ].valid = 0;
        //    return;
        //}

        float fyp = eval( image0, image1, ptimg0, ptt2.xy + ( float2 ) (  0.0f,  1.0f ), patchsize );
        float fym = eval( image0, image1, ptimg0, ptt2.xy + ( float2 ) (  0.0f, -1.0f ), patchsize );
        //hessdiag.y = -2.0f * f0 + fyp + fym;
        //if( hessdiag.y < 0 ) {
        //    output[ tid ].valid = 0;
        //    return;
        //}
#else
        float4 f4;
        float f0 = census_cross( &f4, image0, image1, ptimg0, ptt2.xy );
        if( f0 >= threshold ) {
            entries[ tid ].valid = 0;
            output[ tid ].valid = 0;
            return;
        }
        float fxp = f4.x;
        float fxm = f4.y;
        float fyp = f4.z;
        float fym = f4.w;

#endif
        float2 grad = 0.5f * ( float2 ) ( fxp - fxm, fyp - fym );
        float2 hessdiag = ( float2 ) ( -2.0f * f0 + fxp + fxm, -2.0f * f0 + fyp + fym );

        SE3_screen_jacobian( &dx, &dy, &K, ptt.xyz );

        output[ tid ].cost = f0;
        output[ tid ].valid = select( 0, 1, hessdiag.x >= 0 && hessdiag.y >=0 );
        output[ tid ].jac[ 0 ] = dot( ( float2 ) ( dx.s0, dy.s0 ), grad );
        output[ tid ].jac[ 1 ] = dot( ( float2 ) ( dx.s1, dy.s1 ), grad );
        output[ tid ].jac[ 2 ] = dot( ( float2 ) ( dx.s2, dy.s2 ), grad );
        output[ tid ].jac[ 3 ] = dot( ( float2 ) ( dx.s3, dy.s3 ), grad );
        output[ tid ].jac[ 4 ] = dot( ( float2 ) ( dx.s4, dy.s4 ), grad );
        output[ tid ].jac[ 5 ] = dot( ( float2 ) ( dx.s5, dy.s5 ), grad );

        float8 hdx, hdy;
        //hdx.s012345 = hessdiag.x * dx.s012345;
        //hdy.s012345 = hessdiag.y * dy.s012345;
        hdx = hessdiag.x * dx;
        hdy = hessdiag.y * dy;

        output[ tid ].hess[ 0 ] = dot( ( float2 ) ( dx.s0, dy.s0 ), ( float2 ) ( hdx.s0, hdy.s0 ) );
        output[ tid ].hess[ 1 ] = dot( ( float2 ) ( dx.s0, dy.s0 ), ( float2 ) ( hdx.s1, hdy.s1 ) );
        output[ tid ].hess[ 2 ] = dot( ( float2 ) ( dx.s0, dy.s0 ), ( float2 ) ( hdx.s2, hdy.s2 ) );
        output[ tid ].hess[ 3 ] = dot( ( float2 ) ( dx.s0, dy.s0 ), ( float2 ) ( hdx.s3, hdy.s3 ) );
        output[ tid ].hess[ 4 ] = dot( ( float2 ) ( dx.s0, dy.s0 ), ( float2 ) ( hdx.s4, hdy.s4 ) );
        output[ tid ].hess[ 5 ] = dot( ( float2 ) ( dx.s0, dy.s0 ), ( float2 ) ( hdx.s5, hdy.s5 ) );

        output[ tid ].hess[ 6 + 0 ] = dot( ( float2 ) ( dx.s1, dy.s1 ), ( float2 ) ( hdx.s1, hdy.s1 ) );
        output[ tid ].hess[ 6 + 1 ] = dot( ( float2 ) ( dx.s1, dy.s1 ), ( float2 ) ( hdx.s2, hdy.s2 ) );
        output[ tid ].hess[ 6 + 2 ] = dot( ( float2 ) ( dx.s1, dy.s1 ), ( float2 ) ( hdx.s3, hdy.s3 ) );
        output[ tid ].hess[ 6 + 3 ] = dot( ( float2 ) ( dx.s1, dy.s1 ), ( float2 ) ( hdx.s4, hdy.s4 ) );
        output[ tid ].hess[ 6 + 4 ] = dot( ( float2 ) ( dx.s1, dy.s1 ), ( float2 ) ( hdx.s5, hdy.s5 ) );

        output[ tid ].hess[ 11 + 0 ] = dot( ( float2 ) ( dx.s2, dy.s2 ), ( float2 ) ( hdx.s2, hdy.s2 ) );
        output[ tid ].hess[ 11 + 1 ] = dot( ( float2 ) ( dx.s2, dy.s2 ), ( float2 ) ( hdx.s3, hdy.s3 ) );
        output[ tid ].hess[ 11 + 2 ] = dot( ( float2 ) ( dx.s2, dy.s2 ), ( float2 ) ( hdx.s4, hdy.s4 ) );
        output[ tid ].hess[ 11 + 3 ] = dot( ( float2 ) ( dx.s2, dy.s2 ), ( float2 ) ( hdx.s5, hdy.s5 ) );

        output[ tid ].hess[ 15 + 0 ] = dot( ( float2 ) ( dx.s3, dy.s3 ), ( float2 ) ( hdx.s3, hdy.s3 ) );
        output[ tid ].hess[ 15 + 1 ] = dot( ( float2 ) ( dx.s3, dy.s3 ), ( float2 ) ( hdx.s4, hdy.s4 ) );
        output[ tid ].hess[ 15 + 2 ] = dot( ( float2 ) ( dx.s3, dy.s3 ), ( float2 ) ( hdx.s5, hdy.s5 ) );

        output[ tid ].hess[ 18 + 0 ] = dot( ( float2 ) ( dx.s4, dy.s4 ), ( float2 ) ( hdx.s4, hdy.s4 ) );
        output[ tid ].hess[ 18 + 1 ] = dot( ( float2 ) ( dx.s4, dy.s4 ), ( float2 ) ( hdx.s5, hdy.s5 ) );

        output[ tid ].hess[ 20 ] = dot( ( float2 ) ( dx.s5, dy.s5 ), ( float2 ) ( hdx.s5, hdy.s5 ) );
    } else {
        entries[ tid ].valid = 0;//select( entries[ tid ].valid, 0, remove );
        output[ tid ].valid = 0;
    }

}

kernel void ssdvo_warp2( global RGBDWarpEntry* output, global RGBDEntry* entries, const unsigned int size,
                         const Mat3f K, const Mat4f RTKinv, float pyrscale,
                         read_only image2d_t image0, read_only image2d_t image1, float threshold, int remove )
{
    const unsigned int tid = get_global_id( 0 );
    const float xend = ( ( float ) get_image_width( image1 ) ) - 0.5f; // width - 1 + 0.5f
    const float yend = ( ( float ) get_image_height( image1 ) ) - 0.5f; // height - 1 + 0.5f
    const int patchsize = PATCHRADIUS;

    if( tid >= size )
        return;

    float4 ptt = mat4f_transform( &RTKinv, ( float4 ) ( entries[ tid ].pt * entries[ tid ].depth, entries[ tid ].depth, 1.0f ) );
    float3 ptt2 = mat3f_transform( &K, ptt.xyz );
    ptt2.xy /= ptt2.z;

    // d/d ( px, py ) KRK^-1 * ( px, py, 1)^T -> first two cols of KRK^-1
    float3 dpx = mat3f_transform( &K, ( float3 ) ( RTKinv.m[ 0 ].x, RTKinv.m[ 1 ].x, RTKinv.m[ 2 ].x ) ) * entries[ tid ].depth;
    float3 dpy = mat3f_transform( &K, ( float3 ) ( RTKinv.m[ 0 ].y, RTKinv.m[ 1 ].y, RTKinv.m[ 2 ].y ) ) * entries[ tid ].depth;
    // multiplied with the derviative of the projection
    float4 affine;
    affine.x = ( dpx.x - dpx.z * ptt2.x ) / ptt2.z;
    affine.y = ( dpy.x - dpy.z * ptt2.x ) / ptt2.z;
    affine.z = ( dpx.y - dpx.z * ptt2.y ) / ptt2.z;
    affine.w = ( dpy.y - dpy.z * ptt2.y ) / ptt2.z;

    ptt2.xy *= pyrscale;

    // check if we are inside the other image namely image1, finite and in front of the camera
    if( ptt2.z > 0.5f && ptt2.x > 0.0f && ptt2.x < xend - 0.5f && ptt2.y > 0.0f && ptt2.y < yend - 0.5f
        && all( isfinite( ptt2 ) ) && entries[ tid ].valid ) {
        float2 ptimg0 = entries[ tid ].pt * pyrscale;

#if 0
        float f0 = eval( image0, image1, ptimg0, ptt2.xy, patchsize );
        if( f0 >= threshold ) {
            entries[ tid ].valid = !remove;
            output[ tid ].cost = -1.0f;
            return;
        }

        float fxp = eval( image0, image1, ptimg0, ptt2.xy + ( float2 ) (  1.0f,  0.0f ), patchsize );
        float fxm = eval( image0, image1, ptimg0, ptt2.xy + ( float2 ) ( -1.0f,  0.0f ), patchsize );

        float fyp = eval( image0, image1, ptimg0, ptt2.xy + ( float2 ) (  0.0f,  1.0f ), patchsize );
        float fym = eval( image0, image1, ptimg0, ptt2.xy + ( float2 ) (  0.0f, -1.0f ), patchsize );
#else
        float f0 = eval_ncc_affine2d( image0, image1, ptimg0, ptt2.xy, affine, patchsize );
        if( f0 >= threshold ) {
            entries[ tid ].valid = !remove;
            output[ tid ].cost = -1.0f;
            return;
        }

        float fxp = eval_ncc_affine2d( image0, image1, ptimg0, ptt2.xy + ( float2 ) (  1.0f,  0.0f ), affine, patchsize );
        float fxm = eval_ncc_affine2d( image0, image1, ptimg0, ptt2.xy + ( float2 ) ( -1.0f,  0.0f ), affine, patchsize );

        float fyp = eval_ncc_affine2d( image0, image1, ptimg0, ptt2.xy + ( float2 ) (  0.0f,  1.0f ), affine, patchsize );
        float fym = eval_ncc_affine2d( image0, image1, ptimg0, ptt2.xy + ( float2 ) (  0.0f, -1.0f ), affine, patchsize );

        /* float4 f4;
        float f0 = census_cross( &f4, image0, image1, ptimg0, ptt2.xy );
        if( f0 >= threshold ) {
            entries[ tid ].valid = remove;
            output[ tid ].cost = -1.0f;
            return;
        }
        float fxp = f4.x;
        float fxm = f4.y;
        float fyp = f4.z;
        float fym = f4.w; */
#endif

        float2 grad = 0.5f * ( float2 ) ( fxp - fxm, fyp - fym );
        float2 hessdiag = ( float2 ) ( -2.0f * f0 + fxp + fxm, -2.0f * f0 + fyp + fym );

        output[ tid ].cost     = select( -1.0f, f0, hessdiag.x > 0 && hessdiag.y > 0 );
        output[ tid ].pt       = ptt.xyz;
        output[ tid ].grad     = grad;
        output[ tid ].hessDiag = hessdiag;
    } else {
        entries[ tid ].valid = !remove;//select( entries[ tid ].valid, 0, remove );
        output[ tid ].cost = -1.0f;
    }
}

kernel void ssdvo_transform_entries( global RGBDEntry* entries, unsigned int size, const Mat3f K, const Mat4f RTKinv, int width, int height )
{
    const float BORDER = 5;
    const unsigned int tid = get_global_id( 0 );

    if( tid >= size || !entries[ tid ].valid )
        return;

    float4 ptt = mat4f_transform( &RTKinv, ( float4 ) ( entries[ tid ].pt * entries[ tid ].depth, entries[ tid ].depth, 1.0f ) );
    float3 ptt2 = mat3f_transform( &K, ptt.xyz );
    ptt2.xy /= ptt2.z;

    // check if we are inside the other image namely image1, finite and in front of the camera
    if( ptt.z > 0.5f && ptt2.x > BORDER && ptt2.x < ( ( float ) width ) - BORDER
                     && ptt2.y > BORDER && ptt2.y < ( ( float ) height ) - BORDER
                     && all( isfinite( ptt2 ) ) ) {
        entries[ tid ].valid = 1;
        entries[ tid ].pt    = ptt2.xy;
        entries[ tid ].depth = ptt.z;
        entries[ tid ].weight= entries[ tid ].weight;
    } else
        entries[ tid ].valid = 0;
}

kernel void ssdvo_init_entries( global RGBDEntry* entries, global unsigned int* size, int size_max,
                                const Mat3f Kinv, float fBfactor, global float3* features, int feature_size  )
{
    const int id = get_global_id( 0 );

    if( id >= feature_size )
        return;

    float3 feature = features[ id ];

    if( feature.z > DISPARITY_MAX || feature.z < DISPARITY_MIN )
        return;

    // Convert disparity to depth using focalLength * Baseline factor
    float depth =  fBfactor / feature.z;

    int idx = atomic_inc( size );
    if( idx < size_max ) {
        entries[ idx ].valid  = 1;
        entries[ idx ].pt     = feature.xy;
        entries[ idx ].depth  = depth;
        entries[ idx ].weight = 1.0f;
    }
}

kernel void ssdvo_refine_stereo( read_only image2d_t left, read_only image2d_t right, global float3* features, int feature_size )
{
    const int id = get_global_id( 0 );
    const int patchsize = PATCHRADIUS_STEREO;

    if( id >= feature_size )
        return;

    float3 feature = features[ id ];

    for( int i = 0; i < KLT_STEPS; i++ ) {
        float f0  = eval_stereo( left, right, feature.xy, ( float2 ) ( feature.x - feature.z, feature.y ), patchsize );
        float fxp = eval_stereo( left, right, feature.xy, ( float2 ) ( feature.x - ( feature.z + 1.0f ), feature.y ), patchsize );
        float fxm = eval_stereo( left, right, feature.xy, ( float2 ) ( feature.x - ( feature.z - 1.0f ), feature.y ), patchsize );
        float denom = ( -2.0f * f0 + ( fxp + fxm ) );
        if( denom < 0 ) {
            feature.z = -1;
            break;
        } else if( denom > 1e-4f ) {
            feature.z -= clamp( KLT_FACTOR * ( ( fxp - fxm ) / denom ), -DISPARITY_KLT, DISPARITY_KLT );
        } else
            break;
    }
    if( feature.z > DISPARITY_MAX || feature.z < DISPARITY_MIN )
        feature.z = -1;

    features[ id ].z = feature.z;
}

kernel void ssdvo_refine_depth( global RGBDEntry* entries, unsigned int size,
                                const Mat3f K, const Mat4f RTKinv, const Mat4f RTinvKinv, float fBfactor,
                                read_only image2d_t left, read_only image2d_t right, float threshold )
{
    const float BORDER = 5;
    const unsigned int tid = get_global_id( 0 );
    const int width = get_image_width( left );
    const int height = get_image_height( left );
    const int patchsize = PATCHRADIUS_STEREO;
    const float MAXWEIGHT = 20.0f;

    if( tid >= size || !entries[ tid ].valid )
        return;

    float4 ptt = mat4f_transform( &RTKinv, ( float4 ) ( entries[ tid ].pt * entries[ tid ].depth, entries[ tid ].depth, 1.0f ) );
    float3 ptt2 = mat3f_transform( &K, ptt.xyz );
    ptt2.xy /= ptt2.z;

    // check if we are inside the other image namely image1, finite and in front of the camera
    if( ptt.z > 0.0f && ptt2.x > BORDER && ptt2.x < ( ( float ) width ) - BORDER
                     && ptt2.y > BORDER && ptt2.y < ( ( float ) height ) - BORDER
                     && all( isfinite( ptt2 ) ) ) {

        float disparity = fBfactor / ptt.z;
        float f0;

        for( int i = 0; i < KLT_STEPS; i++ ) {
            f0  = eval_stereo( left, right, ptt2.xy, ( float2 ) ( ptt2.x - disparity, ptt2.y ), patchsize );

            float fxp = eval_stereo( left, right, ptt2.xy, ( float2 ) ( ptt2.x - ( disparity + 1.0f ), ptt2.y ), patchsize );
            float fxm = eval_stereo( left, right, ptt2.xy, ( float2 ) ( ptt2.x - ( disparity - 1.0f ), ptt2.y ), patchsize );

            float denom = ( -2.0f * f0 + ( fxp + fxm ) );
            if( denom < 0 ) {
                entries[ tid ].valid = 0;
                return;
            } else if( denom > 1e-6f ) {
                disparity -= clamp( KLT_FACTOR * ( ( fxp - fxm ) / denom ), -DISPARITY_KLT, DISPARITY_KLT );
            } else
                break;
        }

        f0  = eval_stereo( left, right, ptt2.xy, ( float2 ) ( ptt2.x - disparity, ptt2.y ), patchsize );
        if( f0 >= threshold ) {
            entries[ tid ].valid = 0;
            return;
        }

        float newdepth = fBfactor / disparity;

        float4 pttnew = mat4f_transform( &RTinvKinv, ( float4 ) ( ptt2.x * newdepth, ptt2.y * newdepth, newdepth, 1.0f ) );

        float newweight = 1.0f;
        if( fabs( fBfactor / entries[ tid ].depth - fBfactor/ pttnew.z ) > KLT_STEPS * DISPARITY_KLT * 0.5f ) {
            //newweight = 0.01f;
            entries[ tid ].valid = 0;
            return;
        }
        const float decay = 0.85f;
        float alpha = 1.0f - ( ( entries[ tid ].weight * decay ) / ( entries[ tid ].weight * decay + newweight ) );
        entries[ tid ].depth = mix( entries[ tid ].depth, pttnew.z, alpha );
        entries[ tid ].weight = min( entries[ tid ].weight * decay + newweight, MAXWEIGHT );
    } else
        entries[ tid ].valid = 0;
}

kernel void ssdvo_stereo_match( global float3* matches, global int* match_size, int match_max,
                                read_only image2d_t left, read_only image2d_t right,
                                global Feature* feature1, int size1,
                                global Feature* feature2, int size2, global int2* sclidx2,
                                float threshold )
{
    const int id = get_global_id( 0 );
    const int patchsize = PATCHRADIUS_STEREO;

    if( id >= size1 )
        return;

    Feature f = feature1[ id ];

    int2 scl = sclidx2[ f.pt.y ];
    int best = -1;
    float bestdist = 1e10f;
    float bestdisparity = 0;

    for( int offset = 0; offset < scl.y; offset++ ) {
        Feature f2 = feature2[ scl.x + offset ];
        if( f2.pt.x >= f.pt.x )
            continue;
        if( f.pt.x - f2.pt.x > DISPARITY_MAX )
            break;
        float dist = eval_stereo( left, right, ( float2 ) ( f.pt.x, f.pt.y ), ( float2 ) ( f2.pt.x, f2.pt.y ), patchsize );
        if( dist < bestdist ) {
            bestdist = dist;
            best = offset;
            bestdisparity = f.pt.x - f2.pt.x;
        }
    }

    if( best >= 0 && bestdist <= threshold && bestdisparity <= DISPARITY_MAX && bestdisparity >= DISPARITY_MIN ) {
        int idx = atomic_inc( match_size );
        if( idx < match_max ) {
            matches[ idx ] = ( float3 ) ( f.pt.x, f.pt.y, bestdisparity );
        }
    }
}
