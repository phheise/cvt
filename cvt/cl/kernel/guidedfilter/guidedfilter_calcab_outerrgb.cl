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

void inverse3x3( float4* mat )
{
	float4 tmp, tmp2;
	float4 D;
	float det;

	tmp.x = mat[ 1 ].y * mat[ 2 ].z - mat[ 2 ].y * mat[ 1 ].z;
	tmp.y = mat[ 2 ].x * mat[ 1 ].z - mat[ 1 ].x * mat[ 2 ].z;
	tmp.z = mat[ 1 ].x * mat[ 2 ].y - mat[ 2 ].x * mat[ 1 ].y;
	tmp.w = 0.0f;

	det = dot( mat[ 0 ], tmp );
	if( fabs( det ) < 1e-10f ) {
		mat[ 0 ] = ( float4 ) ( 0.0f, 0.0f, 0.0f, 0.0f );
		mat[ 1 ] = ( float4 ) ( 0.0f, 0.0f, 0.0f, 0.0f );
		mat[ 2 ] = ( float4 ) ( 0.0f, 0.0f, 0.0f, 0.0f );
	} else {
		D = ( float4 ) ( 1.0f / ( det ) );
		tmp = mat[ 0 ];
		mat[ 0 ] = cross( mat[ 1 ], mat[ 2 ] ) * D;
		tmp2 = mat[ 1 ];
		mat[ 1 ] = cross( mat[ 2 ], tmp ) * D;
		mat[ 2 ] = cross( tmp, tmp2 ) * D;
	}
}

float4 LUSolve( float4* mat, float4 b )
{
	// LU decomposition
	mat[ 1 ].x /= mat[ 0 ].x;
	mat[ 2 ].x /= mat[ 0 ].x;

	mat[ 1 ].yz -= mat[ 1 ].x * mat[ 0 ].yz;
	mat[ 2 ].yz -= mat[ 2 ].x * mat[ 0 ].yz;

	mat[ 2 ].y /= mat[ 1 ].y;
	mat[ 2 ].z -= mat[ 2 ].y * mat[ 1 ].z;

	// forward substitution
	b.y -= mat[ 1 ].x * b.x;
	b.z -= dot( mat[ 2 ].xy, b.xy );

	// backward substiution
	float4 x;
	x.z = b.z / mat[ 2 ].z;
	x.y = ( b.y - mat[ 1 ].z * x.z ) / mat[ 1 ].y;
	x.x = ( b.x - dot( mat[ 0 ].yz, x.yz ) ) / mat[ 0 ].x;
	x.w = 0.0f;
	return x;
}

__kernel void guidedfilter_calcab_outerrgb( __write_only image2d_t imga, __write_only image2d_t imgb,
										   __read_only image2d_t imgmeanG, __read_only image2d_t imgmeanS,
										   __read_only image2d_t imgmeanSG, __read_only image2d_t imgmean_RR_RG_RB, __read_only image2d_t imgmean_GG_GB_BB,
										   const float epsilon )
{
    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;
	const int width = get_image_width( imga );
	const int height = get_image_height( imga );
	int2 coord;
	float4 a, b, meanG, meanS, meanSG, meanRR_RG_RB, meanGG_GB_BB, cov;
	float4 var[ 3 ];

	coord.x = get_global_id( 0 );
	coord.y = get_global_id( 1 );

	if( coord.x >= width || coord.y >= height )
		return;

	meanG = read_imagef( imgmeanG, sampler, coord );
	meanS = read_imagef( imgmeanS, sampler, coord );
	meanSG = read_imagef( imgmeanSG, sampler, coord );
	meanRR_RG_RB = read_imagef( imgmean_RR_RG_RB, sampler, coord );
	meanGG_GB_BB = read_imagef( imgmean_GG_GB_BB, sampler, coord );

	cov = meanSG - meanG * meanS;
	cov.w = 0.0f;

	meanRR_RG_RB.xyz -= meanG.x * meanG.xyz;
	meanGG_GB_BB.xy -= meanG.y * meanG.yz;
	meanGG_GB_BB.z -= meanG.z * meanG.z;

	var[ 0 ] = ( float4 ) ( meanRR_RG_RB.xyz, 0.0f );
	var[ 1 ] = ( float4 ) ( meanRR_RG_RB.y, meanGG_GB_BB.xy, 0.0f );
	var[ 2 ] = ( float4 ) ( meanRR_RG_RB.z, meanGG_GB_BB.yz, 0.0f );

	var[ 0 ].x += 1.0f * epsilon;
	var[ 1 ].y += 1.0f * epsilon;
	var[ 2 ].z += 1.0f * epsilon;


	//inverse3x3( var );

	//a.x = dot( var[ 0 ], cov );
	//a.y = dot( var[ 1 ], cov );
	//a.z = dot( var[ 2 ], cov );
	//a.w = 0.0f;

	a = LUSolve( var, cov );
	b = meanS - ( float4 ) dot( meanG, a );

	write_imagef( imga, coord, a );
	write_imagef( imgb, coord, b );
}

