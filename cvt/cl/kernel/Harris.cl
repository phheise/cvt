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

__kernel void harris_color( __write_only image2d_t out, __read_only image2d_t src, __local float2* buf  )
{
    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
    const int BORDER = 4;
    const int HARRIS_RADIUS = 2;
    const float KAPPA = 0.05f;
    const float4 GRAY_WEIGHT =  ( float4 ) ( 0.2126f, 0.7152f, 0.0722f, 0.0f );
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int lx = get_local_id( 0 );
    const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
    const int width = get_image_width( out );
    const int height = get_image_height( out );
    const int2 base = ( int2 )( get_group_id( 0 ) * lw - BORDER, get_group_id( 1 ) * lh - BORDER );
    const int bstride = lw + 2 * BORDER;

    float2 coord;
    float dx, dy;

    /* Fetch image data to local buffer */
#define BUF( x, y ) buf[ mul24( ( y ), bstride ) + ( x )]
#define TOGRAY(x) dot( x, GRAY_WEIGHT )

    for( int y = ly; y < lh + 2 * BORDER; y += lh ) {
        for( int x = lx; x < lw + 2 * BORDER; x += lw ) {
            BUF( x, y ).x = TOGRAY( read_imagef( src, sampler, base + ( int2 )( x, y ) ) );
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );
#if 0
    /* Calculate gradient and store in local buffer */
    for( int y = ly + 1; y < lh + 2 * BORDER - 1; y += lh ) {
        for( int x = lx + 1; x < lw + 2 * BORDER - 1; x += lw ) {

            dx = BUF( x + 1, y ).x - BUF( x - 1, y  ).x;
            dx = dx * 0.5f + 0.25f * ( BUF( x + 1, y - 1 ).x - BUF( x - 1, y - 1 ).x );
            dx = dx        + 0.25f * ( BUF( x + 1, y + 1 ).x - BUF( x - 1, y + 1 ).x );
            dy = BUF( x, y + 1 ).x - BUF( x, y - 1 ).x;
            dy = dy * 0.5f + 0.25f * ( BUF( x - 1, y + 1 ).x - BUF( x - 1, y - 1 ).x );
            dy = dy        + 0.25f * ( BUF( x + 1, y + 1 ).x - BUF( x + 1, y - 1 ).x );
            BUF( x, y ).yz = ( float2 ) ( dx, dy );
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );
#endif

    /* Compute structure tensor from gradient and compute harris score */
    for( int y = ly + BORDER - 1; y < lh + BORDER + 1; y += lh ) {
        for( int x = lx + BORDER - 1; x < lw + BORDER + 1; x += lw ) {

            float3 structure_tensor = ( float3 ) 0;
            for( int hy = -HARRIS_RADIUS; hy <= HARRIS_RADIUS; hy++ ) {
                for( int hx = -HARRIS_RADIUS; hx <= HARRIS_RADIUS; hx++ ) {
                    int tx = x + hx;
                    int ty = y + hy;
                    float2 grad;

                    grad.x = BUF( tx + 1, ty ).x - BUF( tx - 1, ty  ).x;
                    grad.x = grad.x * 0.5f + 0.25f * ( BUF( tx + 1, ty - 1 ).x - BUF( tx - 1, ty - 1 ).x );
                    grad.x = grad.x        + 0.25f * ( BUF( tx + 1, ty + 1 ).x - BUF( tx - 1, ty + 1 ).x );
                    grad.y = BUF( tx, ty + 1 ).x - BUF( tx    , ty - 1 ).x;
                    grad.y = grad.y * 0.5f + 0.25f * ( BUF( tx - 1, ty + 1 ).x - BUF( tx - 1, ty - 1 ).x );
                    grad.y = grad.y        + 0.25f * ( BUF( tx + 1, ty + 1 ).x - BUF( tx + 1, ty - 1 ).x );

                    structure_tensor += ( float3 ) ( grad.x * grad.x, grad.y * grad.y, grad.x * grad.y );
                }
            }
            const float trace = structure_tensor.x + structure_tensor.y;
            float score = ( structure_tensor.x * structure_tensor.y - structure_tensor.z * structure_tensor.z ) - KAPPA * trace * trace;
            BUF( x, y ).y = score;
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );

    /* Perform non-maximum suppression in 3 x 3 window */

    if( gx >= width || gy >= height )
        return;

    float score = BUF( lx + BORDER, ly + BORDER ).y;
    float I = BUF( lx + BORDER, ly + BORDER ).x;
    bool suppressed;
    suppressed =               select( false, true, score < BUF( lx + BORDER - 1, ly + BORDER - 1 ).y );
    suppressed = suppressed || select( false, true, score < BUF( lx + BORDER    , ly + BORDER - 1 ).y );
    suppressed = suppressed || select( false, true, score < BUF( lx + BORDER + 1, ly + BORDER - 1 ).y );
    suppressed = suppressed || select( false, true, score < BUF( lx + BORDER - 1, ly + BORDER     ).y );
    suppressed = suppressed || select( false, true, score < BUF( lx + BORDER + 1, ly + BORDER     ).y );
    suppressed = suppressed || select( false, true, score < BUF( lx + BORDER - 1, ly + BORDER + 1 ).y );
    suppressed = suppressed || select( false, true, score < BUF( lx + BORDER    , ly + BORDER + 1 ).y );
    suppressed = suppressed || select( false, true, score < BUF( lx + BORDER + 1, ly + BORDER + 1 ).y );

    /* Store max candidates in correct grid cell */
    if( !suppressed && score > 0.5f ) {
        write_imagef( out,( int2 )( gx, gy ), ( float4 ) ( 1.0f, 0.0f, 0.0f, 1.0f ) );
    } else {
        write_imagef( out,( int2 )( gx, gy ), ( float4 ) ( I, I, I, 1.0f ) );
    }

#undef BUF
#undef TOGRAY
}
