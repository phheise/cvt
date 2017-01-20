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
#import "Feature.cl"

__kernel void fast9_color( global Feature* features, global int* feature_size, int features_max, __read_only image2d_t src, const float threshold, const int imgborder, __local float2* buf  )
{
    const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;
    const int BORDER = 4;
    const float4 GRAY_WEIGHT =  ( float4 ) ( 0.2126f, 0.7152f, 0.0722f, 0.0f );
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int lx = get_local_id( 0 );
    const int ly = get_local_id( 1 );
    const int lw = get_local_size( 0 );
    const int lh = get_local_size( 1 );
    const int width = get_image_width( src );
    const int height = get_image_height( src );
    const int2 base = ( int2 )( get_group_id( 0 ) * lw - BORDER, get_group_id( 1 ) * lh - BORDER );
    const int bstride = lw + 2 * BORDER;
    float2 coord;
    float dx, dy;

    /* Fetch image data to local buffer */
#define BUF( x, y ) buf[ mad24( ( y ) + BORDER, bstride, ( x ) + BORDER ) ]
#define TOGRAY(x) dot( x, GRAY_WEIGHT )

    for( int y = ly; y < lh + 2 * BORDER; y += lh ) {
        for( int x = lx; x < lw + 2 * BORDER; x += lw ) {
            buf[ mad24( y, bstride, x ) ] = ( float2 ) ( TOGRAY( read_imagef( src, sampler, base + ( int2 )( x, y ) ) ), 0.0f );
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );

    for( int y = ly - 1; y < lh + 1; y += lh ) {
        for( int x = lx - 1; x < lw + 1; x += lw ) {

            local float2* iptr = &BUF( x, y );
            float vc           = iptr[ 0 ].x;
            const float dark   = vc - threshold;
            const float bright = vc + threshold;
            int offset;
            float v0, v1;
            uint darkmask = 0;
            uint brightmask = 0;
            float brightscore = 0;
            float darkscore = 0;

            /* perform all test and set the result bit in [bright|dark]mask and also store the difference above the threshold in the respective score */
#define UPDATE_TEST_MASK_SCORE( idx, tempoffset ) do { \
    offset = tempoffset; v0 = iptr[ offset ].x;v1 = iptr[ -offset ].x; \
    darkmask   |= ( ( v0 <= dark   ) << ( idx ) ) | ( ( v1 <= dark   ) << ( idx + 8 ) ); \
    brightmask |= ( ( v0 >= bright ) << ( idx ) ) | ( ( v1 >= bright ) << ( idx + 8 ) ); \
    brightscore += fabs( v0 - vc ) + fabs( v1 - vc ) - 2 * threshold; \
    darkscore   += fabs( vc - v0 ) + fabs( vc - v1 ) - 2 * threshold; \
} while( 0 )

            UPDATE_TEST_MASK_SCORE( 0, 3 );
            if( ( darkmask | brightmask ) == 0  ) continue;

            UPDATE_TEST_MASK_SCORE( 2, mad24( -bstride, 2,  2 ) );
            UPDATE_TEST_MASK_SCORE( 4, mul24( -bstride, 3     ) );
            UPDATE_TEST_MASK_SCORE( 6, mad24( -bstride, 2, -2 ) );

#define EVEN_MASK ( 0x0 | 0x04 | 0x10 | 0x40 )

            if( ( ( brightmask | ( brightmask >> 8 ) ) & EVEN_MASK ) != EVEN_MASK &&
                ( ( darkmask   | ( darkmask   >> 8 ) ) & EVEN_MASK ) != EVEN_MASK ) continue;

            UPDATE_TEST_MASK_SCORE( 1, -bstride + 3 );
            UPDATE_TEST_MASK_SCORE( 3, mad24( -bstride, 3,  1 ) );
            UPDATE_TEST_MASK_SCORE( 5, mad24( -bstride, 3, -1 ) );
            UPDATE_TEST_MASK_SCORE( 7, -bstride - 3 );

            if( ( ( brightmask | ( brightmask >> 8 ) ) & 0xff ) != 0xff &&
                ( ( darkmask   | ( darkmask   >> 8 ) ) & 0xff ) != 0xff ) continue;

            /* copy bits also to higher positions */
            darkmask   |= darkmask << 16;
            brightmask |= brightmask << 16;

            /* perform test */
#define TEST_SEGMENT9( mask, position ) ( int )( ( ( ( mask ) >> ( position ) ) & 0x1ff ) == 0x1ff )

            int testbright = TEST_SEGMENT9( brightmask, 0 );
            testbright |= TEST_SEGMENT9( brightmask, 1 );
            testbright |= TEST_SEGMENT9( brightmask, 2 );
            testbright |= TEST_SEGMENT9( brightmask, 3 );
            testbright |= TEST_SEGMENT9( brightmask, 4 );
            testbright |= TEST_SEGMENT9( brightmask, 5 );
            testbright |= TEST_SEGMENT9( brightmask, 6 );
            testbright |= TEST_SEGMENT9( brightmask, 7 );
            testbright |= TEST_SEGMENT9( brightmask, 8 );
            testbright |= TEST_SEGMENT9( brightmask, 9 );
            testbright |= TEST_SEGMENT9( brightmask, 10 );
            testbright |= TEST_SEGMENT9( brightmask, 11 );
            testbright |= TEST_SEGMENT9( brightmask, 12 );
            testbright |= TEST_SEGMENT9( brightmask, 13 );
            testbright |= TEST_SEGMENT9( brightmask, 14 );
            testbright |= TEST_SEGMENT9( brightmask, 15 );

            int testdark = TEST_SEGMENT9( darkmask, 0 );
            testdark |= TEST_SEGMENT9( darkmask, 1 );
            testdark |= TEST_SEGMENT9( darkmask, 2 );
            testdark |= TEST_SEGMENT9( darkmask, 3 );
            testdark |= TEST_SEGMENT9( darkmask, 4 );
            testdark |= TEST_SEGMENT9( darkmask, 5 );
            testdark |= TEST_SEGMENT9( darkmask, 6 );
            testdark |= TEST_SEGMENT9( darkmask, 7 );
            testdark |= TEST_SEGMENT9( darkmask, 8 );
            testdark |= TEST_SEGMENT9( darkmask, 9 );
            testdark |= TEST_SEGMENT9( darkmask, 10 );
            testdark |= TEST_SEGMENT9( darkmask, 11 );
            testdark |= TEST_SEGMENT9( darkmask, 12 );
            testdark |= TEST_SEGMENT9( darkmask, 13 );
            testdark |= TEST_SEGMENT9( darkmask, 14 );
            testdark |= TEST_SEGMENT9( darkmask, 15 );

            if( !( testdark | testbright ) )
                continue;

            /* we have a corner so we store the score */
            BUF( x, y ).y = fmax( brightscore, darkscore );
        }
    }

    barrier( CLK_LOCAL_MEM_FENCE );

    float I = BUF( lx , ly  ).x;
    float score = BUF( lx, ly ).y;

    if( gx < imgborder || gy < imgborder || gx >= width - imgborder || gy >= height - imgborder || score < 0.01f )
        return;

    /* Perform non-maximum suppression in 3 x 3 window */
#define NMS_TEST( dx, dy ) do { \
    if( score <= BUF( lx + dx, ly + dy ).y ) return; \
} while( 0 )

    NMS_TEST( -1, -1 );
    NMS_TEST(  0, -1 );
    NMS_TEST(  1, -1 );
    NMS_TEST( -1,  0 );
    NMS_TEST(  1,  0 );
    NMS_TEST( -1,  1 );
    NMS_TEST(  0,  1 );
    NMS_TEST(  1,  1 );

    /* Store corner candidates with score - if we get here we have a corner */
    int idx = atomic_inc( feature_size );
    if( idx < features_max ) {
        features[ idx ].pt    = ( int2 ) ( gx, gy );
        features[ idx ].score = score;
    }
}
