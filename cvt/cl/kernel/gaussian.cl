/*
   The MIT License (MIT)

   Copyright (c) 2011 - 2014, Philipp Heise, Sebastian Klose and Brian Jensen
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

const sampler_t SAMPLER_NN = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

kernel void gaussian_horizontal( write_only image2d_t output, read_only image2d_t input, global const float* kern, int ksize, local float4* buf )
{
    const int width = get_image_width( input );
    const int height = get_image_height( input );
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int lx = get_local_id( 0 );
    const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
    const int bstride = lw + ksize - 1;
    const int ksizehalf = ksize / 2;
    const int2 base = ( int2 )( get_group_id( 0 ) * lw - ksizehalf, get_group_id( 1 ) * lh );

    for( int x = lx; x < lw + ksize - 1; x += lw ) {
        buf[ mul24( ly, bstride ) + x ] = read_imagef( input, SAMPLER_NN, base + ( int2 )( x, ly ) );
    }

    barrier( CLK_LOCAL_MEM_FENCE );

    if( gx >= width || gy >= height )
        return;

    float4 out = 0.0f;
    for( int dx = 0; dx < ksize; dx++ )
        out += kern[ dx ] * buf[ mul24( ly, bstride ) + lx + dx ];

    write_imagef( output, ( int2 ) ( gx, gy ), out );
}

kernel void gaussian_vertical( write_only image2d_t output, read_only image2d_t input, global const float* kern, int ksize, local float4* buf )
{
    const int width = get_image_width( input );
    const int height = get_image_height( input );
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int lx = get_local_id( 0 );
    const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
    const int bstride = lw;
    const int ksizehalf = ksize / 2;
    const int2 base = ( int2 )( get_group_id( 0 ) * lw, get_group_id( 1 ) * lh - ksizehalf );

    for( int y = ly; y < lh + ksize -1; y += lh ) {
        buf[ mul24( y, bstride ) + lx ] = read_imagef( input, SAMPLER_NN, base + ( int2 )( lx, y ) );
    }

    barrier( CLK_LOCAL_MEM_FENCE );

    if( gx >= width || gy >= height )
        return;

    float4 out = 0.0f;
    for( int dy = 0; dy < ksize; dy++ )
        out += kern[ dy ] * buf[ mul24( ( dy + ly ), bstride ) + lx ];

    write_imagef( output, ( int2 ) ( gx, gy ), out );
}
