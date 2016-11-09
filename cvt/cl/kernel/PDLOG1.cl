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

const sampler_t SAMPLER_CLAMP_NN = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
const sampler_t SAMPLER_NN = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

float proxlog1( float z, float tau, float beta )
{
    float fx[ 3 ], x[ 3 ];
    float sgn = sign( z );
    float a   = 1.0f / tau;
    float b   = ( sgn / beta - z ) / tau;
    float c   = -( fabs( z ) / ( beta * tau ) ) + 1.0f;

    float root = sqrt( b * b - 4.0f * a * c );
    x[ 0 ]     = ( -b + root ) / ( 2.0f * a );
    x[ 1 ]     = ( -b - root ) / ( 2.0f * a );
    x[ 2 ]     = 0.0f;

    fx[ 0 ]  = ( 1.0f / ( 2.0f * tau ) ) * ( x[ 0 ] - z ) * ( x[ 0 ] - z ) + log( 1.0f + beta * fabs( x[ 0 ] ) );
    fx[ 1 ]  = ( 1.0f / ( 2.0f * tau ) ) * ( x[ 1 ] - z ) * ( x[ 1 ] - z ) + log( 1.0f + beta * fabs( x[ 1 ] ) );
    fx[ 2 ]  = ( 1.0f / ( 2.0f * tau ) ) * z * z;

    int idx = select( 0, 1, fx[ 1 ] < fx[ 0 ] );
    idx  = select( 2, idx, fx[ idx ] < fx[ 2 ] );

    return x[ idx ];
}


__kernel void PDLOG1( __write_only image2d_t out, __write_only image2d_t outp, __read_only image2d_t last, __read_only image2d_t imgp, __read_only image2d_t image,
                    const float lambda, const float sigma, const float tau, const float theta, const float beta, __local float4* buf, __local float8* buf2  )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int lx = get_local_id( 0 );
    const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
    const int bstride = lw + 2;
    const int2 base = ( int2 )( get_group_id( 0 ) * lw - 1, get_group_id( 1 ) * lh - 1 );
    const int2 base2 = ( int2 )( get_group_id( 0 ) * ( lw << 1 ) - 2, get_group_id( 1 ) * lh - 1 );
    int2 coord;
    float4 dx, dy, div, pxout, pyout, norm;

    for( int y = ly; y < lh + 2; y += lh ) {
        for( int x = lx; x < lw + 2; x += lw ) {
            // read image
            buf[ mad24( y, bstride, x ) ] = read_imagef( last, SAMPLER_CLAMP_NN, base + ( int2 ) ( x, y ) );
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );

#define BUF( x, y ) ( buf[ mad24( ( y ) , bstride, ( x )  ) ] )

#define BUF2( x, y ) ( buf2[ mad24( ( y ) + 1 , bstride, ( x ) + 1  ) ] )

    for( int y = ly; y < lh + 1; y += lh ) {
        coord.y = base2.y + y;
        for( int x = lx; x < lw + 1; x += lw ) {

            dx = BUF( x, y ) - BUF( x + 1, y );
            dy = BUF( x, y ) - BUF( x, y + 1 );

            // p = p + sigma * \nabla last
            float8 p;
            coord.x = base2.x + ( x << 1 );
            p.lo = read_imagef( imgp, SAMPLER_NN, coord ) + sigma * dx;
            coord.x += 1;
            p.hi = read_imagef( imgp, SAMPLER_NN, coord ) + sigma * dy;

            // Moreau identity for solving the prox operator
            float len = sqrt( dot( p.lo, p.lo ) + dot( p.hi, p.hi ) );
            len = select( len, 1.0f, len == 0.0f );
            float8 pn = p / len;
            p = p - sigma * pn * proxlog1( len / sigma, lambda / sigma, beta );

            buf2[ mad24( y, bstride, x ) ] = p;
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );

    if( gx >= get_image_width( image ) || gy >= get_image_height( image ) )
        return;

    float8 p = BUF2( lx, ly );
    div = p.lo - BUF2( lx - 1, ly ).lo + p.hi - BUF2( lx, ly - 1 ).hi;

    float4 img    = read_imagef( image, SAMPLER_NN, ( int2 )( gx, gy ) );
    float4 imgnew = ( BUF( lx + 1, ly + 1 ) + tau * ( 2.0f * img - div ) ) / ( float4 ) ( 1.0f + 2.0f * tau );
    imgnew += theta * ( imgnew - BUF( lx + 1, ly + 1 )  );

    write_imagef( out, ( int2 ) ( gx, gy ), imgnew  );
    write_imagef( outp, ( int2 ) ( gx << 1, gy ), p.lo );
    write_imagef( outp, ( int2 ) ( ( gx << 1 ) + 1, gy ), p.hi );
}

__kernel void PDLOG1_8( __write_only image2d_t out1, __write_only image2d_t out2,
                        __write_only image2d_t outp,
                        __read_only image2d_t last1, __read_only image2d_t last2,
                        __read_only image2d_t imgp,
                        __read_only image2d_t image1, __read_only image2d_t image2,
                        const float lambda, const float sigma, const float tau, const float theta, const float beta, __local float8* buf, __local float16* buf2  )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int lx = get_local_id( 0 );
    const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
    const int bstride = lw + 2;
    const int2 base = ( int2 )( get_group_id( 0 ) * lw - 1, get_group_id( 1 ) * lh - 1 );
    const int2 base2 = ( int2 )( get_group_id( 0 ) * ( lw << 2 ) - 4, get_group_id( 1 ) * lh - 1 );
    int2 coord;
    float2 coordf = ( float2 ) ( gx, gy );
    float8 dx, dy, div, pxout, pyout, norm;
    float8 jacx, jacy;
    const float8 w = ( ( float8 ) ( 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 2.0f, 1.0f, 1.0f ) );

    for( int y = ly; y < lh + 2; y += lh ) {
        for( int x = lx; x < lw + 2; x += lw ) {
            // read image
            float8 last;
            last.lo = read_imagef( last1, SAMPLER_CLAMP_NN, base + ( int2 ) ( x, y ) );
            last.hi = read_imagef( last2, SAMPLER_CLAMP_NN, base + ( int2 ) ( x, y ) );
            buf[ mad24( y, bstride, x ) ] = last;
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );

#define BUF( x, y ) ( buf[ mad24( ( y ) , bstride, ( x )  ) ] )

#define BUF2( x, y ) ( buf2[ mad24( ( y ) + 1 , bstride, ( x ) + 1  ) ] )

    for( int y = ly; y < lh + 1; y += lh ) {
        coord.y = base2.y + y;
        for( int x = lx; x < lw + 1; x += lw ) {

            dx = BUF( x, y ) - BUF( x + 1, y );
            dy = BUF( x, y ) - BUF( x, y + 1 );

            // p = p + sigma * \nabla last
            float16 p;
            coord.x = base2.x + ( x << 2 );
            p.s0123 = read_imagef( imgp, SAMPLER_NN, coord ) + sigma * dx.lo;
            coord.x += 1;
            p.s4567 = read_imagef( imgp, SAMPLER_NN, coord ) + sigma * dx.hi;

            coord.x += 1;
            p.s89ab = read_imagef( imgp, SAMPLER_NN, coord ) + sigma * dy.lo;
            coord.x += 1;
            p.scdef = read_imagef( imgp, SAMPLER_NN, coord ) + sigma * dy.hi;

            p.lo = p.lo;
            p.hi = p.hi;

            // Moreau identity for solving the prox operator
            float len = sqrt( dot( p.s0123, p.s0123 ) + dot( p.s4567, p.s4567 ) + dot( p.s89ab, p.s89ab ) + dot( p.scdef, p.scdef ) );
            //float len = sqrt( dot( p.s0123, p.s0123 * ( float4 ) ( 5.0f, 5.0f, 5.0f, 5.0f ) ) + dot( p.s4567, p.s4567 * ( float4 ) ( 10.0f, 10.0f, 1.0f, 1.0f ) ) + dot( p.s89ab, p.s89ab * ( float4 ) ( 5.0f, 5.0f, 5.0f, 5.0f )) + dot( p.scdef, p.scdef * ( float4 ) ( 10.0f, 10.0f, 1.0f, 1.0f )) );
//            float len = sqrt( dot( p.s0123, p.s0123 * ( float4 ) ( coord.x, coord.x, coord.y, coord.y ) ) + dot( p.s4567, p.s4567 * ( float4 ) ( coord.x, coord.y, 1.0f, 1.0f ) ) + dot( p.s89ab, p.s89ab* ( float4 ) ( coord.x, coord.x, coord.y, coord.y )) + dot( p.scdef, p.scdef * ( float4 ) ( coord.x, coord.y, 1.0f, 1.0f ) ) );
            len = select( len, 1.0f, len == 0.0f );
            float16 pn = p / len;
            p = p - sigma * pn * proxlog1( len / sigma, lambda / sigma, beta );

            buf2[ mad24( y, bstride, x ) ] = p;
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );

    if( gx >= get_image_width( image1 ) || gy >= get_image_height( image1 ) )
        return;

    float16 p = BUF2( lx, ly );

    div = ( p.lo - BUF2( lx - 1, ly ).lo ) + ( p.hi - BUF2( lx, ly - 1 ).hi );

    float8 img;
    img.lo = read_imagef( image1, SAMPLER_NN, ( int2 )( gx, gy ) );
    img.hi = read_imagef( image2, SAMPLER_NN, ( int2 )( gx, gy ) );
    float8 imgnew = ( BUF( lx + 1, ly + 1 ) + tau * ( 2.0f * img - div ) ) / ( float8 ) ( 1.0f + 2.0f * tau );
    imgnew += theta * ( imgnew - BUF( lx + 1, ly + 1 )  );

    write_imagef( out1, ( int2 ) ( gx, gy ), imgnew.lo  );
    write_imagef( out2, ( int2 ) ( gx, gy ), imgnew.hi  );
    write_imagef( outp, ( int2 ) ( gx << 2, gy ), p.s0123 );
    write_imagef( outp, ( int2 ) ( ( gx << 2 ) + 1, gy ), p.s4567 );
    write_imagef( outp, ( int2 ) ( ( gx << 2 ) + 2, gy ), p.s89ab );
    write_imagef( outp, ( int2 ) ( ( gx << 2 ) + 3, gy ), p.scdef );
}

__kernel void PDLOG1_CWISE( __write_only image2d_t out, __write_only image2d_t outp, __read_only image2d_t last, __read_only image2d_t imgp, __read_only image2d_t image,
                    const float lambda, const float sigma, const float tau, const float theta, const float beta, __local float4* buf, __local float8* buf2  )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int lx = get_local_id( 0 );
    const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
    const int bstride = lw + 2;
    const int2 base = ( int2 )( get_group_id( 0 ) * lw - 1, get_group_id( 1 ) * lh - 1 );
    const int2 base2 = ( int2 )( get_group_id( 0 ) * ( lw << 1 ) - 2, get_group_id( 1 ) * lh - 1 );
    int2 coord;
    float4 dx, dy, div, pxout, pyout, norm;

    for( int y = ly; y < lh + 2; y += lh ) {
        for( int x = lx; x < lw + 2; x += lw ) {
            // read image
            buf[ mad24( y, bstride, x ) ] = read_imagef( last, SAMPLER_CLAMP_NN, base + ( int2 ) ( x, y ) );
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );

#define BUF( x, y ) ( buf[ mad24( ( y ) , bstride, ( x )  ) ] )

#define BUF2( x, y ) ( buf2[ mad24( ( y ) + 1 , bstride, ( x ) + 1  ) ] )

    for( int y = ly; y < lh + 1; y += lh ) {
        coord.y = base2.y + y;
        for( int x = lx; x < lw + 1; x += lw ) {

            dx = BUF( x, y ) - BUF( x + 1, y );
            dy = BUF( x, y ) - BUF( x, y + 1 );

            // p = p + sigma * \nabla last
            float8 p;
            coord.x = base2.x + ( x << 1 );
            p.lo = read_imagef( imgp, SAMPLER_NN, coord ) + sigma * dx;
            coord.x += 1;
            p.hi = read_imagef( imgp, SAMPLER_NN, coord ) + sigma * dy;

            // Moreau identity for solving the prox operator
            float4 len = sqrt( p.lo * p.lo + p.hi * p.hi );
            len = select( len, 1.0f, isequal( len, ( float4 ) 0.0f ) );
            float8 pn = p / ( float8 ) ( len, len );
            float4 t;
            t.x = proxlog1( len.x / sigma, lambda / sigma, beta );
            t.y = proxlog1( len.y / sigma, lambda / sigma, beta );
            t.z = proxlog1( len.z / sigma, lambda / sigma, beta );
            t.w = proxlog1( len.w / sigma, lambda / sigma, beta );
            p = p - sigma * pn * ( float8 ) ( t, t );

            buf2[ mad24( y, bstride, x ) ] = p;
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );

    if( gx >= get_image_width( image ) || gy >= get_image_height( image ) )
        return;

    float8 p = BUF2( lx, ly );
    div = p.lo - BUF2( lx - 1, ly ).lo + p.hi - BUF2( lx, ly - 1 ).hi;

    float4 img    = read_imagef( image, SAMPLER_NN, ( int2 )( gx, gy ) );
    float4 imgnew = ( BUF( lx + 1, ly + 1 ) + tau * ( 2.0f * img - div ) ) / ( float4 ) ( 1.0f + 2.0f * tau );
    imgnew += theta * ( imgnew - BUF( lx + 1, ly + 1 )  );

    write_imagef( out, ( int2 ) ( gx, gy ), imgnew  );
    write_imagef( outp, ( int2 ) ( gx << 1, gy ), p.lo );
    write_imagef( outp, ( int2 ) ( ( gx << 1 ) + 1, gy ), p.hi );
}
