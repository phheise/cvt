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

__kernel void prefixsum_pblock_sqr( __write_only image2d_t out,  __read_only image2d_t in, __local float4* buf )
{
    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
	const int lx = get_local_id( 0 );
	const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
//    const int lh = get_local_size( 1 );
	const int width = get_image_width( out );
	const int height = get_image_height( out );
	int2 coord;
	int lid = mul24( lw, ly ) + lx;
	float4 tmp;

	coord.x = get_global_id( 0 );
	coord.y = get_global_id( 1 );

	tmp = read_imagef( in, sampler, coord );
	tmp *= tmp;
	buf[ lid ] = tmp;

	// assume lw == lh
	for( int offset = 1; offset < lw; offset <<= 1 )
	{
		barrier( CLK_LOCAL_MEM_FENCE );
		if( lx >= offset )
			tmp += buf[ lid - offset ];
		barrier( CLK_LOCAL_MEM_FENCE );
		buf[ lid ] = tmp;

		barrier( CLK_LOCAL_MEM_FENCE );
		if( ly >= offset )
			tmp += buf[ lid - mul24( lw, offset ) ];
		barrier( CLK_LOCAL_MEM_FENCE );
		buf[ lid ] = tmp;
	}

	if( coord.x < width && coord.y < height )
		write_imagef( out, coord, tmp );
}

