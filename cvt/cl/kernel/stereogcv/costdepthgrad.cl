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

#define COSTTHRESHOLD 0.028f //0.028f
#define COSTTHRESHOLDGRAD 0.008f //0.008f
#define ALPHA 0.10f

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

__kernel void stereogcv_costdepthgrad( __write_only image2d_t costout, __read_only image2d_t img0, __read_only image2d_t img1,
									   __read_only image2d_t grad0, __read_only image2d_t grad1, float depth )
{
	const float4 dotmul = ( float4 ) ( 0.3333f, 0.3333f, 0.3333f, 0.0f );
	const float4 dotmulGRAY = ( float4 ) (0.2126f, 0.7152f, 0.0722f, 0.0f );
	const float4 dotmulHSL = ( float4 ) ( 0.4f, 0.4f, 0.2f, 0.0f );
	const sampler_t samplerlin = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_LINEAR;
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;
	int2 coord;
	float2 coord2;
	float4 I0, I1, G0, G1, vec, Cout;
	const int width = get_image_width( img0 );
	const int height = get_image_height( img1 );

	coord.x = get_global_id( 0 );
	coord.y = get_global_id( 1 );

	if( coord.x >= width || coord.y >= height )
		return;

	coord2 = ( float2 ) ( coord.x + 0.5f - depth, coord.y + 0.5f );

	if( coord2.x < 0 || coord2.x >= width || coord2.y < 0 || coord2.y >= height ) {
		Cout = ( float4 ) ( COSTTHRESHOLDGRAD + COSTTHRESHOLD );
		//I0 = read_imagef( img0, sampler, coord  );
		//G0 = read_imagef( grad0, sampler, coord );
		//I1 = ( float4 ) 0.5f;//read_imagef( img1, sampler, coord2 );
		//G1 = ( float4 ) 0.5f;//read_imagef( grad1, sampler, coord2 );
		//Cout = ( float4 ) mix( fmin( dot( fabs( I1 - I0 ), dotmul ), COSTTHRESHOLD ) , fmin( dot( fabs( G1 - G0 ), dotmul ), COSTTHRESHOLDGRAD ), ALPHA );
	} else {
//#define BLOCK
#ifdef BLOCK
		float c = 0.0f, wsum = 0.0f;
#define DSIZE 5
//		float delta[ DSIZE ] = { -4.0f, -3.0f, -2.0f, -1.0f, 0.0f, 1.0f, 2.0f, 3.0f, 4.0f };
//		float w[ DSIZE ] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
//		float w[ DSIZE ] = { 0.125f, 0.25f, 0.25f, 0.5f, 0.25f, 0.25f, 0.125f };
		float delta[ DSIZE ] = { -2.0f, -1.0f, 0.0f, 1.0f, 2.0f };
		float w[ DSIZE ] = { 0.125f, 0.25f, 0.75f, 0.25f, 0.125f };
//		float delta[ DSIZE ] = { -0.5f, 0.0f, 0.5f };
//		float w[ DSIZE ] = { 0.25f, 0.5f, 0.25f };

		float Ci = 0, Cg = 0;
		for( int iy = 0; iy < DSIZE; iy++ ) {
			for( int ix = 0; ix < DSIZE; ix++ ) {
				I0 = ( read_imagef( img0, sampler, coord + ( float2 ) ( delta[ ix ], delta[ iy ] ) ) );
				G0 = ( read_imagef( grad0, sampler, coord  + ( float2 ) ( delta[ ix ], delta[ iy ] )) );
				I1 = ( read_imagef( img1, sampler, coord2  + ( float2 ) ( delta[ ix ], delta[ iy ] )) );
				G1 = ( read_imagef( grad1, sampler, coord2  + ( float2 ) ( delta[ ix ], delta[ iy ] )) );
				c +=  w[ ix ] * w[ iy ] * mix( fmin( dot( fabs( I1 - I0 ), dotmul ), COSTTHRESHOLD ) , fmin( dot( fabs( G1 - G0 ), dotmul ), COSTTHRESHOLDGRAD ), ALPHA );

//				Ci +=  w[ ix ] * w[ iy ] * dot( fabs( I1 - I0 ), dotmul );
//			    Cg +=  w[ ix ] * w[ iy ] * dot( fabs( G1 - G0 ), dotmul );
//				c += w[ ix ] * w[ iy ] * ( mix( fmin( fast_length( I1 - I0 ), COSTTHRESHOLD ) , fmin( fast_length( G1 - G0 ), COSTTHRESHOLDGRAD ), ALPHA ) );
				wsum += w[ ix ] * w[ iy ];
			}
		}
		//Ci /= wsum;
		//Cg /= wsum;
		//Ci = fmin( Ci, COSTTHRESHOLD );
		//Cg = fmin( Cg, COSTTHRESHOLDGRAD );
		//Cout = ( float4 ) mix( Ci, Cg, ALPHA );
		Cout = ( float4 ) ( c / wsum );
#else
		I0 = ( read_imagef( img0, sampler, coord  ) );
		G0 = ( read_imagef( grad0, sampler, coord ) );
		I1 = ( read_imagef( img1, samplerlin, coord2 ) );
		G1 = ( read_imagef( grad1, samplerlin, coord2 ) );
		Cout = ( float4 ) mix( fmin( dot( fabs( I1 - I0 ), dotmul ), COSTTHRESHOLD ) , fmin( dot( fabs( G1 - G0 ), dotmul ), COSTTHRESHOLDGRAD ), ALPHA );
//		Cout = ( float4 ) mix( fmin( fast_length( I1 - I0 ), COSTTHRESHOLD ) , fmin( fast_length( G1 - G0 ), COSTTHRESHOLDGRAD ), ALPHA );
#endif
	}

	write_imagef( costout, coord, Cout );
}
