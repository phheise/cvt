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

__kernel void stereogcv_costdepthncc( __write_only image2d_t costout, __read_only image2d_t meanI01,
								 __read_only image2d_t meanI0, __read_only image2d_t meanI1,
								 __read_only image2d_t meanI02, __read_only image2d_t meanI12,
								const float depth )
{
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
	const sampler_t samplerlin = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;
	const int2 coord = { get_global_id( 0 ), get_global_id( 1 ) };
	const float2 coordf = { get_global_id( 0 ) + 0.5f, get_global_id( 1 ) + 0.5f };
	const int width = get_image_width( costout );
	const int height = get_image_height( costout );
	float4 m01, m0, m1, m02, m12, ncc;
	const float4 dotmul = { 0.3333f, 0.3333f, 0.3333f, 0.0f };

	if( coord.x >= width || coord.y >= height )
		return;

	m01 = read_imagef( meanI01, sampler, coord );
	m0 = read_imagef( meanI0, sampler, coord );
	m1 = read_imagef( meanI1, samplerlin, coordf + ( float2 ) ( depth, 0 ) );
	m02 = read_imagef( meanI02, sampler, coord );
	m12 = read_imagef( meanI12, samplerlin, coordf + ( float2 ) ( depth, 0 ) );

	ncc = ( m01 - m0 * m1 ) * rsqrt( ( m02 - m0 * m0 ) * ( m12 - m1 * m1 ) + 1e-10f );
	float4 val = dot( ( float4 ) 1.0f - clamp( ncc, 0.0f, 1.0f ), dotmul );
	write_imagef( costout, coord, val );
}
