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
#import "Matrix3.cl"
#import "Matrix4.cl"
#import "Plane.cl"

#define PROPSIZE 1
#define DEPTHREFINEMUL 2.0f
#define NORMALREFINEMUL 0.2f
#define NORMALCOMPMAX 0.95f
#define NUMRNDTRIES	 1
#define NUMRNDSAMPLE 3

#define COLORWEIGHT 26.0f
#define COLORGRADALPHA 0.05f
#define COLORMAXDIFF 0.04f
#define GRADMAXDIFF 0.01f
#define OVERSAMPLE 1.0f
#define VIEWSAMPLES 4

// #define USELOCALBUF  1

const sampler_t SAMPLER_NN		 = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

inline float4 pmmv_particle_init( RNG* rng, const float2 coord, const int depthmax )
{
	float z = RNG_float( rng ) * depthmax;
	float4 n;
	n.x = ( RNG_float( rng ) - 0.5f ) * normmul * NORMALCOMPMAX;
	n.y = ( RNG_float( rng ) - 0.5f ) * normmul * NORMALCOMPMAX;
//	float nfactor = fmin( native_rsqrt( n.x * n.x + n.y * n.y ), 1.0f * NORMALCOMPMAX );
//	n.xy = nfactor * n.yx;

	return ( float4 ) ( n.x, n.y, z, 0.0f );

}

inline float4 pmmv_particle_refine( const float4 particle, RNG* rng, const float2 coord, const int depthmax )
{
	float4 ret;
	ret.z = self.z + ( RNG_float( rng ) - 0.5f ) * DEPTHREFINEMUL;
	ret.z = clamp( ret.z, 0.0f, depthmax );
	ret.x = self.x + ( RNG_float( rng ) - 0.5f ) * NORMALREFINEMUL;
	ret.y = self.y + ( RNG_float( rng ) - 0.5f ) * NORMALREFINEMUL;
	//FIXME: use NORMALCOMPMAX
	float nfactor = fmax( length( n.xy ) + 0.001f, 1.0f );
	n.xy = n.xy / nfactor;

	return ret;
}

inline Planef pmmv_particle_to_plane( const float4 particle )
{
	Planef ret;
	ret.x = particle.x;
	ret.y = particle.y;
	ret.z = native_sqrt( 1.0f - particle.x * particle.x - particle.y * particle.y );
	ret.w = particle.z;
	return ret;
}

inline float4 pmmv_particle_from_plane( const Planef plane )
{
	float4 ret;
	ret.x = plane.x;
	ret.y = plane.y;
	ret.z = plane.z;
	ret.w = 0.0f;
	return ret;
}


inline float pmmv3_patch_eval( read_only image2d_t colimg1, read_only image2d_t gradimg1,
							   read_only image2d_t colimg2, read_only image2d_t gradimg2,
							   read_only image2d_t colimg3, read_only image2d_t gradimg3,
							   const Mat4f* mat1to2, const Mat4f* mat1to3,
							   const float2 coord, const int patchsize )
{
	float wsum1 = 0;
	float ret1  = 0;
	int width	= get_image_width( colimg2 );
	int height	= get_image_height( colimg2 );

	float4 valcenter = read_imagef( colimg1, SAMPLER_BILINEAR, coord + ( float2 ) ( 0.5f, 0.5f) );

	for( float dy = -patchsize; dy <= patchsize; dy+=1.0f ) {
		for( float dx = -patchsize; dx <= patchsize; dx+=1.0f ) {
			float2 displace = ( float2 ) ( dx * OVERSAMPLE, dy * OVERSAMPLE );
			float2 pos		= coord + displace;

			if( pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height )
				continue;

			float4 val1 = read_imagef( colimg1, SAMPLER_BILINEAR, pos  + ( float2 ) ( 0.5f, 0.5f));
			float4 gval1 = read_imagef( gradimg1, SAMPLER_BILINEAR, pos  + ( float2 ) ( 0.5f, 0.5f));

			float w1 = native_exp( -dot( fabs( valcenter.xyz - val1.xyz ), ( float3 ) 1.0f ) * ( smoothstep( 0.0f, 28.0f, length( displace ) ) * 1.5f * COLORWEIGHT + 5.0f ) );// * exp( -fast_length( displace ) * 0.05f );

//			float w1 = exp( -dot( fabs( valcenter.xyz - val1.xyz ), ( float3 ) 1.0f ) * COLORWEIGHT );// * exp( -fast_length( displace ) * 0.05f );

			// transform point 1
			pos = Mat4_MulVec3Proj2( &mat1to2, pos );
			if( pos.x < 0 || pos.x >= width ) {
				wsum1 += w1;

				float4 val2  = read_imagef( colimg2, SAMPLER_BILINEAR, pos  + ( float2 ) ( 0.5f, 0.5f));
				float4 gval2 = read_imagef( gradimg2, SAMPLER_BILINEAR, pos  + ( float2 ) ( 0.5f, 0.5f));

				float C = COLORGRADALPHA * dot( fmin( fabs( ( val1 - val2 ).xyz ), COLORMAXDIFF ), ( float3 ) 1.0f ) +
					      ( 1.0f - COLORGRADALPHA ) * dot( fmin( fabs( ( gval1 - gval2 ) ), GRADMAXDIFF ), ( float4 ) 1.0f );

				ret1 += w1 * C;
			}

			// transform point 2
			pos = coord + displace;
			pos = Mat4_MulVec3Proj2( &mat1to3, pos );
			if( pos.x < 0 || pos.x >= width ) {
				wsum1 += w1;

				float4 val2  = read_imagef( colimg3, SAMPLER_BILINEAR, pos  + ( float2 ) ( 0.5f, 0.5f));
				float4 gval2 = read_imagef( gradimg3, SAMPLER_BILINEAR, pos  + ( float2 ) ( 0.5f, 0.5f));

				float C = COLORGRADALPHA * dot( fmin( fabs( ( val1 - val2 ).xyz ), COLORMAXDIFF ), ( float3 ) 1.0f ) +
					      ( 1.0f - COLORGRADALPHA ) * dot( fmin( fabs( ( gval1 - gval2 ) ), GRADMAXDIFF ), ( float4 ) 1.0f );

				ret1 += w1 * C;
			}
		}
	}

	if( wsum1 <= 1.1f )
		return 1e5f;
	return ret1 / wsum1;
}


kernel void pmmv3_init( write_only image2d_t output,
							 read_only image2d_t img1, read_only image2d_t gimg1,
							 read_only image2d_t img2, read_only image2d_t gimg2,
							 read_only image2d_t img3, read_only image2d_t gimg3,
							 const Mat3f KSrcInv, const Mat4f TSrc,
							 const Mat3f KDst1, const Mat4f TDst1,
							 const Mat3f KDst2, const Mat4f TDst2,
							 const int patchsize, const float depthmax )
{
	RNG rng;
	Mat3f mat1to2, mat1to3;
	const int2 coord	= ( int2 )	 ( get_global_id( 0 ), get_global_id( 1 ) );
	const float2 coordf = ( float2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
	const int width		= get_image_width( img1 );
	const int height	= get_image_height( img1 );

	if( coord.x >= width || coord.y >= height )
		return;

	RNG_init( &rng, coord.y * width + coord.x, 3 );

	float4 ret = pmmv_particle_init( &rng, coordf, lr, 1.0f, depthmax );

	planef_homography( &mat1to2, pmmv_particle_to_plane( ret ), KDst1, TDst1, KSrcInv, TSrc );
	planef_homography( &mat1to3, pmmv_particle_to_plane( ret ), KDst2, TDst2, KSrcInv, TSrc );

	ret.w	   = pmmv3_patch_eval( img1, gimg1,
								   img2, gimg2,
								   img3, gimg3,
							       &mat1to2m, &mat1to3,
							       coordf, patchsize );

	write_imagef( output, coord, ret );
}

kernel void pmmv3_iteration( write_only image2d_t output,
							 read_only  image2d_t input,
							 read_only image2d_t img1, read_only image2d_t gimg1,
							 read_only image2d_t img2, read_only image2d_t gimg2,
							 read_only image2d_t img3, read_only image2d_t gimg3,
							 const Mat3f KSrcInv, const Mat4f TSrc,
							 const Mat3f KDst1, const Mat4f TDst1,
							 const Mat3f KDst2, const Mat4f TDst2,
							 const int patchsize, const float depthmax )
{
	RNG rng;
	Mat3f mat1to2, mat1to3;
	const int width		= get_image_width( input );
	const int height	= get_image_height( input );
	const int gx		= get_global_id( 0 );
	const int gy		= get_global_id( 1 );
	const int lx		= get_local_id( 0 );
	const int ly		= get_local_id( 1 );
    const int lw		= get_local_size( 0 );
    const int lh		= get_local_size( 1 );
	const int2 base		= ( int2 ) ( get_group_id( 0 ) * lw - PROPSIZE, get_group_id( 1 ) * lh - PROPSIZE );
	const int2 coord	= ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
	const float2 coordf = ( float2 ) ( get_global_id( 0 ), get_global_id( 1 ) );

	local float4 buf[ 16 + 2 * PROPSIZE ][ 16 + 2 * PROPSIZE ];
	float4 self, neighbour;

	for( int y = ly; y < lh + 2 * PROPSIZE; y += lh ) {
		for( int x = lx; x < lw + 2 * PROPSIZE; x += lw ) {
			buf[ y ][ x ] = read_imagef( input, SAMPLER_NN, base + ( int2 )( x, y ) );
		}
	}

	barrier( CLK_LOCAL_MEM_FENCE );

	if(	gx >= width || gy >= height )
		return;

	self = buf[ ly + PROPSIZE ][ lx + PROPSIZE ];

	RNG_init( &rng, ( coord.y * width + coord.x ) + iter, ( ( ( 2 * PROPSIZE + 1 ) * ( 2 * PROPSIZE + 1 ) - 1 ) + NUMRNDTRIES ) * 3 + 2 * NUMRNDSAMPLE );

	// sample the particle state of the neighbours
	for( int py = -PROPSIZE; py <= PROPSIZE; py++ ) {
		for( int px = -PROPSIZE; px <= PROPSIZE; px++ ) {

			if( px == 0 && py == 0 )
				continue;

			neighbour	 = buf[ ly + PROPSIZE + py ][ lx + PROPSIZE + px ];
			planef_homography( &mat1to2, pmmv_particle_to_plane( neighbour ), KDst1, TDst1, KSrcInv, TSrc );
			planef_homography( &mat1to3, pmmv_particle_to_plane( neighbour ), KDst2, TDst2, KSrcInv, TSrc );
			neighbour.w  = pmmv3_patch_eval( img1, gimg1, img2, gimg2, img3, gimg3, &mat1to2, &mat1to3, coordf, patchsize );
			if( neighbour.w < self.w ) self = neighbour;
		}
	}

	// rand neighbourhood tries
	for( int i = 0; i < NUMRNDSAMPLE; i++ ) {
			neighbour = read_imagef( input, SAMPLER_NN, (int2) ( gx, gy ) + ( int2 )( RNG_float( &rng ) * 7.0f + 0.5f, RNG_float( &rng ) * 7.0f + 0.5f ) );
			planef_homography( &mat1to2, pmmv_particle_to_plane( neighbour ), KDst1, TDst1, KSrcInv, TSrc );
			planef_homography( &mat1to3, pmmv_particle_to_plane( neighbour ), KDst2, TDst2, KSrcInv, TSrc );
			neighbour.w  = pmmv3_patch_eval( img1, gimg1, img2, gimg2, img3, gimg3, &mat1to2, &mat1to3, coordf, patchsize );
			if( neighbour.w < self.w ) self = neighbour;
	}

	// random try
	neighbour	 = pmmv_particle_init( &rng, coordf, depthmax );
	planef_homography( &mat1to2, pmmv_particle_to_plane( neighbour ), KDst1, TDst1, KSrcInv, TSrc );
	planef_homography( &mat1to3, pmmv_particle_to_plane( neighbour ), KDst2, TDst2, KSrcInv, TSrc );
	neighbour.w  = pmmv3_patch_eval( img1, gimg1, img2, gimg2, img3, gimg3, &mat1to2, &mat1to3, coordf, patchsize );
	if( neighbour.w < self.w ) self = neighbour;

	// try other view
#if 0
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
		neighbour	 = pmmv_particle_refine( self, &rng, coordf, depthmax );
		planef_homography( &mat1to2, pmmv_particle_to_plane( neighbour ), KDst1, TDst1, KSrcInv, TSrc );
		planef_homography( &mat1to3, pmmv_particle_to_plane( neighbour ), KDst2, TDst2, KSrcInv, TSrc );
		neighbour.w  = pmmv3_patch_eval( img1, gimg1, img2, gimg2, img3, gimg3, &mat1to2, &mat1to3, coordf, patchsize );
		if( neighbour.w < self.w ) self = neighbour;
	}

	// store view prop result
	// maybe inconsistent r/w access - but random anyway
#if 0
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
