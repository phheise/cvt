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

typedef struct {
    int2 pt;
    float score;
} Feature;

kernel void features_draw( write_only image2d_t output, global Feature* features, int size )
{
    const int width = get_image_width( output );
    const int height = get_image_height( output );
    const int id = get_global_id( 0 );

    if( id >= size )
        return;

    int2 pt = features[ id ].pt;

    if( pt.x < 2 || pt.x + 2 >= width || pt.y < 2 || pt.y + 2 >= height )
        return;

    for( int i=-2; i <= 2; i++ ) {
        write_imagef( output, ( int2 )( pt.x + i, pt.y      ), ( float4 ) ( 0.0f, 1.0f, 0.0f, 1.0f ) );
        write_imagef( output, ( int2 )( pt.x    , pt.y + i  ), ( float4 ) ( 0.0f, 1.0f, 0.0f, 1.0f ) );
    }
}
