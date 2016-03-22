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

#import "RNG.cl"

#define PROPSIZE 1
#define DEPTHREFINEMUL 2.0f
#define NORMALREFINEMUL 0.1f
#define NORMALCOMPMAX 0.95f
#define NUMRNDTRIES	 3
#define NUMRNDSAMPLE 6

#define COLORWEIGHT 26.0f
#define COLORGRADALPHA 0.05f
#define COLORMAXDIFF 0.04f
#define GRADMAXDIFF 0.01f
#define OVERSAMPLE 1.0f
#define OVERSAMPLECUBE 0.01f
#define VIEWSAMPLES 4

// #define USELOCALBUF  1

const sampler_t SAMPLER_NN		 = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;


float4 nd_state_viewprop( const float4 state );

float4 nd_state_init( RNG* rng, const float2 coord, int lr, const float normmul, const float depthmax )
{
	float z = RNG_float( rng ) * depthmax;
	float4 n;
	n.x = ( RNG_float( rng ) - 0.5f ) * normmul * NORMALCOMPMAX;
	n.y = ( RNG_float( rng ) - 0.5f ) * normmul * NORMALCOMPMAX;

	n.x = clamp( n.x, -NORMALCOMPMAX, NORMALCOMPMAX );
	n.y = clamp( n.y, -NORMALCOMPMAX, NORMALCOMPMAX );

	float nfactor = fmax( length( n.xy ) + 0.001f, 1.0f );
	n.xy = n.xy / nfactor;
	n.z = native_sqrt( 1.0f - n.x * n.x - n.y * n.y );

	float4 ret = ( float4 ) ( 1.0f, 0.0f, 0.0f, 0.0f ) - ( float4 ) ( - n.x / n.z, - n.y / n.z, ( n.x * coord.x + n.y * coord.y ) / n.z + z, 0.0f );
	if( !lr )
		ret = nd_state_viewprop( ret );
	return ret;
}

float4 nd_state_to_ref_normal_depth( const float4 state, const float2 coord, const int lr )
{
	float4 ret;
	float4 _state = state;
	if( !lr )
		_state = nd_state_viewprop( state );
	_state =  ( float4 ) ( 1.0f, 0.0f, 0.0f, 0.0f ) - _state;
	ret.w = _state.x * coord.x + _state.y * coord.y + _state.z;
	ret.z = native_rsqrt( _state.x * _state.x + _state.y * _state.y + 1.0f );
	ret.x = -_state.x * ret.z;
	ret.y = -_state.y * ret.z;

	if( !all(isfinite(ret.xyz)))
		return (float4) (0.0f,0.0f,1.0f,0.0f);

	return ret;
}

float4 nd_state_refine( RNG* rng, const float4 _state, const float2 coord, const float depthmax, int lr )
{
	float4 n;
	float4 state = _state;
	if( !lr )
		state = nd_state_viewprop( state );
	state =  ( float4 ) ( 1.0f, 0.0f, 0.0f, 0.0f ) - state;
	float z = state.x * coord.x + state.y * coord.y + state.z;
	n.z = native_rsqrt( state.x * state.x + state.y * state.y + 1.0f );
	n.x = -state.x * n.z;
	n.y = -state.y * n.z;

	z += ( RNG_float( rng ) - 0.5f ) * DEPTHREFINEMUL;
	z = clamp( z, 0.0f, depthmax );
	n.x += ( RNG_float( rng ) - 0.5f ) * NORMALREFINEMUL;
	n.y += ( RNG_float( rng ) - 0.5f ) * NORMALREFINEMUL;
	n.x = clamp( n.x, -NORMALCOMPMAX, NORMALCOMPMAX );
	n.y = clamp( n.y, -NORMALCOMPMAX, NORMALCOMPMAX );

	float nfactor = fmax( length( n.xy ) + 0.001f, 1.0f );
	n.xy = n.xy / nfactor;
	n.z = native_sqrt( 1.0f - n.x * n.x - n.y * n.y );


	float4 ret = ( float4 ) ( - n.x / n.z, - n.y / n.z, ( n.x * coord.x + n.y * coord.y ) / n.z + z, 0.0f );
	ret = ( float4 ) ( 1.0f, 0.0f, 0.0f, 0.0f ) - ret;
	if( !lr )
		return nd_state_viewprop( state );
	return ret;
}

float4 nd_state_viewprop( const float4 state )
{
	float4 n = state;
	float4 ret = ( float4 ) ( ( 1.0f / n.x ), -n.y / n.x, -n.z / n.x, 0.0f );
	return ret;
}

float2 nd_state_transform( const float4 state, const float2 coord )
{
	return ( float2 ) ( state.x * coord.x + state.y * coord.y + state.z, coord.y );
}

float4 nd_state_to_color( const float4 _state, const float2 coord, const float depthmax )
{
	float4 n;
	float4 state =  ( float4 ) ( 1.0f, 0.0f, 0.0f, 0.0f ) - _state;
	n.z = native_rsqrt( state.x * state.x + state.y * state.y + 1.0f );
	n.x = -state.x * n.z;
	n.y = -state.y * n.z;
	n.xy = n.xy * 0.5f + 0.5f;
	n.z = ( state.x * coord.x + state.y * coord.y + state.z ) / depthmax;
	n.w = 1.0f;
	return n;
}

float3 nd_state_to_normal( const float4 _state )
{
	float3 n;
	float4 state =  ( float4 ) ( 1.0f, 0.0f, 0.0f, 0.0f ) - _state;
	n.z = native_rsqrt( state.x * state.x + state.y * state.y + 1.0f );
	n.x = -state.x * n.z;
	n.y = -state.y * n.z;
	//n.z = native_sqrt( 1.0f - n.x * n.x - n.y * n.y );
	return n;
}


float4 nd_state_to_normal_color( const float4 _state )
{
	float4 n;
	float4 state =  ( float4 ) ( 1.0f, 0.0f, 0.0f, 0.0f ) - _state;
	n.z = native_rsqrt( state.x * state.x + state.y * state.y + 1.0f );
	n.x = -state.x * n.z;
	n.y = -state.y * n.z;
	n.z = native_sqrt( 1.0f - n.x * n.x - n.y * n.y );
	n.xyz = n.xyz * 0.5f + 0.5f;
	n.w = 1.0f;
	return n;
}


inline float patch_eval_color_grad_weighted( read_only image2d_t colimg1, read_only image2d_t gradimg1,
									  read_only image2d_t colimg2, read_only image2d_t gradimg2,
									  const float2 coord, const float4 state, const int patchsize, const int lr )
{
	float wsum1 = 0;
	float ret1 = 0;
	int width = get_image_width( colimg2 );
	int height = get_image_height( colimg2 );

	if( !all(isfinite(state.xyz)))
		return 1e5f;

//	const float4 grayWeight =  ( float4 ) ( 0.2126f, 0.7152f, 0.0722f, 0.0f );

	float4 valcenter = read_imagef( colimg1, SAMPLER_BILINEAR, coord + ( float2 ) ( 0.5f, 0.5f) );

	for( float dy = -patchsize; dy <= patchsize; dy+=1.0f ) {
		for( float dx = -patchsize; dx <= patchsize; dx+=1.0f ) {

			float2 displace = ( float2 ) ( dx * OVERSAMPLE, dy * OVERSAMPLE );
			//float2 displace = ( float2 ) ( dx + OVERSAMPLECUBE * pow( dx, 3 ), dy + OVERSAMPLECUBE * pow( dy, 3 ) );
			float2 pos = coord + displace;

			if( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height )
				continue;

			float4 val1 = read_imagef( colimg1, SAMPLER_BILINEAR, pos  + ( float2 ) ( 0.5f, 0.5f));
			float4 gval1 = read_imagef( gradimg1, SAMPLER_BILINEAR, pos  + ( float2 ) ( 0.5f, 0.5f));

			float w1 = native_exp( -dot( fabs( valcenter.xyz - val1.xyz ), ( float3 ) 1.0f ) * ( smoothstep( 0.0f, 26.0f, length( displace ) ) * 1.5f * COLORWEIGHT + 5.0f ) );// * exp( -fast_length( displace ) * 0.05f );

//			float w1 = exp( -dot( fabs( valcenter.xyz - val1.xyz ), ( float3 ) 1.0f ) * COLORWEIGHT );// * exp( -fast_length( displace ) * 0.05f );

			// transform point
	//		float d = nd_state_transform( state, pos );
	//		pos.x += select( d, -d, lr );
			pos = nd_state_transform( state, pos );
			if( pos.x < 0 || pos.x >= width )
				continue;

			wsum1 += w1;

#define TOGRAY(x) dot( x, grayWeight )

			float4 val2 = read_imagef( colimg2, SAMPLER_BILINEAR, pos  + ( float2 ) ( 0.5f, 0.5f));
			float4 gval2 = read_imagef( gradimg2, SAMPLER_BILINEAR, pos  + ( float2 ) ( 0.5f, 0.5f));

			float C = COLORGRADALPHA * dot( fmin( fabs( ( val1 - val2 ).xyz ), COLORMAXDIFF ), ( float3 ) 1.0f ) + ( 1.0f - COLORGRADALPHA ) * dot( fmin( fabs( ( gval1 - gval2 ) ), GRADMAXDIFF ), ( float4 ) 1.0f );

//			float C = log( 0.25f * exp(-1.0f * fmin( fast_length( ( val1 - val2 ).xyz ), COLORMAXDIFF  ) ) + 0.75f * exp( -1.0f * fmin( fast_length( ( gval1 - gval2 ) ), GRADMAXDIFF ) ) ) / -1.0f;

			ret1 += w1 * C;
		}
	}

	if( wsum1 <= 1.1f )
		return 1e5f;
	return ret1 / wsum1;
}

kernel void pmhstereo_init( write_only image2d_t output, read_only image2d_t img1, read_only image2d_t img2, read_only image2d_t gimg1, read_only image2d_t gimg2, const int patchsize, const float depthmax, const int lr )
{
	RNG rng;
	const int2 coord = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
	const float2 coordf = ( float2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
	const int width = get_image_width( img1 );
	const int height = get_image_height( img1 );

	if( coord.x >= width || coord.y >= height )
		return;

	RNG_init( &rng, coord.y * width + coord.x, 3 );

	float4 ret = nd_state_init( &rng, coordf, lr, 1.0f, depthmax );

	ret.w  = patch_eval_color_grad_weighted( img1, gimg1, img2, gimg2, coordf, ret, patchsize, lr );

	write_imagef( output, coord, ret );
}

typedef struct {
	int n;
	float4 value[ VIEWSAMPLES ];// __attribute__ ((packed));
} VIEWPROP_t;

kernel void pmhstereo_viewbuf_clear( global VIEWPROP_t* vbuf, const int width, const int height )
{
	const int gx = get_global_id( 0 );
	const int gy = get_global_id( 1 );

	if(	gx >= width || gy >= height )
		return;

	vbuf[ width * gy + gx ].n = 0;
}

float2 smoothDistance( const float4 statea, const float4 stateb, const float4 smooth, const float2 coord, const float depthmax, int lr )
{
  float2 ret;
  float4 a = nd_state_to_ref_normal_depth( statea, coord, lr );
  float4 b = nd_state_to_ref_normal_depth( stateb, coord, lr );
  const float4 diag = ( float4 ) ( 1.0f, 1.0f, 0.0f, 1.0f / ( depthmax * depthmax ) );

  if( !all( isfinite( statea.xyz ) ) || !all( isfinite( stateb.xyz ) ) )
	  return ( float2 ) 0.0f;

  float4 da = a - smooth;
  ret.x = dot( da, diag * da );

  float4 db = b - smooth;
  ret.y = dot( db, diag * db );

  return ret;
}


kernel void pmhstereo_propagate_view( write_only image2d_t output, read_only image2d_t old,
							    read_only image2d_t img1, read_only image2d_t img2,
								read_only image2d_t gimg1, read_only image2d_t gimg2, read_only image2d_t imsmoooth, const float theta,
								const int patchsize, const float depthmax, const int lr, const int iter,
							    global VIEWPROP_t* viewin, global VIEWPROP_t* viewout )
{
	RNG rng;
	const int width = get_image_width( img1 );
	const int height = get_image_height( img1 );
	const int gx = get_global_id( 0 );
	const int gy = get_global_id( 1 );
	const int lx = get_local_id( 0 );
	const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
	const int2 base = ( int2 )( get_group_id( 0 ) * lw - PROPSIZE, get_group_id( 1 ) * lh - PROPSIZE );
	const int2 coord = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
	const float2 coordf = ( float2 ) ( get_global_id( 0 ), get_global_id( 1 ) );

	local float4 buf[ 16 + 2 * PROPSIZE ][ 16 + 2 * PROPSIZE ];
	float4 self, neighbour, smooth;
//	float theta = 0.1f * _theta;

	for( int y = ly; y < lh + 2 * PROPSIZE; y += lh ) {
		for( int x = lx; x < lw + 2 * PROPSIZE; x += lw ) {
			buf[ y ][ x ] = read_imagef( old, SAMPLER_NN, base + ( int2 )( x, y ) );
		}
	}

	barrier( CLK_LOCAL_MEM_FENCE );

	if(	gx >= width || gy >= height )
		return;

	smooth = read_imagef( imsmoooth, SAMPLER_NN, coord );
	float nfactor = fmax( length( smooth.xy ) + 0.001f, 1.0f );
	smooth.xy = smooth.xy / nfactor;
	smooth = ( float4 ) ( smooth.x, smooth.y, native_sqrt( 1.0f - smooth.x * smooth.x - smooth.y * smooth.y ), smooth.z * depthmax );

	self = buf[ ly + PROPSIZE ][ lx + PROPSIZE ];

	RNG_init( &rng, ( coord.y * width + coord.x ) + iter, ( ( ( 2 * PROPSIZE + 1 ) * ( 2 * PROPSIZE + 1 ) - 1 ) + NUMRNDTRIES ) * 3 + 2 * NUMRNDSAMPLE );

	// sample the nd_state of the neighbours
	for( int py = -PROPSIZE; py <= PROPSIZE; py++ ) {
		for( int px = -PROPSIZE; px <= PROPSIZE; px++ ) {

			if( px == 0 && py == 0 )
				continue;

			neighbour = buf[ ly + PROPSIZE + py ][ lx + PROPSIZE + px ];
			neighbour.w  = patch_eval_color_grad_weighted( img1, gimg1, img2, gimg2, coordf, neighbour, patchsize, lr );

			float2 sdist = smoothDistance( neighbour, self, smooth, coordf, depthmax, lr );
			if( neighbour.w + theta * sdist.x <= self.w + theta * sdist.y ) self = neighbour;
		}
	}

	// try smooth
    float2 sdist;
	neighbour = ( float4 ) ( 1.0f, 0.0f, 0.0f, 0.0f ) - ( float4 ) ( - smooth.x / smooth.z, - smooth.y / smooth.z, ( smooth.x * coordf.x + smooth.y * coordf.y ) / smooth.z + smooth.w, 0.0f );
	if( !lr )
		neighbour = nd_state_viewprop( neighbour );
	neighbour.w  = patch_eval_color_grad_weighted( img1, gimg1, img2, gimg2, coordf, neighbour, patchsize, lr );

	sdist = smoothDistance( neighbour, self, smooth, coordf, depthmax, lr );
	if( neighbour.w + theta * sdist.x <= self.w + theta * sdist.y ) self = neighbour;

	// rand neighbourhood tries
	for( int i = 0; i < NUMRNDSAMPLE; i++ ) {
			neighbour = read_imagef( old, SAMPLER_NN, (int2)(gx,gy) + ( int2 )( RNG_float(&rng) * 7.0f + 0.5f, RNG_float(&rng) * 7.0f + 0.5f ) );
			neighbour.w  = patch_eval_color_grad_weighted( img1, gimg1, img2, gimg2, coordf, neighbour, patchsize, lr );

			float2 sdist = smoothDistance( neighbour, self, smooth, coordf, depthmax, lr );
			if( neighbour.w + theta * sdist.x <= self.w + theta * sdist.y ) self = neighbour;
	}

	// random try
	neighbour = nd_state_init( &rng, coordf, lr, 2.0f, depthmax );
	neighbour.w  = patch_eval_color_grad_weighted( img1, gimg1, img2, gimg2, coordf, neighbour, patchsize, lr );

	sdist = smoothDistance( neighbour, self, smooth, coordf, depthmax, lr );
	if( neighbour.w + theta * sdist.x <= self.w + theta * sdist.y ) self = neighbour;

	// try other view
#if 1
	int nview = min( viewin[ width * gy + gx ].n, ( int ) VIEWSAMPLES );
	for( int i = 0; i < nview; i++ ) {
		neighbour = viewin[ width * gy + gx ].value[ i ];
		neighbour.w  = patch_eval_color_grad_weighted( img1, gimg1, img2, gimg2, coordf, neighbour, patchsize, lr );

		float2 sdist = smoothDistance( neighbour, self, smooth, coordf, depthmax, lr );
		if( neighbour.w + theta * sdist.x <= self.w + theta * sdist.y ) self = neighbour;
	}
#endif

	// randomized refinement
	for( int i = 0; i < NUMRNDTRIES - 1; i++ ) {
		neighbour = nd_state_refine( &rng, self, coordf, depthmax, lr );
		neighbour.w  = patch_eval_color_grad_weighted( img1, gimg1, img2, gimg2, coordf, neighbour, patchsize, lr );

		float2 sdist = smoothDistance( neighbour, self, smooth, coordf, depthmax, lr );
		if( neighbour.w + theta * sdist.x <= self.w + theta * sdist.y ) self = neighbour;
	}

	// store view prop result
	// maybe inconsistent r/w access - but random anyway
#if 1
	float2 pos2 = nd_state_transform( self, coordf );
	int disp = ( int ) ( pos2.x + 0.5f );
	if( disp >= 0 && disp < width ) {
		int nold = atomic_inc( &viewout[ gy * width + disp ].n );
		if( nold < VIEWSAMPLES )
			viewout[ gy * width + disp ].value[ nold ] = nd_state_viewprop( self );
	}
#endif

	write_imagef( output, coord, self );
}

kernel void pmhstereo_depthmap( write_only image2d_t depthmap, read_only image2d_t old, const float scale )
{
	int2 coord;
	const int width = get_image_width( depthmap );
	const int height = get_image_height( depthmap );

	coord.x = get_global_id( 0 );
	coord.y = get_global_id( 1 );

	if( coord.x >= width || coord.y >= height )
		return;

	float4 state = read_imagef( old, SAMPLER_NN, coord );
	float4 val;
    val.xyz = ( float3 ) fabs( nd_state_transform( state, ( float2 ) ( coord.x, coord.y ) ).x - ( float ) coord.x );
    if( !all(isfinite(state.xyz)))
        val.xyz = ( float3 ) ( 0.0f, 0.0f, 0.0f );
	val.xyz *= scale;
	val.w = 1.0f;
	write_imagef( depthmap, coord, val );
}

kernel void pmhstereo_consistency( write_only image2d_t output, read_only image2d_t left, read_only image2d_t right, const float maxdepthdiff, const float maxnormaldegdiff, int lr )
{
	int2 coord;
	const int width = get_image_width( output );
	const int height = get_image_height( output );

	coord.x = get_global_id( 0 );
	coord.y = get_global_id( 1 );

	if( coord.x >= width || coord.y >= height )
		return;

	float4 statel = read_imagef( left, SAMPLER_NN, coord );
	float2 coord2 = nd_state_transform( statel, ( float2 ) ( coord.x, coord.y ) );
	float4 stater;
	if( coord2.x < 0 || coord2.x >= width )
		stater = ( float4 ) 1e5f;
	else {
		stater = read_imagef( right, SAMPLER_NN, ( int2 ) ( round(coord2.x), coord2.y ) );
//		stater = read_imagef( right, SAMPLER_BILINEAR, coord2  + ( float2 ) ( 0.5f, 0.5f ) );
	}

	//		stater = read_imagef( right, SAMPLER_BILINEAR, coord2  + ( float2 ) ( 0.5f, 0.5f ) );


	float ndiff;
	if( lr )
		ndiff = dot( nd_state_to_normal( statel ), nd_state_to_normal( nd_state_viewprop( stater ) ) );
	else
		ndiff = dot( nd_state_to_normal( stater ), nd_state_to_normal( nd_state_viewprop( statel ) ) );

	float dmax = fabs( ( float ) coord.x - nd_state_transform( stater, coord2 ).x );
//	float dmax = fmin( dmax, fabs( ( float ) coord.x - nd_state_transform( stater2, coord2 ).x ) );

    float4 val = (dmax>=maxdepthdiff||acospi(ndiff)>=maxnormaldegdiff/180.0f)?( float4 ) 0.0f : ( statel );
	write_imagef( output, coord, val );
}

kernel void pmhstereo_fill_state( write_only image2d_t output, read_only image2d_t input, const float depthmax, int lr )
{
	const int2 coord = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
	const int width = get_image_width( output );
	const int height = get_image_height( output );

	if( coord.x >= width || coord.y >= height )
		return;

	float4 val;
	float4 state = read_imagef( input, SAMPLER_NN, coord );

	if( length( state ) < 1e-1f ) {
		float4 left = ( float4 ) 0;
		int x = coord.x - 1;
		while( length( left ) < 1e-1f && x >= 0 ) {
			left = read_imagef( input, SAMPLER_NN, ( int2 ) ( x, coord.y ) );
			x--;
		}

		float4 right = ( float4 ) 0;
		x = coord.x + 1;
		while( length( right ) < 1e-1f && x < width ) {
			right = read_imagef( input, SAMPLER_NN, ( int2 ) ( x, coord.y ) );
			x++;
		}

		if( length( left ) < 1e-1f ) left = ( float4 ) 1e5f;
		if( length( right) < 1e-1f ) right = ( float4 ) 1e5f;

		left.w = fabs( nd_state_transform( left, ( float2 ) ( coord.x, coord.y ) ).x - ( float ) coord.x );
		right.w = fabs( nd_state_transform( right, ( float2 ) ( coord.x, coord.y ) ).x - ( float ) coord.x );

		if( left.w < right.w )
			val = left;
		else
			val = right;
	} else {
		val = state;
	}

	val = nd_state_to_ref_normal_depth( val, ( float2 ) ( coord.x, coord.y ), lr );
	val = ( float4 ) ( val.x, val.y, val.w / depthmax, 1.0f );
	write_imagef( output, coord, val );
}

kernel void pmhstereo_fill_depthmap( write_only image2d_t output, read_only image2d_t input, const float scale )
{
	const int2 coord = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
	const int width = get_image_width( output );
	const int height = get_image_height( output );

	if( coord.x >= width || coord.y >= height )
		return;

	float4 val;
	float4 state = read_imagef( input, SAMPLER_NN, coord );

	if( length( state.xyz ) < 1e-1f ) {
		float4 left = ( float4 ) 0;
		int x = coord.x - 1;
		while( length( left.xyz ) < 1e-1f && x >= 0 ) {
			left = read_imagef( input, SAMPLER_NN, ( int2 ) ( x, coord.y ) );
			x--;
		}

		float4 right = ( float4 ) 0;
		x = coord.x + 1;
		while( length( right.xyz ) < 1e-1f && x < width ) {
			right = read_imagef( input, SAMPLER_NN, ( int2 ) ( x, coord.y ) );
			x++;
		}

		if( length( left.xyz ) < 1e-1f ) left.xyz = ( float3 ) -1e5f;
		if( length( right.xyz ) < 1e-1f ) right.xyz = ( float3 ) -1e5f;

		left.w = -( nd_state_transform( left, ( float2 ) ( coord.x, coord.y ) ).x - ( float ) coord.x );
		right.w = -( nd_state_transform( right, ( float2 ) ( coord.x, coord.y ) ).x - ( float ) coord.x );

		val.xyz = ( float3 ) fmin( left.w, right.w );
		val.w = 1.0f;

	} else {
		val.xyz = fabs( nd_state_transform( state, ( float2 ) ( coord.x, coord.y ) ).x - ( float ) coord.x );
		val.w = 1.0f;
	}

	val.xyz *= scale;
	write_imagef( output, coord, val );
}

kernel void pmhstereo_fill_normalmap( write_only image2d_t output, read_only image2d_t input )
{
	const int2 coord = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
	const int width = get_image_width( output );
	const int height = get_image_height( output );

	if( coord.x >= width || coord.y >= height )
		return;

	float4 val;
	float4 state = read_imagef( input, SAMPLER_NN, coord );

	if( length( state.xyz ) < 1e-1f ) {
		float4 left = ( float4 ) 0;
		int x = coord.x - 1;
		while( length( left.xyz ) < 1e-1f && x >= 0 ) {
			left = read_imagef( input, SAMPLER_NN, ( int2 ) ( x, coord.y ) );
			x--;
		}

		float4 right = ( float4 ) 0;
		x = coord.x + 1;
		while( length( right.xyz ) < 1e-1f && x < width ) {
			right = read_imagef( input, SAMPLER_NN, ( int2 ) ( x, coord.y ) );
			x++;
		}

		if( length( left.xyz ) < 1e-1f ) left.xyz = ( float3 ) -1e5f;
		if( length( right.xyz ) < 1e-1f ) right.xyz = ( float3 ) -1e5f;

		left.w = -( nd_state_transform( left, ( float2 ) ( coord.x, coord.y ) ).x - ( float ) coord.x );
		right.w = -( nd_state_transform( right, ( float2 ) ( coord.x, coord.y ) ).x - ( float ) coord.x );

        if( left.w < right.w )
		    val.xyz = nd_state_to_normal( left );
        else
		    val.xyz = nd_state_to_normal( right );

		val.w = 0.0f;

	} else {
		val.xyz = nd_state_to_normal( state );
		val.w = 0.0f;
	}

	write_imagef( output, coord, val );
}



kernel void pmhstereo_normalmap( write_only image2d_t normalmap, read_only image2d_t old )

{
	int2 coord;
	const int width = get_image_width( normalmap );
	const int height = get_image_height( normalmap );

	coord.x = get_global_id( 0 );
	coord.y = get_global_id( 1 );

	if( coord.x >= width || coord.y >= height )
		return;

	float4 self = read_imagef( old, SAMPLER_NN, coord );
	float4 val;
	val.xyz = nd_state_to_normal( self );
	val.w = 1.0f;
	write_imagef( normalmap, coord, val );
}

kernel void pmhstereo_colormap( write_only image2d_t normalmap, read_only image2d_t old, const float depthmax )

{
	int2 coord;
	const int width = get_image_width( normalmap );
	const int height = get_image_height( normalmap );

	coord.x = get_global_id( 0 );
	coord.y = get_global_id( 1 );

	if( coord.x >= width || coord.y >= height )
		return;

	float4 self = read_imagef( old, SAMPLER_NN, coord );
	float4 val = nd_state_to_color( self, ( float2 ) ( coord.x, coord.y ), depthmax );
	write_imagef( normalmap, coord, val );
}

kernel void pmhstereo_clear( write_only image2d_t output )

{
	int2 coord;
	const int width = get_image_width( output );
	const int height = get_image_height( output );

	coord.x = get_global_id( 0 );
	coord.y = get_global_id( 1 );

	if( coord.x >= width || coord.y >= height )
		return;

	write_imagef( output, coord, ( float4 ) 0.0f );
}



kernel void pmhstereo_normal_depth( write_only image2d_t output, read_only image2d_t input, const float depthmax, int lr )

{
	int2 coord;
	const int width = get_image_width( output );
	const int height = get_image_height( output );

	coord.x = get_global_id( 0 );
	coord.y = get_global_id( 1 );

	if( coord.x >= width || coord.y >= height )
		return;

	float4 self = read_imagef( input, SAMPLER_NN, coord );
	float4 val = nd_state_to_ref_normal_depth( self, ( float2 ) ( coord.x, coord.y ), lr );
	val = ( float4 ) ( val.x, val.y, val.w / depthmax, 1.0f );
	write_imagef( output, coord, val );
}

kernel void pmhstereo_occmap( write_only image2d_t output, read_only image2d_t left, read_only image2d_t right, const float maxdiff, int lr )
{
	int2 coord;
	const int width = get_image_width( left );
	const int height = get_image_height( left );

	coord.x = get_global_id( 0 );
	coord.y = get_global_id( 1 );

	if( coord.x >= width || coord.y >= height )
		return;

	float4 statel = read_imagef( left, SAMPLER_NN, coord );
	float2 coord2 = nd_state_transform( statel, ( float2 ) ( coord.x, coord.y ) );
	float4 stater;
	if( coord2.x < 0 || coord2.x >= width )
		stater = ( float4 ) 1e5f;
	else {
		stater = read_imagef( right, SAMPLER_NN, ( int2 ) ( round(coord2.x), coord2.y ) );
//		stater = read_imagef( right, SAMPLER_BILINEAR, coord2  + ( float2 ) ( 0.5f, 0.5f ) );
	}

	float ndiff;
	if( lr )
		ndiff = dot( nd_state_to_normal( statel ), nd_state_to_normal( nd_state_viewprop( stater ) ) );
	else
		ndiff = dot( nd_state_to_normal( stater ), nd_state_to_normal( nd_state_viewprop( statel ) ) );

	float val = (fabs( ( float ) coord.x - nd_state_transform( stater, coord2 ).x )>=maxdiff||acospi(ndiff)>=5.0f/180.0f)? 0.0f : 1.0f;
	write_imagef( output, coord, ( float4 ) ( val, val, val, 1.0f ) );
}

kernel void pmhstereo_lr_check( write_only image2d_t output, read_only image2d_t input1, read_only image2d_t input2, const float maxdiff, const float depthmax, int lr )
{
	int2 coord;
	const int width = get_image_width( input1 );
	const int height = get_image_height( input1 );

	coord.x = get_global_id( 0 );
	coord.y = get_global_id( 1 );

	if( coord.x >= width || coord.y >= height )
		return;

	float dorig = read_imagef( input1, SAMPLER_NN, coord ).x;
	float d = depthmax * dorig;

	float2 coord2 = ( float2 ) ( coord.x, coord.y );
	coord2.x += select( d, -d, lr );
	float d2 = depthmax * read_imagef( input2, SAMPLER_BILINEAR, coord2 ).x;
	float4 out = ( float4 ) select( 0.0f, dorig /* * ( depthmax * 4.0f / 256.0f )*/, fabs( d - d2) < maxdiff );
	out.w = 1.0f;
	write_imagef( output, coord, out );
}

__kernel void pmhstereo_weight( __write_only image2d_t out, __read_only image2d_t src  )
{
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
	local float buf[ 18 ][ 18 ];
	const float4 grayWeight =  ( float4 ) ( 0.2126f, 0.7152f, 0.0722f, 0.0f );
//	const float4 grayWeight =  ( float4 ) ( 0.333f, 0.333f, 0.333f, 0.0f );

//	float4 c = read_imagef( src, sampler, ( int2 ) ( gx, gy ) );

	for( int y = ly; y < lh + 2; y += lh ) {
		for( int x = lx; x < lw + 2; x += lw ) {
			buf[ y ][ x ] = dot( read_imagef( src, sampler, base + ( int2 )( x, y ) ), grayWeight );
		}
	}
	barrier( CLK_LOCAL_MEM_FENCE );

#define BUF( x, y ) buf[ y + 1 ][ x + 1 ]

	if( gx >= width || gy >= height )
		return;

	float dx = ( BUF( lx + 1 , ly ) - BUF( lx - 1 , ly ) );// * 0.5f + ( BUF( lx + 1, ly - 1 ) - BUF( lx - 1, ly - 1  ) ) * 0.25f + ( BUF( lx + 1, ly + 1 ) - BUF( lx - 1, ly + 1 ) ) * 0.25f;
	float dy = ( BUF( lx, ly - 1 ) - BUF( lx, ly - 1 ) );// * 0.5f + ( BUF( lx - 1, ly + 1 ) - BUF( lx - 1, ly - 1 ) ) * 0.25f + ( BUF( lx + 1, ly + 1 ) - BUF( lx + 1, ly - 1 ) ) * 0.25f ;
//	float dxx = 0.125f * ( - BUF( lx + 1, ly ) * 0.5f - BUF( lx, ly + 1 ) * 0.5f + BUF( lx, ly ) );
//	float dyy = 0.125f * ( - BUF( lx - 1, ly ) * 0.5f - BUF( lx, ly - 1 ) * 0.5f +  BUF( lx, ly ) );
//	float dxy = BUF( lx + 1, ly + 1 ) - BUF( lx - 1, ly - 1 );
//	float dyx = BUF( lx - 1, ly + 1 ) - BUF( lx + 1, ly - 1 );

//	float lap = - BUF( lx, ly + 1 ) * 0.5f
//				- BUF( lx, ly - 1 ) * 0.5f
//				- BUF( lx + 1, ly ) * 0.25f
//				- BUF( lx - 1, ly ) * 0.25f
//				+  BUF( lx, ly );

	float w = exp(-3.0f * pow( sqrt(  dx * dx + dy * dy ), 0.8f ) ) + 0.0001f;
	write_imagef( out,( int2 )( gx, gy ), ( float4 ) ( w ) );
}

__kernel void pmhstereo_gradxy( __write_only image2d_t out, __read_only image2d_t src  )
{
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
//	const int bstride = lw + 2;
	float dx, dy;
	local float buf[ 18 ][ 18 ];
	const float4 grayWeight =  ( float4 ) ( 0.2126f, 0.7152f, 0.0722f, 0.0f );
//	const float4 grayWeight =  ( float4 ) ( 0.333f, 0.333f, 0.333f, 0.0f );

//	float4 c = read_imagef( src, sampler, ( int2 ) ( gx, gy ) );

	for( int y = ly; y < lh + 2; y += lh ) {
		for( int x = lx; x < lw + 2; x += lw ) {
			buf[ y ][ x ] = dot( read_imagef( src, sampler, base + ( int2 )( x, y ) ), grayWeight );
		}
	}
	barrier( CLK_LOCAL_MEM_FENCE );

#define BUF( x, y ) buf[ y + 1 ][ x + 1 ]

	if( gx >= width || gy >= height )
		return;

	dx = ( BUF( lx + 1, ly ) - BUF( lx - 1 , ly ) ) * 0.5f + ( BUF( lx + 1, ly - 1 ) - BUF( lx - 1, ly - 1  ) ) * 0.25f + ( BUF( lx + 1, ly + 1 ) - BUF( lx - 1, ly + 1 ) ) * 0.25f;
	dy = ( BUF( lx, ly + 1 ) - BUF( lx, ly - 1 ) ) * 0.5f + ( BUF( lx - 1, ly + 1 ) - BUF( lx - 1, ly - 1 ) ) * 0.25f + ( BUF( lx + 1, ly + 1 ) - BUF( lx + 1, ly - 1 ) ) * 0.25f ;
//	float dxx = 0.125f * ( - BUF( lx + 1, ly ) * 0.5f - BUF( lx, ly + 1 ) * 0.5f + BUF( lx, ly ) );
//	float dyy = 0.125f * ( - BUF( lx - 1, ly ) * 0.5f - BUF( lx, ly - 1 ) * 0.5f +  BUF( lx, ly ) );
	float dxy = BUF( lx + 1, ly + 1 ) - BUF( lx - 1, ly - 1 );
	float dyx = BUF( lx - 1, ly + 1 ) - BUF( lx + 1, ly - 1 );

//	float lap = - BUF( lx, ly + 1 ) * 0.5f
//				- BUF( lx, ly - 1 ) * 0.5f
//				- BUF( lx + 1, ly ) * 0.25f
//				- BUF( lx - 1, ly ) * 0.25f
//				+  BUF( lx, ly );

	write_imagef( out,( int2 )( gx, gy ), ( float4 ) ( dx, dy, dxy, dyx ) );
}
