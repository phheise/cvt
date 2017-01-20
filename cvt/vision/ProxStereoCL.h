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
#ifndef CL_PROXSTEREOCL_H
#define CL_PROXSTEREOCL_H

#include <cvt/gfx/Image.h>
#include <cvt/cl/CLKernel.h>
#include <cvt/cl/CLPyramid.h>
#include <cvt/vision/ProxPrecondCL.h>

namespace cvt {

    class ProxStereoCL {
        public:
            ProxStereoCL( float scale, int levels, int patchsize, int iterations );
            ~ProxStereoCL();

            void depthmap( Image& output, const Image& image0, const Image& image1, float lambda, float depthmax );

            void depthmapRefine( Image& output, const Image& image0, const Image& image1, const Image& initdisparity, float lambda, float depthmax );

        private:
            void fill( Image& output, const Vector4f& value );
            void pyrUpMul( Image& output, const Image& input, const Vector4f& mul );
            void pyrUp( Image& output, const Image& input );
            void toDisparity( Image& output, const Image& input, float scale );
            void gradient( Image& output, const Image& input );
            void proxStereoWarp( Image& output, const Image& input,
                                const Image& image0, const Image& gradimage0,
                                const Image& image1, const Image& gradimage1 );
            void proxStereo( Image& output, const Image& input,
                             Image& outputp, const Image& inputp,
                             const Image& warp, float lambda, float depthmax, Image& precond );
            void warpImage( Image& output, const Image& input, const Image& source );

            int       _patchsize;
            int       _iterations;
            int       _warpiterations;
            CLKernel  _clproxstereo;
            CLKernel  _clproxstereowarp;
            CLKernel  _clfill;
            CLKernel  _clgrad;
            CLKernel  _clpyrupmul;
            CLKernel  _clpyrup;
            CLKernel  _cldisparity;
            CLKernel  _clwarpimg;
            CLPyramid _clpyr0;
            CLPyramid _clpyr1;
            ProxPrecondCL _precond;
    };

}

#endif
