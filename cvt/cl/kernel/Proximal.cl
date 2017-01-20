/*
   The MIT License (MIT)

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
#ifndef CL_PROXIMAL_CL
#define CL_PROXIMAL_CL

float __OVERLOADABLE__ length( float8 p )
{
    return sqrt( dot( p.lo, p.lo ) + dot( p.hi, p.hi ) );
}

/*
   Solve y* = prox_{lambda,f}( y ) with f = ( ||x||_2 )
 */
#define PROX_L2( name, type )               \
inline type prox_l2_##name( type y, float lambda ) \
{                                           \
    const float len = length( y );          \
    if( len < lambda )                      \
        return ( type ) 0.0f;               \
    return ( 1.0f - lambda / len ) * y;     \
}

PROX_L2( 1f, float )
PROX_L2( 2f, float2 )
PROX_L2( 3f, float3 )
PROX_L2( 4f, float4 )
PROX_L2( 8f, float8 )

#define PROX_L1( name, type )               \
inline type prox_l1_##name( type y, float lambda ) \
{                                           \
    return fmax( 0.0f, y - lambda ) - fmax( 0.0f, - y - lambda ); \
}

PROX_L1( 1f, float )
PROX_L1( 2f, float2 )
PROX_L1( 3f, float3 )
PROX_L1( 4f, float4 )
PROX_L1( 8f, float8 )

/*
   Solve y* = prox_{lambda,f}( y ) with f = 1/2 ( ||x||_2 )^2
 */
#define PROX_L2SQR( name, type )               \
inline type prox_l2sqr_##name( type y, float lambda ) \
{                                           \
    return ( lambda / ( 1.0f + lambda ) ) * y;\
}

PROX_L2SQR( 1f, float )
PROX_L2SQR( 2f, float2 )
PROX_L2SQR( 3f, float3 )
PROX_L2SQR( 4f, float4 )
PROX_L2SQR( 8f, float8 )

/*
   Solve y* = prox_{lambda,f}( x ) with f = ( ||x-v||_2 )
   yielding argmin_x  1/2 ( x - v )^2 + 1/2\lambda ( x - y )^2
 */
#define PROX_QUADRATIC_DIST( name, type )   \
inline type prox_quadratic_dist_##name( type y, type v, float lambda ) \
{                                           \
    const float len = length( y );          \
    return ( lambda * v + y  ) / ( 1.0f + lambda ); \
}

PROX_QUADRATIC_DIST( 1f, float )
PROX_QUADRATIC_DIST( 2f, float2 )
PROX_QUADRATIC_DIST( 3f, float3 )
PROX_QUADRATIC_DIST( 4f, float4 )
PROX_QUADRATIC_DIST( 8f, float8 )

/*
    Solve projection onto a box or hyper-rectangle
    C = { x | lower <= x <= upper }
 */
#define PROX_PROJ_BOX( name, type )                         \
inline type prox_project_box_##name( type y, type lower, type upper ) \
{                                                      \
    return clamp( y, lower, upper );                  \
}

PROX_PROJ_BOX( 1f, float )
PROX_PROJ_BOX( 2f, float2 )
PROX_PROJ_BOX( 3f, float3 )
PROX_PROJ_BOX( 4f, float4 )
PROX_PROJ_BOX( 8f, float8 )

/*
    Solve projection onto a unit-hyper-sphere
    C = { x | ||x||_2 <= 1 }
 */
#define PROX_PROJ_UNITSPHERE( name, type )    \
inline type prox_project_unitsphere_##name( type y ) \
{                                             \
   const float len = length( y );             \
   return y / fmax( 1.0f, len );              \
}

PROX_PROJ_UNITSPHERE( 1f, float )
PROX_PROJ_UNITSPHERE( 2f, float2 )
PROX_PROJ_UNITSPHERE( 3f, float3 )
PROX_PROJ_UNITSPHERE( 4f, float4 )
PROX_PROJ_UNITSPHERE( 8f, float8 )

/* Solve the prox_{\lambda f}(z) with f(z) = log(1+\beta * |z| )*/
inline float prox_log1p_1f( float z, float lambda, float beta )
{
    float fx[ 3 ], x[ 3 ];
    float sgn = sign( z );
    float a   = 1.0f / lambda;
    float b   = ( sgn / beta - z ) / lambda;
    float c   = -( fabs( z ) / ( beta * lambda ) ) + 1.0f;

    float root = sqrt( b * b - 4.0f * a * c );
    x[ 0 ]     = ( -b + root ) / ( 2.0f * a );
    x[ 1 ]     = ( -b - root ) / ( 2.0f * a );
    x[ 2 ]     = 0.0f;

    fx[ 0 ]  = ( 1.0f / ( 2.0f * lambda ) ) * ( x[ 0 ] - z ) * ( x[ 0 ] - z ) + log( 1.0f + beta * fabs( x[ 0 ] ) );
    fx[ 1 ]  = ( 1.0f / ( 2.0f * lambda ) ) * ( x[ 1 ] - z ) * ( x[ 1 ] - z ) + log( 1.0f + beta * fabs( x[ 1 ] ) );
    fx[ 2 ]  = ( 1.0f / ( 2.0f * lambda ) ) * z * z;

    int idx = select( 0, 1, fx[ 1 ] < fx[ 0 ] );
    idx  = select( 2, idx, fx[ idx ] < fx[ 2 ] );

    return x[ idx ];
}


#define PROX_HL( name, type )    \
inline type prox_hl_##name( type p, float tau, float beta, int iter )   \
{                                                                       \
    float t = 1.0f;                                                     \
    float alpha = tau * pow( length( p ), ( beta - 2.0f ) );            \
                                                                        \
    for( int i = 0; i < iter; i++ ) {                                   \
        float d  = alpha * beta * pow( t, beta - 1.0f ) + t - 1.0f;     \
        float d2 = alpha * ( beta - 1.0f ) * beta * pow( t, beta - 2.0f ) + 1.0f; \
        t = t - d / d2;                                                 \
    }                                                                   \
                                                                        \
    float thres = ( 1.0f / ( 2.0f - beta ) ) * pow( ( 2.0f * ( ( 1.0f - beta  ) / ( 2.0f - beta ) ) ), 1.0f - beta ); \
    t = select( t, 0.0f, isgreater( alpha, thres ) );                   \
    p = p * t;                                                          \
                                                                        \
    return p;                                                           \
}

PROX_HL( 2f, float2 )

#endif
