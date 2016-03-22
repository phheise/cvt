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

#define FBP_LAMBDA 50.0f
#define FBP_DATATRUNC 0.1f
#define FBP_DISCPENALTY 0.5f
#define FBP_DISCTRUNC 1.5f
#define FBP_INF	1e20f

__kernel void stereoCV_FBP_WTA( __write_only image2d_t dmap, global const float2* cv, const int depth )
{
	const int gx = get_global_id( 0 );
	const int gy = get_global_id( 1 );
	const int width = get_image_width( dmap );
	const int height = get_image_height( dmap );
	const int zstride = mul24( width, height );
	global const float2* cvptr = cv + ( mul24( gy, width ) + gx );
	int idx, cmp;
	float val, nval;

	if( gx >= width || gy >= height )
		return;

	idx = 0;
	val = cvptr->y;

	for( int d = 1; d < depth; d++ ) {
		nval = cvptr->y;
		cmp = isless( nval, val );
		val = select( val, nval, cmp );
		idx = select( idx, d, cmp );
		cvptr += zstride;
	}

	val = ( float ) idx / ( float ) depth;
	write_imagef( dmap, ( int2 ) ( gx, gy ), ( float4 ) ( val, val, val, 1.0f ) );
}

__kernel void stereoCV_FBP_HALFDOWN( global float2* out, global const float2* in, const int width, const int height, const int depth )
{
	const int gx = get_global_id( 0 );
	const int gy = get_global_id( 1 );
	const int gz = get_global_id( 2 );
	const int ystride = width;
	const int zstride = mul24( width, height );

	if( gx >= width - 20 || gy >= height - 20 || gz >= depth - 20 )
		return;

	*( out + mul24( zstride, gz ) + mul24( ystride, gy ) + gx ) =
	*( in + ( ( zstride * 4 ) * gz ) + ( ( ystride * 2 ) * ( gy * 2 ) ) + ( gx * 2 ) );
}

__kernel void stereoCV_FBP_HALFUP( global float2* out, global const float2* in, const int width, const int height, const int depth )
{
	const int gx = get_global_id( 0 );
	const int gy = get_global_id( 1 );
	const int gz = get_global_id( 2 );
	const int ystride = width;
	const int zstride = mul24( width, height );

	if( gx >= width || gy >= height || gz >= depth )
		return;

	( out + mul24( zstride, gz ) + mul24( ystride, gy ) + gx )->y =
	( in + mul24( ( zstride >> 2 ), gz ) + mul24( ( ystride >> 2 ), gy ) + ( gx >> 1 ) )->y;
}

__kernel void stereoCV_FBP_Sobel( write_only image2d_t out, read_only image2d_t in )
{
	const int w = get_image_width( in );
	const int h = get_image_height( in );
	const int lx = get_local_id( 0 );
	const int ly = get_local_id( 1 );
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;
	const sampler_t samplerclamp = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
	const int2 base = ( int2 )( ( get_group_id( 0 ) << 4 ) - 1,  ( get_group_id( 1 ) << 4 ) - 1 );
	local float buf[ 18 * 18 ];

    int2 coord;
	coord.x = get_global_id( 0 );
	coord.y = get_global_id( 1 );

	for( int y = ly; y < 18; y += 16 ) {
		for( int x = lx; x < 18; x += 16 ) {
			buf[ mul24( y, 18 ) + x ] = read_imagef( in, samplerclamp, base + ( int2 )( x, y ) ).x;
		}
	}

	barrier( CLK_LOCAL_MEM_FENCE );

	if( coord.x >= w || coord.y >= h )
		return;

#define BUF( x, y ) buf[ mul24( ( ( y ) + 1 ), 18 ) + ( x ) + 1 ]
	float2 d;
	d.x = BUF( lx - 1, ly ) * 2;
	d.x += BUF( lx - 1, ly - 1 );
	d.x += BUF( lx - 1, ly + 1 );
	d.x -= BUF( lx + 1, ly ) * 2;
	d.x -= BUF( lx + 1, ly - 1 );
	d.x -= BUF( lx + 1, ly + 1 );

	d.y = BUF( lx, ly - 1 ) * 2;
	d.y += BUF( lx - 1, ly - 1 );
	d.y += BUF( lx + 1, ly - 1 );
	d.y -= BUF( lx, ly + 1 ) * 2;
	d.y -= BUF( lx - 1, ly + 1 );
	d.y -= BUF( lx + 1, ly + 1 );

	write_imagef( out, coord, ( float4 ) length( d * 0.25f ) );
}

__kernel void stereoCV_FBP_AD( global float2* cv, const int depth, __read_only image2d_t src1, __read_only image2d_t src2, __local float* buf  )
{
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
	const int lx = get_local_id( 0 );
	const int lw = get_local_size( 0 );
	const int gid = get_group_id( 0 );
	const int gx = get_global_id( 0 );
	const int gy = get_global_id( 1 );
	const int base = mul24( gid, lw );
	const int width = get_image_width( src1 );
	const int height = get_image_height( src2 );
	const int buflen = depth + lw;
	const int zstride = mul24( width, height );
	float pixel;
	global float2* cvptr = cv + mul24( gy, width ) + gx;

	/* Fill the local buffer with data from src2 */
	for( int d = lx; d < buflen; d += lw )
		buf[ d ] = read_imagef( src2, sampler, ( int2 )( base + d, gy ) ).x;

	barrier( CLK_LOCAL_MEM_FENCE );

	/* we assume src1.width == src2.width == cv.width */
	if( gx >= width )
		return;

	/* read current pixel from src1 */
	pixel = read_imagef( src1, sampler, ( int2 ) ( gx, gy ) ).x;

	/* store the result of the cost function */
	for( int d = 0; d < depth; d++ ) {
		*cvptr = ( float2 ) ( FBP_LAMBDA * fmin( fabs( pixel - buf[ lx + d ] ), FBP_DATATRUNC ), 0.0f );
		cvptr += zstride;
	}
}

inline void stereoCV_FBP_distanceTransformMinNormalize( float* buf, const int depth )
{
	float minval, maxval;
	float sum = 0;

	for( int d = 1; d < depth; d++ ) {
		buf[ d ] = fmin( buf[ d ], buf[ d - 1 ] + FBP_DISCPENALTY );
	}

	minval = buf[ depth - 1 ];
	for( int d = depth-2; d >= 0; d-- )	{
		buf[ d ] = fmin( buf[ d ], buf[ d + 1 ] + FBP_DISCPENALTY );
		minval = fmin( minval, buf[ d ] );
	}

//	maxval = minval + FBP_DISCTRUNC;
	for( int d = 0; d < depth; d++ ) {
		buf[ d ] = fmin( FBP_DISCTRUNC, buf[ d ] - minval );// - minval;
	}
}

__kernel void stereoCV_FBP( global float2* cv, const int width, const int height, const int depth, const int t )
{
	const int lx = get_local_id( 0 );
	const int ly = get_local_id( 1 );
	const int lw = get_local_size( 0 );
	const int lh = get_local_size( 1 );
	const int gy = get_global_id( 1 );
	const int off = ( ( gy + t ) & 1 );
	const int gx = get_global_id( 0 ) * 2 + off;
	const int ystride = width;
	const int zstride = mul24( width, height );
	global float2* cvptr = cv + mul24( gy, width ) + gx;
	global const float2* cvptr1 = cvptr + 1;
	global const float2* cvptr2 = cvptr - 1;
	global const float2* cvptr3 = cvptr + ystride;
	global const float2* cvptr4 = cvptr - ystride;
	local float lbuf[ 18 ][ 17 ];
	float buf[ 200 ];

	const bool in = gx > 0 && gx < width - 1 && gy > 0 && gy < height - 1;


	for( int d = 0; d < depth; d++ ) {

		if( in ) {
			lbuf[ ly + 1 ][ lx ] = cvptr2->y;

			if( lx == lw - 1  )
				lbuf[ ly + 1 ][ lw ] = cvptr1->y;

			if( ly == 0 )
				lbuf[ 0 ][ lx + off ] = cvptr4->y;

			if( ly == lh - 1 )
				lbuf[ lh + 1 ][ lx + off ] = cvptr3->y;

		}

		barrier( CLK_LOCAL_MEM_FENCE );

		if( in ) {
			buf[ d ] = ( cvptr->x - 1.0f * cvptr->y //* ( 0.8f - ( gx==1 || gx == width - 2) ?0.2f:0 - ( gy == 1 || gy == height - 2)?0.2f:0 ) // current pixel
				+ lbuf[ ly ][ lx + off ]
				+ lbuf[ ly + 1 ][ lx ]
				+ lbuf[ ly + 1 ][ lx + 1 ]
				+ lbuf[ ly + 2 ][ lx + off ] );

			/*				   + cvptr1->y // right
							   + cvptr2->y // left
							   + cvptr3->y // top
							   + cvptr4->y; // bottom*/
		}


		cvptr += zstride;
		cvptr1 += zstride;
		cvptr2 += zstride;
		cvptr3 += zstride;
		cvptr4 += zstride;

		barrier( CLK_LOCAL_MEM_FENCE );
	}

	if( !in )
		return;

	stereoCV_FBP_distanceTransformMinNormalize( buf, depth );

	cvptr = cv + mul24( gy, width ) + gx;
	for( int d = 0; d < depth; d++ ) {
		cvptr->y = buf[ d ]; // update belief
		cvptr += zstride;
	}
}
