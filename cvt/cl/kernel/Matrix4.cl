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

struct Mat4f {
	float4 m[ 4 ];
};

typedef struct Mat4f Mat4f;

inline float4 mat4f_transform( const Mat4f* mat, const float4 vec )
{
	return ( float4 ) ( dot( mat->m[ 0 ], vec ),
					    dot( mat->m[ 1 ], vec ),
					    dot( mat->m[ 2 ], vec ),
					    dot( mat->m[ 3 ], vec ) );
}

inline float4 mat4f_transpose_transform( const Mat4f* mat, const float4 vec )
{
	return ( float4 ) ( dot( ( float4 ) ( mat->m[ 0 ].x, mat->m[ 1 ].x, mat->m[ 2 ].x, mat->m[ 3 ].x ), vec ),
						dot( ( float4 ) ( mat->m[ 0 ].y, mat->m[ 1 ].y, mat->m[ 2 ].y, mat->m[ 3 ].y ), vec ),
						dot( ( float4 ) ( mat->m[ 0 ].z, mat->m[ 1 ].z, mat->m[ 2 ].z, mat->m[ 3 ].z ), vec ),
						dot( ( float4 ) ( mat->m[ 0 ].w, mat->m[ 1 ].w, mat->m[ 2 ].w, mat->m[ 3 ].w ), vec ) );
}


inline void mat4f_outer( Mat4f* dst, const float4 a, const float4 b )
{
	dst->m[ 0 ] = a.x * b;
	dst->m[ 1 ] = a.y * b;
	dst->m[ 2 ] = a.z * b;
	dst->m[ 3 ] = a.w * b;
}

inline void mat4f_mul( Mat4f* dst, const Mat4f* a, const Mat4f* b )
{
	dst->m[ 0 ].x = dot( a->m[ 0 ], ( float4 ) ( b->m[ 0 ].x, b->m[ 1 ].x, b->m[ 2 ].x, b->m[ 3 ].x ) );
	dst->m[ 0 ].y = dot( a->m[ 0 ], ( float4 ) ( b->m[ 0 ].y, b->m[ 1 ].y, b->m[ 2 ].y, b->m[ 3 ].y ) );
	dst->m[ 0 ].z = dot( a->m[ 0 ], ( float4 ) ( b->m[ 0 ].z, b->m[ 1 ].z, b->m[ 2 ].z, b->m[ 3 ].z ) );
	dst->m[ 0 ].w = dot( a->m[ 0 ], ( float4 ) ( b->m[ 0 ].w, b->m[ 1 ].w, b->m[ 2 ].w, b->m[ 3 ].w ) );

	dst->m[ 1 ].x = dot( a->m[ 1 ], ( float4 ) ( b->m[ 0 ].x, b->m[ 1 ].x, b->m[ 2 ].x, b->m[ 3 ].x ) );
	dst->m[ 1 ].y = dot( a->m[ 1 ], ( float4 ) ( b->m[ 0 ].y, b->m[ 1 ].y, b->m[ 2 ].y, b->m[ 3 ].y ) );
	dst->m[ 1 ].z = dot( a->m[ 1 ], ( float4 ) ( b->m[ 0 ].z, b->m[ 1 ].z, b->m[ 2 ].z, b->m[ 3 ].z ) );
	dst->m[ 1 ].w = dot( a->m[ 1 ], ( float4 ) ( b->m[ 0 ].w, b->m[ 1 ].w, b->m[ 2 ].w, b->m[ 3 ].w ) );

	dst->m[ 2 ].x = dot( a->m[ 2 ], ( float4 ) ( b->m[ 0 ].x, b->m[ 1 ].x, b->m[ 2 ].x, b->m[ 3 ].x ) );
	dst->m[ 2 ].y = dot( a->m[ 2 ], ( float4 ) ( b->m[ 0 ].y, b->m[ 1 ].y, b->m[ 2 ].y, b->m[ 3 ].y ) );
	dst->m[ 2 ].z = dot( a->m[ 2 ], ( float4 ) ( b->m[ 0 ].z, b->m[ 1 ].z, b->m[ 2 ].z, b->m[ 3 ].z ) );
	dst->m[ 2 ].w = dot( a->m[ 2 ], ( float4 ) ( b->m[ 0 ].w, b->m[ 1 ].w, b->m[ 2 ].w, b->m[ 3 ].w ) );

	dst->m[ 3 ].x = dot( a->m[ 3 ], ( float4 ) ( b->m[ 0 ].x, b->m[ 1 ].x, b->m[ 2 ].x, b->m[ 3 ].x ) );
	dst->m[ 3 ].y = dot( a->m[ 3 ], ( float4 ) ( b->m[ 0 ].y, b->m[ 1 ].y, b->m[ 2 ].y, b->m[ 3 ].y ) );
	dst->m[ 3 ].z = dot( a->m[ 3 ], ( float4 ) ( b->m[ 0 ].z, b->m[ 1 ].z, b->m[ 2 ].z, b->m[ 3 ].z ) );
	dst->m[ 3 ].w = dot( a->m[ 3 ], ( float4 ) ( b->m[ 0 ].w, b->m[ 1 ].w, b->m[ 2 ].w, b->m[ 3 ].w ) );
}


inline void mat4f_mul_transpose( Mat4f* dst, const Mat4f* a, const Mat4f* b )
{
	dst->m[ 0 ].x = dot( a->m[ 0 ], b->m[ 0 ] );
	dst->m[ 0 ].y = dot( a->m[ 0 ], b->m[ 0 ] );
	dst->m[ 0 ].z = dot( a->m[ 0 ], b->m[ 0 ] );
	dst->m[ 0 ].w = dot( a->m[ 0 ], b->m[ 0 ] );

	dst->m[ 1 ].x = dot( a->m[ 1 ], b->m[ 1 ] );
	dst->m[ 1 ].y = dot( a->m[ 1 ], b->m[ 1 ] );
	dst->m[ 1 ].z = dot( a->m[ 1 ], b->m[ 1 ] );
	dst->m[ 1 ].w = dot( a->m[ 1 ], b->m[ 1 ] );

	dst->m[ 2 ].x = dot( a->m[ 2 ], b->m[ 2 ] );
	dst->m[ 2 ].y = dot( a->m[ 2 ], b->m[ 2 ] );
	dst->m[ 2 ].z = dot( a->m[ 2 ], b->m[ 2 ] );
	dst->m[ 2 ].w = dot( a->m[ 2 ], b->m[ 2 ] );

	dst->m[ 3 ].x = dot( a->m[ 3 ], b->m[ 3 ] );
	dst->m[ 3 ].y = dot( a->m[ 3 ], b->m[ 3 ] );
	dst->m[ 3 ].z = dot( a->m[ 3 ], b->m[ 3 ] );
	dst->m[ 3 ].w = dot( a->m[ 3 ], b->m[ 3 ] );
}

inline void mat4f_mul_scalar( Mat4f* dst, float s, const Mat4f* m )
{
	dst->m[ 0 ] = s * m->m[ 0 ];
	dst->m[ 1 ] = s * m->m[ 1 ];
	dst->m[ 2 ] = s * m->m[ 2 ];
	dst->m[ 3 ] = s * m->m[ 3 ];
}

inline void mat4f_add( Mat4f* dst, const Mat4f* a, const Mat4f* b )
{
	dst->m[ 0 ] = a->m[ 0 ] + b->m[ 0 ];
	dst->m[ 1 ] = a->m[ 1 ] + b->m[ 1 ];
	dst->m[ 2 ] = a->m[ 2 ] + b->m[ 2 ];
	dst->m[ 3 ] = a->m[ 3 ] + b->m[ 3 ];
}

inline void mat4f_sub( Mat4f* dst, const Mat4f* a, const Mat4f* b )
{
	dst->m[ 0 ] = a->m[ 0 ] - b->m[ 0 ];
	dst->m[ 1 ] = a->m[ 1 ] - b->m[ 1 ];
	dst->m[ 2 ] = a->m[ 2 ] - b->m[ 2 ];
	dst->m[ 3 ] = a->m[ 3 ] - b->m[ 3 ];
}

inline void mat4f_transpose( Mat4f* dst, const Mat4f* mat )
{
  dst->m[ 0 ] = ( float4 ) ( mat->m[ 0 ].x, mat->m[ 1 ].x, mat->m[ 2 ].x, mat->m[ 3 ].x );
  dst->m[ 1 ] = ( float4 ) ( mat->m[ 0 ].y, mat->m[ 1 ].y, mat->m[ 2 ].y, mat->m[ 3 ].y );
  dst->m[ 2 ] = ( float4 ) ( mat->m[ 0 ].z, mat->m[ 1 ].z, mat->m[ 2 ].z, mat->m[ 3 ].z );
  dst->m[ 3 ] = ( float4 ) ( mat->m[ 0 ].w, mat->m[ 1 ].w, mat->m[ 2 ].w, mat->m[ 3 ].w );
}

inline void mat4f_axisangle_rotation( Mat4f* mat, float3 rot )
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
		mat->m[ 0 ].w = 0.0f;

        mat->m[ 1 ].x = xy + wz;
        mat->m[ 1 ].y = 1.0f - ( xx + zz );
        mat->m[ 1 ].z = yz - wx;
		mat->m[ 1 ].w = 0.0f;

        mat->m[ 2 ].x = xz - wy;
        mat->m[ 2 ].y = yz + wx;
        mat->m[ 2 ].z = 1.0f - ( xx + yy );
		mat->m[ 2 ].w = 0.0f;

		mat->m[ 3 ] = ( float4 ) ( 0.0f, 0.0f, 0.0f, 1.0f );
}

inline void mat4f_rotation( Mat4f* mat, float3 rot )
{
	float3 c;
	float3 s = sincos( rot, &c );

	mat->m[ 0 ].x =  c.y * c.z;
	mat->m[ 0 ].y = -c.y * s.z;
	mat->m[ 0 ].z =        s.y;
	mat->m[ 0 ].w = 0.0f;

	mat->m[ 1 ].x = c.x * s.z + c.z * s.x * s.y;
	mat->m[ 1 ].y = c.x * c.z - s.x *  s.y * s.z;
	mat->m[ 1 ].z =               -c.y * s.x;
	mat->m[ 1 ].w = 0.0f;

	mat->m[ 2 ].x =  s.x * s.z - c.x * c.z *  s.y;
	mat->m[ 2 ].y =  c.x * s.y * s.z + c.z * s.x;
	mat->m[ 2 ].z =                    c.x * c.y;
	mat->m[ 2 ].w = 0.0f;

	mat->m[ 3 ] = ( float4 ) ( 0.0f, 0.0f, 0.0f, 1.0f );
}

inline void mat4f_rotation_transpose( Mat4f* mat, float3 rot )
{
	float3 c;
	float3 s = sincos( rot, &c );

	mat->m[ 0 ].x =  c.y * c.z;
	mat->m[ 1 ].x = -c.y * s.z;
	mat->m[ 2 ].x =        s.y;
	mat->m[ 3 ].x = 0.0f;

	mat->m[ 0 ].y = c.x * s.z + c.z * s.x *  s.y;
	mat->m[ 1 ].y = c.x * c.z - s.x *  s.y * s.z;
	mat->m[ 2 ].y =               -c.y * s.x;
	mat->m[ 3 ].y = 0.0f;

	mat->m[ 0 ].z =       s.x * s.z - c.x * c.z *  s.y;
	mat->m[ 1 ].z =  c.x * s.y * s.z +     c.z * s.x;
	mat->m[ 2 ].z =                         c.x * c.y;
	mat->m[ 3 ].z = 0.0f;

	mat->m[ 0 ].w = 0.0f;
	mat->m[ 1 ].w = 0.0f;
	mat->m[ 2 ].w = 0.0f;
	mat->m[ 3 ].w = 1.0f;
}
