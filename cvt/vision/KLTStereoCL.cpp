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
#include <cvt/vision/KLTStereoCL.h>
#include <cvt/util/Exception.h>
#include <cvt/cl/CLBuffer.h>
#include <cvt/cl/kernel/KLT_stereo.h>

#define KX 32
#define KY 32

namespace cvt {

    KLTStereoCL::KLTStereoCL(  ) :
        _clkltai( _KLT_stereo_source, "klt_stereo_ai" )
    {
    }

    void KLTStereoCL::klt_ai( Image& disparity, const Image& left, const Image& right, const Image& disparityin, const int patchradius, const int itermax ) const
    {
        disparity.reallocate( disparityin.width(), disparityin.height(), disparityin.format(), IALLOCATOR_CL );
        _clkltai.setArg( 0, disparity );
        _clkltai.setArg( 1, left );
        _clkltai.setArg( 2, right );
        _clkltai.setArg( 3, disparityin );
        _clkltai.setArg<int>( 4, patchradius );
        _clkltai.setArg<int>( 5, itermax );
        _clkltai.runWait( CLNDRange( Math::pad( disparityin.width(), KX ), Math::pad( disparityin.height(), KY ) ), CLNDRange( KX, KY ) );
    }
}
