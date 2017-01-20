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
#ifndef CVT_GAUSSIANCL_H
#define CVT_GAUSSIANCL_H

#include <cvt/gfx/Image.h>
#include <cvt/cl/CLKernel.h>
#include <cvt/cl/kernel/gaussian.h>

namespace cvt {

    class GaussianCL {
        public:
            GaussianCL() :
                _clgaussh( _gaussian_source, "gaussian_horizontal" ),
                _clgaussv( _gaussian_source, "gaussian_vertical" )
            {}
            ~GaussianCL() {}

            void blur( Image& dst, const Image& source, float sigmah, float sigmav );

        private:
            void blurh( Image& dst, const Image& source, const CLBuffer& kern, size_t ksize );
            void blurv( Image& dst, const Image& source, const CLBuffer& kern, size_t ksize );

            CLKernel _clgaussh;
            CLKernel _clgaussv;
    };

    inline void GaussianCL::blur( Image& dst, const Image& source, float sigmah, float sigmav )
    {
        IKernel kh( IKernel::createGaussian1D( sigmah, false ) );
        IKernel kv( IKernel::createGaussian1D( sigmav, true ) );

        CLBuffer clkh( ( void* ) kh.ptr(), sizeof( float ) * kh.width(), CL_MEM_READ_ONLY );
        CLBuffer clkv( ( void* ) kv.ptr(), sizeof( float ) * kv.height(), CL_MEM_READ_ONLY );

        blurh( dst, source, clkh, kh.width() );
        blurv( dst, dst, clkv, kv.height() );
    }

    inline void GaussianCL::blurh( Image& dst, const Image& source, const CLBuffer& kern, size_t ksize )
    {
#define KX 16
#define KY 16
        //dst.reallocate( source, IALLOCATOR_CL );

        _clgaussh.setArg( 0, dst );
        _clgaussh.setArg( 1, source );
        _clgaussh.setArg( 2, kern );
        _clgaussh.setArg<cl_int>( 3, ksize );
        _clgaussh.setArg( 4, CLLocalSpace( sizeof( cl_float4 ) * ( KX + ksize - 1 ) * KY ) );
        _clgaussh.run( CLNDRange( Math::pad( source.width(), KX ), Math::pad( source.height(), KY ) ), CLNDRange( KX, KY ) );
#undef KX
#undef KY
    }


    inline void GaussianCL::blurv( Image& dst, const Image& source, const CLBuffer& kern, size_t ksize )
    {
#define KX 16
#define KY 16
        //        dst.reallocate( source, IALLOCATOR_CL );

        _clgaussv.setArg( 0, dst );
        _clgaussv.setArg( 1, source );
        _clgaussv.setArg( 2, kern );
        _clgaussv.setArg<cl_int>( 3, ksize );
        _clgaussv.setArg( 4, CLLocalSpace( sizeof( cl_float4 ) * ( KY + ksize - 1 ) * KX ) );
        _clgaussv.run( CLNDRange( Math::pad( source.width(), KX ), Math::pad( source.height(), KY ) ), CLNDRange( KX, KY ) );
#undef KX
#undef KY
    }
}

#endif
