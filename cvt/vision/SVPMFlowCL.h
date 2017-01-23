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
#ifndef CVT_SVPMFLOWCL_H
#define CVT_SVPMFLOWCL_H

#include <cvt/gfx/Image.h>
#include <cvt/cl/CLKernel.h>

namespace cvt {
    class SVPMFlowCL {
        public:
            SVPMFlowCL();
            ~SVPMFlowCL();

            void flow( Image& flow, const Image& first, const Image& second, int patchsize = 2, float flowmax = 50, Image* gtflow = NULL  );

            void flowPyramid( Image& flow, const Image& first, const Image& second, int patchsize = 2, float flowmax = 50, Image* gtflow = NULL  );

        private:
            void flowInit( Image& flow, const Image& first, const Image& second, const Image& init, int patchsize = 2, float flowmax = 50 );

            void init( Image& output, const Image& first, const Image& second, int patchsize, float flowmax );
            void initFlow( Image& output, const Image& first, const Image& second, const Image& flow, int patchsize, float flowmax );
            void propagate( Image& output, const Image& old, const Image& first, const Image& second, int patchsize, float flowmax, int iter );
            void propagate_tv( Image& output, const Image& old, Image& outputp, const Image& oldp, const Image& first, const Image& second, int patchsize, float flowmax, int iter );
            void toFlow( Image& flow, const Image& buffer );
            void pyrUpFlow( Image& output, int width, int height, const Image& input );
            void pyrDown( Image& output, int width, int height, const Image& input );

            CLKernel _clsvpm_init;
            CLKernel _clsvpm_initflow;
            CLKernel _clsvpm_prop;
            CLKernel _clsvpm_flow;
            CLKernel _clpyrdown;
            CLKernel _clpyrupmul;
    };

}
#endif
