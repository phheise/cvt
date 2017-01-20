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
#include <cvt/vision/HarrisCL.h>
#include <cvt/cl/kernel/Harris.h>

namespace cvt {

    HarrisCL::HarrisCL() :
        _clharriscolor( _Harris_source, "harris_color" )
    {
    }

    HarrisCL::~HarrisCL()
    {
    }

    void HarrisCL::detect( Image& output, const Image& input )
    {
        output.reallocate( input.width(), input.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );

        _clharriscolor.setArg( 0, output );
        _clharriscolor.setArg( 1, input );
        _clharriscolor.setArg( 2, CLLocalSpace( sizeof( cl_float2 ) * ( 16 + 4 * 2 ) * ( 16 + 4 * 2 ) ) );
        _clharriscolor.runWait( CLNDRange( Math::pad( input.width(), 16 ), Math::pad( input.height(), 16 ) ), CLNDRange( 16, 16 ) );
    }

}
