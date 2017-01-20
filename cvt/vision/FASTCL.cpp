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

#include <cvt/vision/FASTCL.h>
#include <cvt/vision/FeatureCL.h>

#include <cvt/cl/kernel/FAST.h>

namespace cvt {

    FASTCL::FASTCL() :
        _clfast9color( _FAST_source, "fast9_color" ),
        _cldrawfeatures( _FAST_source, "features_draw" )
    {
    }

    FASTCL::~FASTCL()
    {
    }

    void FASTCL::detect( CLBuffer& output, int& size, const Image& input, float threshold, int border )
    {
        size = 0;
        CLBuffer bufidx( &size, sizeof( cl_int ) );

        _clfast9color.setArg( 0, output );
        _clfast9color.setArg( 1, bufidx );
        _clfast9color.setArg( 2, ( int ) ( output.size() / ( sizeof( FeatureCL ) ) ) );
        _clfast9color.setArg( 3, input );
        _clfast9color.setArg( 4, threshold );
        _clfast9color.setArg( 5, border );
        _clfast9color.setArg( 6, CLLocalSpace( sizeof( cl_float2 ) * ( 16 + 4 * 2 ) * ( 16 + 4 * 2 ) ) );
        _clfast9color.runWait( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );

        bufidx.read( &size );
        size = Math::min( size, ( int ) ( output.size() / ( sizeof( FeatureCL ) ) ) );
    }


    void FASTCL::draw( Image& output, const CLBuffer& features, int numfeatures )
    {
        _cldrawfeatures.setArg( 0, output );
        _cldrawfeatures.setArg( 1, features );
        _cldrawfeatures.setArg( 2, numfeatures );
        _cldrawfeatures.run( CLNDRange( Math::pad( numfeatures, 256 ) ), CLNDRange( 256 ) );
    }

}
