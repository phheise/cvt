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

struct Mat3f {
	float3 m[ 3 ];
};

typedef struct Mat3f Mat3f;

inline float3 mat3f_transform( const Mat3f* mat, const float3 vec )
{
	return ( float3 ) ( dot( mat->m[ 0 ].xyz, vec ),
					   dot( mat->m[ 1 ].xyz, vec ),
					   dot( mat->m[ 2 ].xyz, vec ) );
}

inline float3 mat3f_transpose_transform( const Mat3f* mat, const float3 vec )
{
	return ( float3 ) ( dot( ( float3 ) ( mat->m[ 0 ].x, mat->m[ 1 ].x, mat->m[ 2 ].x ), vec ),
						dot( ( float3 ) ( mat->m[ 0 ].y, mat->m[ 1 ].y, mat->m[ 2 ].y ), vec ),
						dot( ( float3 ) ( mat->m[ 0 ].z, mat->m[ 1 ].z, mat->m[ 2 ].z ), vec ) );
}



inline float2 mat3f_transform_proj2( const Mat3f* mat, const float2 vec )
{
	float3 hvec = ( float3 ) ( vec.x, vec.y, 1.0f );
	float3 tmp = ( float3 ) ( dot( mat->m[ 0 ].xyz, hvec ),
						      dot( mat->m[ 1 ].xyz, hvec ),
				              dot( mat->m[ 2 ].xyz, hvec ) );
	return ( float2 ) ( tmp.x / tmp.z, tmp.y / tmp.z );
}

inline void mat3f_outer( Mat3f* dst, const float3 a, const float3 b )
{
	dst->m[ 0 ].xyz = a.x * b;
	dst->m[ 1 ].xyz = a.y * b;
	dst->m[ 2 ].xyz = a.z * b;
}

inline void mat3f_mul( Mat3f* dst, const Mat3f* a, const Mat3f* b )
{
	dst->m[ 0 ].x = dot( a->m[ 0 ].xyz, ( float3 ) ( b->m[ 0 ].x, b->m[ 1 ].x, b->m[ 2 ].x ) );
	dst->m[ 0 ].y = dot( a->m[ 0 ].xyz, ( float3 ) ( b->m[ 0 ].y, b->m[ 1 ].y, b->m[ 2 ].y ) );
	dst->m[ 0 ].z = dot( a->m[ 0 ].xyz, ( float3 ) ( b->m[ 0 ].z, b->m[ 1 ].z, b->m[ 2 ].z ) );

	dst->m[ 1 ].x = dot( a->m[ 1 ].xyz, ( float3 ) ( b->m[ 0 ].x, b->m[ 1 ].x, b->m[ 2 ].x ) );
	dst->m[ 1 ].y = dot( a->m[ 1 ].xyz, ( float3 ) ( b->m[ 0 ].y, b->m[ 1 ].y, b->m[ 2 ].y ) );
	dst->m[ 1 ].z = dot( a->m[ 1 ].xyz, ( float3 ) ( b->m[ 0 ].z, b->m[ 1 ].z, b->m[ 2 ].z ) );

	dst->m[ 2 ].x = dot( a->m[ 2 ].xyz, ( float3 ) ( b->m[ 0 ].x, b->m[ 1 ].x, b->m[ 2 ].x ) );
	dst->m[ 2 ].y = dot( a->m[ 2 ].xyz, ( float3 ) ( b->m[ 0 ].y, b->m[ 1 ].y, b->m[ 2 ].y ) );
	dst->m[ 2 ].z = dot( a->m[ 2 ].xyz, ( float3 ) ( b->m[ 0 ].z, b->m[ 1 ].z, b->m[ 2 ].z ) );
}


inline void mat3f_mul_transpose( Mat3f* dst, const Mat3f* a, const Mat3f* b )
{
	dst->m[ 0 ].x = dot( a->m[ 0 ].xyz, b->m[ 0 ].xyz );
	dst->m[ 0 ].y = dot( a->m[ 0 ].xyz, b->m[ 1 ].xyz );
	dst->m[ 0 ].z = dot( a->m[ 0 ].xyz, b->m[ 2 ].xyz );

	dst->m[ 1 ].x = dot( a->m[ 1 ].xyz, b->m[ 0 ].xyz );
	dst->m[ 1 ].y = dot( a->m[ 1 ].xyz, b->m[ 1 ].xyz );
	dst->m[ 1 ].z = dot( a->m[ 1 ].xyz, b->m[ 2 ].xyz );

	dst->m[ 2 ].x = dot( a->m[ 2 ].xyz, b->m[ 0 ].xyz );
	dst->m[ 2 ].y = dot( a->m[ 2 ].xyz, b->m[ 1 ].xyz );
	dst->m[ 2 ].z = dot( a->m[ 2 ].xyz, b->m[ 2 ].xyz );
}

inline void mat3f_mul_scalar( Mat3f* dst, float s, const Mat3f* m )
{
	dst->m[ 0 ] = s * m->m[ 0 ];
	dst->m[ 1 ] = s * m->m[ 1 ];
	dst->m[ 2 ] = s * m->m[ 2 ];
}

inline void mat3f_add( Mat3f* dst, const Mat3f* a, const Mat3f* b )
{
	dst->m[ 0 ] = a->m[ 0 ] + b->m[ 0 ];
	dst->m[ 1 ] = a->m[ 1 ] + b->m[ 1 ];
	dst->m[ 2 ] = a->m[ 2 ] + b->m[ 2 ];
}

inline void mat3f_sub( Mat3f* dst, const Mat3f* a, const Mat3f* b )
{
	dst->m[ 0 ] = a->m[ 0 ] - b->m[ 0 ];
	dst->m[ 1 ] = a->m[ 1 ] - b->m[ 1 ];
	dst->m[ 2 ] = a->m[ 2 ] - b->m[ 2 ];
}

inline void mat3f_transpose( Mat3f* dst, const Mat3f* mat )
{
  dst->m[ 0 ] = ( float3 ) ( mat->m[ 0 ].x, mat->m[ 1 ].x, mat->m[ 2 ].x );
  dst->m[ 1 ] = ( float3 ) ( mat->m[ 0 ].y, mat->m[ 1 ].y, mat->m[ 2 ].y );
  dst->m[ 2 ] = ( float3 ) ( mat->m[ 0 ].z, mat->m[ 1 ].z, mat->m[ 2 ].z );
}

inline void mat3f_axisangle_rotation( Mat3f* mat, float3 rot )
{
        float rad = length( rot );
		float3 axis = normalize( rot );
        float x, y, z, c, s;
        float wx, wy, wz;
        float xx, yy, yz;
        float xy, xz, zz;
        float x2, y2, z2;

        c = cos( rad * 0.5f );
        s = sin( rad * 0.5f );

        x = axis.x * s;
        y = axis.y * s;
        z = axis.z * s;

        x2 = x + x;
        y2 = y + y;
        z2 = z + z;

        xx = x * x2;
        xy = x * y2;
        xz = x * z2;

        yy = y * y2;
        yz = y * z2;
        zz = z * z2;

        wx = c * x2;
        wy = c * y2;
        wz = c * z2;

        mat->m[ 0 ].x = 1.0f - ( yy + zz );
        mat->m[ 0 ].y = xy - wz;
        mat->m[ 0 ].z = xz + wy;

        mat->m[ 1 ].x = xy + wz;
        mat->m[ 1 ].y = 1.0f - ( xx + zz );
        mat->m[ 1 ].z = yz - wx;

        mat->m[ 2 ].x = xz - wy;
        mat->m[ 2 ].y = yz + wx;
        mat->m[ 2 ].z = 1.0f - ( xx + yy );
}

inline void mat3f_rotation( Mat3f* mat, float3 rot )
{
	float3 c;
	float3 s = sincos( rot, &c );

	mat->m[ 0 ].x =  c.y * c.z;
	mat->m[ 0 ].y = -c.y * s.z;
	mat->m[ 0 ].z =        s.y;

	mat->m[ 1 ].x = c.x * s.z + c.z * s.x * s.y;
	mat->m[ 1 ].y = c.x * c.z - s.x * s.y * s.z;
	mat->m[ 1 ].z =           -		  c.y * s.x;

	mat->m[ 2 ].x =		  s.x * s.z - c.x * c.z * s.y;
	mat->m[ 2 ].y = c.x * s.y * s.z +		c.z * s.x;
	mat->m[ 2 ].z =							c.x * c.y;
}

inline void mat3f_rotation_transpose( Mat3f* mat, float3 rot )
{
	float3 c;
	float3 s = sincos( rot, &c );

	mat->m[ 0 ].x =  c.y * c.z;
	mat->m[ 1 ].x = -c.y * s.z;
	mat->m[ 2 ].x =        s.y;

	mat->m[ 0 ].y = c.x * s.z + c.z * s.x * s.y;
	mat->m[ 1 ].y = c.x * c.z - s.x * s.y * s.z;
	mat->m[ 2 ].y =           -		  c.y * s.x;

	mat->m[ 0 ].z =       s.x * s.z - c.x * c.z * s.y;
	mat->m[ 1 ].z = c.x * s.y * s.z +       c.z * s.x;
	mat->m[ 2 ].z =                         c.x * c.y;
}

void mat3f_inverse( Mat3f* dst, Mat3f* src )
{
	float3 tmp, tmp2;
	float3 D;
	float det;

	tmp.x = src->m[ 1 ].y * src->m[ 2 ].z - src->m[ 2 ].y * src->m[ 1 ].z;
	tmp.y = src->m[ 2 ].x * src->m[ 1 ].z - src->m[ 1 ].x * src->m[ 2 ].z;
	tmp.z = src->m[ 1 ].x * src->m[ 2 ].y - src->m[ 2 ].x * src->m[ 1 ].y;

	det = dot( src->m[ 0 ], tmp );
	if( fabs( det ) < 1e-10f ) {
		dst->m[ 0 ] = ( float3 ) ( 0.0f );
		dst->m[ 1 ] = ( float3 ) ( 0.0f );
		dst->m[ 2 ] = ( float3 ) ( 0.0f );
	} else {
		D = ( float3 ) ( 1.0f / ( det ) );
		tmp = src->m[ 0 ];
		dst->m[ 0 ] = cross( src->m[ 1 ], src->m[ 2 ] ) * D;
		tmp2 = src->m[ 1 ];
		dst->m[ 1 ] = cross( src->m[ 2 ], tmp ) * D;
		dst->m[ 2 ] = cross( tmp, tmp2 ) * D;
	}
}

float3 mat3_lusolve( Mat3f* mat, float3 b )
{
	// LU decomposition
	mat->m[ 1 ].x /= mat->m[ 0 ].x;
	mat->m[ 2 ].x /= mat->m[ 0 ].x;

	mat->m[ 1 ].yz -= mat->m[ 1 ].x * mat->m[ 0 ].yz;
	mat->m[ 2 ].yz -= mat->m[ 2 ].x * mat->m[ 0 ].yz;

	mat->m[ 2 ].y /= mat->m[ 1 ].y;
	mat->m[ 2 ].z -= mat->m[ 2 ].y * mat->m[ 1 ].z;

	// forward substitution
	b.y -= mat->m[ 1 ].x * b.x;
	b.z -= dot( mat->m[ 2 ].xy, b.xy );

	// backward substiution
	float3 x;
	x.z = b.z / mat->m[ 2 ].z;
	x.y = ( b.y - mat->m[ 1 ].z * x.z ) / mat->m[ 1 ].y;
	x.x = ( b.x - dot( mat->m[ 0 ].yz, x.yz ) ) / mat->m[ 0 ].x;
	return x;
}


