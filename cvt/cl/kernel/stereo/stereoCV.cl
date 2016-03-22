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

inline float costRGB_L1( const float4 i1, const float4 i2 )
{
	const float4 dotmul = { 0.333333, 0.333333, 0.333333, 0 };
	return dot( fabs( i2 - i1 ), dotmul );
}

inline float costRGB_L2( const float4 i1, const float4 i2 )
{
	const float4 dotmul = { 0.333333, 0.333333, 0.333333, 0 };
	float4 tmp = i2 - i1;
	tmp.w = 0;
	return fast_length( tmp );
}

inline float costGRAY_L1( const float4 i1, const float4 i2 )
{
	return fabs( i2.x - i1.x );
}

inline float costGRAY_L2( const float4 i1, const float4 i2 )
{
	float tmp = i2.x - i2.y;
	return tmp * tmp;
}

__kernel void stereoCV( global float* cv, const int depth, __read_only image2d_t src1, __read_only image2d_t src2, __local float4* buf  )
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
	const int stride = mul24( width, height );
	float4 pixel;
	global float* cvptr = cv + ( mul24( gy, width ) + gx );

	/* Fill the local buffer with data from src2 */
	for( int d = lx; d < buflen; d += lw )
		buf[ d ] = read_imagef( src2, sampler, ( int2 )( base + d, gy ) );

	barrier( CLK_LOCAL_MEM_FENCE );

	/* we assume src1.width == src2.width == cv.width */
	if( gx >= width )
		return;

	/* read current pixel from src1 */
	pixel = read_imagef( src1, sampler, ( int2 ) ( gx, gy ) );

	/* store the result of the cost function */
	for( int d = 0; d < depth; d++ )
		cvptr[ d * stride ] = costRGB_L1( pixel, buf[ lx + d ] );
}

__kernel void stereoCV_GRAY_AD( global float* cv, const int depth, __read_only image2d_t src1, __read_only image2d_t src2, __local float* buf  )
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
	const int stride = mul24( width, height );
	float pixel;
	global float* cvptr = cv + ( mul24( gy, width ) + gx );

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
	for( int d = 0; d < depth; d++ )
		cvptr[ d * stride ] = fabs( pixel - buf[ lx + d ] );
}

__kernel void stereoCV_GRAY_SAD( global float* cv, const int depth, __read_only image2d_t src1, __read_only image2d_t src2, __local float* buf  )
{
#define R 1
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
	const int lx = get_local_id( 0 );
	const int lw = get_local_size( 0 );
	const int gid = get_group_id( 0 );
	const int gx = get_global_id( 0 );
	const int gy = get_global_id( 1 );
	const int base = mul24( gid, lw );
	const int width = get_image_width( src1 );
	const int height = get_image_height( src1 );
	const int buflen = depth + lw;
	const int stride = mul24( width, height );
	float pixel[ ( 2 * R + 1 ) * ( 2 * R + 1 ) ];
	global float* cvptr = cv + ( mul24( gy, width ) + gx );

	/* Fill the local buffer with data from src2 */
	for( int d = lx; d < buflen; d += lw )
		for( int dy =-R; dy <= R; dy++ )
			for( int dx =-R; dx <= R; dx++ )
				buf[ d * ( ( 2 * R + 1 ) * ( 2 * R + 1 ) ) + ( dy + R ) * ( 2 * R + 1 ) + dx + R ] = read_imagef( src2, sampler, ( int2 )( base + d + dx, gy + dy ) ).x;

	barrier( CLK_LOCAL_MEM_FENCE );

	/* we assume src1.width == src2.width == cv.width */
	if( gx >= width )
		return;

	/* read current pixel from src1 */
	for( int dy =-R; dy <= R; dy++ )
		for( int dx =-R; dx <= R; dx++ )
			pixel[ ( dy + R ) * R + dx + R ] = read_imagef( src1, sampler, ( int2 ) ( gx + dx, gy + dy ) ).x;

	/* store the result of the cost function */
	for( int d = 0; d < depth; d++ ) {
		float val = 0;
		for( int dy =-R; dy <= R; dy++ ) {
			for( int dx =-R; dx <= R; dx++ ) {
				val += fabs( pixel[ ( dy + R ) * ( 2 * R + 1 ) + dx + R ] - buf[ ( lx + d ) * ( ( 2 * R + 1 ) * ( 2 * R + 1 ) ) + ( dy + R ) * ( 2 * R + 1 ) + dx + R] );
			}
		}
		cvptr[ d * stride ] = val / ( float ) ( ( 2 * R + 1 ) * ( 2 * R + 1 ) );
	}
}


__kernel void stereoCV_GRAY_SAD2( global float* cv, const int depth, __read_only image2d_t src1, __read_only image2d_t src2 )
{
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
	const int gx = get_global_id( 0 );
	const int gy = get_global_id( 1 );
	const int gz = get_global_id( 2 );
	const int width = get_image_width( src1 );
	const int height = get_image_height( src1 );
	const int stride = mul24( width, height );
	global float* cvptr = cv + gy * width + gx;

	if( gx >= width || gy >= height || gz >= depth )
		return;

#define RADIUS 1
	float val = 0;
	for( int dy =-RADIUS; dy <= RADIUS; dy++ ) {
		for( int dx =-RADIUS; dx <= RADIUS; dx++ ) {
			val += costRGB_L1( read_imagef( src1, sampler, ( int2 ) ( gx + dx, gy + dy ) ), read_imagef( src2, sampler, ( int2 ) ( gx - gz + dx, gy - gz + dy ) ) );
		}
	}
	cvptr[ gz * stride ] = val / ( float ) ( ( 2 * RADIUS + 1 ) * ( 2 * RADIUS + 1 ) );
}

__kernel void stereoCV_WTA( __write_only image2d_t dmap, global const float* cv, const int depth )
{
	const int gx = get_global_id( 0 );
	const int gy = get_global_id( 1 );
	const int width = get_image_width( dmap );
	const int height = get_image_height( dmap );
	const int stride = mul24( width, height );
	global const float* cvptr = cv + ( mul24( gy, width ) + gx );
	int idx, cmp;
	float val, nval;

	if( gx >= width || gy >= height )
		return;

	idx = 0;
	val = cvptr[ 0 ];

	for( int d = 1; d < depth; d++ ) {
		nval = cvptr[ d * stride ];
		cmp = isless( nval, val );
		val = select( val, nval, cmp );
		idx = select( idx, d, cmp );
	}

	val = ( float ) idx / ( float ) depth;
	write_imagef( dmap, ( int2 ) ( gx, gy ), ( float4 ) ( val, val, val, 1.0f ) );
}

__kernel void stereoCV_WTAMINMAX( __write_only image2d_t dmap, global const float* cv, const int depth )
{
	const int gx = get_global_id( 0 );
	const int gy = get_global_id( 1 );
	const int width = get_image_width( dmap );
	const int height = get_image_height( dmap );
	const int stride = mul24( width, height );
	global const float* cvptr = cv + ( mul24( gy, width ) + gx );
	int idx, cmp;
	float val, val2, nval, cmin, cmax, avg, avg2, curv;

	if( gx >= width || gy >= height )
		return;

	idx = 0;
	val = cvptr[ 0 ];
	val2 = cvptr[ 0 ];
	cmax = val;
	cmin = val;
	avg = val;
	avg2 = val * val;
	curv = 0;
	float nmins = 0;

	for( int d = 1; d < depth; d++ ) {
		nval = cvptr[ d * stride ];
		avg += nval;
		avg2 += nval * nval;
		cmax = fmax( nval, cmax );
		cmin = fmin( nval, cmin );
		cmp = isless( nval, val );
		val2 = select( val2, val, cmp );
		val = select( val, nval, cmp );
		idx = select( idx, d, cmp );
//		curv += ( cvptr[ ( max( d - 1, 0 ) ) * stride ] + cvptr[ ( min( d + 1, depth - 1 ) ) *stride ] - 2.0f * nval ) < 0;
//		curv += fabs( cvptr[ ( max( d - 1, 0 ) ) * stride ] - nval );

	/*	if( d < depth - 1 ) {
			if( cvptr[ ( d - 1 ) * stride ] >= nval && cvptr[ ( d + 1 ) *stride ] >= nval )
				nmins += 1;
		}*/
	}

	avg /= ( float ) depth;
	avg2 /= ( float ) depth;

	float gammaval = ( val2 + 1e-4f ) / ( val + 1e-4f );// ( cvptr[ ( max( idx - 1, 0 ) ) * stride ] + cvptr[ ( min( idx + 1, depth - 1 ) ) *stride ] - 2.0f * val );// + clamp( 0.5f - avg, 0.0f, 0.5f );//( avg - cmin ) * ( avg - cmin );//( avg2 - avg * avg );//avg - cmin;
//	gammaval = gammaval * gammaval;
#define GAMMA 0.01f
	gammaval = clamp( ( 1.0 - exp( -0.05f * gammaval ) ), 0.0f, 1.0f ) * 0.25f + 0.5f * exp( -val * 300.0f ) + 0.25f * ( exp( -150.0 * ( avg - val ) ) ); ;//1.0f / ( 1.0f + exp( -( 1.0f * ( val2 / ( val + 1e-4f) - 3.0f ) ) ) );
	gammaval = clamp( gammaval, 0.0f, 1.0f );
	write_imagef( dmap, ( int2 ) ( gx, gy ), ( float4 ) ( ( float ) idx / ( float ) ( depth - 1 ), cmax - cmin, 0.0f, 1.0f ) );
//	write_imagef( dmap, ( int2 ) ( gx, gy ), ( float4 ) ( ( float ) idx / ( float ) depth, clamp( 1.0f * exp( -1.0f * fabs( avg - cmin ) - 1.0f * ( avg2 - avg * avg ) ), 1e-4f, 1.0f ) , 0.0f, 1.0f ) );
//	write_imagef( dmap, ( int2 ) ( gx, gy ), ( float4 ) ( ( float ) idx / ( float ) depth, clamp( pow( fabs( avg - cmin ) * 1.0f, -1.0f ), 1e-5f, 1.0f ), 0.0f, 1.0f ) );
}

__kernel void stereoCV_boxfilter( global float* cvout, global const float* cvin, const int width, const int height, const int depth, const int r, local float* buf )
{
	const int lx = get_local_id( 0 );
	const int ly = get_local_id( 1 );
	const int gx = get_global_id( 0 );
	const int gy = get_global_id( 1 );
	const int lw = get_local_size( 0 );
	const int lh = get_local_size( 1 );
	const int r2 = r * 2;
	const int bstride = lw + r2;
	const int stride = mul24( width, height );
	int2 mincoord, maxcoord;
	global const float* cvptr = cvin + stride * get_global_id( 2 );// + ( mul24( gy, width ) + gx );
	global float* cvoptr = cvout + stride * get_global_id( 2 );
	float size;
	const int basex = get_group_id( 0 ) * lw - r;
	const int basey = get_group_id( 1 ) * lh - r;

	mincoord.x = max( 0, basex );
	mincoord.y = max( 0, basey );
	maxcoord.x = min( width - 1, basex + bstride );
	maxcoord.y = min( height - 1, basey + bstride );

	for( int y = mincoord.y + ly; y < maxcoord.y; y += lh ) {
		for( int x = mincoord.x + lx; x < maxcoord.x; x += lw ) {
//			int2 cvpos = min( max( ( int2 )( basex + x - r, basey + y - r  ), ( int2 ) ( 0, 0 ) ), ( int2 ) ( width - 1, height - 1 ) );
//			int2 cvpos =( int2 )( basex + x - r, basey + y - r  );
//			if(cvpos.x >= 0 && cvpos.x < width && cvpos.y  >= 0 && cvpos.y < height )
			buf[ mul24( y - basey, bstride ) + x - basex ] = cvptr[ mul24( y, width ) + x ];
		}
	}
	barrier( CLK_LOCAL_MEM_FENCE );

	if( gx >= width || gy >= height )
		return;

#define BUF( x, y ) ( buf[ mul24( ( y ) + r, bstride ) + ( x ) + r ] )

	mincoord.x = max( 0, gx - r );
	mincoord.y = max( 0, gy - r );
	maxcoord.x = min( width - 1, gx + r );
	maxcoord.y = min( height - 1, gy + r );

	size = ( maxcoord.x - mincoord.x + 1 ) * ( maxcoord.y - mincoord.y + 1 );
	mincoord -= ( int2 ) ( gx, gy );
	maxcoord -= ( int2 ) ( gx, gy );

	float value = 0;
	for( int y = mincoord.y; y <= maxcoord.y; y++ )
		for( int x = mincoord.x; x <= maxcoord.x; x++ )
			value += BUF( lx + x, ly + y );
	value /= size;
	cvoptr[ mul24( gy, width ) + gx ] = value;
}
