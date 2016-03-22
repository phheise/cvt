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

__kernel void fgp( __write_only image2d_t out, __read_only image2d_t img, __read_only image2d_t e1, __read_only image2d_t e0, const float lambda, const float t, __local float4* buf, __local float4* buf2  )
{
    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
    const sampler_t samplerb = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
	const int gx = get_global_id( 0 );
	const int gy = get_global_id( 1 );
	const int lx = get_local_id( 0 );
    const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
    const int bstride = ( lw << 1 ) + 4;
	const int2 base  = ( int2 )( get_group_id( 0 ) * lw, get_group_id( 1 ) * lh );
	const int2 base2 = ( int2 )( get_group_id( 0 ) * ( lw << 1 ) - 2, get_group_id( 1 ) * lh - 1 );
	int2 coord;
	float4 dx, dy, pxout, pyout, norm;

	for( int y = ly; y < lh + 2; y += lh ) {
		coord.y = y;
		for( int x = lx; x < lw + 2; x += lw ) {
			coord.x = x << 1;
			// px
			buf[ mul24( coord.y, bstride ) + coord.x ] = read_imagef( e1, samplerb, base2 + coord ) * ( 1.0f + t ) - t * read_imagef( e0, samplerb, base2 + coord );
			// py
			coord.x += 1;
			buf[ mul24( coord.y, bstride ) + coord.x ] = read_imagef( e1, samplerb, base2 + coord ) * ( 1.0f + t ) - t * read_imagef( e0, samplerb, base2 + coord );
		}
	}

	barrier( CLK_LOCAL_MEM_FENCE );

#define XBUF( x, y ) ( buf[ mul24( ( y ) + 1, bstride ) + ( ( ( x ) + 1 ) << 1 ) ] )
#define YBUF( x, y ) ( buf[ mul24( ( y ) + 1, bstride ) + ( ( ( x ) + 1 ) << 1 ) + 1 ] )
#define BUF2( x, y ) ( buf2[ mul24( ( y ), ( lw + 1 ) ) + ( x ) ] )

	for( int y = ly; y < lh + 1; y += lh ) {
		for( int x = lx; x < lw + 1; x += lw ) {
			dx = XBUF( x, y ) - XBUF( x - 1, y );
			dy = YBUF( x, y ) - YBUF( x, y - 1 );
			// image + lambda * div( p )
			BUF2( x, y ) = read_imagef( img, sampler, base + ( int2 )( x, y ) ) + lambda * ( dx + dy );
		}
	}

	barrier( CLK_LOCAL_MEM_FENCE );

	if( gx >= get_image_width( img ) || gy >= get_image_height( img ) )
		return;

	dx = BUF2( lx + 1, ly ) - BUF2( lx, ly );
	dy = BUF2( lx, ly + 1 ) - BUF2( lx, ly );

	pxout = XBUF( lx, ly ) + ( 0.125f / lambda ) * dx;
	pyout = YBUF( lx, ly ) + ( 0.125f / lambda ) * dy;

	norm = fmin( ( float4 ) 1.0f, native_rsqrt( pxout * pxout + pyout * pyout ) );
	pxout = pxout * norm;
	pyout = pyout * norm;

	write_imagef( out, ( int2 ) ( gx << 1, gy ), pxout );
	write_imagef( out, ( int2 ) ( ( gx << 1 ) + 1, gy ), pyout );

}
