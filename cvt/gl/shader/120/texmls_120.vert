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

#version 120

uniform mat4 MVP;
uniform sampler2D DPTex;
uniform float DPTexSize;

attribute vec3 in_Vertex;
attribute vec4 in_Color;
varying vec4 vtx_Color;
varying vec2 vtx_TexCoord;

vec4 data[ 5 ];

#define ALPHA 2.0

float sgn( in float x )
{
	if( x >= 0 ) return 1.0f;
	return -1.0f;
}

void jacobi( out float c, out float s, in float x, in float y, in float z )
{
	if( y == 0 ) {
		c = sgn( x );
		s = 0;
	} else {
		float t, u;
		float tau = ( z - x ) / ( 2.0f * y );
		t = abs( 1.0f / ( abs( tau ) + sqrt( 1.0f + ( tau * tau ) ) ) ) * sgn( tau );
		u = sqrt( 1.0f + t * t );
		c = 1.0f / u;
		s = c * t;
	}
}

void givens( out float c, out float s, in float a, in float b )
{
	float t, u;
	if( b == 0 ) {
		c = sgn( a );
		s = 0;
	} else if( a == 0) {
		c = 0;
		s = sgn( b );
	} else if( abs( b ) > abs( a ) ) {
		t = a / b;
		u = abs( sqrt( 1.0f + ( t * t ) )) * sgn( b );
		s = 1.0f / u;
		c = s * t;
	} else {
		t = b / a;
		u = abs( sqrt( 1.0f + ( t * t ) ) ) * sgn( a );
		c = 1.0f / u;
		s = c * t;
	}
}

float det( in mat2 mat )
{
	return mat[ 0 ][ 0 ] * mat[ 1 ][ 1 ] - mat[ 0 ][ 1 ] * mat[ 1 ][ 0 ];
}

mat2 inv( in mat2 mat )
{
	float D, invD, tmp;

	D = mat[ 0 ][ 0 ] * mat[ 1 ][ 1 ] - mat[ 1 ][ 0 ] * mat[ 0 ][ 1 ];

	invD = 1.0f / D;

	tmp = mat[ 0 ][ 0 ];
	mat[ 0 ][ 0 ] =   mat[ 1 ][ 1 ] * invD;
	mat[ 0 ][ 1 ] = - mat[ 0 ][ 1 ] * invD;
	mat[ 1 ][ 0 ] = - mat[ 1 ][ 0 ] * invD;
	mat[ 1 ][ 1 ] =   tmp * invD;

	return mat;
}

mat2 solverigid( in mat2 mat )
{
	float c, s;	
	mat2 u, vt, diag;
	givens( c, s, mat[ 0 ][ 0 ] + mat[ 1 ][ 1 ], mat[ 1 ][ 0 ] - mat[ 0 ][ 1 ] );
	vt = mat2( c, s, -s, c );
	diag = mat * vt;
	jacobi( c, s, diag[ 0 ][ 0 ], diag[ 0 ][ 1 ], diag[ 1 ][ 1 ] );
	u = mat2( c, s, -s, c );
	diag = mat2( c, -s, s, c ) * diag * u;
	vt = vt * u;
/*	if( diag[ 0 ][ 0 ] < 0.0) {
		diag[ 0 ][ 0 ] = -diag[ 0 ][ 0 ];
		vt[ 0 ] = -vt[ 0 ];
	}
	if( diag[ 1 ][ 1 ] < 0.0) {
		diag[ 1 ][ 1 ] = -diag[ 1 ][ 1 ];
		vt[ 1 ] = -vt[ 1 ];
	}*/
#if 0	
	/* make singular values positive */
		for( int i = 0; i < 2; i++ ) {
			if( diag[ i ][ i ] < 0 ) {
				diag[ i ][ i ] = - diag[ i ][ i ];
				for( int k = 0; k < 2; k++ )
					u[ k ][ i ] = -u[ k ][ i ];
			}
		}
#endif

	/*if( diag[ 0 ][ 0 ] < diag[ 1 ][ 1 ] ) {
		float tmp;
		for( int i = 0; i < 2; i++ ) {
			tmp = u[ i ][ 0 ];
			u[ i ][ 0 ] = u[ i ][ 1 ];
			u[ i ][ 1 ] = tmp;

			tmp = vt[ i ][ 0 ];
			vt[ i ][ 0 ] = vt[ i ][ 1 ];
			vt[ i ][ 1 ] = tmp;
		}
	}*/
	mat2 scale = mat2( 1.0 );
	if( det( mat ) < 0 )
		scale[ 1 ][ 1 ] = -1.0;
	return u * scale * transpose( vt );
}

void main()
{
	mat2 covar = mat2( 0 );
	mat2 T;
	vec2 cp = vec2( 0 );
    vec2 cq = vec2( 0 );
	vec4 dp;
	float w, wsum = 0;

	data[ 0 ] = vec4( 0, 0, 0, 0 );
	data[ 1 ] = vec4( 0, 1, 0, 1 );
	data[ 2 ] = vec4( 1, 0, 1, 0 );
	data[ 3 ] = vec4( 1, 1, 1, 1 );
	data[ 4 ] = vec4( 0.5, 0.5, 0.6, 0.6 );

	vtx_Color = in_Color;
	gl_Position = MVP * vec4( in_Vertex, 1.0 ); 

	for( int i = 0; i < DPTexSize; i++ ) {
		dp = texture2D( DPTex, vec2( ( float( i ) + 0.5 ) * 1.0 / DPTexSize, 0.5 ) );
		//dp = data[ i ];
		float dist2 = dot( dp.xy - in_Vertex.xy, dp.xy - in_Vertex.xy );
		dist2 = max( dist2, 1e-6 );
		w = pow( dist2, -1.5 );
		//w = 1.0 / ( 1.0 + pow( dist2 * 100.0, 1.0 ) );
		//w = exp( -dist2 * 15.0 );
		wsum = wsum + w;
		cp = cp + dp.xy * w;
		cq = cq + dp.zw * w;
		covar = covar + w * outerProduct( dp.xy, dp.zw );
	}

	covar = covar / wsum;
	cp = cp / wsum;
	cq = cq / wsum;
	covar = covar - outerProduct( cp, cq );

	T = solverigid( covar );
	vtx_TexCoord =  transpose( T ) * ( in_Vertex.xy - cp ) + cq;
}
