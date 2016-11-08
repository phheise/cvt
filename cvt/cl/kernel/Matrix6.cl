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

#ifndef CL_MAT6F_CL
#define CL_MAT6F_CL

struct Mat6f {
    float m[ 6 ][ 6 ];
};

typedef struct Mat6f Mat6f;

inline void mat6f_assign( Mat6f* dst, const Mat6f* src )
{
    for( int y = 0; y < 6; y++ )
        for( int x = 0; x < 6; x++ )
            dst->m[ y ][ x ] = src->m[ y ][ x ];
}

inline float8 mat6f_transform( const Mat6f* mat, const float8 vec )
{
    return ( float8 ) ( mat->m[ 0 ][ 0 ] * vec.s0 + mat->m[ 0 ][ 1 ] * vec.s1 + mat->m[ 0 ][ 2 ] * vec.s2 + mat->m[ 0 ][ 3 ] * vec.s3 + mat->m[ 0 ][ 4 ] * vec.s4 + mat->m[ 0 ][ 5 ] * vec.s5,
                        mat->m[ 1 ][ 0 ] * vec.s0 + mat->m[ 1 ][ 1 ] * vec.s1 + mat->m[ 1 ][ 2 ] * vec.s2 + mat->m[ 1 ][ 3 ] * vec.s3 + mat->m[ 1 ][ 4 ] * vec.s4 + mat->m[ 1 ][ 5 ] * vec.s5,
                        mat->m[ 2 ][ 0 ] * vec.s0 + mat->m[ 2 ][ 1 ] * vec.s1 + mat->m[ 2 ][ 2 ] * vec.s2 + mat->m[ 2 ][ 3 ] * vec.s3 + mat->m[ 2 ][ 4 ] * vec.s4 + mat->m[ 2 ][ 5 ] * vec.s5,
                        mat->m[ 3 ][ 0 ] * vec.s0 + mat->m[ 3 ][ 1 ] * vec.s1 + mat->m[ 3 ][ 2 ] * vec.s2 + mat->m[ 3 ][ 3 ] * vec.s3 + mat->m[ 3 ][ 4 ] * vec.s4 + mat->m[ 3 ][ 5 ] * vec.s5,
                        mat->m[ 4 ][ 0 ] * vec.s0 + mat->m[ 4 ][ 1 ] * vec.s1 + mat->m[ 4 ][ 2 ] * vec.s2 + mat->m[ 4 ][ 3 ] * vec.s3 + mat->m[ 4 ][ 4 ] * vec.s4 + mat->m[ 4 ][ 5 ] * vec.s5,
                        mat->m[ 5 ][ 0 ] * vec.s0 + mat->m[ 5 ][ 1 ] * vec.s1 + mat->m[ 5 ][ 2 ] * vec.s2 + mat->m[ 5 ][ 3 ] * vec.s3 + mat->m[ 5 ][ 4 ] * vec.s4 + mat->m[ 5 ][ 5 ] * vec.s5,
                        0.0f, 0.0f );
}

inline float8 mat6f_transpose_transform( const Mat6f* mat, const float8 vec )
{
    return ( float8 ) ( mat->m[ 0 ][ 0 ] * vec.s0 + mat->m[ 1 ][ 0 ] * vec.s1 + mat->m[ 2 ][ 0 ] * vec.s2 + mat->m[ 3 ][ 0 ] * vec.s3 + mat->m[ 4 ][ 0 ] * vec.s4 + mat->m[ 5 ][ 0 ] * vec.s5,
                        mat->m[ 0 ][ 1 ] * vec.s0 + mat->m[ 1 ][ 1 ] * vec.s1 + mat->m[ 2 ][ 1 ] * vec.s2 + mat->m[ 3 ][ 1 ] * vec.s3 + mat->m[ 4 ][ 1 ] * vec.s4 + mat->m[ 5 ][ 1 ] * vec.s5,
                        mat->m[ 0 ][ 2 ] * vec.s0 + mat->m[ 1 ][ 2 ] * vec.s1 + mat->m[ 2 ][ 2 ] * vec.s2 + mat->m[ 3 ][ 2 ] * vec.s3 + mat->m[ 4 ][ 2 ] * vec.s4 + mat->m[ 5 ][ 2 ] * vec.s5,
                        mat->m[ 0 ][ 3 ] * vec.s0 + mat->m[ 1 ][ 3 ] * vec.s1 + mat->m[ 2 ][ 3 ] * vec.s2 + mat->m[ 3 ][ 3 ] * vec.s3 + mat->m[ 4 ][ 3 ] * vec.s4 + mat->m[ 5 ][ 3 ] * vec.s5,
                        mat->m[ 0 ][ 4 ] * vec.s0 + mat->m[ 1 ][ 4 ] * vec.s1 + mat->m[ 2 ][ 4 ] * vec.s2 + mat->m[ 3 ][ 4 ] * vec.s3 + mat->m[ 4 ][ 4 ] * vec.s4 + mat->m[ 5 ][ 4 ] * vec.s5,
                        mat->m[ 0 ][ 5 ] * vec.s0 + mat->m[ 1 ][ 5 ] * vec.s1 + mat->m[ 2 ][ 5 ] * vec.s2 + mat->m[ 3 ][ 5 ] * vec.s3 + mat->m[ 4 ][ 5 ] * vec.s4 + mat->m[ 5 ][ 5 ] * vec.s5,
                        0.0f, 0.0f );
}

inline void mat5f_outer( Mat6f* dst, const float8 a, const float8 b )
{
    dst->m[ 0 ][ 0 ] = a.s0 * b.s0;
    dst->m[ 0 ][ 1 ] = a.s0 * b.s1;
    dst->m[ 0 ][ 2 ] = a.s0 * b.s2;
    dst->m[ 0 ][ 3 ] = a.s0 * b.s3;
    dst->m[ 0 ][ 4 ] = a.s0 * b.s4;
    dst->m[ 0 ][ 5 ] = a.s0 * b.s5;

    dst->m[ 1 ][ 0 ] = a.s1 * b.s0;
    dst->m[ 1 ][ 1 ] = a.s1 * b.s1;
    dst->m[ 1 ][ 2 ] = a.s1 * b.s2;
    dst->m[ 1 ][ 3 ] = a.s1 * b.s3;
    dst->m[ 1 ][ 4 ] = a.s1 * b.s4;
    dst->m[ 1 ][ 5 ] = a.s1 * b.s5;

    dst->m[ 2 ][ 0 ] = a.s2 * b.s0;
    dst->m[ 2 ][ 1 ] = a.s2 * b.s1;
    dst->m[ 2 ][ 2 ] = a.s2 * b.s2;
    dst->m[ 2 ][ 3 ] = a.s2 * b.s3;
    dst->m[ 2 ][ 4 ] = a.s2 * b.s4;
    dst->m[ 2 ][ 5 ] = a.s2 * b.s5;

    dst->m[ 3 ][ 0 ] = a.s3 * b.s0;
    dst->m[ 3 ][ 1 ] = a.s3 * b.s1;
    dst->m[ 3 ][ 2 ] = a.s3 * b.s2;
    dst->m[ 3 ][ 3 ] = a.s3 * b.s3;
    dst->m[ 3 ][ 4 ] = a.s3 * b.s4;
    dst->m[ 3 ][ 5 ] = a.s3 * b.s5;

    dst->m[ 4 ][ 0 ] = a.s4 * b.s0;
    dst->m[ 4 ][ 1 ] = a.s4 * b.s1;
    dst->m[ 4 ][ 2 ] = a.s4 * b.s2;
    dst->m[ 4 ][ 3 ] = a.s4 * b.s3;
    dst->m[ 4 ][ 4 ] = a.s4 * b.s4;
    dst->m[ 4 ][ 5 ] = a.s4 * b.s5;

    dst->m[ 5 ][ 0 ] = a.s5 * b.s0;
    dst->m[ 5 ][ 1 ] = a.s5 * b.s1;
    dst->m[ 5 ][ 2 ] = a.s5 * b.s2;
    dst->m[ 5 ][ 3 ] = a.s5 * b.s3;
    dst->m[ 5 ][ 4 ] = a.s5 * b.s4;
    dst->m[ 5 ][ 5 ] = a.s5 * b.s5;

}

inline void mat6f_mul( Mat6f* dst, const Mat6f* a, const Mat6f* b )
{
    for( int y = 0; y < 6; y++ ) {
        for( int x = 0; x < 6; x++ ) {
            float sum = 0.0f;
            for( int w = 0; w < 6; w++ )
                sum += a->m[ y ][ w ] * b->m[ w ][ x ];
            dst->m[ y ][ x ] = sum;
        }
    }
}

inline void mat6f_mul_transpose( Mat6f* dst, const Mat6f* a, const Mat6f* b )
{
    for( int y = 0; y < 6; y++ ) {
        for( int x = 0; x < 6; x++ ) {
            float sum = 0.0f;
            for( int w = 0; w < 6; w++ )
                sum += a->m[ y ][ w ] * b->m[ x ][ w ];
            dst->m[ y ][ x ] = sum;
        }
    }
}

inline void mat6f_mul_scalar( Mat6f* dst, float s, const Mat6f* m )
{
    for( int y = 0; y < 6; y++ )
        for( int x = 0; x < 6; x++ )
            dst->m[ y ][ x ] = s * m->m[ y ][ x ];
}

inline void mat6f_add( Mat6f* dst, const Mat6f* a, const Mat6f* b )
{
    for( int y = 0; y < 6; y++ )
        for( int x = 0; x < 6; x++ )
            dst->m[ y ][ x ] = a->m[ y ][ x ] + b->m[ y ][ x ];
}

inline void mat6f_sub( Mat6f* dst, const Mat6f* a, const Mat6f* b )
{
    for( int y = 0; y < 6; y++ )
        for( int x = 0; x < 6; x++ )
            dst->m[ y ][ x ] = a->m[ y ][ x ] - b->m[ y ][ x ];
}

inline void mat6f_transpose( Mat6f* dst, const Mat6f* mat )
{
    for( int y = 0; y < 6; y++ )
        for( int x = 0; x < 6; x++ )
            dst->m[ y ][ x ] = mat->m[ x ][ y ];
}

inline void mat6f_set_zero( Mat6f* dst )
{
    for( int y = 0; y < 6; y++ )
        for( int x = 0; x < 6; x++ )
            dst->m[ y ][ x ] = 0.0f;
}

/*
FIXME: TODO
float8 mat5f_lusolve( const Mat6f* mat, float3 b, float2 b2 )
{
    Mat6f lu;
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

kernel void mat5f_test( global float8* output, global Mat6f* mat, global float8* b, int n )
{
    int id = get_global_id( 0 );

    if( id > n )
        return;

    Mat6f A = *( mat );
    float8 bptr = *( b + id );
    *( output + id ) = mat5f_lusolve( &A , bptr.s012, bptr.s34 );
}
*/

#endif
