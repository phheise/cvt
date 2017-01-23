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

static inline void assignchecked( float2* out, const float2* in1, const float2* in2, const float2* in3 )
{
    if( in1->x <= 0 ) {
        if( in2->x <= 0 ) {
            *out = *in3; // maybe less than zero
        } else {
            *out = *in2;
        }
    } else {
        *out = *in1;
    }
}

const sampler_t SAMPLER_NN = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__kernel void pyrdownchecked( __write_only image2d_t out,  __read_only image2d_t in )
{
    const int gx = get_global_id( 0 );
    const int gy = get_global_id( 1 );
    const int width = get_image_width( in );
    const int height = get_image_height( in );
    const int dstwidth = get_image_width( out );
    const int dstheight = get_image_height( out );
    float incx = ( float ) width / ( float ) dstwidth;
    float incy = ( float ) height / ( float ) dstheight;
    float2 coord;
    int2 icoord;

    if( gx >= dstwidth || gy >= dstheight )
        return;

    coord.x = ( incx * ( gx + 0.5f ) ) - 0.5f;
    coord.y = ( incy * ( gy + 0.5f ) ) - 0.5f;

    float2 fcoord= floor( coord );
    icoord = ( int2 ) ( fcoord.x, fcoord.y );

    float2 val1 = read_imagef( in, SAMPLER_NN, icoord ).xy;
    float2 val2 = read_imagef( in, SAMPLER_NN, icoord + ( int2 ) ( 1, 0 ) ).xy;
    float2 val3 = read_imagef( in, SAMPLER_NN, icoord + ( int2 ) ( 0, 1 ) ).xy;
    float2 val4 = read_imagef( in, SAMPLER_NN, icoord + ( int2 ) ( 1, 1 ) ).xy;

    float2 v1 = val1;
    float2 v2 = val2;
    float2 v3 = val3;
    float2 v4 = val4;

    if( v1.x <= 0 ) assignchecked( &v1, &val2, &val3, &val4 );
    if( v2.x <= 0 ) assignchecked( &v2, &val1, &val4, &val3 );
    if( v3.x <= 0 ) assignchecked( &v3, &val4, &val1, &val2 );
    if( v4.x <= 0 ) assignchecked( &v4, &val3, &val2, &val1 );

    float2 alpha = coord - fcoord;

    float2 val = mix( mix( v1, v2, alpha.x ), mix( v3, v4, alpha.x ), alpha.y );

    write_imagef( out, ( int2 )( gx, gy ), ( float4 ) ( val.x, val.y, 0.0f, 1.0f ) );
}
