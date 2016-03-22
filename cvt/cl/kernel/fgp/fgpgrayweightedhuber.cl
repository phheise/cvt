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

__kernel void fgp( __write_only image2d_t eout, __read_only image2d_t ein, __read_only image2d_t img, const float lambda, const float t, __local float4* buf, __local float4* buf2  )
{
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
	const sampler_t samplerb = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
	const int width = get_image_width( img );
	const int height = get_image_height( img );
	const int gx = get_global_id( 0 );
	const int gy = get_global_id( 1 );
	const int lx = get_local_id( 0 );
	const int ly = get_local_id( 1 );
	const int lw = get_local_size( 0 );
	const int lh = get_local_size( 1 );
	const int bstride = lw + 2;
	const int2 base  = ( int2 )( get_group_id( 0 ) * lw, get_group_id( 1 ) * lh );
	const int2 base2 = ( int2 )( get_group_id( 0 ) * lw - 1, get_group_id( 1 ) * lh - 1 );
	int2 coord;
	float2 delta;
	float4 enew;

	for( int y = ly; y < lh + 2; y += lh ) {
		int offset = mul24( y, bstride );
		if( lx == 0) {
			// get vec e_t, e_{t-1}
			float4 tmp = read_imagef( ein, samplerb, base2 + ( int2 ) ( 0, y ) );
			// store p_t and e_t
			buf[ offset + 0 ] = ( float4 ) ( tmp.xy * ( 1.0f + t ) - t * tmp.zw, tmp.xy );
		}
		if( lx == lw - 1 ) {
			// get vec e_t, e_{t-1}
			float4 tmp = read_imagef( ein, samplerb, base2 + ( int2 ) ( lw + 1, y ) );
			// store p_t and e_t
			buf[ offset + lw + 1 ] = ( float4 ) ( tmp.xy * ( 1.0f + t ) - t * tmp.zw, tmp.xy );
		}

//		for( int x = lx + 1; x < lw + 1; x += lw ) 
		{
			// get vec e_t, e_{t-1}
			float4 tmp = read_imagef( ein, samplerb, base2 + ( int2 ) ( lx + 1, y ) );
			// store p_t and e_t
			buf[ offset + lx + 1 ] = ( float4 ) ( tmp.xy * ( 1.0f + t ) - t * tmp.zw, tmp.xy );
		}
	}

	barrier( CLK_LOCAL_MEM_FENCE );

#define BUF( x, y ) ( buf[ mul24( ( y ) + 1, bstride ) + ( ( x ) + 1 ) ] )
#define BUF2( x, y ) ( buf2[ mul24( ( y ), ( lw + 1 ) ) + ( x ) ] )

	for( int y = ly; y < lh + 1; y += lh ) 
	{
		if( lx == lw - 1 ) {
			int x = lw;
			float2 center = BUF( x, y ).xy;
			float  top    = BUF( x, y - 1 ).y;
			float  left   = BUF( x - 1, y ).x;
			delta = center - ( float2 ) ( left, top );
			// image + lambda * div( p )
			float4 pixel = read_imagef( img, sampler, base + ( int2 )( x, y ) );
			pixel.x += lambda * ( delta.x + delta.y );
			BUF2( x, y ) = pixel;
		}
//		for( int x = lx; x < lw + 1; x += lw ) {
			float2 center = BUF( lx, y ).xy;
			float  top    = BUF( lx, y - 1 ).y;
			float  left   = BUF( lx - 1, y ).x;
			delta = center - ( float2 ) ( left, top );
			// image + lambda * div( p )
			float4 pixel = read_imagef( img, sampler, base + ( int2 )( lx, y ) );
			pixel.x += lambda * ( delta.x + delta.y );
			BUF2( lx, y ) = pixel;
//		}
	}

	barrier( CLK_LOCAL_MEM_FENCE );

	if( gx >= width || gy >= height )
		return;

	float4 pixel = BUF2( lx, ly );
	float dx = BUF2( lx + 1, ly ).x - pixel.x;
	float dy = BUF2( lx, ly + 1 ).x - pixel.x;

	// calculate e_{ t + 1 }
	enew = BUF( lx, ly );
	enew.xy = enew.xy + ( ( 0.125f / lambda ) ) * ( float2 ) ( dx, dy ) - 0.01f * enew.xy;

	float norm = fmax( 1.0f, fast_length( enew.xy ) / pixel.z  );
	enew.xy = enew.xy / norm;

	// store e_{ t + 1 }, e_t
	write_imagef( eout, ( int2 ) ( gx, gy ), enew );
}
