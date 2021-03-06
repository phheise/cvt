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
#ifndef BHFLOW_H
#define BHFLOW_H

typedef struct {
    uint offset;
    uint size;
} BHBucket;

const sampler_t SAMPLER_NN       = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

#if 0
__constant const uint BHPOSITIONS[ 10 ][ 14 ] = {
    { 148, 162, 69, 57, 45, 239, 228, 93, 194, 22, 228, 179, 185, 254},
    { 170, 248, 182, 234, 167, 158, 110, 28, 2, 234, 201, 236, 96, 157},
    { 41, 9, 190, 62, 44, 131, 119, 216, 115, 219, 54, 181, 242, 154},
    { 106, 44, 25, 164, 207, 127, 77, 237, 105, 89, 52, 92, 185, 81},
    { 133, 67, 143, 195, 111, 146, 186, 199, 7, 151, 125, 188, 138, 24},
    { 166, 54, 176, 60, 218, 128, 59, 40, 169, 18, 221, 4, 70, 58},
    { 61, 23, 63, 128, 39, 4, 185, 55, 64, 86, 180, 253, 96, 76},
    { 36, 150, 124, 223, 114, 252, 155, 26, 72, 200, 241, 134, 10, 66},
    { 7, 49, 197, 246, 4, 173, 171, 226, 47, 82, 172, 173, 196, 73},
    { 94, 14, 139, 92, 215, 125, 222, 94, 160, 183, 102, 91, 35, 206},
};
#endif

__constant const uint BHPOSITIONS[ 20 ][ 14 ] = {
    { 164, 223, 98, 212, 158, 15, 202, 97, 95, 132, 53, 139, 117, 125},
    { 172, 216, 116, 194, 224, 233, 111, 138, 24, 138, 77, 3, 105, 63},
    { 56, 133, 180, 92, 228, 150, 176, 131, 37, 250, 228, 127, 154, 143},
    { 244, 151, 60, 77, 139, 0, 47, 117, 238, 185, 141, 248, 134, 145},
    { 226, 197, 73, 231, 249, 165, 204, 144, 86, 207, 181, 81, 58, 208},
    { 207, 201, 69, 103, 134, 146, 242, 134, 193, 232, 118, 129, 135, 199},
    { 208, 57, 193, 118, 83, 76, 85, 9, 157, 10, 195, 237, 217, 13},
    { 51, 192, 147, 70, 52, 153, 135, 156, 143, 253, 27, 79, 107, 8},
    { 15, 98, 141, 174, 234, 237, 174, 186, 71, 110, 136, 106, 121, 191},
    { 107, 246, 196, 134, 17, 114, 205, 32, 91, 2, 189, 80, 43, 28},
    { 12, 242, 88, 122, 123, 168, 183, 144, 43, 251, 130, 239, 199, 128},
    { 117, 149, 33, 74, 112, 35, 46, 144, 217, 45, 42, 5, 36, 72},
    { 173, 210, 127, 64, 61, 143, 75, 215, 14, 3, 170, 173, 163, 218},
    { 226, 239, 95, 134, 183, 14, 55, 22, 198, 26, 254, 209, 96, 125},
    { 47, 99, 40, 168, 176, 195, 75, 13, 243, 132, 48, 78, 230, 109},
    { 215, 59, 70, 142, 137, 219, 163, 218, 166, 177, 8, 106, 102, 1},
    { 195, 106, 49, 145, 81, 63, 190, 250, 132, 182, 96, 167, 187, 50},
    { 109, 57, 186, 205, 163, 190, 94, 222, 16, 252, 96, 188, 162, 124},
    { 21, 44, 53, 79, 185, 82, 24, 230, 100, 170, 230, 200, 148, 120},
    { 119, 140, 90, 170, 220, 132, 22, 166, 155, 34, 38, 206, 20, 4},
};

inline uint hash_bindesc( const uint8 bindesc, const uint bits, const uint hashnum )
{
    uint hash = 0;
    for( uint i = 0; i < bits; i++ ) {
        const uint pos = BHPOSITIONS[ hashnum ][ i ] >> 5;
        const uint off = BHPOSITIONS[ hashnum ][ i ] & 0x1f;
        hash |= ( ( ( *( ( ( const uint* ) &bindesc ) + pos ) ) >> off ) & 1 ) << i;
    }

    return hash;
}

#import "BRPattern.h"

inline bool brief_test( read_only image2d_t image, const float2 pos1, const float2 pos2 )
{
 #define TOGRAY(x) dot( x, grayWeight )
    const float4 grayWeight =  ( float4 ) ( 0.2126f, 0.7152f, 0.0722f, 0.0f );
    const float2 offsethalf = ( float2 ) 0.5f;
    float a = ( read_imagef( image, SAMPLER_BILINEAR, pos1 + offsethalf ).x );
    float b = ( read_imagef( image, SAMPLER_BILINEAR, pos2 + offsethalf ).x );

#if 0
    return ( ( a - b ) / ( a + b ) ) < 0;
#else
    return ( a < b );
#endif
}

inline uint8 brief_descriptor( read_only image2d_t image, int2 pos )
{
    uint8 result = ( uint8 ) 0;
    const float SCALE = 2.0f;

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

kernel void bhflow_brief32( global uint8* brief, const int width, const int height, read_only image2d_t image )
{
    const int iwidth = get_image_width( image );
    const int iheight = get_image_height( image );
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );

    if( gx >= iwidth || gy >= iheight )
        return;

    uint8 bdesc = brief_descriptor( image, ( int2 ) ( gx, gy ) );
    brief[ width * gy + gx ] = bdesc;
}

inline uint brief_descriptor_N( read_only image2d_t image, const int2 pos, const int N )
{
    uint result = ( uint ) 0;
    const float SCALE = 2.0f;

    #define DOBRIEFTEST( testn ) ( brief_test( image, ( float2 ) ( pos.x + SCALE * _brief_pattern[ testn ][ 0 ], pos.y + SCALE * _brief_pattern[ testn ][ 1 ] ), ( float2 ) ( pos.x + SCALE * _brief_pattern[ testn ][ 2 ], pos.y + SCALE * _brief_pattern[ testn ][ 3 ] ) ) )

    for( int t = 0; t < 32; t++ ) result |= ( DOBRIEFTEST( N * 32 + t ) << t );

    #undef DOBRIEFTEST

    return result;
}

kernel void bhflow_brief32_N( global uint8* brief, const int width, const int height, read_only image2d_t image )
{
    const int iwidth = get_image_width( image );
    const int iheight = get_image_height( image );
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int gz = get_global_id( 2 ); // must be 4

    if( gx >= iwidth || gy >= iheight )
        return;

    uint bdesc = brief_descriptor_N( image, ( int2 ) ( gx, gy ), gz * 2 );
    *( ( ( global uint* ) &brief[ width * gy + gx ] ) + gz * 2 ) = bdesc;
    bdesc = brief_descriptor_N( image, ( int2 ) ( gx, gy ), gz * 2 + 1 );
    *( ( ( global uint* ) &brief[ width * gy + gx ] ) + gz * 2 + 1 ) = bdesc;

}

kernel void bhflow_brief32_local( global uint8* brief, const int width, const int height, read_only image2d_t image )
{
    const int iwidth = get_image_width( image );
    const int iheight = get_image_height( image );
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int lx = get_local_id( 0 );
    const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
#define BORDER 32
    const int2 base = ( int2 )( get_group_id( 0 ) * lw - BORDER, get_group_id( 1 ) * lh - BORDER );
    const int bstride = lw + 2;

    local float buf[ 8 + BORDER * 2 ][ 16 + BORDER * 2 ];

    for( int y = ly; y < lh + 2 * BORDER; y += lh ) {
        for( int x = lx; x < lw + 2 * BORDER; x += lw ) {
            buf[ y ][ x ] = read_imagef( image, SAMPLER_NN, base + ( int2 )( x, y ) ).x;
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );


    if( gx >= iwidth || gy >= iheight )
        return;

    uint8 result = ( uint8 ) 0;
    const float SCALE = 1.0f;
    int2 pos = ( int2 ) ( gx, gy );

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
    #undef BORDER

    brief[ width * gy + gx ] = result;
}

kernel void bhflow_hash_clear( global BHBucket* buckets, const uint size )
{
    const int id = get_global_id( 0 );

    if( id >= size )
        return;

    buckets[ id ].offset = 0;
    buckets[ id ].size   = 0;
}

kernel void bhflow_hash_count( global BHBucket* buckets, const uint bits, const uint numhash, int hashwidth, int hashheight, global uint8* brief, const int width, const int height )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const uint size = ( 1 << bits );

    if( gx >= width || gy >= height )
        return;

    uint8 value = brief[ gy * width + gx ];

    int bx = gx / hashwidth;
    int by = gy / hashheight;
    int bwidth = ( width + hashwidth - 1 ) / hashwidth;
    int bsize = hashwidth * hashheight;

    for( uint nhash = 0; nhash < numhash; nhash++ ) {
        uint hash = hash_bindesc( value, bits, nhash );
        uint index = ( bwidth * by + bx ) * size * numhash + size * nhash + hash;
        atomic_inc( &buckets[ index ].size );
    }
}

kernel void bhflow_hash_calcoffsets( global BHBucket* buckets, const uint bits, const uint numhash, const uint hashwidth, const uint hashheight, const int bwidth, const int bheight )
{
    const int bx = get_global_id( 0 );
    const int by = get_global_id( 1 );
    const int bz = get_global_id( 2 );
    const uint size = ( 1 << bits );

    if( bx >= bwidth || by >= bheight || bz >= numhash )
        return;


    uint bsize = hashwidth * hashheight;
    uint index = ( bwidth * by + bx ) * size * numhash + size * bz;
    uint offset = ( bwidth * by + bx ) * bsize * numhash + bsize * bz;

    buckets[ index ].offset = offset;
    for( uint i = 1; i < size; i++ ) {
        offset += buckets[ index + i - 1 ].size;
        // clear current size
        buckets[ index + i - 1 ].size = 0;
        buckets[ index + i ].offset = offset;
    }
    buckets[ index + size - 1 ].size = 0;
}

kernel void bhflow_hash( global uint2* map, global BHBucket* buckets, const uint bits, const uint numhash, const uint hashwidth, const uint hashheight, global uint8* briefin, const int width, const int height )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int gz = get_global_id( 2 );
    const int lz = get_local_size( 2 );
    const uint size = ( 1 << bits );

    if( gx >= width || gy >= height )
        return;

    const int bx = gx / hashwidth;
    const int by = gy / hashheight;
    const int bwidth = ( width + hashwidth - 1 ) / hashwidth;

    const uint8 value = briefin[ gy * width + gx ];

    const uint baseindex = ( bwidth * by + bx ) * size * numhash;
    for( uint nhash = gz; nhash < numhash; nhash += lz ) {
        const uint hash = hash_bindesc( value, bits, nhash );
        const uint index = baseindex + size * nhash + hash;
        const uint offset = atomic_inc( &buckets[ index ].size ) + buckets[ index ].offset;
        map[ offset ] = ( uint2 ) ( gx, gy );
    }
}

/*
 * Hacker's Delight 32 bit pop function:
 * http://www.hackersdelight.org/HDcode/newCode/pop_arrayHS.c.txt
 */
inline uint popcnt( uint x )
{
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x + (x >> 4)) & 0x0F0F0F0F;
    x = x + (x >> 8);
    x = x + (x >> 16);
    return x & 0x0000003F;
}

inline uint hamming_distance( const uint8 a, const uint8 b )
{
    uint8 ret = popcount( a^b );
    ret.s0123 += ret.s4567;
    return ret.s0 + ret.s1 + ret.s2 + ret.s3;
    //uint8 val = a^b;
    //uint ret;
    //ret  = popcnt( val.s0 );
    //ret += popcnt( val.s1 );
    //ret += popcnt( val.s2 );
    //ret += popcnt( val.s3 );
    //ret += popcnt( val.s4 );
    //ret += popcnt( val.s5 );
    //ret += popcnt( val.s6 );
    //ret += popcnt( val.s7 );
   // return ret;
}

kernel void bhflow_hash_match( write_only image2d_t flow, global BHBucket* buckets, const uint bits, const uint numhash, const uint hashwidth, const uint hashheight, global uint8* brief, global uint2* map, global uint8* brief2, const int iwidth, const int iheight )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const uint size = ( 1 << bits );

    if( gx >= iwidth || gy >= iheight )
        return;

    int bx = gx / hashwidth;
    int by = gy / hashheight;
    const int bwidth = ( iwidth + hashwidth - 1 ) / hashwidth;
    const int bheight = ( iheight + hashheight - 1 ) / hashheight;

    const uint8 bindesc = brief2[ gy * iwidth + gx ];
    uint3 val = ( uint3 )( gx, gy, 256 );
    uint3 tval;

#define NEIGHBOURS 1
    const int startbx = max( bx - NEIGHBOURS, 0 );
    const int endbx = min( bx + NEIGHBOURS, bwidth - 1 );
    const int startby = max( by - NEIGHBOURS, 0 );
    const int endby = min( by + NEIGHBOURS, bheight - 1 );

    for( by = startby; by <= endby; by++ ) {
        for( bx = startbx; bx <= endbx; bx++ ) {
            for( uint nhash = 0; nhash < numhash; nhash++ ) {
                const uint hash = hash_bindesc( bindesc, bits, nhash );
                const uint index = ( bwidth * by + bx ) * size * numhash + size * nhash + hash;
                const uint offset = buckets[ index ].offset;
                // Runtime guarantee
                // const uint N      = min( buckets[ index ].size, (uint) 10 );
                const uint N      = buckets[ index ].size;

                for( uint i = 0; i < N; i++ ) {
                    tval.xy = map[ offset + i ];
                    tval.z  = hamming_distance( bindesc, brief[ tval.y * iwidth + tval.x ] );
                    val = select( val, tval, ( uint3 ) -( tval.z < val.z ) );
                }
            }
        }
    }

    write_imagef( flow, ( int2 ) ( gx, gy ), ( float4 ) ( ( ( float ) val.x ) - ( ( float ) gx ), ( ( float ) val.y ) - ( ( float ) gy ), 0.0f, 0.0f ) );
}

#endif
