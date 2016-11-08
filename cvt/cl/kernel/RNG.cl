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
#ifndef CVT_RNG_CL
#define CVT_RNG_CL

#import "MWC64X.cl"

float MWC64X_NextFloat(mwc64x_state_t *s)
{
    uint res=s->x ^ s->c;
    MWC64X_Step(s);
    return  2.3283064365386962890625e-10f * ( float ) res;
}

float MWC64X_NextFloatGaussian( mwc64x_state_t* s )
{

    float x1, x2, w;//, y1, y2;
    do {
        x1 = 2.0f * MWC64X_NextFloat( s ) - 1.0f;
        x2 = 2.0f * MWC64X_NextFloat( s ) - 1.0f;
        w = x1 * x1 + x2 * x2;
    } while ( w >= 1.0f || w == 0.0f );

    w = sqrt( (-2.0 * log( w ) ) / w );
    return x1 * w;
    //y2 = x2 * w;
}

typedef mwc64x_state_t RNG;

#define RNG_init( x, linpos, numdraws ) do {\
    MWC64X_SeedStreams( x, 0, 0 ); \
    MWC64X_Skip( x, ( linpos ) * numdraws ); \
} while( 0 )

#define RNG_float( x ) MWC64X_NextFloat( x )

#define RNG_uint( x ) MWC64X_NextUint( x )

#define RNG_float_normal( x ) MWC64X_NextFloatGaussian( x )

#endif
