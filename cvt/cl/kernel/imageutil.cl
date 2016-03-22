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

__kernel void image_rgba_to_gray_x( __write_only image2d_t out, __read_only image2d_t in, int chan )
{
	int w = get_image_width( out );
	int h = get_image_height( out );
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

    int2 coord;
	coord.x = get_global_id( 0 );
	coord.y = get_global_id( 1 );

	if( coord.x >= w || coord.y >= h )
		return;

	float4 pixel = read_imagef( in, sampler, coord );
	write_imagef( out, coord, ( float4 ) *( ( float* )( &pixel ) + chan ) );
}

__kernel __attribute__((reqd_work_group_size( 16, 16, 1))) void image_gradexp_to_x( __write_only image2d_t out, __read_only image2d_t in, __read_only image2d_t img, int chan )
{
	const int w = get_image_width( in );
	const int h = get_image_height( in );
	const int lx = get_local_id( 0 );
	const int ly = get_local_id( 1 );
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;
	const sampler_t samplerclamp = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
	const float4 dotmulGRAY = ( float4 ) (0.2126f, 0.7152f, 0.0722f, 0.0f );
	const int2 base = ( int2 )( ( get_group_id( 0 ) << 4 ) - 1,  ( get_group_id( 1 ) << 4 ) - 1 );
	local float buf[ 18 * 18 ];

    int2 coord;
	coord.x = get_global_id( 0 );
	coord.y = get_global_id( 1 );

	for( int y = ly; y < 18; y += 16 ) {
		for( int x = lx; x < 18; x += 16 ) {
			//buf[ mul24( y, 18 ) + x ] = dot( read_imagef( img, samplerclamp, base + ( int2 )( x, y ) ), dotmulGRAY );
			buf[ mul24( y, 18 ) + x ] = read_imagef( img, samplerclamp, base + ( int2 )( x, y ) ).x;
		}
	}

	barrier( CLK_LOCAL_MEM_FENCE );

	if( coord.x >= w || coord.y >= h )
		return;

#define BUF( x, y ) buf[ mul24( ( ( y ) + 1 ), 18 ) + ( x ) + 1 ]
#define ALPHA 2.0f
#define BETA 5.0f
#define GAMMA 1.0f
	float2 delta = ( float2 ) ( BUF( lx - 1, ly ) - BUF( lx + 1, ly ), BUF( lx, ly - 1 ) - BUF( lx, ly + 1 ) );
	float4 pixel = read_imagef( in, sampler, coord );
	float weight = GAMMA * exp( - BETA * pow( fast_length( delta ), ALPHA ) );
	*( ( float* )( &pixel ) + chan ) = weight;
	write_imagef( out, coord, ( float4 ) pixel );
#undef BUF
}

inline float4 rgba2hsla(float4 rgba)
{
	float r = rgba.x,
		  g = rgba.y,
		  b = rgba.z,
		  a = rgba.w;

	float mn = min(r, min(g, b));
	float mx = max(r, max(g, b));

	float l = (mn + mx) / 2.f;
	float s, h;
	if (mn == mx) {
		s = h = 0;
	} else {
		float diff = mx - mn;
		float sum = mx + mn;
		if (l < 0.5f)
			s = diff / sum;
		else
			s = diff / (2.0f - sum);
		if (r == mx)
			h = (g - b) / diff;
		else if (g == mx)
			h = 2.0f + (b - r) / diff;
		else //if (b == mx)
			h = 4.0f + (r - g) / diff;
	}
	h = clamp(h / 6.0f, 0.0f, 1.0f);
	return (float4)(h, s, l, a);
}

__kernel __attribute__((reqd_work_group_size( 16, 16, 1))) void image_structure_tensor( __write_only image2d_t out, __read_only image2d_t in )
{
	const int w = get_image_width( in );
	const int h = get_image_height( in );
	const int lx = get_local_id( 0 );
	const int ly = get_local_id( 1 );
	const sampler_t samplerclamp = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
//	const float4 dotmulGRAY = ( float4 ) (0.2126f, 0.7152f, 0.0722f, 0.0f );
	const float4 dotmulGRAY = ( float4 ) (0.33333f, 0.33333f, 0.33333f, 0.0f );
//	const float4 dotmulGRAY = ( float4 ) (0.25, 0.0f, 0.75f, 0.0f );
	const int2 base = ( int2 )( ( get_group_id( 0 ) << 4 ) - 3,  ( get_group_id( 1 ) << 4 ) - 3 );
	local float buf[ 22 * 22 ];
	local float2 buf2[ 20 * 20 ];

    int2 coord;
	coord.x = get_global_id( 0 );
	coord.y = get_global_id( 1 );

	for( int y = ly; y < 22; y += 16 ) {
		for( int x = lx; x < 22; x += 16 ) {
			buf[ mul24( y, 22 ) + x ] = dot( ( read_imagef( in, samplerclamp, base + ( int2 )( x, y ) ) ), dotmulGRAY );
		}
	}

	barrier( CLK_LOCAL_MEM_FENCE );

#define BUF( x, y ) ( buf[ mul24( ( ( y ) + 1 ), 22 ) + ( x ) + 1 ] )

	for( int y = ly; y < 20; y += 16 ) {
		for( int x = lx; x < 20; x += 16 ) {
			buf2[ mul24( y, 20 ) + x ] = ( float2 ) ( BUF( x + 1, y ) - BUF( x - 1 , y ), BUF( x, y + 1 ) - BUF( x, y - 1 ) );
		}
	}

	barrier( CLK_LOCAL_MEM_FENCE );

	if( coord.x >= w || coord.y >= h )
		return;

#define OUTER( a ) ( ( float4 )( a.x * a, a.y * a ) )
#define BUF2( x, y ) ( buf2[ mul24( ( ( y ) + 2 ), 20 ) + ( x ) + 2 ] )


	float4 result = ( float4 ) 0.0f;
	float gaussweights[ 5 ] = { 0.1f, 0.2f, 0.4f, 0.2f, 0.1f };
	for( int y = -2; y <= 2; y++ ) {
		float w = gaussweights[ y + 2 ];
		for( int x = -2; x <= 2; x++ ) {
			float2 d = BUF2( lx + x, ly + y );
			result = result + OUTER( d ) * w * gaussweights[ x + 2 ];
		}
	}
//	result = result / 9.0f;
//	result.w = 0.0f;
//	float2 d =  BUF2( lx, ly );
//	result = result * exp( -0.001f * sqrt( result.x + result.y ) );

//	write_imagef( out, coord, result );
//	d *= ( 1.0f /  9.0f );
#if 0
#define GAMMA 1.0f
	float4 wght = ( float4 ) ( exp( -GAMMA * sqrt( result.x ) ), exp( -GAMMA * sqrt( result.x ) ), exp( -GAMMA * sqrt( result.w ) ), exp( -GAMMA * sqrt( result.w ) ) );
//	float4 wght = ( float4 ) exp( -5.0f * fast_length( BUF2( lx, ly ) ) );
//	float4 wght = ( float4 ) 1.0f;
	{
		float trace = result.x + result.w;
		float det = result.x * result.w - result.y * result.z;

		float4 wght = ( float4 ) exp( -1.0f * sqrt( ( result.x + result.w ) ) );

		float dsc = sqrt( trace * trace * 0.25f - det );
		float ev1 = 0.5f * trace + dsc;
		float ev2 = 0.5f * trace - dsc;

		if( fabs( result.z ) > 1e-2f ) {
			result = ( float4 ) ( ev1 - result.w, result.z, ev2 - result.w, result.z );
			result = ( float4 ) ( normalize( result.xy ), normalize( result.zw ) );
			result = result * wght;
		} else if( fabs( result.y ) > 1e-2f ) {
			result += ( float4 ) ( result.y, ev1 - result.x, result.y, ev2 - result.x );
			result = ( float4 ) ( normalize( result.xy ), normalize( result.zw ) );
			result = result * wght;
		} else {
			result = ( float4 ) ( 1.0f, 0.0f, 0.0f, 1.0f );
		}

//			result = ( float4 ) ( 1.0f, 0.0f, 0.0f, 1.0f );
//		result = fabs( result );
//		result.w = 1.0f;


//		result = sqrt( result );
//		wght = ( float4 ) exp( -1.0f * fabs( ev1 - ev2 ) );//( ev1, ev1, ev2, ev2 );
	}
#else
	float2 d =  /*(float2)( sqrt( result.x), sqrt( result.w ));*/ BUF2( lx, ly );
	float2 wd = d;
//	float4 wght = ( float4 ) exp( -1.0f * fast_length( d ) );
	if( fast_length( d ) > 0.01f ) {
		d = normalize( d );

		result = ( float4 ) ( d.x * d, d.y * d );

//		result = result * exp( -10.0f * fast_length( wd ) );
		result *= exp( -1.0f * fabs( wd.x ) );
		d = ( float2 ) ( d.y, -d.x );
		result = result + exp( -1.0f * fabs( wd.y ) ) * ( float4 ) ( d.x * d, d.y * d );
//		result = result + ( float4 ) ( d.x * d, d.y * d );

//		result = fabs( result );
//		result.w = 1.0f;

//		result = ( float4 ) ( d.x, d.y, 0.0f, 1.0f );
//		result = result * wght;
		//result = result * wght;
	} else {
		result = ( float4 ) ( 1.0f, 0.0f, 0.0f, 1.0f );
	}
//	result += ( float4 ) ( d.x * d.x, d.y, d.y, d.x );
#endif

	write_imagef( out, coord, result );

}
