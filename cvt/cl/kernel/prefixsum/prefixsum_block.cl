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

__kernel void prefixsum_block( __write_only image2d_t out,  __read_only image2d_t in, __local float4* buf )
{
    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;
	const int lid = get_local_id( 0 );
    const int lsize = get_local_size( 0 );
	const int width = get_image_width( out );
	const int height = get_image_height( out );
	int2 coord, block;

	coord.x = get_global_id( 0 ) & ( ~( lsize - 1 ) );
	coord.y = mul24( ( int ) get_global_id( 1 ), lsize );

	block.x = min( width - coord.x, lsize );
	block.y = min( height - coord.y, lsize );

	if( lid < block.y  ) {
		int off = mul24( lid, lsize );
		float4 tmp = ( float4 )( 0 );
		for( int i = 0; i < block.x; i++ ) {
			tmp += read_imagef( in, sampler, coord + ( int2 ) ( i, lid ) );
			buf[ off + i ] = tmp;
		}
	}

	barrier( CLK_LOCAL_MEM_FENCE );

	if( lid < block.x ) {
		int off = lid;
		float4 tmp = ( float4 ) ( 0 );
		for( int i = 0; i < block.y; i++ ) {
			tmp += buf[ off ];
			write_imagef( out, coord + ( int2 ) ( lid, i ), tmp );
			off += lsize;
		}
	}

}

