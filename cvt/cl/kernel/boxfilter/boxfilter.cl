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


__kernel void boxfilter( __write_only image2d_t out,  __read_only image2d_t in, const int r, __local float4* buf )
{
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
	const int width = get_image_width( out );
	const int height = get_image_height( out );
	const int lx = get_local_id( 0 );
    const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
	const int r2 = r * 2;
    const int bstride = lw + r2;
	const int2 base = ( int2 )( get_group_id( 0 ) * lw - r, get_group_id( 1 ) * lh - r );
	int2 coord, mincoord, maxcoord;
	float size;
	float4 value;

	for( int y = ly; y < lh + r2; y += lh ) {
		for( int x = lx; x < lw + r2; x += lw ) {
			buf[ mul24( y, bstride ) + x ] = read_imagef( in, sampler, base + ( int2 )( x, y ) );
		}
	}

	barrier( CLK_LOCAL_MEM_FENCE );

	coord.x = get_global_id( 0 );
	coord.y = get_global_id( 1 );

	if( coord.x >= width || coord.y >= height )
		return;

#define BUF( x, y ) ( buf[ mul24( ( y ) + r, bstride ) + ( x ) + r ] )

	mincoord.x = max( 0, coord.x - r );
	mincoord.y = max( 0, coord.y - r );
	maxcoord.x = min( width - 1, coord.x + r );
	maxcoord.y = min( height - 1, coord.y + r );

	size = ( maxcoord.x - mincoord.x + 1 ) * ( maxcoord.y - mincoord.y + 1 );
	mincoord -= coord;
	maxcoord -= coord;

	value = 0;
	for( int y = mincoord.y; y <= maxcoord.y; y++ )
		for( int x = mincoord.x; x <= maxcoord.x; x++ )
			value += BUF( lx + x, ly + y );
	value /= ( float4 ) size;

	write_imagef( out, coord, value );
}

void prefixScanHorizontal( local float4* block, const int len )
{
	const int lx = get_local_id( 0 );
	const int ly = get_local_id( 1 );
	const int lw = get_local_size( 0 );
	const int lh = get_local_size( 1 );
	const int lenhalf = len >> 1;
	const int tidx2 = lx << 1;
	const int tidy2 = ly << 1;
	int offset = 1;

	for(int d = lenhalf; d > 0; d >>= 1 ) {
		barrier(CLK_LOCAL_MEM_FENCE);
		int ai = offset * ( tidx2 + 1) - 1;
		int bi = offset * ( tidx2 + 2) - 1;
		if( bi < len ) {
			for( int y = ly; y < len; y += lh ) {
				block[ bi + y * len ] += block[ ai + y * len ];
			}
		}
		barrier(CLK_LOCAL_MEM_FENCE);
		ai = offset * ( tidy2 + 1 ) - 1;
		bi = offset * ( tidy2 + 2 ) - 1;
		if( bi < len ) {
			for( int x = lx; x < len; x += lw ) {
				block[ bi * len + x ] += block[ai * len + x ];
			}
		}
		offset <<= 1;
	}
	// scan back down the tree
	offset >>= 1;
	// traverse down the tree building the scan in the place
	for(int d = 1; d <= lenhalf; d <<= 1 ) {
		barrier(CLK_LOCAL_MEM_FENCE);
		int bi = offset * ( tidx2 + 3 ) - 1;
		int ai = offset * ( tidx2 + 2 ) - 1;
		if( bi < len ) {
			for( int y = ly; y < len; y += lh ) {
				block[bi + y * len ] += block[ ai + y * len ];
			}
		}
		barrier(CLK_LOCAL_MEM_FENCE);
		bi = offset * ( tidy2 + 3 ) - 1;
		ai = offset * ( tidy2 + 2 ) - 1;
		if( bi < len ) {
			for( int x = lx; x < len; x += lw ) {
				block[ bi * len + x ] += block[ai * len + x ];
			}
		}
		offset >>= 1;
	}
}


__kernel void boxfilter2( __write_only image2d_t out,  __read_only image2d_t in, const int r, local float4* buf )
{
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
	const int width = get_image_width( out );
	const int height = get_image_height( out );
	const int lx = get_local_id( 0 );
    const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
	const int r2 = r * 2;
    const int bstride = lw + r2;
	const int2 base = ( int2 )( get_group_id( 0 ) * lw - r, get_group_id( 1 ) * lh - r );
	int2 coord, mincoord, maxcoord;
	float size;
	float4 value;

	for( int y = ly; y < lh + r2; y += lh ) {
		for( int x = lx; x < lw + r2; x += lw ) {
			buf[ y * bstride + x ] = read_imagef( in, sampler, base + ( int2 )( x, y ) );
		}
	}

	barrier( CLK_LOCAL_MEM_FENCE );

	prefixScanHorizontal( buf, bstride );

	barrier( CLK_LOCAL_MEM_FENCE );
	coord.x = get_global_id( 0 );
	coord.y = get_global_id( 1 );

	if( coord.x >= width || coord.y >= height )
		return;

#define BUF( x, y ) ( buf[ mul24( ( y ) + r, bstride ) + ( x ) + r ] )

	value = BUF( lx + r , ly + r );
	if( lx > 0 )
		value -= BUF( lx - r - 1 , ly + r  );
	if( ly > 0 )
		value -= BUF( lx +  r, ly - r - 1 );
	if( lx > 0 && ly > 0 )
		value += BUF( lx - r - 1, ly - r - 1 );

	write_imagef( out, coord, value / ( float) ( ( 2 * r + 1 ) * ( 2 * r  + 1 ) ) );
}


