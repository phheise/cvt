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

#define MAXDIFF 2.0f

__kernel void stereogcv_occlusioncheck( __write_only image2d_t out, __read_only image2d_t img0, __read_only image2d_t img1, const float dscale )
{
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
	const sampler_t samplerlin = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;
	int2 coord;
	const int width = get_image_width( img0 );
	const int height = get_image_height( img0 );
	float d0, d1, din0, din1, diff;

	coord.x = get_global_id( 0 );
	coord.y = get_global_id( 1 );

	if( coord.x >= width || coord.y >= height )
		return;

	din0 = read_imagef( img0, sampler, coord ).x;
	d0 = din0 * dscale;
	din1 = read_imagef( img1, samplerlin, ( float2 ) ( - d0 + ( float ) coord.x + 0.5f, coord.y + 0.5f ) ).x;
	d1 = din1 * dscale;

	diff = fabs( - d0 + d1 );
	if( diff <= MAXDIFF )
		write_imagef( out, coord, ( float4 ) ( din0 + din1 ) * 0.5f );
	else
		write_imagef( out, coord, ( float4 ) 0 );
}
