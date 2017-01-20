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

#import "Feature.cl"
#import "BRIEFPattern.h"


inline bool brief_test( read_only image2d_t image, const float2 pos1, const float2 pos2 )
{
 #define TOGRAY(x) dot( x, grayWeight )
    const float4 grayWeight =  ( float4 ) ( 0.2126f, 0.7152f, 0.0722f, 0.0f );
    const float2 offsethalf = ( float2 ) 0.5f;
    const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
    float a = TOGRAY( read_imagef( image, SAMPLER_BILINEAR, pos1 + offsethalf ) );
    float b = TOGRAY( read_imagef( image, SAMPLER_BILINEAR, pos2 + offsethalf ) );

#if 0
    return ( ( a - b ) / ( a + b ) ) < 0;
#else
    return ( a < b );
#endif
}

inline uint8 brief_descriptor( read_only image2d_t image, int2 pos )
{
    uint8 result = ( uint8 ) 0;
    const float SCALE = 1.0f;

    #define DOBRIEFTEST( testn ) ( brief_test( image, ( float2 ) ( pos.x + SCALE * _brief_pattern[ testn ][ 0 ], pos.y + SCALE * _brief_pattern[ testn ][ 1 ] ), ( float2 ) ( pos.x + SCALE * _brief_pattern[ testn ][ 2 ], pos.y + SCALE * _brief_pattern[ testn ][ 3 ] ) ) )

    for( int t = 0; t < 32; t++ ) result.s0 |= ( DOBRIEFTEST( 0 * 32 + t ) << t );
    for( int t = 0; t < 32; t++ ) result.s1 |= ( DOBRIEFTEST( 1 * 32 + t ) << t );
    for( int t = 0; t < 32; t++ ) result.s2 |= ( DOBRIEFTEST( 2 * 32 + t ) << t );
    for( int t = 0; t < 32; t++ ) result.s3 |= ( DOBRIEFTEST( 3 * 32 + t ) << t );
    for( int t = 0; t < 32; t++ ) result.s4 |= ( DOBRIEFTEST( 4 * 32 + t ) << t );
    for( int t = 0; t < 32; t++ ) result.s5 |= ( DOBRIEFTEST( 5 * 32 + t ) << t );
    for( int t = 0; t < 32; t++ ) result.s6 |= ( DOBRIEFTEST( 6 * 32 + t ) << t );
    for( int t = 0; t < 32; t++ ) result.s7 |= ( DOBRIEFTEST( 7 * 32 + t ) << t );

    #undef DOBRIEFTEST

    return result;
}

kernel void brief( global uint8* brief, global Feature* features, int size, read_only image2d_t image )
{
    const int id = get_global_id( 0 );

    if( id >= size )
        return;

    brief[ id ] = brief_descriptor( image, features[ id ].pt );
}

inline uint hamming_distance( const uint8 a, const uint8 b )
{
    uint8 ret = popcount( a^b );
    ret.s0123 += ret.s4567;
    ret.s01   += ret.s23;
    return ret.s0 + ret.s1;
}

kernel void brief_stereo_match( global float3* matches, global int* match_size, int match_max,
                                global Feature* feature1, global uint8* brief1, int size1,
                                global Feature* feature2, global uint8* brief2, int size2, global int2* sclidx2,
                                int threshold )
{
    const int id = get_global_id( 0 );

    if( id >= size1 )
        return;

    Feature f  = feature1[ id ];
    uint8 desc = brief1[ id ];

    int2 scl = sclidx2[ f.pt.y ];
    int best = -1;
    uint bestdist = 0xff;
    int bestdisparity = 0;

    for( int offset = 0; offset < scl.y; offset++ ) {
        Feature f2 = feature2[ scl.x + offset ];
        if( f2.pt.x >= f.pt.x )
            continue;
        uint8 desc2 = brief2[ scl.x + offset ];
        uint dist = hamming_distance( desc, desc2 );
        if( dist < bestdist ) {
            bestdist = dist;
            best = offset;
            bestdisparity = f.pt.x  - f2.pt.x;
        }
    }

    if( best >= 0 && bestdist <= threshold ) {
        int idx = atomic_inc( match_size );
        if( idx < match_max ) {
            matches[ idx ] = ( float3 ) ( f.pt.x, f.pt.y, bestdisparity );
        }
    }
}
