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
#import "GUMM.cl"
#import "PatchCostFunctions.cl"

#define PATCHRADIUS 1
#define CENSUS_COLOR_WEIGHT 12.0f
#define CENSUS_COLOR_EXPONENT 1.0f
#define THRESHOLD 0.9f

const sampler_t SAMPLER_NN       = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
const sampler_t SAMPLER_NN_ZERO  = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

/* --- Likelihood functions --- */

#if defined( AD_CENSUS_LOCAL )
inline float eval_census_ad_local( float* fxp, float* fxm, float* fyp, float* fym,
                                   const local float3* buf, const int bstride, read_only image2d_t colimg2, const float2 coord2 )
{
    float3 buf2[ 25 ];
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );
    const float ALPHA = 0.01f;
    const int PR2 = PATCHRADIUS * 2;
    float ret[ 5 ] = {0, 0, 0, 0, 0};
    const float w[ 9 ] = { 1.0f / 12.0f, 1.5f / 12.0f, 1.0f / 12.0f,
                           1.5f / 12.0f, 2.0f / 12.0f, 1.5f / 12.0f,
                           1.0f / 12.0f, 1.5f / 12.0f, 1.0f / 12.0f };
    const float3 grayWeight =  ( float3 ) ( 0.2126f, 0.7152f, 0.0722f );

    for( int dy = -PR2; dy <= PR2; dy++ ) {
        for( int dx = -PR2; dx <= PR2; dx++ ) {
            buf2[ ( dy + PR2 ) * 5 + ( dx + PR2 ) ] = read_imagef( colimg2, SAMPLER_BILINEAR, coord2 + ( float2 ) ( dx, dy ) + OFFSETHALF ).xyz;
        }
    }

#define EVAL_ONE( out, ox, oy, dx, dy ) do { \
            const float3 inval2 = buf2[ ( ( oy ) + ( dy ) ) * 5 + ( ox ) + ( dx ) ];                            \
            const float3 val2   = inval2 - buf2[ ( oy ) * 5 + ( ox ) ];                                     \
            int3 b              = signbit( val2 );                                                                          \
            const uint3 c       = abs( b - a );                                                                                 \
            const float census  = 0.333f * ( float )( c.x + c.y + c.z );                                                        \
            const float cost    = census  * ( 1.0f - ALPHA ) + ALPHA * dot( fmin( fabs( inval2 - inval1 ), 0.05f ), ( float3 ) 1.0f ) / 0.15f; \
            ret[ out ]          = ret[ out ] +  weight * cost; \
        } while( 0 )

    float3 cval = buf[ 0 ];
    float wsum  = 0.0f;
#pragma unroll
    for( int dy = -PATCHRADIUS; dy <= PATCHRADIUS; dy++ ) {
#pragma unroll
        for( int dx = -PATCHRADIUS; dx <= PATCHRADIUS; dx++ ) {
            const float3 inval1 = *( buf +  ( dy ) * bstride + ( dx ) );
            const float3 val1   = inval1 - cval;
            const int3 a        = signbit( val1 );
            const float weightx = w[ ( dy + 1 ) * 3 + ( dx + 1 ) ];
            const float weight  = weightx * 0.25f + 0.75f * exp( -pow( fabs( dot( val1, grayWeight ) ), CENSUS_COLOR_EXPONENT ) * CENSUS_COLOR_WEIGHT );
            wsum += weight;
            EVAL_ONE( 0, PR2 + 0, PR2 + 0, dx, dy );
            EVAL_ONE( 1, PR2 + 1, PR2 + 0, dx, dy );
            EVAL_ONE( 2, PR2 - 1, PR2 + 0, dx, dy );
            EVAL_ONE( 3, PR2 + 0, PR2 + 1, dx, dy );
            EVAL_ONE( 4, PR2 + 0, PR2 - 1, dx, dy );
        }
    }

#undef EVAL_ONE

    const float z = 1.0f / wsum;

    *fxp = ret[ 1 ] * z;
    *fxm = ret[ 2 ] * z;
    *fyp = ret[ 3 ] * z;
    *fym = ret[ 4 ] * z;
    return ret[ 0 ] * z;
}
#endif

inline float eval_census( read_only image2d_t colimg1, read_only image2d_t colimg2,
                          const float2 coord, const float2 coord2 )
{
    float ret = 0;
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );
    const float CENSUS_EPSILON = 0.04f;
    const float ALPHA = 0.05f;

    float3 valc1 = read_imagef( colimg1, SAMPLER_BILINEAR, coord + OFFSETHALF ).xyz;
    float3 valc2 = read_imagef( colimg2, SAMPLER_BILINEAR, coord2 + OFFSETHALF ).xyz;

for( int dy = -PATCHRADIUS; dy <= PATCHRADIUS; dy++ ) {
#pragma unroll
    for( int dx = -PATCHRADIUS; dx <= PATCHRADIUS; dx++ ) {

            const float2 pos = coord + ( float2 ) ( dx, dy );
            const float2 pos2 = coord2 + ( float2 ) ( dx, dy );

            const float3 inval1 = read_imagef( colimg1, SAMPLER_BILINEAR, pos + OFFSETHALF ).xyz;
            const float3 inval2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF ).xyz;

            const float3 val1 = inval1 - valc1;
            const float3 val2 = inval2 - valc2;

//          const int3 a = as_int3( as_uint3( isgreaterequal( fabs( val1 ), CENSUS_EPSILON ) ) & as_uint3( ( int3 ) 1 - as_int3( ( as_uint3( val1 ) >> 31 ) << 1 )  ) );
//          const int3 b = as_int3( as_uint3( isgreaterequal( fabs( val2 ), CENSUS_EPSILON ) ) & as_uint3( ( int3 ) 1 - as_int3( ( as_uint3( val2 ) >> 31 ) << 1 )  ) );
            int3 a = signbit( val1 );
            int3 b = signbit( val2 ); // returns -1 if bit set, 0 else
            const uint3 c = abs( b - a );
            float census = 0.333f * ( float )( c.x + c.y + c.z );

            ret += census * ( 1.0f - ALPHA ) + ALPHA * dot( fmin( fabs( inval1 - inval2 ), 0.05f ), ( float3 ) 1.0f ) / 0.15f;
        }
    }

    return ret / ( ( float ) ( PATCHRADIUS * 2 + 1 ) * ( PATCHRADIUS * 2 + 1 ) );
}

inline float patch_eval_census_ad( read_only image2d_t colimg1,
                          read_only image2d_t colimg2,
                          const float2 coord, const float2 coord2 )
{
    float wsum = 0;
    float census = 0;
    int width = get_image_width( colimg2 );
    int height = get_image_height( colimg2 );
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );
    const float3 grayWeight =  ( float3 ) ( 0.2126f, 0.7152f, 0.0722f );
    const float alpha = 0.125f;
    float m2 = 0;
    float m2sum = 0;

    float3 valc1 = read_imagef( colimg1, SAMPLER_BILINEAR, coord + OFFSETHALF ).xyz;
    float3 valc2 = read_imagef( colimg2, SAMPLER_BILINEAR, coord2 + OFFSETHALF ).xyz;

    for( int dy = -PATCHRADIUS; dy <= PATCHRADIUS; dy++ ) {
#pragma unroll
        for( int dx = -PATCHRADIUS; dx <= PATCHRADIUS; dx++ ) {
            float2 displace = ( float2 ) ( ( ( float ) dx ) , ( ( float ) dy ) );
            float2 pos = coord + displace;

            float2 pos2 = coord2 + displace;
            if( pos2.x < 0 || pos2.x >= width ||
                pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height ) {
                continue;
            }

            float3 valin = read_imagef( colimg1, SAMPLER_BILINEAR, pos + OFFSETHALF ).xyz;
            float3 val1 = valin - valc1;

            float w = exp( -pow( fabs( dot( val1, grayWeight ) ), CENSUS_COLOR_EXPONENT ) * CENSUS_COLOR_WEIGHT );

            float3 cval1 = read_imagef( colimg1, SAMPLER_BILINEAR, pos + OFFSETHALF ).xyz - valc1;
            float3 cval2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF ).xyz - valc2;

            float3 val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF ).xyz;
            float3 c = fmin( fabs( val2 - valin ), ( float3 ) 0.05f ) / 0.05f;

            float cad = 0.333f * ( float )( c.x + c.y + c.z );
            float cost = alpha * cad;

            int3 ga = signbit(  cval1 );
            int3 gb = signbit(  cval2 );
            uint3 gc = abs( ga - gb );
            float cc =  0.333f * ( float )( gc.x + gc.y + gc.z );
            cost += ( 1.0f - alpha ) * cc;

            census += w * cost;
            wsum += w;
        }
    }

    if( wsum <= 1.0 )
        return 1.0f;

//    m2 /= m2sum;

    return ( census / wsum );// * 0.9f + 0.1f * ( 1.0f - exp( -pow( m2, 2.0f ) * 200.0f ) );
}

#define eval( i0, i1, p0, p1 ) eval_census( i0, i1, p0, p1 )

/* --- DECAF functions --- */

struct __attribute__ ((packed)) DECAFGN {
    float  cost;
    float  weight;
    float2 grad;
    float2 hess;
    float  jacx[ 6 ];
    float  jacy[ 6 ];
};
typedef struct DECAFGN DECAFGN;

struct __attribute__ ((packed)) DECAFGNLS {
    // we only store the uppper triangular part
    float cost;
    float weight;
    float b[ 6 ];
    float A[ 6 + 5 + 4 + 3 + 2 + 1 ];
};
typedef struct DECAFGNLS DECAFGNLS;

/* to coalesce memory access we use SoA instead of AoS */

#define DECAF_GETSET(x,offset) \
inline void decaf_set_##x( global float* ptr, uint gid, uint stride, float value ) \
{ \
    ptr[ stride * ( offset ) + gid ] = value;\
} \
\
inline float decaf_get_##x( global float* ptr, uint gid, uint stride ) \
{ \
    return ptr[ stride * ( offset ) + gid ]; \
}

DECAF_GETSET(cost,0)
DECAF_GETSET(weight,1)
DECAF_GETSET(gradx,2)
DECAF_GETSET(grady,3)
DECAF_GETSET(hessx,4)
DECAF_GETSET(hessy,5)
DECAF_GETSET(jacx0,6)
DECAF_GETSET(jacx1,7)
DECAF_GETSET(jacx2,8)
DECAF_GETSET(jacx3,9)
DECAF_GETSET(jacx4,10)
DECAF_GETSET(jacx5,11)
DECAF_GETSET(jacy0,12)
DECAF_GETSET(jacy1,13)
DECAF_GETSET(jacy2,14)
DECAF_GETSET(jacy3,15)
DECAF_GETSET(jacy4,16)
DECAF_GETSET(jacy5,17)

kernel void decaf_warp( global float* output, uint output_stride, global uint* output_size,
                        read_only image2d_t image0, read_only image2d_t dispvar0,
                        read_only image2d_t image1, read_only image2d_t dispvar1,
                        const Mat3f K, const Mat3f Kinv, const Mat4f RT, float fB )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int width = get_image_width( image0 );
    const int height = get_image_height( image0 );
    const float DSTEP = 1.0f;

#if defined( AD_CENSUS_LOCAL )
    const int lx = get_local_id( 0 );
    const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
    const int2 base = ( int2 )( get_group_id( 0 ) * lw - 2 * PATCHRADIUS, get_group_id( 1 ) * lh - 2 * PATCHRADIUS );
    local float3 buf[ ( 16 + PATCHRADIUS * 4 ) * ( 16 + PATCHRADIUS * 4 ) ];
    const int bstride = lw + 4 * PATCHRADIUS;

    for( int y = ly; y < lh + 4 * PATCHRADIUS; y += lh ) {
        for( int x = lx; x < lw + 4 * PATCHRADIUS; x += lw ) {
            buf[ ( y * bstride ) + x ] = read_imagef( image0, SAMPLER_NN, base + ( int2 )( x, y ) ).xyz;
        }
    }
    barrier( CLK_LOCAL_MEM_FENCE );
#endif

    if( gx >= width || gy >= height )
        return;

    float2 dvar = read_imagef( dispvar0, SAMPLER_NN, ( int2 ) ( gx, gy ) ).xy;

    if( dvar.x <= 0.0f || !isfinite( dvar.x ) )
        return;

    float3 pt1p = ( float3 ) ( gx, gy, 1.0f );
    float3 pt1  = ( fB / dvar.x ) * mat3f_transform( &Kinv, pt1p );
    float4 pt2  = mat4f_transform( &RT, ( float4 ) ( pt1, 1.0f ) );
    float3 pt2p = mat3f_transform( &K, pt2.xyz );
    pt2p.xy    /= pt2p.z;

    // check if we are inside the other image and in front of the camera
    if( !(pt2p.z > 0.0f && isfinite( pt2p.z )
          && pt2p.x > ( PATCHRADIUS + 0.5f ) && pt2p.x < ( ( ( float ) width ) - ( PATCHRADIUS + 0.5f ) )
          && pt2p.y > ( PATCHRADIUS + 0.5f ) && pt2p.y < ( ( ( float ) height ) - ( PATCHRADIUS + 0.5f ) ) ) )
        return;

    float2 grad, hessdiag, dispgrad;
#if defined( AD_CENSUS )
    float f0 = eval( image0, image1, pt1p.xy, pt2p.xy );

    if( f0 >= THRESHOLD )
        return;

    float fxp = eval( image0, image1, pt1p.xy, pt2p.xy + ( float2 ) (  DSTEP,  0.0f ) );
    float fxm = eval( image0, image1, pt1p.xy, pt2p.xy + ( float2 ) ( -DSTEP,  0.0f ) );
    grad.x = ( fxp - fxm ) /  ( 2.0f * DSTEP );
    hessdiag.x = ( -2.0f * f0 + fxp + fxm ) / ( DSTEP * DSTEP );
    if( hessdiag.x < 0.0f )
        return;

    float fyp = eval( image0, image1, pt1p.xy, pt2p.xy + ( float2 ) (  0.0f,  DSTEP ) );
    float fym = eval( image0, image1, pt1p.xy, pt2p.xy + ( float2 ) (  0.0f, -DSTEP ) );
    grad.y = ( fyp - fym ) / ( 2.0f * DSTEP );
    hessdiag.y = ( -2.0f * f0 + fyp + fym ) / ( DSTEP * DSTEP );
    if( hessdiag.y < 0.0f )
        return;

    dispgrad = grad;
#elif defined( AD_CENSUS_LOCAL )
    float fxp, fxm, fyp, fym;
    float f0  = eval_census_ad_local( &fxp, &fxm, &fyp, &fym,
                                      buf + ( ly + 2 * PATCHRADIUS ) * bstride + ( lx + 2 * PATCHRADIUS ), bstride, image1, pt2p.xy );

    grad.x = ( fxp - fxm ) * 0.5f;
    grad.y = ( fyp - fym ) * 0.5f;
    hessdiag.x = ( -2.0f * f0 + fxp + fxm );
    hessdiag.y = ( -2.0f * f0 + fyp + fym );
    dispgrad = grad;

    if( f0 >= THRESHOLD /*&& length(grad) < 0.15f ) || hessdiag.x < 0.0f || hessdiag.y < 0.0f */ )
        return;

    if( hessdiag.x < 0 || hessdiag.y < 0 ) {
        //hessdiag = fmax( hessdiag, 0.0f );
        //grad *= 0.25f;
        return;
        //hessdiag = 0.01f * grad * grad;// + ( float2 ) ( 0.01f );
        //grad *= f0 * 0.01f;
    } /*else {
        hessdiag += grad * grad;
        grad += grad * f0;
    }*/
    //hessdiag = fmax( hessdiag, 0.0f );
    //if( hessdiag.x < 0.0f ) { hessdiag.x = 0.25f * fabs( grad.x );/* 0.001f;*/ }
    //if( hessdiag.y < 0.0f ) { hessdiag.y = 0.25f * fabs( grad.y );/* 0.001f;*/ }

#elif defined( CLASSIC_L2 ) || defined( CLASSIC_HUBER ) || defined( CLASSIC_TUKEY )
    const float3 grayWeight =  ( float3 ) ( 0.2126f, 0.7152f, 0.0722f );
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );
#define TOGRAY( x ) dot( x, grayWeight )

    float c1   = TOGRAY( read_imagef( image0, SAMPLER_BILINEAR, pt1p.xy + OFFSETHALF ).xyz );
    float c2   = TOGRAY( read_imagef( image1, SAMPLER_BILINEAR, pt2p.xy + OFFSETHALF ).xyz );
    float c2xm = TOGRAY( read_imagef( image1, SAMPLER_BILINEAR, pt2p.xy + ( float2 ) ( -1.0f,  0.0f ) + OFFSETHALF ).xyz );
    float c2xp = TOGRAY( read_imagef( image1, SAMPLER_BILINEAR, pt2p.xy + ( float2 ) (  1.0f,  0.0f ) + OFFSETHALF ).xyz );
    float c2ym = TOGRAY( read_imagef( image1, SAMPLER_BILINEAR, pt2p.xy + ( float2 ) (  0.0f, -1.0f ) + OFFSETHALF ).xyz );
    float c2yp = TOGRAY( read_imagef( image1, SAMPLER_BILINEAR, pt2p.xy + ( float2 ) (  0.0f,  1.0f ) + OFFSETHALF ).xyz );

    float f0 = c2 - c1;
#if defined( CLASSIC_L2 )
    float weight = 1.0f;
#elif defined( CLASSIC_HUBER )
    const float c = 0.05f;
    float huber = fabs( f0 ) < c ? 1.0f : fmin( c / fabs( f0 ), 1e8f );
    float weight = huber;
    if( weight < 1e-6f )
        return;
#elif defined( CLASSIC_TUKEY )
    const float c = 0.75f;
    float tmp = 1.0f - ( f0 * f0 ) / ( c * c );
    float tukey = fabs( f0 ) < c ? tmp * tmp : 0.0f;
    float weight = tukey;
    if( weight < 1e-6f )
        return;
#else
#error "undefined cost function"
#endif
    grad.x = ( c2xp - c2xm ) * 0.5f;
    grad.y = ( c2yp - c2ym ) * 0.5f;
    dispgrad = grad;
    hessdiag.x = grad.x * grad.x;
    hessdiag.y = grad.y * grad.y;
    grad *= f0;

#endif

    // screen jacobian
    float8 dx, dy;
    SE3_screen_jacobian( &dx, &dy, &K, pt2.xyz );

    // grad * d/dx pi \times  KRK^-1 x * fB/d^2
    float dispjac = -dot( mat3f_transform( &K, pt2.xyz - ( float3 ) ( RT.m[ 0 ].w,  RT.m[ 1 ].w, RT.m[ 2 ].w ) ) / dvar.x,
                         ( ( float3 ) ( dispgrad.x, dispgrad.y, -dot( dispgrad, pt2p.xy ) ) ) / pt2p.z ); // grad.xy *( 1/w 0 x/w^2 \\ 0 1/w y/w^2 ) with (x/w)/w and (y/w)/w

    uint gid = atomic_inc( output_size );
#if defined( AD_CENSUS ) || defined( AD_CENSUS_LOCAL )
    // resulting likelihood uncertainty
    float variance = dispjac * dispjac * dvar.y;
    //float ivariance = 1.0f / ( 1.0f + 0.5f * pow( fmax( variance, 1e-6f ), 2.0f ) );
    float ivariance = 1.0f / fmax( variance, 0.1f );
    decaf_set_cost( output, gid, output_stride, ivariance * f0 );
    decaf_set_weight( output, gid, output_stride, ivariance );
    decaf_set_gradx( output, gid, output_stride, ivariance * grad.x );
    decaf_set_grady( output, gid, output_stride, ivariance * grad.y );
    decaf_set_hessx( output, gid, output_stride, ivariance * hessdiag.x );
    decaf_set_hessy( output, gid, output_stride, ivariance * hessdiag.y );
#elif defined( CLASSIC_L2 ) || defined( CLASSIC_HUBER ) || defined( CLASSIC_TUKEY )
    decaf_set_cost( output, gid, output_stride, f0 );
    decaf_set_weight( output, gid, output_stride, weight );
    decaf_set_gradx( output, gid, output_stride, weight * grad.x );
    decaf_set_grady( output, gid, output_stride, weight * grad.y );
    decaf_set_hessx( output, gid, output_stride, weight * hessdiag.x );
    decaf_set_hessy( output, gid, output_stride, weight * hessdiag.y );
#endif
    decaf_set_jacx0( output, gid, output_stride, dx.s0 );
    decaf_set_jacx1( output, gid, output_stride, dx.s1 );
    decaf_set_jacx2( output, gid, output_stride, dx.s2 );
    decaf_set_jacx3( output, gid, output_stride, dx.s3 );
    decaf_set_jacx4( output, gid, output_stride, dx.s4 );
    decaf_set_jacx5( output, gid, output_stride, dx.s5 );
    decaf_set_jacy0( output, gid, output_stride, dy.s0 );
    decaf_set_jacy1( output, gid, output_stride, dy.s1 );
    decaf_set_jacy2( output, gid, output_stride, dy.s2 );
    decaf_set_jacy3( output, gid, output_stride, dy.s3 );
    decaf_set_jacy4( output, gid, output_stride, dy.s4 );
    decaf_set_jacy5( output, gid, output_stride, dy.s5 );
}

#define REDUCE_LOCAL_SIZE 256

#define REDUCE_GLOBAL_TO_LOCAL( lid, gid, stride ) \
    do { \
        gndata.cost      = decaf_get_cost( buffer, gid, stride ); \
        gndata.weight    = decaf_get_weight( buffer, gid, stride ); \
        gndata.grad.x    = decaf_get_gradx( buffer, gid, stride );     \
        gndata.grad.y    = decaf_get_grady( buffer, gid, stride );     \
        gndata.hess.x    = decaf_get_hessx( buffer, gid, stride );     \
        gndata.hess.y    = decaf_get_hessy( buffer, gid, stride );     \
        gndata.jacx[ 0 ] = decaf_get_jacx0( buffer, gid, stride );     \
        gndata.jacx[ 1 ] = decaf_get_jacx1( buffer, gid, stride );     \
        gndata.jacx[ 2 ] = decaf_get_jacx2( buffer, gid, stride );     \
        gndata.jacx[ 3 ] = decaf_get_jacx3( buffer, gid, stride );     \
        gndata.jacx[ 4 ] = decaf_get_jacx4( buffer, gid, stride );     \
        gndata.jacx[ 5 ] = decaf_get_jacx5( buffer, gid, stride );     \
        gndata.jacy[ 0 ] = decaf_get_jacy0( buffer, gid, stride );     \
        gndata.jacy[ 1 ] = decaf_get_jacy1( buffer, gid, stride );     \
        gndata.jacy[ 2 ] = decaf_get_jacy2( buffer, gid, stride );     \
        gndata.jacy[ 3 ] = decaf_get_jacy3( buffer, gid, stride );     \
        gndata.jacy[ 4 ] = decaf_get_jacy4( buffer, gid, stride );     \
        gndata.jacy[ 5 ] = decaf_get_jacy5( buffer, gid, stride );     \
        hdx[ 0 ] = gndata.hess.x * gndata.jacx[ 0 ]; \
        hdx[ 1 ] = gndata.hess.x * gndata.jacx[ 1 ]; \
        hdx[ 2 ] = gndata.hess.x * gndata.jacx[ 2 ]; \
        hdx[ 3 ] = gndata.hess.x * gndata.jacx[ 3 ]; \
        hdx[ 4 ] = gndata.hess.x * gndata.jacx[ 4 ]; \
        hdx[ 5 ] = gndata.hess.x * gndata.jacx[ 5 ]; \
        hdy[ 0 ] = gndata.hess.y * gndata.jacy[ 0 ]; \
        hdy[ 1 ] = gndata.hess.y * gndata.jacy[ 1 ]; \
        hdy[ 2 ] = gndata.hess.y * gndata.jacy[ 2 ]; \
        hdy[ 3 ] = gndata.hess.y * gndata.jacy[ 3 ]; \
        hdy[ 4 ] = gndata.hess.y * gndata.jacy[ 4 ]; \
        hdy[ 5 ] = gndata.hess.y * gndata.jacy[ 5 ]; \
        localbuf[ lid ].cost        += gndata.cost; \
        localbuf[ lid ].weight      += gndata.weight; \
        localbuf[ lid ].b[ 0 ]      += gndata.grad.x * gndata.jacx[ 0 ] + gndata.grad.y * gndata.jacy[ 0 ]; \
        localbuf[ lid ].b[ 1 ]      += gndata.grad.x * gndata.jacx[ 1 ] + gndata.grad.y * gndata.jacy[ 1 ]; \
        localbuf[ lid ].b[ 2 ]      += gndata.grad.x * gndata.jacx[ 2 ] + gndata.grad.y * gndata.jacy[ 2 ]; \
        localbuf[ lid ].b[ 3 ]      += gndata.grad.x * gndata.jacx[ 3 ] + gndata.grad.y * gndata.jacy[ 3 ]; \
        localbuf[ lid ].b[ 4 ]      += gndata.grad.x * gndata.jacx[ 4 ] + gndata.grad.y * gndata.jacy[ 4 ]; \
        localbuf[ lid ].b[ 5 ]      += gndata.grad.x * gndata.jacx[ 5 ] + gndata.grad.y * gndata.jacy[ 5 ]; \
        localbuf[ lid ].A[ 0 ]      += gndata.jacx[ 0 ] * hdx[ 0 ] + gndata.jacy[ 0 ] * hdy[ 0 ]; \
        localbuf[ lid ].A[ 1 ]      += gndata.jacx[ 0 ] * hdx[ 1 ] + gndata.jacy[ 0 ] * hdy[ 1 ]; \
        localbuf[ lid ].A[ 2 ]      += gndata.jacx[ 0 ] * hdx[ 2 ] + gndata.jacy[ 0 ] * hdy[ 2 ]; \
        localbuf[ lid ].A[ 3 ]      += gndata.jacx[ 0 ] * hdx[ 3 ] + gndata.jacy[ 0 ] * hdy[ 3 ]; \
        localbuf[ lid ].A[ 4 ]      += gndata.jacx[ 0 ] * hdx[ 4 ] + gndata.jacy[ 0 ] * hdy[ 4 ]; \
        localbuf[ lid ].A[ 5 ]      += gndata.jacx[ 0 ] * hdx[ 5 ] + gndata.jacy[ 0 ] * hdy[ 5 ]; \
        localbuf[ lid ].A[ 6 + 0 ]  += gndata.jacx[ 1 ] * hdx[ 1 ] + gndata.jacy[ 1 ] * hdy[ 1 ]; \
        localbuf[ lid ].A[ 6 + 1 ]  += gndata.jacx[ 1 ] * hdx[ 2 ] + gndata.jacy[ 1 ] * hdy[ 2 ]; \
        localbuf[ lid ].A[ 6 + 2 ]  += gndata.jacx[ 1 ] * hdx[ 3 ] + gndata.jacy[ 1 ] * hdy[ 3 ]; \
        localbuf[ lid ].A[ 6 + 3 ]  += gndata.jacx[ 1 ] * hdx[ 4 ] + gndata.jacy[ 1 ] * hdy[ 4 ]; \
        localbuf[ lid ].A[ 6 + 4 ]  += gndata.jacx[ 1 ] * hdx[ 5 ] + gndata.jacy[ 1 ] * hdy[ 5 ]; \
        localbuf[ lid ].A[ 11 + 0 ] += gndata.jacx[ 2 ] * hdx[ 2 ] + gndata.jacy[ 2 ] * hdy[ 2 ]; \
        localbuf[ lid ].A[ 11 + 1 ] += gndata.jacx[ 2 ] * hdx[ 3 ] + gndata.jacy[ 2 ] * hdy[ 3 ]; \
        localbuf[ lid ].A[ 11 + 2 ] += gndata.jacx[ 2 ] * hdx[ 4 ] + gndata.jacy[ 2 ] * hdy[ 4 ]; \
        localbuf[ lid ].A[ 11 + 3 ] += gndata.jacx[ 2 ] * hdx[ 5 ] + gndata.jacy[ 2 ] * hdy[ 5 ]; \
        localbuf[ lid ].A[ 15 + 0 ] += gndata.jacx[ 3 ] * hdx[ 3 ] + gndata.jacy[ 3 ] * hdy[ 3 ]; \
        localbuf[ lid ].A[ 15 + 1 ] += gndata.jacx[ 3 ] * hdx[ 4 ] + gndata.jacy[ 3 ] * hdy[ 4 ]; \
        localbuf[ lid ].A[ 15 + 2 ] += gndata.jacx[ 3 ] * hdx[ 5 ] + gndata.jacy[ 3 ] * hdy[ 5 ]; \
        localbuf[ lid ].A[ 18 + 0 ] += gndata.jacx[ 4 ] * hdx[ 4 ] + gndata.jacy[ 4 ] * hdy[ 4 ]; \
        localbuf[ lid ].A[ 18 + 1 ] += gndata.jacx[ 4 ] * hdx[ 5 ] + gndata.jacy[ 4 ] * hdy[ 5 ]; \
        localbuf[ lid ].A[ 20 + 0 ] += gndata.jacx[ 5 ] * hdx[ 5 ] + gndata.jacy[ 5 ] * hdy[ 5 ]; \
    } while( 0 )

#define REDUCE_LOCAL_TO_LOCAL( lid1, lid2 ) \
    do { \
        localbuf[ lid1 ].cost        += localbuf[ lid2 ].cost;        \
        localbuf[ lid1 ].weight      += localbuf[ lid2 ].weight;      \
        localbuf[ lid1 ].b[ 0 ]      += localbuf[ lid2 ].b[ 0 ];      \
        localbuf[ lid1 ].b[ 1 ]      += localbuf[ lid2 ].b[ 1 ];      \
        localbuf[ lid1 ].b[ 2 ]      += localbuf[ lid2 ].b[ 2 ];      \
        localbuf[ lid1 ].b[ 3 ]      += localbuf[ lid2 ].b[ 3 ];      \
        localbuf[ lid1 ].b[ 4 ]      += localbuf[ lid2 ].b[ 4 ];      \
        localbuf[ lid1 ].b[ 5 ]      += localbuf[ lid2 ].b[ 5 ];      \
        localbuf[ lid1 ].A[ 0 ]      += localbuf[ lid2 ].A[ 0 ];      \
        localbuf[ lid1 ].A[ 1 ]      += localbuf[ lid2 ].A[ 1 ];      \
        localbuf[ lid1 ].A[ 2 ]      += localbuf[ lid2 ].A[ 2 ];      \
        localbuf[ lid1 ].A[ 3 ]      += localbuf[ lid2 ].A[ 3 ];      \
        localbuf[ lid1 ].A[ 4 ]      += localbuf[ lid2 ].A[ 4 ];      \
        localbuf[ lid1 ].A[ 5 ]      += localbuf[ lid2 ].A[ 5 ];      \
        localbuf[ lid1 ].A[ 6 + 0 ]  += localbuf[ lid2 ].A[ 6 + 0 ];  \
        localbuf[ lid1 ].A[ 6 + 1 ]  += localbuf[ lid2 ].A[ 6 + 1 ];  \
        localbuf[ lid1 ].A[ 6 + 2 ]  += localbuf[ lid2 ].A[ 6 + 2 ];  \
        localbuf[ lid1 ].A[ 6 + 3 ]  += localbuf[ lid2 ].A[ 6 + 3 ];  \
        localbuf[ lid1 ].A[ 6 + 4 ]  += localbuf[ lid2 ].A[ 6 + 4 ];  \
        localbuf[ lid1 ].A[ 11 + 0 ] += localbuf[ lid2 ].A[ 11 + 0 ]; \
        localbuf[ lid1 ].A[ 11 + 1 ] += localbuf[ lid2 ].A[ 11 + 1 ]; \
        localbuf[ lid1 ].A[ 11 + 2 ] += localbuf[ lid2 ].A[ 11 + 2 ]; \
        localbuf[ lid1 ].A[ 11 + 3 ] += localbuf[ lid2 ].A[ 11 + 3 ]; \
        localbuf[ lid1 ].A[ 15 + 0 ] += localbuf[ lid2 ].A[ 15 + 0 ]; \
        localbuf[ lid1 ].A[ 15 + 1 ] += localbuf[ lid2 ].A[ 15 + 1 ]; \
        localbuf[ lid1 ].A[ 15 + 2 ] += localbuf[ lid2 ].A[ 15 + 2 ]; \
        localbuf[ lid1 ].A[ 18 + 0 ] += localbuf[ lid2 ].A[ 18 + 0 ]; \
        localbuf[ lid1 ].A[ 18 + 1 ] += localbuf[ lid2 ].A[ 18 + 1 ]; \
        localbuf[ lid1 ].A[ 20 + 0 ] += localbuf[ lid2 ].A[ 20 + 0 ]; \
    } while( 0 )

kernel void decaf_reduce( write_only global DECAFGNLS* output, read_only global float* buffer, uint size, uint stride )
{
    const uint lid = get_local_id( 0 );
    const uint grpid = get_group_id( 0 );
    const uint istride = get_num_groups( 0 ) * 2 * REDUCE_LOCAL_SIZE;
    uint idx = grpid * 2 * REDUCE_LOCAL_SIZE + lid;
    DECAFGN gndata;
    float hdx[ 6 ], hdy[ 6 ];
    local DECAFGNLS localbuf[ REDUCE_LOCAL_SIZE ];

    // set the local buffer to zero
    localbuf[ lid ].cost = 0;
    localbuf[ lid ].weight = 0;

#pragma unroll
    for( int i = 0; i < 6; i++ ) {
        localbuf[ lid ].b[ i ] = 0.0f;
    }
#pragma unroll
    for( int i = 0; i < 21; i++ ) {
        localbuf[ lid ].A[ i ] = 0.0f;
    }

    while( idx < size ) {
        REDUCE_GLOBAL_TO_LOCAL( lid, idx, stride );
        if( idx + REDUCE_LOCAL_SIZE < size )
            REDUCE_GLOBAL_TO_LOCAL( lid, idx + REDUCE_LOCAL_SIZE, stride );
        idx += istride;
    }
    barrier( CLK_LOCAL_MEM_FENCE );

#if ( REDUCE_LOCAL_SIZE >= 1024 )
    if( lid < 512 ) REDUCE_LOCAL_TO_LOCAL( lid, lid + 512 );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif
#if ( REDUCE_LOCAL_SIZE >= 512 )
    if( lid < 256 ) REDUCE_LOCAL_TO_LOCAL( lid, lid + 256 );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif
#if ( REDUCE_LOCAL_SIZE >= 256 )
    if( lid < 128 ) REDUCE_LOCAL_TO_LOCAL( lid, lid + 128 );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif
#if ( REDUCE_LOCAL_SIZE >= 128 )
    if( lid < 64 ) REDUCE_LOCAL_TO_LOCAL( lid, lid + 64 );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif

    // we have the barrier because not all platforms can guarante synchronous operations for less than 32 threads

#if ( REDUCE_LOCAL_SIZE >= 64 )
    if( lid < 32 ) REDUCE_LOCAL_TO_LOCAL( lid, lid + 32 );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif
#if ( REDUCE_LOCAL_SIZE >= 32 )
    if( lid < 16 ) REDUCE_LOCAL_TO_LOCAL( lid, lid + 16 );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif
#if ( REDUCE_LOCAL_SIZE >= 16 )
    if( lid < 8 ) REDUCE_LOCAL_TO_LOCAL( lid, lid + 8 );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif
#if ( REDUCE_LOCAL_SIZE >= 8 )
    if( lid < 4 ) REDUCE_LOCAL_TO_LOCAL( lid, lid + 4 );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif
#if ( REDUCE_LOCAL_SIZE >= 4 )
    if( lid < 2 ) REDUCE_LOCAL_TO_LOCAL( lid, lid + 2 );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif

//#if ( REDUCE_LOCAL_SIZE >= 2 )
    if( lid < 1 ) {
        REDUCE_LOCAL_TO_LOCAL( lid, lid + 1 );
        output[ grpid ] = localbuf[ 0 ];
    }

//    barrier( CLK_LOCAL_MEM_FENCE );
//#endif

//   if( lid == 0 ) output[ grpid ] = localbuf[ 0 ];
}

kernel void decaf_draw_warp( write_only image2d_t output,
                              read_only image2d_t image0, read_only image2d_t dispvar0,
                              const Mat3f K, const Mat3f Kinv, const Mat4f RT, float fB )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int width = get_image_width( output );
    const int height = get_image_height( output );

    if( gx >= width || gy >= height )
        return;

    float3 rgb     = read_imagef( image0, SAMPLER_NN, ( int2 ) ( gx, gy ) ).xyz;
    float2 dvar    = read_imagef( dispvar0, SAMPLER_NN, ( int2 ) ( gx, gy ) ).xy;

    if( dvar.x > 0.0f ) {
        float3 pt   = ( fB / dvar.x ) * mat3f_transform( &Kinv, ( float3 ) ( gx, gy, 1.0f ) );
        float4 pt2  = mat4f_transform( &RT, ( float4 ) ( pt, 1.0f ) );
        float3 pt2p = mat3f_transform( &K, pt2.xyz );
        pt2p.xy    /= pt2p.z;

        // check if we are inside the other image and in front of the camera
        if( pt2p.z > 0.0f
            && pt2p.x > 0.5f && pt2p.x < ( ( float ) width ) - 0.5f
            && pt2p.y > 0.5f && pt2p.y < ( ( float ) height ) - 0.5f ) {
            write_imagef( output, ( int2 ) ( convert_int_rte( pt2p.x ), convert_int_rte( pt2p.y ) ), ( float4 ) ( rgb, 1.0f ) );
        }
    }
}

kernel void decaf_warp_disparity_variance( write_only image2d_t output,
                                           read_only image2d_t dispvar0,
                                           const Mat3f K, const Mat3f Kinv, const Mat4f RT, const Mat3f KRKinv, float fB )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int width = get_image_width( output );
    const int height = get_image_height( output );

    if( gx >= width || gy >= height )
        return;

    float4 in = read_imagef( dispvar0, SAMPLER_NN, ( int2 ) ( gx, gy ) );

    if( in.x > 0.0f && isfinite( in.x ) ) {
        float3 pt   = ( fB / in.x ) * mat3f_transform( &Kinv, ( float3 ) ( gx, gy, 1.0f ) );
        float4 pt2  = mat4f_transform( &RT, ( float4 ) ( pt, 1.0f ) );
        float3 pt2p = mat3f_transform( &K, pt2.xyz );
        pt2p.xy    /= pt2p.z;

        // check if we are inside the other image and in front of the camera
        if( pt2p.z > 0.0f
            && pt2p.x > 1.5f && pt2p.x < ( ( float ) width ) - 1.5f
            && pt2p.y > 1.5f && pt2p.y < ( ( float ) height ) - 1.5f ) {
            float variance = ( ( pt.z * pt.z ) / ( pt2p.z * pt2p.z ) ) * mat3f_transform( &KRKinv, ( float3 ) ( gx, gy, 1.0f ) ).z;
            variance = variance * variance * in.y;
            float2 diff = pt2p.xy - convert_float2( convert_int2_rte( pt2p.xy ) );
            variance += dot( diff, diff ) * 1.0f;
            write_imagef( output, ( int2 ) ( convert_int_rte( pt2p.x ), convert_int_rte( pt2p.y ) ), ( float4 ) ( fB / pt2p.z, variance, in.z, in.w ) );
        }
    }
}

kernel void decaf_uniform_variance( write_only image2d_t output, read_only image2d_t input, float variance )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int width = get_image_width( output );
    const int height = get_image_height( output );

    if( gx >= width || gy >= height )
        return;

    float val = read_imagef( input, SAMPLER_NN, ( int2 ) ( gx, gy ) ).x;
    write_imagef( output, ( int2 ) ( gx, gy ) , ( float4 ) ( val, variance, 1.0f, 1.0f ) );
}

kernel void decaf_depth_uniform_variance( write_only image2d_t output, read_only image2d_t input, float fB, float scale, float variance )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int width = get_image_width( output );
    const int height = get_image_height( output );

    if( gx >= width || gy >= height )
        return;

    float val = read_imagef( input, SAMPLER_NN, ( int2 ) ( gx, gy ) ).x;
    if( val > 1e-7f ) {
        val = ( fB * scale ) / ( /*pow(val, 1.00f ) - 0.0065f*/ val );
    }
    write_imagef( output, ( int2 ) ( gx, gy ) , ( float4 ) ( val, variance, 1.0f, 1.0f ) );
}

kernel void decaf_fuse_disparity_variance( write_only image2d_t output, read_only image2d_t input0, read_only image2d_t input1, float procnoise, float maxdiff )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int width = get_image_width( output );
    const int height = get_image_height( output );

    if( gx >= width || gy >= height )
        return;

    float2 in0 = read_imagef( input0, SAMPLER_NN, ( int2 ) ( gx, gy ) ).xy;
    float2 in1 = read_imagef( input1, SAMPLER_NN, ( int2 ) ( gx, gy ) ).xy;
    in1.y += procnoise;

    float2 out = ( float2 ) 0.0f;
    if( in0.x > 0.0f && isfinite( in0.x ) && ( in1.x <= 0.0f || !isfinite( in1.x ) || ( fabs( in0.x - in1.x ) > maxdiff * in0.x ) ) ) {
        // first value available but second value not available or difference to high
        out = in0;
    } else if( ( in0.x <= 0.0f || !isfinite( in0.x ) ) && in1.x > 0.0f && isfinite( in1.x ) ) {
        // only second value available
        out = in1;
    } else if( in0.x > 0.0f && in1.x > 0.0f && isfinite( in0.x ) && isfinite( in1.x ) ) {
        // Kalman fusion... both values available
        out.x = ( in0.y * in1.x + in1.y * in0.x ) / ( in0.y + in1.y );
        out.y = ( in0.y * in1.y ) / ( in0.y + in1.y );
    }
    write_imagef( output, ( int2 ) ( gx, gy ), ( float4 ) ( out.x, out.y, 0.0f, 0.0f ) );
}

kernel void decaf_gumm_init( write_only image2d_t output, read_only image2d_t input, float variance )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int width = get_image_width( output );
    const int height = get_image_height( output );
    GUMM gumm;

    if( gx >= width || gy >= height )
        return;

    float val = read_imagef( input, SAMPLER_NN, ( int2 ) ( gx, gy ) ).x;
    float sqr = variance * variance;
    GUMM_init( &gumm, val, variance, 0.9f, 10.0f / ( 1.0f + 2.0f * sqr ) );
    // mu2 to variance to float4
    float4 ret = GUMM_to_float4( &gumm );
    ret.y -= ret.x * ret.x;
    write_imagef( output, ( int2 ) ( gx, gy ) , ret );
}

kernel void decaf_gumm_init_disparity_variance( write_only image2d_t output, read_only image2d_t input )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int width = get_image_width( output );
    const int height = get_image_height( output );
    GUMM gumm;

    if( gx >= width || gy >= height )
        return;

    float2 val = read_imagef( input, SAMPLER_NN, ( int2 ) ( gx, gy ) ).xy;
    float sqr  = val.y * val.y;
    GUMM_init( &gumm, val.x, val.y, 0.9f, 10.0f / ( 1.0f + 2.0f * sqr ) );
    // mu2 to variance to float4
    float4 ret = GUMM_to_float4( &gumm );
    ret.y -= ret.x * ret.x;
    write_imagef( output, ( int2 ) ( gx, gy ) , ret );
}

kernel void decaf_gumm_fuse( write_only image2d_t output, read_only image2d_t gumm_input0, read_only image2d_t input1, float procnoise )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int width = get_image_width( output );
    const int height = get_image_height( output );
    GUMM gumm;

    if( gx >= width || gy >= height )
        return;

    // variance to mu2 to GUMM
    float4 in0 = read_imagef( gumm_input0, SAMPLER_NN, ( int2 ) ( gx, gy ) );
    in0.y += in0.x * in0.x;

    GUMM_init_float4( &gumm, in0 );
    float2 in1 = read_imagef( input1, SAMPLER_NN, ( int2 ) ( gx, gy ) ).xy;
    in1.y += procnoise;

    float2 out = ( float2 ) 0.0f;
    if( in0.x > 0.0f && isfinite( in0.x ) && ( in1.x <= 0.0f || !isfinite( in1.x ) ) ) {
        // first value available but second value not available or difference to high
        // gumm stays the same
    } else if( ( in0.x <= 0.0f || !isfinite( in0.x ) ) && in1.x > 0.0f && isfinite( in1.x ) ) {
        // only second value available
        GUMM_init( &gumm, in1.x, in1.y, 0.9f, 1.0f );
    } else if( in0.x > 0.0f && in1.x > 0.0f && isfinite( in0.x ) && isfinite( in1.x ) ) {
        // we bound the maximum weight here by some fixed value calculated from the inverse variance
        float sqr = in1.y * in1.y;
        float pi = GUMM_update( &gumm, in1.x, 1.0f / ( 1.0f + 0.5f * sqr ), 1.0f / 200.0f ); // we assume disparity goes from 0 to 200
    }

    // mu2 to variance to float4
    float4 ret = GUMM_to_float4( &gumm );
    ret.y -= ret.x * ret.x;
    write_imagef( output, ( int2 ) ( gx, gy ), ret );
}

kernel void decaf_draw_disparity( write_only image2d_t output, read_only image2d_t input, float fB, float scale )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int width = get_image_width( output );
    const int height = get_image_height( output );

    if( gx >= width || gy >= height )
        return;

    float disp = read_imagef( input, SAMPLER_NN, ( int2 ) ( gx, gy ) ).x;
    float out = disp > 0.0f ? clamp( ( fB / disp ) * scale, 0.0f, 1.0f ) : 0.0f;
    write_imagef( output, ( int2 ) ( gx, gy ) , ( float4 ) ( out, 1.0f, 1.0f, 1.0f ) );
}

kernel void decaf_draw_variance( write_only image2d_t output, read_only image2d_t input, float scale )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int width = get_image_width( output );
    const int height = get_image_height( output );

    if( gx >= width || gy >= height )
        return;

    float var = read_imagef( input, SAMPLER_NN, ( int2 ) ( gx, gy ) ).y;
    write_imagef( output, ( int2 ) ( gx, gy ) , ( float4 ) ( clamp( var * scale, 0.0f, 1.0f ), 1.0f, 1.0f, 1.0f ) );
}
