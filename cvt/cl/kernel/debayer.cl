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
const sampler_t SAMPLER_NN = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

inline int mirror( int value, int len )
{
    if( value < 0 )
        return - ( value % ( len - 1 ) );
    if( value >= len )
        return len - 1 - ( value % ( len - 1 ) );
    return value;
}

kernel void debayer_RGGB_RGBA( write_only image2d_t dst, read_only image2d_t src, local float* buf)
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int lx = get_local_id( 0 );
    const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
    const int width = get_image_width( src );
    const int height = get_image_height( src );
    const int2 base = ( int2 )( get_group_id( 0 ) * lw - 1, get_group_id( 1 ) * lh - 1 );
    const int bstride = lw + 2;
    float2 coord;
    float4 dx, dy;

    for( int y = ly; y < lh + 2; y += lh ) {
        for( int x = lx; x < lw + 2; x += lw ) {
            int2 coord = base + ( int2 )( x, y );
            coord.x = mirror( coord.x, width );
            coord.y = mirror( coord.y, height );
            buf[ mul24( y, bstride ) + x ] = read_imagef( src, SAMPLER_NN, coord ).x;
        }
    }
    barrier( CLK_LOCAL_MEM_FENCE );

#define BUF( x, y ) buf[ mul24( y + 1 , bstride ) + ( x ) + 1 ]

    if( gx >= width || gy >= height )
        return;

    float4 output;
    output.w = 1.0f;

    if( gy & 1 ) {
        if( gx & 1 ) {  // BLUE
            output.x = 0.25f * ( BUF( lx - 1, ly - 1 ) + BUF( lx - 1, ly + 1 ) + BUF( lx + 1, ly - 1 ) + BUF( lx + 1, ly + 1 ) );
            output.y = 0.25f * ( BUF( lx, ly - 1 ) + BUF( lx, ly + 1 ) + BUF( lx - 1, ly ) + BUF( lx + 1, ly ) );
            output.z = BUF( lx, ly );
        } else { // GREEN 2
            output.x = 0.5f * ( BUF( lx, ly - 1 ) + BUF( lx, ly + 1 ) );
            output.y = BUF( lx, ly );
            output.z = 0.5f * ( BUF( lx - 1, ly ) + BUF( lx + 1, ly ) );
        }
    } else {
        if( gx & 1 ) { // GREEN 1
            output.x = 0.5f * ( BUF( lx - 1, ly ) + BUF( lx + 1, ly ) );
            output.y = BUF( lx, ly );
            output.z = 0.5f * ( BUF( lx, ly - 1 ) + BUF( lx, ly + 1 ) );
        } else { // RED
            output.x = BUF( lx, ly );
            output.y = 0.25f * ( BUF( lx, ly - 1 ) + BUF( lx, ly + 1 ) + BUF( lx - 1, ly ) + BUF( lx + 1, ly ) );
            output.z = 0.25f * ( BUF( lx - 1, ly - 1 ) + BUF( lx - 1, ly + 1 ) + BUF( lx + 1, ly - 1 ) + BUF( lx + 1, ly + 1 ) );
        }
    }

    write_imagef( dst,( int2 )( gx, gy ), output );
#undef BUF
}

kernel void debayer_GBRG_RGBA( write_only image2d_t dst, read_only image2d_t src, local float* buf)
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int lx = get_local_id( 0 );
    const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
    const int width = get_image_width( src );
    const int height = get_image_height( src );
    const int2 base = ( int2 )( get_group_id( 0 ) * lw - 1, get_group_id( 1 ) * lh - 1 );
    const int bstride = lw + 2;
    float2 coord;
    float4 dx, dy;

    for( int y = ly; y < lh + 2; y += lh ) {
        for( int x = lx; x < lw + 2; x += lw ) {
            int2 coord = base + ( int2 )( x, y );
            coord.x = mirror( coord.x, width );
            coord.y = mirror( coord.y, height );
            buf[ mul24( y, bstride ) + x ] = read_imagef( src, SAMPLER_NN, coord ).x;
        }
    }
    barrier( CLK_LOCAL_MEM_FENCE );

#define BUF( x, y ) buf[ mul24( y + 1 , bstride ) + ( x ) + 1 ]

    if( gx >= width || gy >= height )
        return;

    float4 output;
    output.w = 1.0f;

    if( gy & 1 ) {
        if( gx & 1 ) { // GREEN 1
            output.x = 0.5f * ( BUF( lx - 1, ly ) + BUF( lx + 1, ly ) );
            output.y = BUF( lx, ly );
            output.z = 0.5f * ( BUF( lx, ly - 1 ) + BUF( lx, ly + 1 ) );
        } else { // RED
            output.x = BUF( lx, ly );
            output.y = 0.25f * ( BUF( lx, ly - 1 ) + BUF( lx, ly + 1 ) + BUF( lx - 1, ly ) + BUF( lx + 1, ly ) );
            output.z = 0.25f * ( BUF( lx - 1, ly - 1 ) + BUF( lx - 1, ly + 1 ) + BUF( lx + 1, ly - 1 ) + BUF( lx + 1, ly + 1 ) );
        }
    } else {
        if( gx & 1 ) {  // BLUE
            output.x = 0.25f * ( BUF( lx - 1, ly - 1 ) + BUF( lx - 1, ly + 1 ) + BUF( lx + 1, ly - 1 ) + BUF( lx + 1, ly + 1 ) );
            output.y = 0.25f * ( BUF( lx, ly - 1 ) + BUF( lx, ly + 1 ) + BUF( lx - 1, ly ) + BUF( lx + 1, ly ) );
            output.z = BUF( lx, ly );
        } else { // GREEN 2
            output.x = 0.5f * ( BUF( lx, ly - 1 ) + BUF( lx, ly + 1 ) );
            output.y = BUF( lx, ly );
            output.z = 0.5f * ( BUF( lx - 1, ly ) + BUF( lx + 1, ly ) );
        }
    }

    write_imagef( dst,( int2 )( gx, gy ), output );
#undef BUF
}

kernel void debayer_GRBG_RGBA( write_only image2d_t dst, read_only image2d_t src, local float* buf)
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int lx = get_local_id( 0 );
    const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
    const int width = get_image_width( src );
    const int height = get_image_height( src );
    const int2 base = ( int2 )( get_group_id( 0 ) * lw - 1, get_group_id( 1 ) * lh - 1 );
    const int bstride = lw + 2;
    float2 coord;
    float4 dx, dy;

    for( int y = ly; y < lh + 2; y += lh ) {
        for( int x = lx; x < lw + 2; x += lw ) {
            int2 coord = base + ( int2 )( x, y );
            coord.x = mirror( coord.x, width );
            coord.y = mirror( coord.y, height );
            buf[ mul24( y, bstride ) + x ] = read_imagef( src, SAMPLER_NN, coord ).x;
        }
    }
    barrier( CLK_LOCAL_MEM_FENCE );

#define BUF( x, y ) buf[ mul24( y + 1 , bstride ) + ( x ) + 1 ]

    if( gx >= width || gy >= height )
        return;

    float4 output;
    output.w = 1.0f;

    if( gy & 1 ) {
        if( gx & 1 ) { // GREEN
            output.x = 0.5f * ( BUF( lx, ly - 1 ) + BUF( lx, ly + 1 ) );
            output.y = BUF( lx, ly );
            output.z = 0.5f * ( BUF( lx - 1, ly ) + BUF( lx + 1, ly ) );
        } else { // BLUE
            output.x = 0.25f * ( BUF( lx - 1, ly - 1 ) + BUF( lx - 1, ly + 1 ) + BUF( lx + 1, ly - 1 ) + BUF( lx + 1, ly + 1 ) );
            output.y = 0.25f * ( BUF( lx, ly - 1 ) + BUF( lx, ly + 1 ) + BUF( lx - 1, ly ) + BUF( lx + 1, ly ) );
            output.z = BUF( lx, ly );
        }
    } else {
        if( gx & 1 ) {  // RED
            output.x = BUF( lx, ly );
            output.y = 0.25f * ( BUF( lx, ly - 1 ) + BUF( lx, ly + 1 ) + BUF( lx - 1, ly ) + BUF( lx + 1, ly ) );
            output.z = 0.25f * ( BUF( lx - 1, ly - 1 ) + BUF( lx - 1, ly + 1 ) + BUF( lx + 1, ly - 1 ) + BUF( lx + 1, ly + 1 ) );
        } else { // GREEN
            output.x = 0.5f * ( BUF( lx - 1, ly ) + BUF( lx + 1, ly ) );
            output.y = BUF( lx, ly );
            output.z = 0.5f * ( BUF( lx, ly - 1 ) + BUF( lx, ly + 1 ) );
        }
    }

    write_imagef( dst,( int2 )( gx, gy ), output );
#undef BUF
}

kernel void debayer_BGGR_RGBA( write_only image2d_t dst, read_only image2d_t src, local float* buf)
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int lx = get_local_id( 0 );
    const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
    const int width = get_image_width( src );
    const int height = get_image_height( src );
    const int2 base = ( int2 )( get_group_id( 0 ) * lw - 1, get_group_id( 1 ) * lh - 1 );
    const int bstride = lw + 2;
    float2 coord;
    float4 dx, dy;

    for( int y = ly; y < lh + 2; y += lh ) {
        for( int x = lx; x < lw + 2; x += lw ) {
            int2 coord = base + ( int2 )( x, y );
            coord.x = mirror( coord.x, width );
            coord.y = mirror( coord.y, height );
            buf[ mul24( y, bstride ) + x ] = read_imagef( src, SAMPLER_NN, coord ).x;
        }
    }
    barrier( CLK_LOCAL_MEM_FENCE );

#define BUF( x, y ) buf[ mul24( y + 1 , bstride ) + ( x ) + 1 ]

    if( gx >= width || gy >= height )
        return;

    float4 output;
    output.w = 1.0f;

    if( gy & 1 ) {
        if( gx & 1 ) {  // RED
            output.x = BUF( lx, ly );
            output.y = 0.25f * ( BUF( lx, ly - 1 ) + BUF( lx, ly + 1 ) + BUF( lx - 1, ly ) + BUF( lx + 1, ly ) );
            output.z = 0.25f * ( BUF( lx - 1, ly - 1 ) + BUF( lx - 1, ly + 1 ) + BUF( lx + 1, ly - 1 ) + BUF( lx + 1, ly + 1 ) );
        } else { // GREEN
            output.x = 0.5f * ( BUF( lx - 1, ly ) + BUF( lx + 1, ly ) );
            output.y = BUF( lx, ly );
            output.z = 0.5f * ( BUF( lx, ly - 1 ) + BUF( lx, ly + 1 ) );
        }
    } else {
        if( gx & 1 ) { // GREEN
            output.x = 0.5f * ( BUF( lx, ly - 1 ) + BUF( lx, ly + 1 ) );
            output.y = BUF( lx, ly );
            output.z = 0.5f * ( BUF( lx - 1, ly ) + BUF( lx + 1, ly ) );
        } else { // BLUE
            output.x = 0.25f * ( BUF( lx - 1, ly - 1 ) + BUF( lx - 1, ly + 1 ) + BUF( lx + 1, ly - 1 ) + BUF( lx + 1, ly + 1 ) );
            output.y = 0.25f * ( BUF( lx, ly - 1 ) + BUF( lx, ly + 1 ) + BUF( lx - 1, ly ) + BUF( lx + 1, ly ) );
            output.z = BUF( lx, ly );
        }
    }

    write_imagef( dst,( int2 )( gx, gy ), output );
#undef BUF
}
