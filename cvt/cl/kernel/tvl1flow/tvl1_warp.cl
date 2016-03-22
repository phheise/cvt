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

__kernel void tvl1_warp( __write_only image2d_t out, __read_only image2d_t u, __read_only image2d_t src1, __read_only image2d_t src2, __local float4* buf  )
{
	const sampler_t samplerlin = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR;
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
	const int gx = get_global_id( 0 );
	const int gy = get_global_id( 1 );
	const int lx = get_local_id( 0 );
	const int ly = get_local_id( 1 );
	const int lw = get_local_size( 0 );
	const int lh = get_local_size( 1 );
	const int width = get_image_width( out );
	const int height = get_image_height( out );
	const int2 base = ( int2 )( get_group_id( 0 ) * lw - 1, get_group_id( 1 ) * lh - 1 );
	const int bstride = lw + 2;
	float2 coord;
	float4 dx2, dy2, dx1, dy1, value;

	for( int y = ly; y < lh + 2; y += lh ) {
		for( int x = lx; x < lw + 2; x += lw ) {
			buf[ mul24( y, bstride ) + x ] = read_imagef( src1, sampler, base + ( int2 )( x, y ) );
		}
	}

	barrier( CLK_LOCAL_MEM_FENCE );

#define ALPHA 1.0f
#define BETA 15.0f
#define BUF( x, y ) buf[ mul24( ( ( y ) + 1 ), bstride ) + ( x ) + 1 ]
#define CSELECT( val ) dot( val, ( float4 )( 0.3333f, 0.3333f, 0.3333f, 0.0f ) )
//#define CSELECT( val ) ( ( val ).x )

	if( gx >= width || gy >= height )
		return;

	coord = ( float2 )( gx + 0.5f,gy + 0.5f ) + read_imagef( u, sampler, ( int2 )( gx, gy ) ).xy;
	dx1 = BUF( lx + 1, ly ) - BUF( lx - 1, ly  );
	dy1 = BUF( lx, ly + 1 ) - BUF( lx , ly - 1 );

	float4 warped = read_imagef( src2, samplerlin, coord );
	dx2 = read_imagef( src2, samplerlin, coord + ( float2 )( 1.0f, 0.0f ) ) - read_imagef( src2, samplerlin, coord - ( float2 )( 1.0f, 0.0f ) );
	dy2 = read_imagef( src2, samplerlin, coord + ( float2 )( 0.0f, 1.0f ) ) - read_imagef( src2, samplerlin, coord - ( float2 )( 0.0f, 1.0f ) );

	value.x  = CSELECT( warped - BUF( lx, ly ) );
	value.y  = CSELECT( mix( dx2, dx1, 0.0f ) );
	value.z  = CSELECT( mix( dy2, dy1, 0.0f ) );
//	value.w  = max( 1e-4f, 1.0f * exp( - pow( BETA * ( fast_length( mix( dx2, dx1, 0.5f ) ) + fast_length( mix( dy2, dy1, 0.5f )) ), ALPHA ) ) );

	value.w  = max( 1e-4f, 1.0f * exp( - pow( BETA * ( CSELECT( mix( fabs( dx2 ), fabs( dx1 ), 0.0f ) ) + CSELECT( mix( fabs( dy2 ), fabs( dy1 ), 0.0f )) ), ALPHA ) ) );

	write_imagef( out,( int2 )( gx, gy ), value );
}
