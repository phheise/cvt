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
#ifndef CL_MAT5F_CL
#define CL_MAT5F_CL

struct Mat5f {
    float m[ 5 ][ 5 ];
};

typedef struct Mat5f Mat5f;

inline void mat5f_assign( Mat5f* dst, const Mat5f* src )
{
    for( int y = 0; y < 5; y++ )
        for( int x = 0; x < 5; x++ )
            dst->m[ y ][ x ] = src->m[ y ][ x ];
}

inline float8 mat5f_transform( const Mat5f* mat, const float3 vec1, const float2 vec2 )
{
    return ( float8 ) ( mat->m[ 0 ][ 0 ] * vec1.x + mat->m[ 0 ][ 1 ] * vec1.y + mat->m[ 0 ][ 2 ] * vec1.z + mat->m[ 0 ][ 3 ] * vec2.x + mat->m[ 0 ][ 4 ] * vec2.y,
                        mat->m[ 1 ][ 0 ] * vec1.x + mat->m[ 1 ][ 1 ] * vec1.y + mat->m[ 1 ][ 2 ] * vec1.z + mat->m[ 1 ][ 3 ] * vec2.x + mat->m[ 1 ][ 4 ] * vec2.y,
                        mat->m[ 2 ][ 0 ] * vec1.x + mat->m[ 2 ][ 1 ] * vec1.y + mat->m[ 2 ][ 2 ] * vec1.z + mat->m[ 2 ][ 3 ] * vec2.x + mat->m[ 2 ][ 4 ] * vec2.y,
                        mat->m[ 3 ][ 0 ] * vec1.x + mat->m[ 3 ][ 1 ] * vec1.y + mat->m[ 3 ][ 2 ] * vec1.z + mat->m[ 3 ][ 3 ] * vec2.x + mat->m[ 3 ][ 4 ] * vec2.y,
                        mat->m[ 4 ][ 0 ] * vec1.x + mat->m[ 4 ][ 1 ] * vec1.y + mat->m[ 4 ][ 2 ] * vec1.z + mat->m[ 4 ][ 3 ] * vec2.x + mat->m[ 4 ][ 4 ] * vec2.y,
                        0.0f, 0.0f, 0.0f );
}

inline float8 mat5f_transpose_transform( const Mat5f* mat, const float3 vec1, const float2 vec2 )
{
    return ( float8 ) ( mat->m[ 0 ][ 0 ] * vec1.x + mat->m[ 1 ][ 0 ] * vec1.y + mat->m[ 2 ][ 0 ] * vec1.z + mat->m[ 3 ][ 0 ] * vec2.x + mat->m[ 4 ][ 0 ] * vec2.y,
                        mat->m[ 0 ][ 1 ] * vec1.x + mat->m[ 1 ][ 1 ] * vec1.y + mat->m[ 2 ][ 1 ] * vec1.z + mat->m[ 3 ][ 1 ] * vec2.x + mat->m[ 4 ][ 1 ] * vec2.y,
                        mat->m[ 0 ][ 2 ] * vec1.x + mat->m[ 1 ][ 2 ] * vec1.y + mat->m[ 2 ][ 2 ] * vec1.z + mat->m[ 3 ][ 2 ] * vec2.x + mat->m[ 4 ][ 2 ] * vec2.y,
                        mat->m[ 0 ][ 3 ] * vec1.x + mat->m[ 1 ][ 3 ] * vec1.y + mat->m[ 2 ][ 3 ] * vec1.z + mat->m[ 3 ][ 3 ] * vec2.x + mat->m[ 4 ][ 3 ] * vec2.y,
                        mat->m[ 0 ][ 4 ] * vec1.x + mat->m[ 1 ][ 4 ] * vec1.y + mat->m[ 2 ][ 4 ] * vec1.z + mat->m[ 3 ][ 4 ] * vec2.x + mat->m[ 4 ][ 4 ] * vec2.y,
                        0.0f, 0.0f, 0.0f );
}

inline void mat5f_outer( Mat5f* dst, const float3 a, const float2 a2, const float3 b, const float2 b2 )
{
    dst->m[ 0 ][ 0 ] = a.x * b.x;
    dst->m[ 0 ][ 1 ] = a.x * b.y;
    dst->m[ 0 ][ 2 ] = a.x * b.z;
    dst->m[ 0 ][ 3 ] = a.x * b2.x;
    dst->m[ 0 ][ 4 ] = a.x * b2.y;

    dst->m[ 1 ][ 0 ] = a.y * b.x;
    dst->m[ 1 ][ 1 ] = a.y * b.y;
    dst->m[ 1 ][ 2 ] = a.y * b.z;
    dst->m[ 1 ][ 3 ] = a.y * b2.x;
    dst->m[ 1 ][ 4 ] = a.y * b2.y;

    dst->m[ 2 ][ 0 ] = a.z * b.x;
    dst->m[ 2 ][ 1 ] = a.z * b.y;
    dst->m[ 2 ][ 2 ] = a.z * b.z;
    dst->m[ 2 ][ 3 ] = a.z * b2.x;
    dst->m[ 2 ][ 4 ] = a.z * b2.y;

    dst->m[ 3 ][ 0 ] = a2.x * b.x;
    dst->m[ 3 ][ 1 ] = a2.x * b.y;
    dst->m[ 3 ][ 2 ] = a2.x * b.z;
    dst->m[ 3 ][ 3 ] = a2.x * b2.x;
    dst->m[ 3 ][ 4 ] = a2.x * b2.y;

    dst->m[ 4 ][ 0 ] = a2.y * b.x;
    dst->m[ 4 ][ 1 ] = a2.y * b.y;
    dst->m[ 4 ][ 2 ] = a2.y * b.z;
    dst->m[ 4 ][ 3 ] = a2.y * b2.x;
    dst->m[ 4 ][ 4 ] = a2.y * b2.y;
}

inline void mat5f_mul( Mat5f* dst, const Mat5f* a, const Mat5f* b )
{
    for( int y = 0; y < 5; y++ ) {
        for( int x = 0; x < 5; x++ ) {
            float sum = 0.0f;
            for( int w = 0; w < 5; w++ )
                sum += a->m[ y ][ w ] * b->m[ w ][ x ];
            dst->m[ y ][ x ] = sum;
        }
    }
}

inline void mat5f_mul_transpose( Mat5f* dst, const Mat5f* a, const Mat5f* b )
{
    for( int y = 0; y < 5; y++ ) {
        for( int x = 0; x < 5; x++ ) {
            float sum = 0.0f;
            for( int w = 0; w < 5; w++ )
                sum += a->m[ y ][ w ] * b->m[ x ][ w ];
            dst->m[ y ][ x ] = sum;
        }
    }
}

inline void mat5f_mul_scalar( Mat5f* dst, float s, const Mat5f* m )
{
    for( int y = 0; y < 5; y++ )
        for( int x = 0; x < 5; x++ )
            dst->m[ y ][ x ] = s * m->m[ y ][ x ];
}

inline void mat5f_add( Mat5f* dst, const Mat5f* a, const Mat5f* b )
{
    for( int y = 0; y < 5; y++ )
        for( int x = 0; x < 5; x++ )
            dst->m[ y ][ x ] = a->m[ y ][ x ] + b->m[ y ][ x ];
}

inline void mat5f_sub( Mat5f* dst, const Mat5f* a, const Mat5f* b )
{
    for( int y = 0; y < 5; y++ )
        for( int x = 0; x < 5; x++ )
            dst->m[ y ][ x ] = a->m[ y ][ x ] - b->m[ y ][ x ];
}

inline void mat5f_transpose( Mat5f* dst, const Mat5f* mat )
{
    for( int y = 0; y < 5; y++ )
        for( int x = 0; x < 5; x++ )
            dst->m[ y ][ x ] = mat->m[ x ][ y ];
}

inline void mat5f_set_zero( Mat5f* dst )
{
    for( int y = 0; y < 5; y++ )
        for( int x = 0; x < 5; x++ )
            dst->m[ y ][ x ] = 0.0f;
}

float8 mat5f_lusolve( const Mat5f* mat, float3 b, float2 b2 )
{
    Mat5f lu;
    mat5f_assign( &lu, mat );

    // LU decomposition
    for( int k = 0; k < 5; k++ ) {
        for( int i = k + 1; i < 5; i++ ) {
            lu.m[ i ][ k ] /= lu.m[ k ][ k ];
            for( int l = k + 1; l < 5; l++ ) {
                lu.m[ i ][ l ] -= lu.m[ i ][ k ] * lu.m[ k ][ l ];
            }
        }
    }

    // forward substitution
    b.y  -= lu.m[ 1 ][ 0 ] * b.x;
    b.z  -= lu.m[ 2 ][ 1 ] * b.y  + lu.m[ 2 ][ 0 ] * b.x;
    b2.x -= lu.m[ 3 ][ 2 ] * b.z  + lu.m[ 3 ][ 1 ] * b.y + lu.m[ 3 ][ 0 ] * b.x;
    b2.y -= lu.m[ 4 ][ 3 ] * b2.x + lu.m[ 4 ][ 2 ] * b.z + lu.m[ 4 ][ 1 ] * b.y + lu.m[ 4 ][ 0 ] * b.x;

    // backward substiution
    float8 x = ( float8 ) 0.0f;
    x.s4 =   b2.y / lu.m[ 4 ][ 4 ];
    x.s3 = ( b2.x - lu.m[ 3 ][ 4 ] * x.s4 ) / lu.m[ 3 ][ 3 ];
    x.s2 = ( b.z  - lu.m[ 2 ][ 4 ] * x.s4 - lu.m[ 2 ][ 3 ] * x.s3 ) / lu.m[ 2 ][ 2 ];
    x.s1 = ( b.y  - lu.m[ 1 ][ 4 ] * x.s4 - lu.m[ 1 ][ 3 ] * x.s3 - lu.m[ 1 ][ 2 ] * x.s2 ) / lu.m[ 1 ][ 1 ];
    x.s0 = ( b.x  - lu.m[ 0 ][ 4 ] * x.s4 - lu.m[ 0 ][ 3 ] * x.s3 - lu.m[ 0 ][ 2 ] * x.s2 - lu.m[ 0 ][ 1 ] * x.s1 ) / lu.m[ 0 ][ 0 ];

    return x;
}

kernel void mat5f_test( global float8* output, global Mat5f* mat, global float8* b, int n )
{
    int id = get_global_id( 0 );

    if( id > n )
        return;

    Mat5f A = *( mat );
    float8 bptr = *( b + id );
    *( output + id ) = mat5f_lusolve( &A , bptr.s012, bptr.s34 );
}
#endif
