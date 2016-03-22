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

__kernel void guidedfilter_calcab( __write_only image2d_t imga, __write_only image2d_t imgb,
								   __read_only image2d_t imgmeanG, __read_only image2d_t imgmeanS,
								   __read_only image2d_t imgmeanSG, __read_only image2d_t imgmeanGG, const float epsilon )
{
    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
	const int width = get_image_width( imga );
	const int height = get_image_height( imga );
	int2 coord;
	float4 a, b, meanG, meanS, meanSG, meanGG, cov, var;

	coord.x = get_global_id( 0 );
	coord.y = get_global_id( 1 );

	meanG = read_imagef( imgmeanG, sampler, coord );
	meanS = read_imagef( imgmeanS, sampler, coord );
	meanSG = read_imagef( imgmeanSG, sampler, coord );
	meanGG = read_imagef( imgmeanGG, sampler, coord );

	cov = meanSG - meanG * meanS;
	var = meanGG - meanG * meanG;
	a = cov / ( var + ( float4 ) epsilon );
	b = meanS - a * meanG;

	if( coord.x < width && coord.y < height ) {
		write_imagef( imga, coord, a );
		write_imagef( imgb, coord, b );
	}
}

