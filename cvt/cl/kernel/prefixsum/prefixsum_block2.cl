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

#if 0
__kernel void prefixsum_block2( __write_only image2d_t out,  __read_only image2d_t in )
{
    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
	const int lx = get_local_id( 0 );
	const int ly = get_local_id( 1 );
	const size_t lw = get_local_size( 0 );
	const size_t lh = get_local_size( 1 );
	const int width = get_image_width( out );
	const int height = get_image_height( out );
	int2 coord;
	int2 base = ( int2 ) ( mul24( get_group_id( 0 ), lw ), mul24( get_group_id( 1 ), lh ) );
	float4 tmp;
	float4 s, top, left;

	coord.x = get_global_id( 0 );
	coord.y = get_global_id( 1 );

	if(  coord.x >= width || coord.y >= height )
		return;

	s = read_imagef( in, sampler, ( int2 ) base - ( int2 ) ( 1, 1 ) );
	top = read_imagef( in, sampler, base + ( int2 ) ( lx, -1 ) );
	left = read_imagef( in, sampler, base + ( int2 ) ( -1, ly ) );

	if( lx < lw - 1 && ly < lh - 1 ) {
		tmp = read_imagef( in, sampler, coord );
		tmp += top;
		tmp += left;
		tmp += s;
		write_imagef( out, coord, tmp );

		if( ly == 0 && coord.y )
			write_imagef( out, base + ( int2 ) ( lx, -1 ), top + s );
		if( lx == 0 && coord.x )
			write_imagef( out, base + ( int2 ) ( -1, ly ), left + s );
	} else {
		/* just for last lines exactly at the edge of the local size patch*/
		if( coord.y == height - 1 && lx < lw - 1 && ly == lh - 1 ) {
			tmp = read_imagef( in, sampler, coord );
			write_imagef( out, coord, tmp + left );
		}

		if( coord.x == width - 1 && ly < lh - 1 && lx == lw - 1 ) {
			tmp = read_imagef( in, sampler, coord );
			write_imagef( out, coord, tmp + top );
		}
	}

}
#endif

__kernel void prefixsum_block2( __write_only image2d_t out,  __read_only image2d_t in )
{
    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
	const int lx = get_local_id( 0 );
	const int ly = get_local_id( 1 );
	const size_t lw = get_local_size( 0 );
	const size_t lh = get_local_size( 1 );
	const int width = get_image_width( out );
	const int height = get_image_height( out );
	int2 coord;
	int2 base = ( int2 ) ( mul24( get_group_id( 0 ), lw ), mul24( get_group_id( 1 ), lh ) );
	float4 tmp;
	local float4 buf[ 16 ][ 16 ];

	buf[ ly ][ lx ] = read_imagef( in, sampler, ( int2 ) base + ( int2 ) ( lx - 1, ly - 1 ) );
/*	if( ly == 0 ) {
		top[ lx ] = read_imagef( in, sampler, ( int2 ) base + ( int2 ) ( lx, -1 ) );
		left[ lx ] = read_imagef( in, sampler, ( int2 ) base + ( int2 ) ( -1, lx ) );
			s = read_imagef( in, sampler, ( int2 ) base - ( int2 ) ( 1, 1 ) );
	}*/

	barrier( CLK_LOCAL_MEM_FENCE );

	if( lx )
		buf[ ly ][ lx ] += buf[ ly ][ 0 ];

	barrier( CLK_LOCAL_MEM_FENCE );

	if( ly )
		buf[ ly ][ lx ] += buf[ 0 ][ lx ];

	coord.x = get_global_id( 0 );
	coord.y = get_global_id( 1 );

	if(  coord.x > width || coord.y > height || !coord.x || !coord.y )
		return;

	write_imagef( out, ( int2 ) base + ( int2 ) ( lx - 1, ly - 1 ), buf[ ly ][ lx ] );
//	s = read_imagef( in, sampler, ( int2 ) base - ( int2 ) ( 1, 1 ) );
//	top = read_imagef( in, sampler, base + ( int2 ) ( lx, -1 ) );
//	left = read_imagef( in, sampler, base + ( int2 ) ( -1, ly ) );

//	if( lx < lw - 1 && ly < lh - 1 ) {
/*		tmp = read_imagef( in, sampler, coord );
		tmp += top[ lx ];
		tmp += left[ ly ];
		tmp += s;*/
//		tmp = buf[ ly + 1 ][ lx + 1 ];
//		tmp += buf[ ly + 1 ][ 0 ];
//		tmp += buf[ 0 ][ lx + 1 ];
//		tmp += buf[ 0 ][ 0 ];

//		if( ly == 0 && coord.y )
//			write_imagef( out, base + ( int2 ) ( lx, -1 ), top[ lx ] + s );
//		if( lx == 0 && coord.x )
//			write_imagef( out, base + ( int2 ) ( -1, ly ), left[ ly ] + s );
//	}
#if 1
{
		/* just for last lines exactly at the edge of the local size patch*/

	// FIXME
		if( coord.y == height - 1 && lx < lw - 1 && ly == lh - 1 && coord.x < width ) {
			tmp = read_imagef( in, sampler, coord );
			tmp += read_imagef( in, sampler, base + ( int2 ) ( -1, ly ) );
			write_imagef( out, coord, tmp );
		}

		if( coord.x == width - 1 && ly < lh - 1 && lx == lw - 1 && coord.y < height ) {
			tmp = read_imagef( in, sampler, coord );
			tmp += read_imagef( in, sampler, base + ( int2 ) ( lx, -1 ) );
			write_imagef( out, coord, tmp );
		}
	}
#endif
}


