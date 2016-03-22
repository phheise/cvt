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

/*void async_work_group_image_copy( __local float4* buf, __read_only image2d_t input, const sampler_t sampler, int4 xywh )
{
	const int lx = get_local_id( 0 );
	const int ly = get_local_id( 1 );
	const int lw = get_local_size( 0 );
	const int lh = get_local_size( 1 );
	const int width = get_image_width( input );
	const int height = get_image_height( input );

	int offsety = xywh.z * ly; // y-offset to buf for this work-item
	int incy = xywh.z * lh; // x-offset to buf for this work item

	for( int y = ly; y < xywh.w; y += lh ) {
		for( int x = lx, offsetx = lx; x < xywh.z; x += lw, offsetx += lw ) {
			buf[ offsety + offsetx ] = read_imagef( input, sampler, xywh.xy + ( int2 ) ( x, y ) );
		}
		offsety += incy;
	}
}*/


__kernel void boxfilter_prefixsum( __write_only image2d_t out,  __read_only image2d_t in, const int r )
{
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;
	const int width = get_image_width( out );
	const int height = get_image_height( out );
	int2 mincoord, maxcoord;
	int2 coord = { get_global_id( 0 ), get_global_id( 1 ) };
	float size;
	float4 value;

	if( coord.x >= width || coord.y >= height )
		return;

	mincoord = max( ( int2 ) 0, coord - ( int2 ) ( r + 1 ) );
	maxcoord = min( ( int2 ) ( width - 1, height - 1 ), coord + ( int2 ) ( r ) );

	int2 tmp = maxcoord - mincoord;

	value  = read_imagef( in, sampler, mincoord );
	value -= read_imagef( in, sampler, ( int2 )( mincoord.x, maxcoord.y ) );
	value -= read_imagef( in, sampler, ( int2 )( maxcoord.x, mincoord.y ) );
	value += read_imagef( in, sampler, maxcoord );
	value /= ( float4 ) ( tmp.x * tmp.y );

	write_imagef( out, coord, value );
}

