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

#define COSTMAX 1.0f
#define COSTTHRESHOLD 1.0f

__kernel void stereogcv_costdepth( __write_only image2d_t costout, __read_only image2d_t img0, __read_only image2d_t img1, __read_only global float4* proj, float depth )
{
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_LINEAR;
	int2 coord;
	float2 coord2;
	float4 mat[ 3 ];
	float4 I0, I1, vec, Cout;
	const int width = get_image_width( costout );
	const int height = get_image_height( costout );
	float z;

	coord.x = get_global_id( 0 );
	coord.y = get_global_id( 1 );

	if( coord.x >= width || coord.y >= height )
		return;

	mat[ 0 ] = proj[ 0 ];
	mat[ 1 ] = proj[ 1 ];
	mat[ 2 ] = proj[ 2 ];

	vec = ( float4 )( coord.x, coord.y, depth, 1.0f );

	z = dot( vec, mat[ 2 ] );
	coord2.x = dot( vec, mat[ 0 ] ) / z;
	coord2.y = dot( vec, mat[ 1 ] ) / z;

	if( coord2.x < 0 || coord2.x >= width || coord2.y < 0 || coord2.y >= height || fabs( z ) < 1e-6f  ) {
		Cout.x = COSTMAX;
	} else {
		I0 = read_imagef( img0, sampler, coord );
		I1 = read_imagef( img1, sampler, coord2 );
		Cout.x = fmin( fast_length( I1 - I0 ), COSTTHRESHOLD );
	}

	write_imagef( costout, coord, Cout );
}
