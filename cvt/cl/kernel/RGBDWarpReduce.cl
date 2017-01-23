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
#import "SE3.cl"

typedef struct {
    float  cost; // marked as invalid for values < 0
    float3 pt;
    float2 grad;
    float2 hessDiag;
} RGBDWarpEntry;

typedef struct {
    float  cost;
    float  count;
    float  b[ 6 ];
    float  A[ 6 + 5 + 4 + 3 + 2 + 1 ];
} RGBDWarpLS;

inline void rgbdwarpentry_reduce_rgbdwarpls( local RGBDWarpLS* warpls, global const RGBDWarpEntry* entry, const Mat3f* K, float threshold )
{
    float8 dx, dy;

    if( entry->cost < 0.0f || entry->cost >= threshold || !isfinite( entry->cost ) )
        return;

    // get the screen jacobian for the transformed 3d point
    SE3_screen_jacobian( &dx, &dy, K, entry->pt );

    // costs are accumulated
    warpls->cost += entry->cost;
    // count is increased
    warpls->count += 1;

    // data term gradient times the screen jacobian
    warpls->b[ 0 ] += dot( ( float2 ) ( dx.s0, dy.s0 ), entry->grad );
    warpls->b[ 1 ] += dot( ( float2 ) ( dx.s1, dy.s1 ), entry->grad );
    warpls->b[ 2 ] += dot( ( float2 ) ( dx.s2, dy.s2 ), entry->grad );
    warpls->b[ 3 ] += dot( ( float2 ) ( dx.s3, dy.s3 ), entry->grad );
    warpls->b[ 4 ] += dot( ( float2 ) ( dx.s4, dy.s4 ), entry->grad );
    warpls->b[ 5 ] += dot( ( float2 ) ( dx.s5, dy.s5 ), entry->grad );

    float8 hdx, hdy;
    hdx.s012345 = entry->hessDiag.x * dx.s012345;
    hdy.s012345 = entry->hessDiag.y * dy.s012345;

    // upper triangle of the Hessian approximation
    warpls->A[  0 + 0 ] += dot( ( float2 ) ( dx.s0, dy.s0 ), ( float2 ) ( hdx.s0, hdy.s0 ) );
    warpls->A[  0 + 1 ] += dot( ( float2 ) ( dx.s0, dy.s0 ), ( float2 ) ( hdx.s1, hdy.s1 ) );
    warpls->A[  0 + 2 ] += dot( ( float2 ) ( dx.s0, dy.s0 ), ( float2 ) ( hdx.s2, hdy.s2 ) );
    warpls->A[  0 + 3 ] += dot( ( float2 ) ( dx.s0, dy.s0 ), ( float2 ) ( hdx.s3, hdy.s3 ) );
    warpls->A[  0 + 4 ] += dot( ( float2 ) ( dx.s0, dy.s0 ), ( float2 ) ( hdx.s4, hdy.s4 ) );
    warpls->A[  0 + 5 ] += dot( ( float2 ) ( dx.s0, dy.s0 ), ( float2 ) ( hdx.s5, hdy.s5 ) );

    warpls->A[  6 + 0 ] += dot( ( float2 ) ( dx.s1, dy.s1 ), ( float2 ) ( hdx.s1, hdy.s1 ) );
    warpls->A[  6 + 1 ] += dot( ( float2 ) ( dx.s1, dy.s1 ), ( float2 ) ( hdx.s2, hdy.s2 ) );
    warpls->A[  6 + 2 ] += dot( ( float2 ) ( dx.s1, dy.s1 ), ( float2 ) ( hdx.s3, hdy.s3 ) );
    warpls->A[  6 + 3 ] += dot( ( float2 ) ( dx.s1, dy.s1 ), ( float2 ) ( hdx.s4, hdy.s4 ) );
    warpls->A[  6 + 4 ] += dot( ( float2 ) ( dx.s1, dy.s1 ), ( float2 ) ( hdx.s5, hdy.s5 ) );

    warpls->A[ 11 + 0 ] += dot( ( float2 ) ( dx.s2, dy.s2 ), ( float2 ) ( hdx.s2, hdy.s2 ) );
    warpls->A[ 11 + 1 ] += dot( ( float2 ) ( dx.s2, dy.s2 ), ( float2 ) ( hdx.s3, hdy.s3 ) );
    warpls->A[ 11 + 2 ] += dot( ( float2 ) ( dx.s2, dy.s2 ), ( float2 ) ( hdx.s4, hdy.s4 ) );
    warpls->A[ 11 + 3 ] += dot( ( float2 ) ( dx.s2, dy.s2 ), ( float2 ) ( hdx.s5, hdy.s5 ) );

    warpls->A[ 15 + 0 ] += dot( ( float2 ) ( dx.s3, dy.s3 ), ( float2 ) ( hdx.s3, hdy.s3 ) );
    warpls->A[ 15 + 1 ] += dot( ( float2 ) ( dx.s3, dy.s3 ), ( float2 ) ( hdx.s4, hdy.s4 ) );
    warpls->A[ 15 + 2 ] += dot( ( float2 ) ( dx.s3, dy.s3 ), ( float2 ) ( hdx.s5, hdy.s5 ) );

    warpls->A[ 18 + 0 ] += dot( ( float2 ) ( dx.s4, dy.s4 ), ( float2 ) ( hdx.s4, hdy.s4 ) );
    warpls->A[ 18 + 1 ] += dot( ( float2 ) ( dx.s4, dy.s4 ), ( float2 ) ( hdx.s5, hdy.s5 ) );

    warpls->A[ 20 + 0 ] += dot( ( float2 ) ( dx.s5, dy.s5 ), ( float2 ) ( hdx.s5, hdy.s5 ) );
}

inline void rgbdwarpls_reduce( local RGBDWarpLS* accum, local const RGBDWarpLS* src )
{
    accum->cost  += src->cost;
    accum->count += src->count;
#pragma unroll
    for( int i = 0; i < 6; i++ )
        accum->b[ i ] += src->b[ i ];

#pragma unroll
    for( int i = 0; i < 21; i++ )
        accum->A[ i ] += src->A[ i ];
}

inline void rgbdwarpls_init( local RGBDWarpLS* warpls )
{
    warpls->cost  = 0;
    warpls->count = 0;
#pragma unroll
    for( int i = 0; i < 6; i++ )
        warpls->b[ i ] = 0;

#pragma unroll
    for( int i = 0; i < 21; i++ )
        warpls->A[ i ] = 0;
}

#define REDUCE_LOCAL_SIZE 256

kernel void rgbdwarp_reduce( write_only global RGBDWarpLS* output, read_only global RGBDWarpEntry* input, uint size, const Mat3f K, float threshold )
{
    const uint lid = get_local_id( 0 );
    const uint grpid = get_group_id( 0 );
    const uint stride = get_num_groups( 0 ) * 2 * REDUCE_LOCAL_SIZE;
    uint idx = grpid * 2 * REDUCE_LOCAL_SIZE + lid;
    local RGBDWarpLS localbuf[ REDUCE_LOCAL_SIZE ];

    rgbdwarpls_init( localbuf + lid );

    while( idx < size ) {
        rgbdwarpentry_reduce_rgbdwarpls( localbuf + lid, input + idx, &K, threshold );
        if( idx + REDUCE_LOCAL_SIZE < size )
            rgbdwarpentry_reduce_rgbdwarpls( localbuf + lid, input + idx + REDUCE_LOCAL_SIZE, &K, threshold );
        idx += stride;
    }
    barrier( CLK_LOCAL_MEM_FENCE );

#if ( REDUCE_LOCAL_SIZE >= 1024 )
    if( lid < 512 ) rgbdwarpls_reduce( ( localbuf + lid ), ( localbuf + lid + 512 ) );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif

#if ( REDUCE_LOCAL_SIZE >= 512 )
    if( lid < 256 ) rgbdwarpls_reduce( ( localbuf + lid ), ( localbuf + lid + 256 ) );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif

#if ( REDUCE_LOCAL_SIZE >= 256 )
    if( lid < 128 ) rgbdwarpls_reduce( ( localbuf + lid ), ( localbuf + lid + 128 ) );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif

#if ( REDUCE_LOCAL_SIZE >= 128 )
    if( lid <  64 ) rgbdwarpls_reduce( ( localbuf + lid ), ( localbuf + lid + 64 ) );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif

    /* we have the barrier here because not all platforms can guarante synchronous operations */
    /* for less than 32 threads*/

#if ( REDUCE_LOCAL_SIZE >= 64 )
    if( lid < 32 ) rgbdwarpls_reduce( ( localbuf + lid ), ( localbuf + lid + 32 ) );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif

#if ( REDUCE_LOCAL_SIZE >= 32 )
    if( lid < 16 ) rgbdwarpls_reduce( ( localbuf + lid ), ( localbuf + lid + 16 ) );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif

#if ( REDUCE_LOCAL_SIZE >= 16 )
    if( lid <  8 ) rgbdwarpls_reduce( ( localbuf + lid ), ( localbuf + lid + 8 ) );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif

#if ( REDUCE_LOCAL_SIZE >= 8 )
    if( lid <  4 ) rgbdwarpls_reduce( ( localbuf + lid ), ( localbuf + lid + 4 ) );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif

#if ( REDUCE_LOCAL_SIZE >= 4 )
    if( lid <  2 ) rgbdwarpls_reduce( ( localbuf + lid ), ( localbuf + lid + 2 ) );
    barrier( CLK_LOCAL_MEM_FENCE );
#endif

    if( lid < 1 ) {
        rgbdwarpls_reduce( ( localbuf + lid ), ( localbuf + lid + 1 ) );
        output[ grpid ] = localbuf[ 0 ];
    }
}
