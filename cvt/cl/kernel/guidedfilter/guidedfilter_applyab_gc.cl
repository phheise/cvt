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

__kernel void guidedfilter_applyab_gc( __write_only image2d_t imgout,  __read_only image2d_t imgin, __read_only image2d_t imga, __read_only image2d_t imgb )
{
    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
	const int width = get_image_width( imgout );
	const int height = get_image_height( imgout );
	int2 coord;
	float4 in, a, b, out;

	coord.x = get_global_id( 0 );
	coord.y = get_global_id( 1 );

	in = read_imagef( imgin, sampler, coord );
	a = read_imagef( imga, sampler, coord );
	b = read_imagef( imgb, sampler, coord );

	out = a * in + b;
	out = ( float4 ) dot( out, ( float4 ) ( 0.3333f, 0.3333f, 0.3333f, 0.0f ) );
	out.w = 1.0f;

	if( coord.x < width && coord.y < height )
		write_imagef( imgout, coord, out );
}

