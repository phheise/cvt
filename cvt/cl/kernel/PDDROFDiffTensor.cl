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

#import "Matrix3.cl"

const sampler_t SAMPLER_CLAMP_NN = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
const sampler_t SAMPLER_NN = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

//#define SIGMA ( 1.0f / sqrt( 8.0f ) )
//#define TAU   ( 1.0f / sqrt( 8.0f ) )
//#define ETA   ( 1.0f / sqrt( 8.0f ) )
#define SIGMA 4.0f
#define TAU   0.05f
#define ETA   8.0f
//#define SIGMA ( 0.01f )
//#define TAU   ( 0.01f )
//#define ETA   ( 0.01f )

#define THETA  0.5f

__kernel void PDDROFDiffTensor( __write_only image2d_t outu, __write_only image2d_t outp, __write_only image2d_t outq,
                      __read_only image2d_t imgu, __read_only image2d_t imgp, __read_only image2d_t imgq,
                      __read_only image2d_t image, __read_only image2d_t dtimg, const float4 lambda, __local float4* buf, __local float8* buf2  )
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
            buf[ mad24( y, bstride, x ) ] = read_imagef( imgu, SAMPLER_CLAMP_NN, base + ( int2 ) ( x, y ) );
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );

#define BUF( x, y ) ( buf[ mad24( ( y ) , bstride, ( x ) ) ] )
#define BUF2( x, y ) ( buf2[ mad24( ( y ) + 1 , bstride, ( x ) + 1 ) ] )

    for( int y = ly; y < lh + 1; y += lh ) {
        coord.y = base2.y + y;
        for( int x = lx; x < lw + 1; x += lw ) {

            float4 DT = read_imagef( dtimg, SAMPLER_NN, base + ( int2 )( x, y ) );

            dx = ( BUF( x, y ) - BUF( x + 1, y ) );
            dy = ( BUF( x, y ) - BUF( x, y + 1 ) );

            float4 DTdx = DT.x * dx + DT.z * dy;
            float4 DTdy = DT.y * dx + DT.w * dy;

            float8 p;
            coord.x = base2.x + ( x << 1 );
            p.lo = read_imagef( imgp, SAMPLER_NN, coord ) + SIGMA * DTdx;
            coord.x += 1;
            p.hi = read_imagef( imgp, SAMPLER_NN, coord ) + SIGMA * DTdy;

        //  float4 pproj = fmax( ( float4 ) 1.0f, sqrt( p.lo * p.lo + p.hi * p.hi ) );
            float4 norm = p.lo * p.lo + p.hi * p.hi;
            norm.xyz = ( float3 ) sqrt( dot( norm.xyz, ( float3 ) 1.0f ) );
            norm.w = sqrt( norm.w );
            float4 pproj = ( float4 ) fmax( 1.0f, norm );

            p = p / ( float8 ) ( pproj, pproj );

            float4 DTplo = DT.x * p.lo + DT.z * p.hi;
            float4 DTphi = DT.y * p.lo + DT.w * p.hi;

            buf2[ mad24( y, bstride, x ) ] = ( float8 ) ( DTplo, DTphi );
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );

    if( gx >= get_image_width( image ) || gy >= get_image_height( image ) )
        return;

    float8 p = BUF2( lx, ly );
    div = p.lo - BUF2( lx - 1, ly ).lo + p.hi - BUF2( lx, ly - 1 ).hi;

    float4 img    = read_imagef( image, SAMPLER_NN, ( int2 )( gx, gy ) );
    float4 q      = read_imagef( imgq, SAMPLER_NN, ( int2 )( gx, gy ) );
    float4 outnew = ( BUF( lx + 1, ly + 1 ) + TAU * ( lambda * img - div - q ) ) / ( float4 ) ( 1.0f + TAU * lambda );

    outnew = outnew + THETA * ( outnew - BUF( lx + 1, ly + 1 )  );

    //outnew.xyz = normalize( outnew.xyz );

    float4 qnew   = ( q + ETA * ( outnew - img ) ) / ( float4 ) ( 1.0f + 4.0f * ETA / lambda );
    qnew =  qnew + THETA * ( qnew - q );

    write_imagef( outu, ( int2 ) ( gx, gy ), outnew  );
    write_imagef( outq, ( int2 ) ( gx, gy ), qnew  );
    write_imagef( outp, ( int2 ) ( gx << 1, gy ), p.lo );
    write_imagef( outp, ( int2 ) ( ( gx << 1 ) + 1, gy ), p.hi );
}

__kernel void PDDROFNDDiffTensor( __write_only image2d_t outu, __write_only image2d_t outp, __write_only image2d_t outq,
                      __read_only image2d_t imgu, __read_only image2d_t imgp, __read_only image2d_t imgq,
                      __read_only image2d_t image, __read_only image2d_t dtimg, const Mat3f KSrcInv, const float4 lambda, __local float4* buf, __local float8* buf2  )
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
            buf[ mad24( y, bstride, x ) ] = read_imagef( imgu, SAMPLER_CLAMP_NN, base + ( int2 ) ( x, y ) );
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );

#define BUF( x, y ) ( buf[ mad24( ( y ) , bstride, ( x ) ) ] )
#define BUF2( x, y ) ( buf2[ mad24( ( y ) + 1 , bstride, ( x ) + 1 ) ] )

    for( int y = ly; y < lh + 1; y += lh ) {
        coord.y = base2.y + y;
        for( int x = lx; x < lw + 1; x += lw ) {

            float4 DT = read_imagef( dtimg, SAMPLER_NN, base + ( int2 )( x, y ) );

            dx = ( BUF( x, y ) - BUF( x + 1, y ) );
            dy = ( BUF( x, y ) - BUF( x, y + 1 ) );

//            if(  length( BUF( x + 1, y ).xyz ) == 0 )
//                dx = 0;
//            if(  length( BUF( x , y + 1 ).xyz ) == 0 )
//                dy = 0;

            float3 KIx = 1.0f * mat3f_transform( &KSrcInv, ( float3 ) ( base.x + x, base.y + y , 1.0f ) );

            dx.w = dot( KIx, dx.xyz );
            dy.w = dot( KIx, dy.xyz );

            float4 DTdx = DT.x * dx + DT.z * dy;
            float4 DTdy = DT.y * dx + DT.w * dy;

            DTdx.xyz *= 1.0f;
            DTdy.xyz *= 1.0f;

            float8 p;
            coord.x = base2.x + ( x << 1 );
            p.lo = read_imagef( imgp, SAMPLER_NN, coord ) + SIGMA * DTdx;
            coord.x += 1;
            p.hi = read_imagef( imgp, SAMPLER_NN, coord ) + SIGMA * DTdy;

        //  float4 pproj = fmax( ( float4 ) 1.0f, sqrt( p.lo * p.lo + p.hi * p.hi ) );
            float norm = sqrt( dot( ( p.lo * p.lo + p.hi * p.hi ), ( float4 ) 1.0f ) );
            float4 pproj = ( float4 ) fmax( 1.0f, norm );

            p = p / ( float8 ) ( pproj, pproj );

            float4 DTplo = DT.x * p.lo + DT.z * p.hi;
            float4 DTphi = DT.y * p.lo + DT.w * p.hi;

            DTplo.xyz = DTplo.xyz * 1.0f + KIx * DTplo.w;
            DTphi.xyz = DTphi.xyz * 1.0f + KIx * DTphi.w;

            buf2[ mad24( y, bstride, x ) ] = ( float8 ) ( DTplo, DTphi );
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );

    if( gx >= get_image_width( image ) || gy >= get_image_height( image ) )
        return;

    float8 p = BUF2( lx, ly );
    div = p.lo - BUF2( lx - 1, ly ).lo + p.hi - BUF2( lx, ly - 1 ).hi;

    float4 img    = read_imagef( image, SAMPLER_NN, ( int2 )( gx, gy ) );
    float4 q      = read_imagef( imgq, SAMPLER_NN, ( int2 )( gx, gy ) );
    float4 outnew = ( BUF( lx + 1, ly + 1 ) + TAU * ( lambda * img - div - q ) ) / ( float4 ) ( 1.0f + TAU * lambda );
    outnew = outnew + THETA * ( outnew - BUF( lx + 1, ly + 1 )  );

    float4 qnew   = ( q + ETA * ( outnew - img ) ) / ( float4 ) ( 1.0f + 4.0f * ETA / lambda );
    qnew =  qnew + THETA * ( qnew - q );

    write_imagef( outu, ( int2 ) ( gx, gy ), outnew  );
    write_imagef( outq, ( int2 ) ( gx, gy ), qnew  );
    write_imagef( outp, ( int2 ) ( gx << 1, gy ), p.lo );
    write_imagef( outp, ( int2 ) ( ( gx << 1 ) + 1, gy ), p.hi );
}

__kernel void PDDROFPOSDiffTensor( __write_only image2d_t outu, __write_only image2d_t outp, __write_only image2d_t outq,
                      __read_only image2d_t imgu, __read_only image2d_t imgp, __read_only image2d_t imgq,
                      __read_only image2d_t image, __read_only image2d_t dtimg, const float4 lambda, __local float4* buf, __local float8* buf2  )
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
            buf[ mad24( y, bstride, x ) ] = read_imagef( imgu, SAMPLER_CLAMP_NN, base + ( int2 ) ( x, y ) );
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );

#define BUF( x, y ) ( buf[ mad24( ( y ) , bstride, ( x ) ) ] )
#define BUF2( x, y ) ( buf2[ mad24( ( y ) + 1 , bstride, ( x ) + 1 ) ] )

    for( int y = ly; y < lh + 1; y += lh ) {
        coord.y = base2.y + y;
        for( int x = lx; x < lw + 1; x += lw ) {

            float4 DT = read_imagef( dtimg, SAMPLER_NN, base + ( int2 )( x, y ) );

            dx = ( BUF( x, y ) - BUF( x + 1, y ) );
            dy = ( BUF( x, y ) - BUF( x, y + 1 ) );

            float3 pos = ( float3 ) ( base.x + x, base.y + y , 1.0f );
            dx.w = 0.01f * dot( pos, dx.xyz );
            dy.w = 0.01f * dot( pos, dy.xyz );

            float4 DTdx = DT.x * dx + DT.z * dy;
            float4 DTdy = DT.y * dx + DT.w * dy;

            DTdx.xyz *= 0.0f;
            DTdy.xyz *= 0.0f;

            float8 p;
            coord.x = base2.x + ( x << 1 );
            p.lo = read_imagef( imgp, SAMPLER_CLAMP_NN, coord ) + SIGMA * DTdx;
            coord.x += 1;
            p.hi = read_imagef( imgp, SAMPLER_CLAMP_NN, coord ) + SIGMA * DTdy;

        //  float4 pproj = fmax( ( float4 ) 1.0f, sqrt( p.lo * p.lo + p.hi * p.hi ) );
            float norm = sqrt( dot( ( p.lo * p.lo + p.hi * p.hi ), ( float4 ) 1.0f ) );
            float4 pproj = ( float4 ) fmax( 1.0f, norm );

            p = p / ( float8 ) ( pproj, pproj );

            float4 DTplo = DT.x * p.lo + DT.z * p.hi;
            float4 DTphi = DT.y * p.lo + DT.w * p.hi;

            DTplo.xyz = DTplo.xyz * 0.0f + 0.01f * DTplo.w;
            DTphi.xyz = DTphi.xyz * 0.0f + 0.01f * DTphi.w;

            DTplo.w = 0.0f;
            DTphi.w = 0.0f;

            buf2[ mad24( y, bstride, x ) ] = ( float8 ) ( DTplo, DTphi );
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );

    if( gx >= get_image_width( image ) || gy >= get_image_height( image ) )
        return;

    float8 p = BUF2( lx, ly );
    div = p.lo - BUF2( lx - 1, ly ).lo + p.hi - BUF2( lx, ly - 1 ).hi;

    float4 img    = read_imagef( image, SAMPLER_NN, ( int2 )( gx, gy ) );
    float4 q      = read_imagef( imgq, SAMPLER_NN, ( int2 )( gx, gy ) );
    float4 outnew = ( BUF( lx + 1, ly + 1 ) + TAU * ( lambda * img - div - q ) ) / ( float4 ) ( 1.0f + TAU * lambda );
    outnew = outnew + THETA * ( outnew - BUF( lx + 1, ly + 1 )  );

    float4 qnew   = ( q + ETA * ( outnew - img ) ) / ( float4 ) ( 1.0f + 4.0f * ETA / lambda );
    qnew =  qnew + THETA * ( qnew - q );

    write_imagef( outu, ( int2 ) ( gx, gy ), outnew  );
    write_imagef( outq, ( int2 ) ( gx, gy ), qnew  );
    write_imagef( outp, ( int2 ) ( gx << 1, gy ), p.lo );
    write_imagef( outp, ( int2 ) ( ( gx << 1 ) + 1, gy ), p.hi );
}
