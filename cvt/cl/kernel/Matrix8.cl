/*
   The MIT License (MIT)

   Copyright (c) 2011 - 2013, Philipp Heise and Sebastian Klose
   Copyright (c) 2016, BMW Car IT GmbH, Philipp Heise (philipp.heise@bmw.de)

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
#ifndef CL_MAT8F_CL
#define CL_MAT8F_CL

struct Mat8f {
    float8 m[ 8 ];
};

typedef struct Mat8f Mat8f;

inline float dot8( float8 a, float8 b )
{
    return dot( a.lo, b.lo ) + dot( a.hi, b.hi );
}

inline void mat8f_identity( Mat8f* dst )
{
    dst->m[ 0 ] = ( float8 ) ( 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f );
    dst->m[ 1 ] = ( float8 ) ( 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f );
    dst->m[ 2 ] = ( float8 ) ( 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f );
    dst->m[ 3 ] = ( float8 ) ( 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f );
    dst->m[ 4 ] = ( float8 ) ( 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f );
    dst->m[ 5 ] = ( float8 ) ( 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f );
    dst->m[ 6 ] = ( float8 ) ( 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f );
    dst->m[ 7 ] = ( float8 ) ( 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f );
}

inline void mat8f_assign( Mat8f* dst, const Mat8f* src )
{
    dst->m[ 0 ] = src->m[ 0 ];
    dst->m[ 1 ] = src->m[ 1 ];
    dst->m[ 2 ] = src->m[ 2 ];
    dst->m[ 3 ] = src->m[ 3 ];
    dst->m[ 4 ] = src->m[ 4 ];
    dst->m[ 5 ] = src->m[ 5 ];
    dst->m[ 6 ] = src->m[ 6 ];
    dst->m[ 7 ] = src->m[ 7 ];
}

inline void mat8f_set_diag( Mat8f* dst, const float8 vec )
{
    dst->m[ 0 ].s0 = vec.s0;
    dst->m[ 1 ].s1 = vec.s1;
    dst->m[ 2 ].s2 = vec.s2;
    dst->m[ 3 ].s3 = vec.s3;
    dst->m[ 4 ].s4 = vec.s4;
    dst->m[ 5 ].s5 = vec.s5;
    dst->m[ 6 ].s6 = vec.s6;
    dst->m[ 7 ].s7 = vec.s7;
}

inline float8 mat8f_transform( const Mat8f* mat, const float8 vec )
{
    return ( float8 ) ( dot8( mat->m[ 0 ], vec ), dot8( mat->m[ 1 ], vec ), dot8( mat->m[ 2 ], vec ), dot8( mat->m[ 3 ], vec ),
                        dot8( mat->m[ 4 ], vec ), dot8( mat->m[ 5 ], vec ), dot8( mat->m[ 6 ], vec ), dot8( mat->m[ 7 ], vec ) );
}

inline float8 mat8f_transpose_transform( const Mat8f* mat, const float8 vec )
{
    return ( float8 ) ( dot8( ( float8 ) ( mat->m[ 0 ].s0, mat->m[ 1 ].s0, mat->m[ 2 ].s0, mat->m[ 3 ].s0,
                                          mat->m[ 4 ].s0, mat->m[ 5 ].s0, mat->m[ 6 ].s0, mat->m[ 7 ].s0 ), vec ),
                        dot8( ( float8 ) ( mat->m[ 0 ].s1, mat->m[ 1 ].s1, mat->m[ 2 ].s1, mat->m[ 3 ].s1,
                                          mat->m[ 4 ].s1, mat->m[ 5 ].s1, mat->m[ 6 ].s1, mat->m[ 7 ].s1 ), vec ),
                        dot8( ( float8 ) ( mat->m[ 0 ].s2, mat->m[ 1 ].s2, mat->m[ 2 ].s2, mat->m[ 3 ].s2,
                                          mat->m[ 4 ].s2, mat->m[ 5 ].s2, mat->m[ 6 ].s2, mat->m[ 7 ].s2 ), vec ),
                        dot8( ( float8 ) ( mat->m[ 0 ].s3, mat->m[ 1 ].s3, mat->m[ 2 ].s3, mat->m[ 3 ].s3,
                                          mat->m[ 4 ].s3, mat->m[ 5 ].s3, mat->m[ 6 ].s3, mat->m[ 7 ].s3 ), vec ),
                        dot8( ( float8 ) ( mat->m[ 0 ].s4, mat->m[ 1 ].s4, mat->m[ 2 ].s4, mat->m[ 3 ].s4,
                                          mat->m[ 4 ].s4, mat->m[ 5 ].s4, mat->m[ 6 ].s4, mat->m[ 7 ].s4 ), vec ),
                        dot8( ( float8 ) ( mat->m[ 0 ].s5, mat->m[ 1 ].s5, mat->m[ 2 ].s5, mat->m[ 3 ].s5,
                                          mat->m[ 4 ].s5, mat->m[ 5 ].s5, mat->m[ 6 ].s5, mat->m[ 7 ].s5 ), vec ),
                        dot8( ( float8 ) ( mat->m[ 0 ].s6, mat->m[ 1 ].s6, mat->m[ 2 ].s6, mat->m[ 3 ].s6,
                                          mat->m[ 4 ].s6, mat->m[ 5 ].s6, mat->m[ 6 ].s6, mat->m[ 7 ].s6 ), vec ),
                        dot8( ( float8 ) ( mat->m[ 0 ].s7, mat->m[ 1 ].s7, mat->m[ 2 ].s7, mat->m[ 3 ].s7,
                                          mat->m[ 4 ].s7, mat->m[ 5 ].s7, mat->m[ 6 ].s7, mat->m[ 7 ].s7 ), vec ) );
}

inline void mat8f_outer( Mat8f* dst, const float8 a, const float8 b )
{
    dst->m[ 0 ] = a.s0 * b;
    dst->m[ 1 ] = a.s1 * b;
    dst->m[ 2 ] = a.s2 * b;
    dst->m[ 3 ] = a.s3 * b;
    dst->m[ 4 ] = a.s4 * b;
    dst->m[ 5 ] = a.s5 * b;
    dst->m[ 6 ] = a.s6 * b;
    dst->m[ 7 ] = a.s7 * b;
}

inline void mat8f_mul( Mat8f* dst, const Mat8f* a, const Mat8f* b )
{
    for( int i = 0; i < 8; i++ ) {
        dst->m[ i ].s0 = dot8( a->m[ i ], ( float8 ) ( b->m[ 0 ].s0, b->m[ 1 ].s0, b->m[ 2 ].s0, b->m[ 3 ].s0, b->m[ 4 ].s0, b->m[ 5 ].s0, b->m[ 6 ].s0, b->m[ 7 ].s0 ) );
        dst->m[ i ].s1 = dot8( a->m[ i ], ( float8 ) ( b->m[ 0 ].s1, b->m[ 1 ].s1, b->m[ 2 ].s1, b->m[ 3 ].s1, b->m[ 4 ].s1, b->m[ 5 ].s1, b->m[ 6 ].s1, b->m[ 7 ].s1 ) );
        dst->m[ i ].s2 = dot8( a->m[ i ], ( float8 ) ( b->m[ 0 ].s2, b->m[ 1 ].s2, b->m[ 2 ].s2, b->m[ 3 ].s2, b->m[ 4 ].s2, b->m[ 5 ].s2, b->m[ 6 ].s2, b->m[ 7 ].s2 ) );
        dst->m[ i ].s3 = dot8( a->m[ i ], ( float8 ) ( b->m[ 0 ].s3, b->m[ 1 ].s3, b->m[ 2 ].s3, b->m[ 3 ].s3, b->m[ 4 ].s3, b->m[ 5 ].s3, b->m[ 6 ].s3, b->m[ 7 ].s3 ) );
        dst->m[ i ].s4 = dot8( a->m[ i ], ( float8 ) ( b->m[ 0 ].s4, b->m[ 1 ].s4, b->m[ 2 ].s4, b->m[ 3 ].s4, b->m[ 4 ].s4, b->m[ 5 ].s4, b->m[ 6 ].s4, b->m[ 7 ].s4 ) );
        dst->m[ i ].s5 = dot8( a->m[ i ], ( float8 ) ( b->m[ 0 ].s5, b->m[ 1 ].s5, b->m[ 2 ].s5, b->m[ 3 ].s5, b->m[ 4 ].s5, b->m[ 5 ].s5, b->m[ 6 ].s5, b->m[ 7 ].s5 ) );
        dst->m[ i ].s6 = dot8( a->m[ i ], ( float8 ) ( b->m[ 0 ].s6, b->m[ 1 ].s6, b->m[ 2 ].s6, b->m[ 3 ].s6, b->m[ 4 ].s6, b->m[ 5 ].s6, b->m[ 6 ].s6, b->m[ 7 ].s6 ) );
        dst->m[ i ].s7 = dot8( a->m[ i ], ( float8 ) ( b->m[ 0 ].s7, b->m[ 1 ].s7, b->m[ 2 ].s7, b->m[ 3 ].s7, b->m[ 4 ].s7, b->m[ 5 ].s7, b->m[ 6 ].s7, b->m[ 7 ].s7 ) );
    }
}

inline void mat8f_mul_scalar( Mat8f* dst, const Mat8f* m, const float s )
{
    dst->m[ 0 ] = s * m->m[ 0 ];
    dst->m[ 1 ] = s * m->m[ 1 ];
    dst->m[ 2 ] = s * m->m[ 2 ];
    dst->m[ 3 ] = s * m->m[ 3 ];
    dst->m[ 4 ] = s * m->m[ 4 ];
    dst->m[ 5 ] = s * m->m[ 5 ];
    dst->m[ 6 ] = s * m->m[ 6 ];
    dst->m[ 7 ] = s * m->m[ 7 ];
}

inline void mat8f_add( Mat8f* dst, const Mat8f* a, const Mat8f* b )
{
    dst->m[ 0 ] = a->m[ 0 ] + b->m[ 0 ];
    dst->m[ 1 ] = a->m[ 1 ] + b->m[ 1 ];
    dst->m[ 2 ] = a->m[ 2 ] + b->m[ 2 ];
    dst->m[ 3 ] = a->m[ 3 ] + b->m[ 3 ];
    dst->m[ 4 ] = a->m[ 4 ] + b->m[ 4 ];
    dst->m[ 5 ] = a->m[ 5 ] + b->m[ 5 ];
    dst->m[ 6 ] = a->m[ 6 ] + b->m[ 6 ];
    dst->m[ 7 ] = a->m[ 7 ] + b->m[ 7 ];
}

inline void mat8f_sub( Mat8f* dst, const Mat8f* a, const Mat8f* b )
{
    dst->m[ 0 ] = a->m[ 0 ] - b->m[ 0 ];
    dst->m[ 1 ] = a->m[ 1 ] - b->m[ 1 ];
    dst->m[ 2 ] = a->m[ 2 ] - b->m[ 2 ];
    dst->m[ 3 ] = a->m[ 3 ] - b->m[ 3 ];
    dst->m[ 4 ] = a->m[ 4 ] - b->m[ 4 ];
    dst->m[ 5 ] = a->m[ 5 ] - b->m[ 5 ];
    dst->m[ 6 ] = a->m[ 6 ] - b->m[ 6 ];
    dst->m[ 7 ] = a->m[ 7 ] - b->m[ 7 ];
}

inline void mat8f_transpose( Mat8f* dst, const Mat8f* mat )
{
    dst->m[ 0 ] = ( float8 ) ( mat->m[ 0 ].s0, mat->m[ 1 ].s0, mat->m[ 2 ].s0, mat->m[ 3 ].s0, mat->m[ 4 ].s0, mat->m[ 5 ].s0, mat->m[ 6 ].s0, mat->m[ 7 ].s0 );
    dst->m[ 1 ] = ( float8 ) ( mat->m[ 0 ].s1, mat->m[ 1 ].s1, mat->m[ 2 ].s1, mat->m[ 3 ].s1, mat->m[ 4 ].s1, mat->m[ 5 ].s1, mat->m[ 6 ].s1, mat->m[ 7 ].s1 );
    dst->m[ 2 ] = ( float8 ) ( mat->m[ 0 ].s2, mat->m[ 1 ].s2, mat->m[ 2 ].s2, mat->m[ 3 ].s2, mat->m[ 4 ].s2, mat->m[ 5 ].s2, mat->m[ 6 ].s2, mat->m[ 7 ].s2 );
    dst->m[ 3 ] = ( float8 ) ( mat->m[ 0 ].s3, mat->m[ 1 ].s3, mat->m[ 2 ].s3, mat->m[ 3 ].s3, mat->m[ 4 ].s3, mat->m[ 5 ].s3, mat->m[ 6 ].s3, mat->m[ 7 ].s3 );
    dst->m[ 4 ] = ( float8 ) ( mat->m[ 0 ].s4, mat->m[ 1 ].s4, mat->m[ 2 ].s4, mat->m[ 3 ].s4, mat->m[ 4 ].s4, mat->m[ 5 ].s4, mat->m[ 6 ].s4, mat->m[ 7 ].s4 );
    dst->m[ 5 ] = ( float8 ) ( mat->m[ 0 ].s5, mat->m[ 1 ].s5, mat->m[ 2 ].s5, mat->m[ 3 ].s5, mat->m[ 4 ].s5, mat->m[ 5 ].s5, mat->m[ 6 ].s5, mat->m[ 7 ].s5 );
    dst->m[ 6 ] = ( float8 ) ( mat->m[ 0 ].s6, mat->m[ 1 ].s6, mat->m[ 2 ].s6, mat->m[ 3 ].s6, mat->m[ 4 ].s6, mat->m[ 5 ].s6, mat->m[ 6 ].s6, mat->m[ 7 ].s6 );
    dst->m[ 7 ] = ( float8 ) ( mat->m[ 0 ].s7, mat->m[ 1 ].s7, mat->m[ 2 ].s7, mat->m[ 3 ].s7, mat->m[ 4 ].s7, mat->m[ 5 ].s7, mat->m[ 6 ].s7, mat->m[ 7 ].s7 );

}

inline void mat8f_set_zero( Mat8f* dst )
{
    dst->m[ 0 ] = ( float8 ) 0.0f;
    dst->m[ 1 ] = ( float8 ) 0.0f;
    dst->m[ 2 ] = ( float8 ) 0.0f;
    dst->m[ 3 ] = ( float8 ) 0.0f;
    dst->m[ 4 ] = ( float8 ) 0.0f;
    dst->m[ 5 ] = ( float8 ) 0.0f;
    dst->m[ 6 ] = ( float8 ) 0.0f;
    dst->m[ 7 ] = ( float8 ) 0.0f;
}

inline float8 mat8f_lusolve( const Mat8f* _mat, const float8 b )
{
    Mat8f mat;
    float8 tmp, ret;
    mat8f_assign( &mat, _mat );

    // LU decomposition
    mat.m[ 1 ].s0 /= mat.m[ 0 ].s0;
    mat.m[ 2 ].s0 /= mat.m[ 0 ].s0;
    mat.m[ 3 ].s0 /= mat.m[ 0 ].s0;
    mat.m[ 4 ].s0 /= mat.m[ 0 ].s0;
    mat.m[ 5 ].s0 /= mat.m[ 0 ].s0;
    mat.m[ 6 ].s0 /= mat.m[ 0 ].s0;
    mat.m[ 7 ].s0 /= mat.m[ 0 ].s0;

    mat.m[ 1 ].s1234567 -= mat.m[ 1 ].s0 * mat.m[ 0 ].s1234567;
    mat.m[ 2 ].s1234567 -= mat.m[ 2 ].s0 * mat.m[ 0 ].s1234567;
    mat.m[ 3 ].s1234567 -= mat.m[ 3 ].s0 * mat.m[ 0 ].s1234567;
    mat.m[ 4 ].s1234567 -= mat.m[ 4 ].s0 * mat.m[ 0 ].s1234567;
    mat.m[ 5 ].s1234567 -= mat.m[ 5 ].s0 * mat.m[ 0 ].s1234567;
    mat.m[ 6 ].s1234567 -= mat.m[ 6 ].s0 * mat.m[ 0 ].s1234567;
    mat.m[ 7 ].s1234567 -= mat.m[ 7 ].s0 * mat.m[ 0 ].s1234567;

    /*--------------*/

    mat.m[ 2 ].s1 /= mat.m[ 1 ].s1;
    mat.m[ 3 ].s1 /= mat.m[ 1 ].s1;
    mat.m[ 4 ].s1 /= mat.m[ 1 ].s1;
    mat.m[ 5 ].s1 /= mat.m[ 1 ].s1;
    mat.m[ 6 ].s1 /= mat.m[ 1 ].s1;
    mat.m[ 7 ].s1 /= mat.m[ 1 ].s1;

    mat.m[ 2 ].s234567 -= mat.m[ 2 ].s1 * mat.m[ 1 ].s234567;
    mat.m[ 3 ].s234567 -= mat.m[ 3 ].s1 * mat.m[ 1 ].s234567;
    mat.m[ 4 ].s234567 -= mat.m[ 4 ].s1 * mat.m[ 1 ].s234567;
    mat.m[ 5 ].s234567 -= mat.m[ 5 ].s1 * mat.m[ 1 ].s234567;
    mat.m[ 6 ].s234567 -= mat.m[ 6 ].s1 * mat.m[ 1 ].s234567;
    mat.m[ 7 ].s234567 -= mat.m[ 7 ].s1 * mat.m[ 1 ].s234567;

    /*--------------*/

    mat.m[ 3 ].s2 /= mat.m[ 2 ].s2;
    mat.m[ 4 ].s2 /= mat.m[ 2 ].s2;
    mat.m[ 5 ].s2 /= mat.m[ 2 ].s2;
    mat.m[ 6 ].s2 /= mat.m[ 2 ].s2;
    mat.m[ 7 ].s2 /= mat.m[ 2 ].s2;

    mat.m[ 3 ].s34567 -= mat.m[ 3 ].s2 * mat.m[ 2 ].s34567;
    mat.m[ 4 ].s34567 -= mat.m[ 4 ].s2 * mat.m[ 2 ].s34567;
    mat.m[ 5 ].s34567 -= mat.m[ 5 ].s2 * mat.m[ 2 ].s34567;
    mat.m[ 6 ].s34567 -= mat.m[ 6 ].s2 * mat.m[ 2 ].s34567;
    mat.m[ 7 ].s34567 -= mat.m[ 7 ].s2 * mat.m[ 2 ].s34567;

    /*--------------*/

    mat.m[ 4 ].s3 /= mat.m[ 3 ].s3;
    mat.m[ 5 ].s3 /= mat.m[ 3 ].s3;
    mat.m[ 6 ].s3 /= mat.m[ 3 ].s3;
    mat.m[ 7 ].s3 /= mat.m[ 3 ].s3;

    mat.m[ 4 ].s4567 -= mat.m[ 4 ].s3 * mat.m[ 3 ].s4567;
    mat.m[ 5 ].s4567 -= mat.m[ 5 ].s3 * mat.m[ 3 ].s4567;
    mat.m[ 6 ].s4567 -= mat.m[ 6 ].s3 * mat.m[ 3 ].s4567;
    mat.m[ 7 ].s4567 -= mat.m[ 7 ].s3 * mat.m[ 3 ].s4567;

    /*--------------*/

    mat.m[ 5 ].s4 /= mat.m[ 4 ].s4;
    mat.m[ 6 ].s4 /= mat.m[ 4 ].s4;
    mat.m[ 7 ].s4 /= mat.m[ 4 ].s4;

    mat.m[ 5 ].s567 -= mat.m[ 5 ].s4 * mat.m[ 4 ].s567;
    mat.m[ 6 ].s567 -= mat.m[ 6 ].s4 * mat.m[ 4 ].s567;
    mat.m[ 7 ].s567 -= mat.m[ 7 ].s4 * mat.m[ 4 ].s567;

    /*--------------*/

    mat.m[ 6 ].s5 /= mat.m[ 5 ].s5;
    mat.m[ 7 ].s5 /= mat.m[ 5 ].s5;

    mat.m[ 6 ].s67 -= mat.m[ 6 ].s5 * mat.m[ 5 ].s67;
    mat.m[ 7 ].s67 -= mat.m[ 7 ].s5 * mat.m[ 5 ].s67;

    /*--------------*/

    mat.m[ 7 ].s6 /= mat.m[ 6 ].s6;
    mat.m[ 7 ].s7 -= mat.m[ 7 ].s6 * mat.m[ 6 ].s7;

    // forward substitution
    tmp.s0 = b.s0;
    tmp.s1 = b.s1 - dot( tmp.s0   , mat.m[ 1 ].s0 );
    tmp.s2 = b.s2 - dot( tmp.s01  , mat.m[ 2 ].s01 );
    tmp.s3 = b.s3 - dot( tmp.s012 , mat.m[ 3 ].s012 );
    tmp.s4 = b.s4 - dot( tmp.s0123, mat.m[ 4 ].s0123 );
    tmp.s5 = b.s5 - dot( tmp.s0123, mat.m[ 5 ].s0123 ) - tmp.s4 * mat.m[ 5 ].s4;
    tmp.s6 = b.s6 - dot( tmp.s0123, mat.m[ 6 ].s0123 ) - dot( tmp.s45, mat.m[ 6 ].s45 );
    tmp.s7 = b.s7 - dot( tmp.s0123, mat.m[ 7 ].s0123 ) - dot( tmp.s456, mat.m[ 7 ].s456 );

    // backward substiution
    ret.s7 =   tmp.s7 / mat.m[ 7 ].s7;
    ret.s6 = ( tmp.s6 - dot( mat.m[ 6 ].s7   , ret.s7 )    ) / mat.m[ 6 ].s6;
    ret.s5 = ( tmp.s5 - dot( mat.m[ 5 ].s67  , ret.s67 )   ) / mat.m[ 5 ].s5;
    ret.s4 = ( tmp.s4 - dot( mat.m[ 4 ].s567 , ret.s567 )  ) / mat.m[ 4 ].s4;
    ret.s3 = ( tmp.s3 - dot( mat.m[ 3 ].s4567, ret.s4567 ) ) / mat.m[ 3 ].s3;
    ret.s2 = ( tmp.s2 - dot( mat.m[ 2 ].s3456, ret.s3456 ) - mat.m[ 2 ].s7 * ret.s7  ) / mat.m[ 2 ].s2;
    ret.s1 = ( tmp.s1 - dot( mat.m[ 1 ].s2345, ret.s2345 ) - dot( mat.m[ 1 ].s67, ret.s67 )  ) / mat.m[ 1 ].s1;
    ret.s0 = ( tmp.s0 - dot( mat.m[ 0 ].s1234, ret.s1234 ) - dot( mat.m[ 0 ].s567, ret.s567 ) ) / mat.m[ 0 ].s0;

    return ret;
}

kernel void mat8f_test( global float8* output, global Mat8f* mat, global float8* b, int n )
{
    int id = get_global_id( 0 );

    if( id > n )
        return;

    Mat8f A = *( mat + id );
    *( output + id ) = mat8f_lusolve( &A , *( b + id ) );
}
#endif
