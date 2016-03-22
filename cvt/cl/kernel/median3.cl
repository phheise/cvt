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

__kernel void median3( __write_only image2d_t out, __read_only image2d_t in, __local float4* buf )
{
    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
	const int gx = get_global_id( 0 );
	const int gy = get_global_id( 1 );
	const int lx = get_local_id( 0 );
	const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
	const int width = get_image_width( out );
	const int height = get_image_height( out );
	const int2 base = ( int2 )( get_group_id( 0 ) * lw - 1, get_group_id( 1 ) * lh - 1 );
	float4 v[ 9 ], temp;

	for( int y = ly; y < lh + 2; y += lh ) {
		for( int x = lx; x < lw + 2; x += lw ) {
			buf[ mul24( y, ( lw + 2 ) ) + x ] = read_imagef( in, sampler, base + ( int2 )( x, y ) );
		}
	}

	barrier( CLK_LOCAL_MEM_FENCE );

	if( gx >= width || gy >= height )
		return;

#define sort2(a, b)				temp = a; a = fmin(a, b); b = fmax(temp, b)
#define sort3( a, b, c )		sort2( a, b );sort2( b, c );sort2( a, b )
#define min3( a, b, c )			fmin( a, fmin( b, c ) )
#define max3( a, b, c )			fmax( a, fmax( b, c ) )
#define mid3( a, b, c )			fmax( a, fmin( b, c ) )
#define BUF( x, y ) buf[ mul24( ( ( y ) + 1 ), ( lw + 2 ) ) + ( x ) + 1 ]

	v[ 0 ] = BUF( lx - 1, ly -1 );
	v[ 1 ] = BUF( lx, ly - 1 );
	v[ 2 ] = BUF( lx + 1, ly - 1 );
	v[ 3 ] = BUF( lx - 1, ly );
	v[ 4 ] = BUF( lx, ly );
	v[ 5 ] = BUF( lx + 1, ly );
	v[ 6 ] = BUF( lx - 1, ly + 1 );
	v[ 7 ] = BUF( lx, ly + 1 );
	v[ 8 ] = BUF( lx + 1, ly + 1 );


	sort3( v[ 0 ], v[ 1 ], v[ 2 ] );
	sort3( v[ 3 ], v[ 4 ], v[ 5 ] );
	sort3( v[ 6 ], v[ 7 ], v[ 8 ] );

	v[ 5 ] = min3( v[ 2 ], v[ 5 ], v[ 8 ] );
	v[ 3 ] = max3( v[ 0 ], v[ 3 ], v[ 6 ] );

	sort3( v[ 1 ], v[ 4 ], v[ 7 ] );
	sort3( v[ 3 ], v[ 4 ], v[ 5 ] );

    write_imagef( out, ( int2 )( gx, gy ), v[ 4 ] );
}
