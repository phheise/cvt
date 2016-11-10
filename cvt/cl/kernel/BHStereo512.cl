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


typedef struct {
    uint offset;
    uint size;
} BHBucket;

const sampler_t SAMPLER_NN       = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

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

/*{ 436, 38, 140, 74, 40, 28, 251, 68, 337, 314, 16, 193, 364, 117},
{ 34, 263, 423, 358, 5, 169, 290, 91, 83, 237, 190, 410, 64, 358},
{ 282, 78, 466, 192, 116, 79, 266, 156, 501, 224, 413, 304, 224, 95},
{ 141, 325, 113, 405, 221, 456, 410, 391, 235, 485, 458, 457, 165, 341},
{ 10, 507, 113, 72, 446, 289, 188, 509, 44, 328, 18, 25, 487, 306},
{ 249, 55, 447, 47, 152, 341, 269, 97, 225, 149, 317, 183, 80, 348},
{ 422, 257, 328, 8, 329, 247, 297, 6, 246, 334, 320, 343, 343, 280},
{ 138, 81, 335, 58, 128, 487, 384, 381, 98, 3, 281, 84, 127, 103},
{ 490, 368, 415, 498, 186, 284, 176, 382, 98, 494, 175, 425, 326, 439},
{ 36, 391, 94, 8, 223, 478, 390, 281, 49, 393, 145, 315, 461, 272},*/
};

inline uint hash_bindesc( const uint16 bindesc, uint bits, uint hashnum )
{
    uint hash = 0;
    for( uint i = 0; i < bits; i++ ) {
        uint pos = BHPOSITIONS[ hashnum ][ i ] >> 5;
        uint off = BHPOSITIONS[ hashnum ][ i ] & 0x1f;
        hash |= ( ( ( *( ( ( const uint* ) &bindesc ) + pos ) ) >> off ) & 1 ) << i;
    }

    return hash;
}

#import "SLPattern512.h"

inline uint16 brief_descriptor( read_only image2d_t image, int2 pos )
{
    uint16 result = ( uint16 ) 0;

    #define DOBRIEFTEST( testn ) ( read_imagef( image, SAMPLER_BILINEAR, ( float2 ) ( pos.x + _brief_pattern[ testn ][ 0 ] * 0.5f, pos.y + _brief_pattern[ testn ][ 1 ] * 0.5f ) ).x < \
                                   read_imagef( image, SAMPLER_BILINEAR, ( float2 ) ( pos.x + _brief_pattern[ testn ][ 2 ] * 0.5f, pos.y + _brief_pattern[ testn ][ 3 ] * 0.5f ) ).x )

    for( int t = 0; t < 32; t++ ) result.s0  |= ( DOBRIEFTEST( 0 * 32 + t ) << t );
    for( int t = 0; t < 32; t++ ) result.s1  |= ( DOBRIEFTEST( 1 * 32 + t ) << t );
    for( int t = 0; t < 32; t++ ) result.s2  |= ( DOBRIEFTEST( 2 * 32 + t ) << t );
    for( int t = 0; t < 32; t++ ) result.s3  |= ( DOBRIEFTEST( 3 * 32 + t ) << t );
    for( int t = 0; t < 32; t++ ) result.s4  |= ( DOBRIEFTEST( 4 * 32 + t ) << t );
    for( int t = 0; t < 32; t++ ) result.s5  |= ( DOBRIEFTEST( 5 * 32 + t ) << t );
    for( int t = 0; t < 32; t++ ) result.s6  |= ( DOBRIEFTEST( 6 * 32 + t ) << t );
    for( int t = 0; t < 32; t++ ) result.s7  |= ( DOBRIEFTEST( 7 * 32 + t ) << t );
    for( int t = 0; t < 32; t++ ) result.s8  |= ( DOBRIEFTEST( 8  * 32 + t ) << t );
    for( int t = 0; t < 32; t++ ) result.s9  |= ( DOBRIEFTEST( 9  * 32 + t ) << t );
    for( int t = 0; t < 32; t++ ) result.sa |= ( DOBRIEFTEST( 10 * 32 + t ) << t );
    for( int t = 0; t < 32; t++ ) result.sb |= ( DOBRIEFTEST( 11 * 32 + t ) << t );
    for( int t = 0; t < 32; t++ ) result.sc |= ( DOBRIEFTEST( 12 * 32 + t ) << t );
    for( int t = 0; t < 32; t++ ) result.sd |= ( DOBRIEFTEST( 13 * 32 + t ) << t );
    for( int t = 0; t < 32; t++ ) result.se |= ( DOBRIEFTEST( 14 * 32 + t ) << t );
    for( int t = 0; t < 32; t++ ) result.sf |= ( DOBRIEFTEST( 15 * 32 + t ) << t );

    #undef DOBRIEFTEST

    return result;
}

kernel void bhstereo_brief32( global uint16* brief, const int width, const int height, read_only image2d_t image )
{
    const int iwidth = get_image_width( image );
    const int iheight = get_image_height( image );
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );

    if( gx >= iwidth || gy >= iheight )
        return;

    brief[ width * gy + gx ] = brief_descriptor( image, ( int2 ) ( gx, gy ) );
}

kernel void bhstereo_hash_clear( global BHBucket* buckets, const uint size )
{
    const int id = get_global_id( 0 );

    if( id >= size )
        return;

    buckets[ id ].offset = 0;
    buckets[ id ].size   = 0;
}

kernel void bhstereo_hash_count( global BHBucket* buckets, const uint bits, const uint numhash, global uint16* brief, const int width, const int height )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const uint size = ( 1 << bits );

    if( gx >= width || gy >= height )
        return;

    uint16 value = brief[ gy * width + gx ];

    for( uint nhash = 0; nhash < numhash; nhash++ ) {
        uint hash = hash_bindesc( value, bits, nhash );
        atomic_inc( &buckets[ ( size * height ) * nhash + gy * size + hash ].size );
    }
}

kernel void bhstereo_hash_calcoffsets( global BHBucket* buckets, const uint bits, const uint height )
{
    const int gx = get_global_id( 0 );
    const uint size = ( 1 << bits );

    if( gx >= height )
        return;

    int offset = 0;
    buckets[ gx * size + 0 ].offset = 0;

    for( int i = 1; i < size; i++ ) {
        offset += buckets[ gx * size + i - 1 ].size;
        // clear current size
        buckets[ gx * size + i - 1 ].size = 0;
        buckets[ gx * size + i ].offset = offset;
    }
    buckets[ gx * size + size - 1 ].size = 0;
}

kernel void bhstereo_hash( global uint* map, global BHBucket* buckets, const uint bits, const uint numhash, global uint16* briefin, const int width, const int height )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const uint size = ( 1 << bits );

    if( gx >= width || gy >= height )
        return;

    const uint16 value = briefin[ gy * width + gx ];

    for( uint nhash = 0; nhash < numhash; nhash++ ) {
        uint hash = hash_bindesc( value, bits, nhash );
        int offset = atomic_inc( &buckets[ ( size * height ) * nhash + gy * size + hash ].size );
        offset += buckets[ ( size * height ) * nhash + gy * size + hash ].offset;
        map[ ( width * height ) * nhash + gy * width + offset ] = gx;
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

inline uint hamming_distance( const uint16 a, const uint16 b )
{
    uint16 val = a^b;
    uint ret;
    ret  = popcnt( val.s0 );
    ret += popcnt( val.s1 );
    ret += popcnt( val.s2 );
    ret += popcnt( val.s3 );
    ret += popcnt( val.s4 );
    ret += popcnt( val.s5 );
    ret += popcnt( val.s6 );
    ret += popcnt( val.s7 );
    ret += popcnt( val.s8 );
    ret += popcnt( val.s9 );
    ret += popcnt( val.sa );
    ret += popcnt( val.sb );
    ret += popcnt( val.sc );
    ret += popcnt( val.sd );
    ret += popcnt( val.se );
    ret += popcnt( val.sf );
    return ret;
}

kernel void bhstereo_hash_match( write_only image2d_t disparity, global BHBucket* buckets, const uint bits, const uint numhash, global uint16* brief, global uint* map, read_only image2d_t image )
{
    uint16 result;
    const int iwidth = get_image_width( image );
    const int iheight = get_image_height( image );
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const uint size = ( 1 << bits );

    if( gx >= iwidth || gy >= iheight )
        return;

    result = brief_descriptor( image, ( int2 ) ( gx, gy ) );
    int val = gx;
    uint dist  = 512;

    for( uint nhash = 0; nhash < numhash; nhash++ ) {
        uint hash = hash_bindesc( result, bits, nhash );

        int offset = buckets[ ( size * iheight ) * nhash + gy * size + hash ].offset;
        int N      = buckets[ ( size * iheight ) * nhash + gy * size + hash ].size;

        for( int i = 0; i < N; i++ ) {
            uint cval  = map[ ( iwidth * iheight ) * nhash + gy * iwidth + offset + i ];
            if( cval < gx ) {
                uint cdist = hamming_distance( result, brief[ gy * iwidth + cval ] );
                if( cdist < dist ) {
                    dist = cdist;
                    val  = cval;
                }
            }
        }
    }

    write_imagef( disparity, ( int2 ) ( gx, gy ), abs( val - gx ) );
}
