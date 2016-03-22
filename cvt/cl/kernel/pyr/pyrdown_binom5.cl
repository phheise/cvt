/*
   The MIT License (MIT)

   Copyright (c) 2011 - 2013, Philipp Heise and Sebastian Klose

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

__kernel void pyrdown_binom5( __write_only image2d_t out,  __read_only image2d_t in, __local float4* buf, __local float4* buf2 )
{
    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;
	const int gx = get_global_id( 0 );
	const int gy = get_global_id( 1 );
	const int lx = get_local_id( 0 );
	const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
	const int width = get_image_width( in );
	const int height = get_image_height( in );
	const int dstwidth = get_image_width( out );
	const int dstheight = get_image_height( out );
	const int basex = get_group_id( 0 ) * lw - 2;
	const int basey = get_group_id( 1 ) * lh - 2;
	float incx = ( float ) width / ( float ) dstwidth;
	float incy = ( float ) height / ( float ) dstheight;
	float2 coord;

	for( int y = ly; y < lh + 4; y += lh ) {
		for( int x = lx; x < lw + 4; x += lw ) {
			coord.x = ( basex + x + 0.5f ) * incx;
			coord.y = ( basey + y + 0.5f ) * incy;
			buf[ y * ( lw + 4 ) + x ] = read_imagef( in, sampler, coord );
		}
	}

	barrier( CLK_LOCAL_MEM_FENCE );

	for( int y = ly; y < lh + 4; y += lh ) {
		buf2[ y * lw + lx ] =  0.375f  * buf[ y * ( lw + 4 ) + lx + 2 ] +
							   0.25f   * ( buf[ y * ( lw + 4 ) + lx + 1 ] + buf[ y * ( lw + 4 ) + lx + 3 ] ) +
							   0.0625f * ( buf[ y * ( lw + 4 ) + lx + 0 ] + buf[ y * ( lw + 4 ) + lx + 4 ] );
	}

	barrier( CLK_LOCAL_MEM_FENCE );

	if( gx >= dstwidth || gy >= dstheight )
		return;


	float4 val =  0.375f  * buf2[ ( ly + 2 ) * lw + lx ] +
				  0.25f   * ( buf2[ ( ly + 1 ) * lw + lx ] + buf2[ ( ly + 3 ) * lw + lx ] ) +
				  0.0625f * ( buf2[ ( ly + 0 ) * lw + lx ] + buf2[ ( ly + 4 ) * lw + lx ] );

	write_imagef( out, ( int2 )( gx, gy ), val );

//	write_imagef( out, ( int2 )( gx, gy ), buf[ ( ly + 2 ) * ( lw + 4 ) + lx + 2 ] );
}

