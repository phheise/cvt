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
#ifndef SVPMFLOW_CL
#define SVPMFLOW_CL

#import "cvt/cl/kernel/RNG.cl"

#define NUMRNDTRIES  2
#define NUMRNDSAMPLE 2
#define NUMRNDREFINE 2
#define RNDSAMPLERADIUS 5.0f

const sampler_t SAMPLER_NN       = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
const sampler_t SAMPLER_NN_ZERO  = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

float2 box_mueller( RNG* rng )
{
    float2 ret;
    ret.x = RNG_float( rng );
    ret.y = RNG_float( rng );
    float r = sqrt( -2.0f * log( ret.x ) );
    float theta = 2.0f * M_PI * ret.y;
    return ( float2 ) ( r * sin( theta ), r * cos( theta ) );
}

inline float4 svpmflow_state_init( RNG* rng, const float flowmax )
{
    float4 ret = ( float4 ) 0;
    //ret.x = ( RNG_float( rng ) - 0.5f ) * 2.0f * flowmax;
    //ret.y = ( RNG_float( rng ) - 0.5f ) * 2.0f * flowmax;
    ret.xy = box_mueller( rng ) * flowmax * 0.5f;
    ret.z = 1e10f;
    return ret;
}

inline float4 svpmflow_state_refine( const float4 state, RNG* rng, const float refinemax )
{
    float4 ret;
    ret.xy += box_mueller( rng ) * refinemax * 0.5f;
    ret.z = 1e10f;
    return ret;
}

inline float svpmflow_eval_ncc( read_only image2d_t colimg1, read_only image2d_t colimg2,
                             const float2 coord, const float4 state, const int patchsize )
{
    float sum = 0;
    float3 mean1 = ( float3 ) 0;
    float3 mean2 = ( float3 ) 0;
    float3 mean12 = ( float3 ) 0;
    float3 msqr1 = ( float3 ) 0;
    float3 msqr2 = ( float3 ) 0;
    int width = get_image_width( colimg2 );
    int height = get_image_height( colimg2 );
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );
    const float COLORWEIGHT = 2.6f;

    if( !all(isfinite(state.xy)))
        return 1e10f;

    float4 valcenter = read_imagef( colimg1, SAMPLER_BILINEAR, coord + OFFSETHALF );

    for( float dy = -patchsize; dy <= patchsize; dy+=1.0f ) {
        for( float dx = -patchsize; dx <= patchsize; dx+=1.0f ) {

            float2 pos = coord + ( float2 ) ( dx, dy );
            float2 pos2 = pos + state.xy;

            if( !( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height ||
                  pos2.x < 0 || pos2.x >= width || pos2.y < 0 || pos2.y >= height ) ) {
                float3 val1 = read_imagef( colimg1, SAMPLER_BILINEAR, pos + OFFSETHALF ).xyz;
                float3 val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF ).xyz;
                float w = native_exp( -dot( fabs( valcenter.xyz - val1.xyz ), ( float3 ) 1.0f ) * COLORWEIGHT );
                sum += w;
                mean1 += val1 * w;
                mean2 += val2 * w;
                mean12 += val1 * val2 * w;
                msqr1 += val1 * val1 * w;
                msqr2 += val2 * val2 * w;

            }
        }
    }

    if( sum <= 1.1f ) return 1e10f;
    float3 iwsum = 1.0f / sum;
    mean1  = mean1 * iwsum;
    mean2  = mean2 * iwsum;
    mean12 = mean12 * iwsum;
    msqr1  = msqr1 * iwsum;
    msqr2  = msqr2 * iwsum;
    float3 nom = mean12 - mean1 * mean2;
    float3 denom = sqrt( ( msqr1 - mean1 * mean1 ) * ( msqr2 - mean2 * mean2  ) );
    if( denom.x < 1e-8f || !isfinite( denom.x ) ) denom.x = 1.0f;
    if( denom.y < 1e-8f || !isfinite( denom.y ) ) denom.y = 1.0f;
    if( denom.z < 1e-8f || !isfinite( denom.z ) ) denom.z = 1.0f;
    float3 v = nom / denom;

//    if( !all( isfinite( v ) ) ) return 1e5f;
//    if( any( isless( v, ( float3 ) 0.0f ) ) ) return 1e5f;
//    if( any( isgreater( v, ( float3 ) 1.0f ) ) ) return 1e5f;


//    return dot( fmax( ( ( float3 ) 1.0f ) - v, ( float3 ) 0.0f ), ( float3 ) 1.0f );

    return dot( clamp( fabs( ( ( float3 ) 1.0f ) - v ), ( float3 ) 0.0f, ( float3 ) 0.5f ), ( float3 ) 1.0f ) / 1.5f;

    //return dot( fmin( acos( v ) * ( float3 ) ( 2.0f / M_PI ), ( float3 ) 0.5f ), ( float3 ) 1.0f );
}

inline float svpmflow_eval_trunc_sad( read_only image2d_t colimg1, read_only image2d_t colimg2,
                             const float2 coord, const float4 state, const int patchsize )
{
    float ret = 0;
    float3 mean1 = ( float3 ) 0;
    float3 mean2 = ( float3 ) 0;
    float3 mean12 = ( float3 ) 0;
    float3 msqr1 = ( float3 ) 0;
    float3 msqr2 = ( float3 ) 0;
    int width = get_image_width( colimg2 );
    int height = get_image_height( colimg2 );
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );
    const float THRESHOLD = 0.03f;
    float wsum = 0.0f;

    if( !all(isfinite(state.xy)))
        return 1e10f;

    for( float dy = -patchsize; dy <= patchsize; dy+=1.0f ) {
        for( float dx = -patchsize; dx <= patchsize; dx+=1.0f ) {

            float2 pos = coord + ( float2 ) ( dx, dy );
            float2 pos2 = pos + state.xy;
            if( !( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height ||
                  pos2.x < 0 || pos2.x >= width || pos2.y < 0 || pos2.y >= height ) ) {
                float4 val1 = read_imagef( colimg1, SAMPLER_BILINEAR, pos + OFFSETHALF );
                float4 val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF );
                ret += dot( fmin( fabs( val1.xyz - val2.xyz ), ( float3 ) THRESHOLD ), ( float3 ) 1.0f );
                wsum += 1.0f;
            }
        }
    }

    if( wsum < 1.1f ) return 1e5f;
    return ret / wsum;
}

inline float svpmflow_eval_bilateral_trunc_sad( read_only image2d_t colimg1, read_only image2d_t colimg2,
                             const float2 coord, const float4 state, const int patchsize )
{
    const float COLORWEIGHT = 2.6f;
    float ret = 0;
    float3 mean1 = ( float3 ) 0;
    float3 mean2 = ( float3 ) 0;
    float3 mean12 = ( float3 ) 0;
    float3 msqr1 = ( float3 ) 0;
    float3 msqr2 = ( float3 ) 0;
    int width = get_image_width( colimg2 );
    int height = get_image_height( colimg2 );
    const float2 OFFSETHALF = ( float2 ) ( 0.5f, 0.5f );
    const float THRESHOLD = 0.02f;
    float wsum = 0.0f;

    if( !all(isfinite(state.xy)))
        return 1e10f;

    float4 valcenter = read_imagef( colimg1, SAMPLER_BILINEAR, coord + OFFSETHALF );

    for( float dy = -patchsize; dy <= patchsize; dy+=1.0f ) {
        for( float dx = -patchsize; dx <= patchsize; dx+=1.0f ) {

            float2 pos = coord + ( float2 ) ( dx, dy );
            float2 pos2 = pos + state.xy;
            if( !( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height ||
                  pos2.x < 0 || pos2.x >= width || pos2.y < 0 || pos2.y >= height ) ) {
                float4 val1 = read_imagef( colimg1, SAMPLER_BILINEAR, pos + OFFSETHALF );
                float4 val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos2 + OFFSETHALF );
                float w = native_exp( -dot( fabs( valcenter.xyz - val1.xyz ), ( float3 ) 1.0f ) * COLORWEIGHT );
                wsum += w;
                ret += w * dot( fmin( fabs( val1.xyz - val2.xyz ), ( float3 ) THRESHOLD ), ( float3 ) 1.0f );
            }
        }
    }

    if( wsum < 1.1f ) return 1e5f;
    return ret / wsum;
}

#define svpmflow_eval svpmflow_eval_bilateral_trunc_sad

kernel void svpmflow_init( write_only image2d_t output,
                          read_only image2d_t img1, read_only image2d_t img2,
                          const int patchsize, const float flowmax )
{
    RNG rng;
    const int2 coord = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const float2 coordf = ( float2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const int width = get_image_width( img1 );
    const int height = get_image_height( img1 );

    if( coord.x >= width || coord.y >= height )
        return;

    RNG_init( &rng, coord.y * width + coord.x, 2 );

    float4 ret;
    ret        = svpmflow_state_init( &rng, flowmax );
    ret.z      = svpmflow_eval( img1, img2, coordf, ret, patchsize );

    write_imagef( output, coord, ret );
}

kernel void svpmflow_init_from_flow( write_only image2d_t output, read_only image2d_t flow,
                          read_only image2d_t img1, read_only image2d_t img2,
                          const int patchsize, const float flowmax )
{
    RNG rng;
    const int2 coord = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const float2 coordf = ( float2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const int width = get_image_width( img1 );
    const int height = get_image_height( img1 );

    if( coord.x >= width || coord.y >= height )
        return;

    float4 ret = ( float4 ) 0;
    ret.xy     = read_imagef( flow, SAMPLER_NN, coord ).xy;
    ret.z      = svpmflow_eval( img1, img2, coordf, ret, patchsize );

    write_imagef( output, coord, ret );
}

kernel void svpmflow_prop( write_only image2d_t output, read_only image2d_t input,
                          read_only image2d_t img1, read_only image2d_t img2,
                          const int patchsize, const float flowmax, int iter )
{
    RNG rng;
    const int2 coord = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const float2 coordf = ( float2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const int width = get_image_width( img1 );
    const int height = get_image_height( img1 );

    if( coord.x >= width || coord.y >= height )
        return;

    RNG_init( &rng, coord.y * width + coord.x, 2 * ( NUMRNDTRIES + NUMRNDSAMPLE ) * iter );

    float4 ret = read_imagef( input, SAMPLER_NN, coord  );

    // rand neighbour propagation
    for( int i = 0; i < NUMRNDSAMPLE; i++ ) {
        float4 neighbour = read_imagef( input, SAMPLER_NN, coord + ( int2 )( ( RNG_float(&rng) - 0.5f ) * 2.0f * RNDSAMPLERADIUS + 0.5f, ( RNG_float(&rng) - 0.5f ) * 2.0f * RNDSAMPLERADIUS + 0.5f ) );
        float cost = svpmflow_eval( img1, img2, coordf, neighbour, patchsize );
        if( cost < ret.z ) {
            ret.xy = neighbour.xy;
            ret.z = cost;
        }
    }

    // random try
    for( int i = 0; i < NUMRNDTRIES; i++ ) {
        float4 neighbour = svpmflow_state_init( &rng, flowmax );
        float cost = svpmflow_eval( img1, img2, coordf, neighbour, patchsize );

        if( cost < ret.z ) {
            ret.xy = neighbour.xy;
            ret.z = cost;
        }
    }

    write_imagef( output, coord, ret );
}

kernel void svpmflow_prop_local( write_only image2d_t output, read_only image2d_t input,
                          read_only image2d_t img1, read_only image2d_t img2,
                          const int patchsize, const float flowmax, int iter )
{
#define OFFSET 4
    RNG rng;
    const int lx = get_local_id( 0 );
    const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
    const int2 coord = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const float2 coordf = ( float2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const int2 base = ( int2 )( get_group_id( 0 ) * lw - OFFSET, get_group_id( 1 ) * lh - OFFSET );
    const int width = get_image_width( img1 );
    const int height = get_image_height( img1 );
    local float4 buf[ 16 + 2 * OFFSET  ][ 16 + 2 * OFFSET ];

    RNG_init( &rng, coord.y * width + coord.x, 2 * ( NUMRNDTRIES ) * iter );

    for( int y = ly; y < lh + 2 * OFFSET; y += lh ) {
        for( int x = lx; x < lw + 2 * OFFSET; x += lw ) {
            buf[ y ][ x ] = read_imagef( input, SAMPLER_NN, base + ( int2 ) ( x, y )  );
        }
    }
    barrier( CLK_LOCAL_MEM_FENCE );

    if( coord.x < width && coord.y < height ) {
        float4 ret = buf[ ly + OFFSET ][ lx + OFFSET ];
        float4 neighbour;
        float cost;

        // random try
        for( int i = 0; i < NUMRNDTRIES; i++ ) {
            neighbour = svpmflow_state_init( &rng, flowmax );
            cost = svpmflow_eval( img1, img2, coordf, neighbour, patchsize );

            if( cost < ret.z ) {
                ret.xy = neighbour.xy;
                ret.z = cost;
            }
        }

        // rand neighbour propagation
#if 1
        for( int i = 0; i < NUMRNDSAMPLE; i++ ) {
            float4 neighbour = read_imagef( input, SAMPLER_NN, coord + ( int2 )( ( RNG_float(&rng) - 0.5f ) * 2.0f * RNDSAMPLERADIUS + 0.5f, ( RNG_float(&rng) - 0.5f ) * 2.0f * RNDSAMPLERADIUS + 0.5f ) );
            float cost = svpmflow_eval( img1, img2, coordf, neighbour, patchsize );
            if( cost < ret.z ) {
                ret.xy = neighbour.xy;
                ret.z = cost;
            }
        }
#endif

        buf[ ly + OFFSET ][ lx + OFFSET ] = ret;
        barrier( CLK_LOCAL_MEM_FENCE );

        // rand neighbour propagation
        for( unsigned int i = 4; i >= 1; i >>= 1 ) {

#define NTEST( dx, dy ) \
            neighbour = buf[ ( ly + OFFSET + ( dy ) ) % ( 16 + 2 * OFFSET ) ][ ( lx + OFFSET + ( dx ) ) % ( 16 + 2 * OFFSET ) ]; \
            cost = svpmflow_eval( img1, img2, coordf, neighbour, patchsize ); \
            if( cost < ret.z ) { \
                ret.xy = neighbour.xy; \
                ret.z = cost; \
            }

            NTEST( i, 0 )
            buf[ ly + OFFSET ][ lx + OFFSET ] = ret;
            barrier( CLK_LOCAL_MEM_FENCE );

            NTEST( 0, i )
            buf[ ly + OFFSET ][ lx + OFFSET ] = ret;
            barrier( CLK_LOCAL_MEM_FENCE );

            NTEST( -i, 0 )
            buf[ ly + OFFSET ][ lx + OFFSET ] = ret;
            barrier( CLK_LOCAL_MEM_FENCE );

            NTEST( 0, -i )
            buf[ ly + OFFSET ][ lx + OFFSET ] = ret;
            barrier( CLK_LOCAL_MEM_FENCE );


            //NTEST( -1, 0 )
            //NTEST(  1, 0 )
            //NTEST(  0, -1 )
            //NTEST(  0, 1 )

            //NTEST(  -1, -1 )
            //NTEST(  -1,  1 )
            //NTEST(  1, -1 )
            //NTEST(  1,  1 )

            //barrier( CLK_LOCAL_MEM_FENCE );
            //buf[ ly + OFFSET ][ lx + OFFSET ] = ret;
#undef NTEST
        }

        // random refine
#if 1
        for( int i = 0; i < NUMRNDREFINE; i++ ) {
            neighbour = svpmflow_state_refine( ret, &rng, 10.0f );
            cost = svpmflow_eval( img1, img2, coordf, neighbour, patchsize );

            if( cost < ret.z ) {
                ret.xy = neighbour.xy;
                ret.z = cost;
            }
        }
#endif

        write_imagef( output, coord, ret );
    }
}

static inline float dotsqr( float2 value )
{
    return dot( value, value );
}

kernel void svpmflow_prop_local_tv( write_only image2d_t output, read_only image2d_t input,
                          read_only image2d_t img1, read_only image2d_t img2,
                          write_only image2d_t poutput, read_only image2d_t pinput,
                          const int patchsize, const float flowmax, int iter )
{
#define OFFSET 4
    RNG rng;
    const int lx = get_local_id( 0 );
    const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
    const int2 coord = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const float2 coordf = ( float2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const int2 base = ( int2 )( get_group_id( 0 ) * lw - OFFSET, get_group_id( 1 ) * lh - OFFSET );
    const int2 basep = ( int2 )( get_group_id( 0 ) * lw - 1, get_group_id( 1 ) * lh - 1 );
    const int width = get_image_width( img1 );
    const int height = get_image_height( img1 );
    local float4 buf[ 16 + 2 * OFFSET  ][ 16 + 2 * OFFSET ];
    local float4 bufp[ 16 + 2 ][ 16 + 2 ];
    const float tau = 1.0f;//0.5f *  smoothstep( 0.001f, 1.0f, 1.0f - ( ( float ) iter ) / 199.0f );// * pow( 1.05, iter );
    float lambda = 0.04f;//0.5f *  smoothstep( 0.001f, 1.0f, 1.0f - ( ( float ) iter ) / 199.0f );// * pow( 1.05, iter );

    RNG_init( &rng, coord.y * width + coord.x, 2 * ( NUMRNDTRIES ) * iter );

    for( int y = ly; y < lh + 2 * OFFSET; y += lh ) {
        for( int x = lx; x < lw + 2 * OFFSET; x += lw ) {
            buf[ y ][ x ] = read_imagef( input, SAMPLER_NN, base + ( int2 ) ( x, y )  );
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );

    for( int y = ly; y < lh + 2; y += lh ) {
        for( int x = lx; x < lw + 2 ; x += lw ) {
            float4 p = read_imagef( pinput, SAMPLER_NN_ZERO, basep + ( int2 ) ( x, y ) );
            float2 dx = buf[ y + OFFSET - 1 ][ x + OFFSET - 1 + 1 ].xy - buf[ y + OFFSET - 1 ][ x + OFFSET - 1 ].xy;
            float2 dy = buf[ y + OFFSET - 1 + 1 ][ x + OFFSET - 1 ].xy - buf[ y + OFFSET - 1 ][ x + OFFSET - 1 ].xy;
            p = p + tau * ( float4 ) ( dx, dy );
//            float2 norm = ( float2 ) ( length( dx), length( dy ) );
            float2 pproj = ( float2 ) fmax( ( float2 )1.0f, ( float2 ) ( length( p.xy ), length( p.zw ) ) );
            bufp[ y ][ x ] = p / pproj.xxyy;
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );

//    if( coord.x < width && coord.y < height ) {
        const float4 p = bufp[ ly + 1 ][ lx + 1 ];
        float2 div =  p.lo - bufp[ ly + 1 ][ lx + 1 - 1 ].lo + p.hi - bufp[ ly + 1 - 1 ][ lx + 1 ].hi;
        float4 ret = buf[ ly + OFFSET ][ lx + OFFSET ];
        const float2 current = ret.xy + lambda * div;
        float4 neighbour;
        float cost;

        ret.z = svpmflow_eval( img1, img2, coordf, ret, patchsize );

        lambda = iter < 5 ? 0.0f : 10.0f;//5000.0f * smoothstep( 0.0f, 1.0f, ( ( float ) iter ) / 199.0f );
        //lambda = 0.000001f / ( 2.0f * lambda );

        //neighbour.xy = current;
        //cost = svpmflow_eval( img1, img2, coordf, neighbour, patchsize );
        //if( cost + lambda * dotsqr( neighbour.xy - current )  < ret.z + lambda * dotsqr( ret.xy - current ) ) {
        //    ret.xy = neighbour.xy;
        //    ret.z = cost;
        //}


        // random try
        for( int i = 0; i < NUMRNDTRIES; i++ ) {
            neighbour = svpmflow_state_init( &rng, flowmax );
            cost = svpmflow_eval( img1, img2, coordf, neighbour, patchsize );

            if( cost + lambda * dotsqr( neighbour.xy - current )  < ret.z + lambda * dotsqr( ret.xy - current ) ) {
                ret.xy = neighbour.xy;
                ret.z = cost;
            }
        }

        // rand neighbour propagation
#if 1
        for( int i = 0; i < NUMRNDSAMPLE; i++ ) {
            neighbour = read_imagef( input, SAMPLER_NN, coord + ( int2 )( ( RNG_float(&rng) - 0.5f ) * 2.0f * RNDSAMPLERADIUS + 0.5f, ( RNG_float(&rng) - 0.5f ) * 2.0f * RNDSAMPLERADIUS + 0.5f ) );
            cost = svpmflow_eval( img1, img2, coordf, neighbour, patchsize );
            if( cost + lambda * dotsqr( neighbour.xy - current )  < ret.z + lambda * dotsqr( ret.xy - current ) ) {
                ret.xy = neighbour.xy;
                ret.z = cost;
            }
        }
#endif

        buf[ ly + OFFSET ][ lx + OFFSET ] = ret;
        barrier( CLK_LOCAL_MEM_FENCE );

        // rand neighbour propagation
        for( unsigned int i = 4; i >= 1; i >>= 1 ) {

#define NTEST( dx, dy ) \
            neighbour = buf[ ( ly + OFFSET + ( dy ) ) % ( 16 + 2 * OFFSET ) ][ ( lx + OFFSET + ( dx ) ) % ( 16 + 2 * OFFSET ) ]; \
            cost = svpmflow_eval( img1, img2, coordf, neighbour, patchsize ); \
            if( cost + lambda * dotsqr( neighbour.xy - current )  < ret.z + lambda * dotsqr( ret.xy - current ) ) { \
                ret.xy = neighbour.xy; \
                ret.z = cost; \
            }

            NTEST( i, 0 )
            buf[ ly + OFFSET ][ lx + OFFSET ] = ret;
            barrier( CLK_LOCAL_MEM_FENCE );

            NTEST( 0, i )
            buf[ ly + OFFSET ][ lx + OFFSET ] = ret;
            barrier( CLK_LOCAL_MEM_FENCE );

            NTEST( -i, 0 )
            buf[ ly + OFFSET ][ lx + OFFSET ] = ret;
            barrier( CLK_LOCAL_MEM_FENCE );

            NTEST( 0, -i )
            buf[ ly + OFFSET ][ lx + OFFSET ] = ret;
            barrier( CLK_LOCAL_MEM_FENCE );


            //NTEST( -1, 0 )
            //NTEST(  1, 0 )
            //NTEST(  0, -1 )
            //NTEST(  0, 1 )

            //NTEST(  -1, -1 )
            //NTEST(  -1,  1 )
            //NTEST(  1, -1 )
            //NTEST(  1,  1 )

            //barrier( CLK_LOCAL_MEM_FENCE );
            //buf[ ly + OFFSET ][ lx + OFFSET ] = ret;

#undef NTEST
        }

        // random refine
#if 1
        for( int i = 0; i < NUMRNDREFINE; i++ ) {
            neighbour = svpmflow_state_refine( ret, &rng, 1.0f );
            cost = svpmflow_eval( img1, img2, coordf, neighbour, patchsize );

            if( cost + lambda * dotsqr( neighbour.xy - current )  < ret.z + lambda * dotsqr( ret.xy - current ) ) {
                ret.xy = neighbour.xy;
                ret.z = cost;
            }
        }
#endif

    if( coord.x < width && coord.y < height ) {
        write_imagef( poutput, coord, p );
        write_imagef( output, coord, ret );
    }
}

kernel void svpmflow_prop_local_tv_admm( write_only image2d_t output, read_only image2d_t input,
                          read_only image2d_t img1, read_only image2d_t img2,
                          write_only image2d_t poutput, read_only image2d_t pinput,
                          const int patchsize, const float flowmax, int iter )
{
#define OFFSET 4
    RNG rng;
    const int lx = get_local_id( 0 );
    const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
    const int2 coord = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const float2 coordf = ( float2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const int2 base = ( int2 )( get_group_id( 0 ) * lw - OFFSET, get_group_id( 1 ) * lh - OFFSET );
    const int2 basep = ( int2 )( get_group_id( 0 ) * lw - 1, get_group_id( 1 ) * lh - 1 );
    const int width = get_image_width( img1 );
    const int height = get_image_height( img1 );
    local float4 buf[ 16 + 2 * OFFSET  ][ 16 + 2 * OFFSET ];
    local float4 bufp[ 16 + 2 ][ 16 + 2 ];
    float tau = 0.1f;// * smoothstep( 0.001f, 1.0f, 1.0f - ( ( float ) iter ) / 199.0f );// * pow( 1.05, iter );
    float lambda = 0.01f;// * smoothstep( 0.001f, 1.0f, 1.0f - ( ( float ) iter ) / 199.0f );// * pow( 1.05, iter );


    RNG_init( &rng, coord.y * width + coord.x, 2 * ( NUMRNDTRIES ) * iter );

    for( int y = ly; y < lh + 2 * OFFSET; y += lh ) {
        for( int x = lx; x < lw + 2 * OFFSET; x += lw ) {
            buf[ y ][ x ] = read_imagef( input, SAMPLER_NN, base + ( int2 ) ( x, y )  );
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );

    for( int y = ly; y < lh + 2; y += lh ) {
        for( int x = lx; x < lw + 2 ; x += lw ) {
            float4 p = read_imagef( pinput, SAMPLER_NN_ZERO, basep + ( int2 ) ( x, y ) );
            float2 dx = buf[ y + OFFSET - 1 ][ x + OFFSET - 1 + 1 ].xy - buf[ y + OFFSET - 1 ][ x + OFFSET - 1 ].xy;
            float2 dy = buf[ y + OFFSET - 1 + 1 ][ x + OFFSET - 1 ].xy - buf[ y + OFFSET - 1 ][ x + OFFSET - 1 ].xy;
            float4 z = p + ( float4 ) ( dx, dy );
//            float2 norm = ( float2 ) ( length( dx), length( dy ) );
//          float2 zproj = ( float2 ) fmax( ( float2 )1.0f, ( float2 ) ( length( z.xy ), length( z.zw ) ) );
            float2 zproj;
            zproj.x = fmax( 0.0f, 1.0f - tau / fmax( length( z.xy ), 1e-10f ) );
            zproj.y = fmax( 0.0f, 1.0f - tau / fmax( length( z.zw ), 1e-10f ) );
            float4 pnew = p + ( float4 )( dx, dy ) - z * zproj.xxyy;
            bufp[ y ][ x ] = pnew + 0.5f * ( pnew - p );
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );

//    if( coord.x < width && coord.y < height ) {
        const float4 p = iter < 5 ? ( float4 ) 0.0f : bufp[ ly + 1 ][ lx + 1 ];
        float2 div =  p.lo - bufp[ ly + 1 ][ lx + 1 - 1 ].lo + p.hi - bufp[ ly + 1 - 1 ][ lx + 1 ].hi;
        float4 ret = buf[ ly + OFFSET ][ lx + OFFSET ];
        const float2 current = ret.xy + ( lambda / tau ) * div;
        float4 neighbour;
        float cost;

//        lambda = iter < 5 ? 0.0f : 1.0f / ( lambda * 2.0f );//5000.0f * smoothstep( 0.0f, 1.0f, ( ( float ) iter ) / 199.0f );
        lambda = iter < 5 ? 0.0f : 1.0f / ( 2.0f * lambda * 5.0f );

        neighbour.xy = current;
        cost = svpmflow_eval( img1, img2, coordf, neighbour, patchsize );
        if( cost + lambda * dotsqr( neighbour.xy - current )  < ret.z + lambda * dotsqr( ret.xy - current ) ) {
            ret.xy = neighbour.xy;
            ret.z = cost;
        }


        // random try
        for( int i = 0; i < NUMRNDTRIES; i++ ) {
            neighbour = svpmflow_state_init( &rng, flowmax );
            cost = svpmflow_eval( img1, img2, coordf, neighbour, patchsize );

            if( cost + lambda * dotsqr( neighbour.xy - current )  < ret.z + lambda * dotsqr( ret.xy - current ) ) {
                ret.xy = neighbour.xy;
                ret.z = cost;
            }
        }

        // rand neighbour propagation
#if 1
        for( int i = 0; i < NUMRNDSAMPLE; i++ ) {
            neighbour = read_imagef( input, SAMPLER_NN, coord + ( int2 )( ( RNG_float(&rng) - 0.5f ) * 2.0f * RNDSAMPLERADIUS + 0.5f, ( RNG_float(&rng) - 0.5f ) * 2.0f * RNDSAMPLERADIUS + 0.5f ) );
            cost = svpmflow_eval( img1, img2, coordf, neighbour, patchsize );
            if( cost + lambda * dotsqr( neighbour.xy - current )  < ret.z + lambda * dotsqr( ret.xy - current ) ) {
                ret.xy = neighbour.xy;
                ret.z = cost;
            }
        }
#endif

        buf[ ly + OFFSET ][ lx + OFFSET ] = ret;
        barrier( CLK_LOCAL_MEM_FENCE );

        // rand neighbour propagation
        for( unsigned int i = 4; i >= 1; i >>= 1 ) {

#define NTEST( dx, dy ) \
            neighbour = buf[ ( ly + OFFSET + ( dy ) ) % ( 16 + 2 * OFFSET ) ][ ( lx + OFFSET + ( dx ) ) % ( 16 + 2 * OFFSET ) ]; \
            cost = svpmflow_eval( img1, img2, coordf, neighbour, patchsize ); \
            if( cost + lambda * dotsqr( neighbour.xy - current )  < ret.z + lambda * dotsqr( ret.xy - current ) ) { \
                ret.xy = neighbour.xy; \
                ret.z = cost; \
            }

            NTEST( i, 0 )
            buf[ ly + OFFSET ][ lx + OFFSET ] = ret;
            barrier( CLK_LOCAL_MEM_FENCE );

            NTEST( 0, i )
            buf[ ly + OFFSET ][ lx + OFFSET ] = ret;
            barrier( CLK_LOCAL_MEM_FENCE );

            NTEST( -i, 0 )
            buf[ ly + OFFSET ][ lx + OFFSET ] = ret;
            barrier( CLK_LOCAL_MEM_FENCE );

            NTEST( 0, -i )
            buf[ ly + OFFSET ][ lx + OFFSET ] = ret;
            barrier( CLK_LOCAL_MEM_FENCE );


            //NTEST( -1, 0 )
            //NTEST(  1, 0 )
            //NTEST(  0, -1 )
            //NTEST(  0, 1 )

            //NTEST(  -1, -1 )
            //NTEST(  -1,  1 )
            //NTEST(  1, -1 )
            //NTEST(  1,  1 )

            //barrier( CLK_LOCAL_MEM_FENCE );
            //buf[ ly + OFFSET ][ lx + OFFSET ] = ret;

#undef NTEST
        }

        // random refine
#if 1
        for( int i = 0; i < NUMRNDREFINE; i++ ) {
            neighbour = svpmflow_state_refine( ret, &rng, 1.0f );
            cost = svpmflow_eval( img1, img2, coordf, neighbour, patchsize );

            if( cost + lambda * dotsqr( neighbour.xy - current )  < ret.z + lambda * dotsqr( ret.xy - current ) ) {
                ret.xy = neighbour.xy;
                ret.z = cost;
            }
        }
#endif

    if( coord.x < width && coord.y < height ) {
        write_imagef( poutput, coord, p );
        write_imagef( output, coord, ret );
    }
}

kernel void svpmflow_flow( write_only image2d_t output, read_only image2d_t input )
{
    const int2 coord = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const int width = get_image_width( input );
    const int height = get_image_height( input );

    if( coord.x >= width || coord.y >= height )
        return;

    float2 flow = read_imagef( input, SAMPLER_NN, coord ).xy;
    write_imagef( output, coord, ( float4 ) ( flow.x, flow.y, 0.0f, 0.0f ) );
}

#endif
