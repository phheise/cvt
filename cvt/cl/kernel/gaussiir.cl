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

__kernel void gaussiir( __global float4* buffer, __read_only image2d_t input, const int w, const int h, const float4 n, const float4 m, const float4 d )
{
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;
	int2 coord;
    float4 x[ 4 ];
    float4 y[ 4 ];
	float4 yn;

	coord.x = 0;
    coord.y = get_global_id( 0 );
	buffer += get_global_id( 0 ) * w;

	if( coord.y >= h )
		return;

	float b1 = ( n.s0 + n.s1 + n.s2 + n.s3 ) / ( d.s0 + d.s1 + d.s2 + d.s3 + 1.0f );
	float b2 = ( m.s0 + m.s1 + m.s2 + m.s3 ) / ( d.s0 + d.s1 + d.s2 + d.s3 + 1.0f );

	// forward pass
	x[ 0 ] = read_imagef( input, sampler, coord );
	x[ 1 ] = read_imagef( input, sampler, coord + ( int2 ) ( 1, 0 ) );
	x[ 2 ] = read_imagef( input, sampler, coord + ( int2 ) ( 2, 0 ) );
	x[ 3 ] = read_imagef( input, sampler, coord + ( int2 ) ( 3, 0 ) );

	y[ 0 ] = n.s0 * x[ 0 ] + n.s1 * x[ 0 ] + n.s2 * x[ 0 ] + n.s3 * x[ 0 ]
		- b1 * ( d.s0 * x[ 0 ] + d.s1 * x[ 0 ] + d.s2 * x[ 0 ] + d.s3 * x[ 0 ] );
	y[ 1 ] = n.s0 * x[ 1 ] + n.s1 * x[ 0 ] + n.s2 * x[ 0 ] + n.s3 * x[ 0 ]
		- d.s0 * y[ 0 ] - b1 * ( d.s1 * x[ 0 ] + d.s2 * x[ 0 ] + d.s3 * x[ 0 ] );
	y[ 2 ] = n.s0 * x[ 2 ] + n.s1 * x[ 1 ] + n.s2 * x[ 0 ] + n.s3 * x[ 0 ]
		- d.s0 * y[ 1 ] - d.s1 * y[ 0 ] - b1 * (  d.s2 * x[ 0 ] + d.s3 * x[ 0 ] );
	y[ 3 ] = n.s0 * x[ 3 ] + n.s1 * x[ 2 ] + n.s2 * x[ 1 ] + n.s3 * x[ 0 ]
		- d.s0 * y[ 2 ] - d.s1 * y[ 1 ] - d.s2 * y[ 0 ] - b1 * d.s3 * x[ 0 ];

	buffer[ 0 ] = y[ 0 ];
	buffer[ 1 ] = y[ 1 ];
	buffer[ 2 ] = y[ 2 ];
	buffer[ 3 ] = y[ 3 ];

    for( int i = 4; i < w; i++ ) {
		coord.x = i;
        x[ 0 ] = x[ 1 ]; x[ 1 ] = x[ 2 ]; x[ 2 ] = x[ 3 ];
        x[ 3 ] = read_imagef( input, sampler, coord );
        yn = n.s0 * x[ 3 ] + n.s1 * x[ 2 ] + n.s2 * x[ 1 ] + n.s3 * x[ 0 ]
			 - d.s0 * y[ 3 ] - d.s1 * y[ 2 ] - d.s2 * y[ 1 ] - d.s3 * y[ 0 ];
		buffer[ i ] = yn;
		y[ 0 ] = y[ 1 ]; y[ 1 ] = y[ 2 ]; y[ 2 ] = y[ 3 ]; y[ 3 ]= yn;
    }

    // reverse pass
	coord.x = w - 1;
	x[ 0 ] = read_imagef( input, sampler, coord );
	x[ 1 ] = read_imagef( input, sampler, coord - ( int2 ) ( 1, 0 ) );
	x[ 2 ] = read_imagef( input, sampler, coord - ( int2 ) ( 2, 0 ) );
	x[ 3 ] = read_imagef( input, sampler, coord - ( int2 ) ( 3, 0 ) );

	y[ 0 ] = m.s0 * x[ 0 ] + m.s1 * x[ 0 ] + m.s2 * x[ 0 ] + m.s3 * x[ 0 ]
		- b2 * ( d.s0 * x[ 0 ] + d.s1 * x[ 0 ] + d.s2 * x[ 0 ] + d.s3 * x[ 0 ] );
	y[ 1 ] = m.s0 * x[ 1 ] + m.s1 * x[ 0 ] + m.s2 * x[ 0 ] + m.s3 * x[ 0 ]
		- d.s0 * y[ 0 ] - b2 * ( d.s1 * x[ 0 ] + d.s2 * x[ 0 ] + d.s3 * x[ 0 ] );
	y[ 2 ] = m.s0 * x[ 2 ] + m.s1 * x[ 1 ] + m.s2 * x[ 0 ] + m.s3 * x[ 0 ]
		- d.s0 * y[ 1 ] - d.s1 * y[ 0 ] - b2 * (  d.s2 * x[ 0 ] + d.s3 * x[ 0 ] );
	y[ 3 ] = m.s0 * x[ 3 ] + m.s1 * x[ 2 ] + m.s2 * x[ 1 ] + m.s3 * x[ 0 ]
		- d.s0 * y[ 2 ] - d.s1 * y[ 1 ] - d.s2 * y[ 0 ] - b2 * d.s3 * x[ 0 ];

	buffer[ w - 1 ] += y[ 0 ];
	buffer[ w - 2 ] += y[ 1 ];
	buffer[ w - 3 ] += y[ 2 ];
	buffer[ w - 4 ] += y[ 3 ];

    for (int i = w-5; i >= 0; i--) {
		coord.x = i;
        x[ 0 ] = x[ 1 ]; x[ 1 ] = x[ 2 ]; x[ 2 ] = x[ 3 ];
        x[ 3 ] = read_imagef( input, sampler, coord );
		yn = m.s0 * x[ 3 ] + m.s1 * x[ 2 ] + m.s2 * x[ 1 ] + m.s3 * x[ 0 ]
			 - d.s0 * y[ 3 ] - d.s1 * y[ 2 ] - d.s2 * y[ 1 ] - d.s3 * y[ 0 ];
		buffer[ i ] += yn;
		y[ 0 ] = y[ 1 ]; y[ 1 ] = y[ 2 ]; y[ 2 ] = y[ 3 ]; y[ 3 ]= yn;
    }
}
