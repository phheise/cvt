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

#include <cvt/cl/CLWarp.h>
#include <cvt/cl/kernel/warp.h>

namespace cvt {
#define KX 32
#define KY 32

    CLWarp::CLWarp() :
        _clwarp( _warp_source, "warp" )
    {
    }

    CLWarp::~CLWarp()
    {
    }

    void CLWarp::warp( Image& dst, const Image& src, const Image& warp )
    {
        dst.reallocate( src, IALLOCATOR_CL );

        _clwarp.setArg( 0, dst );
        _clwarp.setArg( 1, src );
        _clwarp.setArg( 2, warp );
        _clwarp.run( CLNDRange( Math::pad( src.width(), KX ), Math::pad( src.height(), KY ) ), CLNDRange( KX, KY ) );
    }
}

