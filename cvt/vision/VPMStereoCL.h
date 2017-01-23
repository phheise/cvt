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
#ifndef CVT_VPMSTEREOCL_H
#define CVT_VPMSTEREOCL_H

#include <cvt/gfx/Image.h>
#include <cvt/cl/CLKernel.h>
#include <cvt/cl/CLPyramid.h>

namespace cvt {

    class VPMStereoCL {
        public:
            VPMStereoCL( int patchsize, int iterations, float disparitymax );
            ~VPMStereoCL();

            void disparitySimple( Image& output, const Image& image0, const Image& image1, const Image* initdisparity = NULL );

        private:
            void fill( Image& output, const Vector4f& value );
            void toDisparity( Image& output, const Image& input, float scale = 1.0f );
            void gradient( Image& output, const Image& input );
            void warpImage( Image& output, const Image& input, const Image& source );
            void init( Image& output,
                      const Image& colimg0, const Image& gradimg0,
                      const Image& colimg1, const Image& gradimg1 );
            void initDisparity( Image& output, const Image& disparity,
                               const Image& colimg0, const Image& gradimg0,
                               const Image& colimg1, const Image& gradimg1 );
            void iterSimple( Image& output, const Image& input,
                            const Image& colimg0, const Image& gradimg0,
                            const Image& colimg1, const Image& gradimg1, int iter );

            int       _patchsize;
            int       _iterations;
            float     _disparitymax;
            CLKernel  _clvpmsinit;
            CLKernel  _clvpmsinitdisparity;
            CLKernel  _clvpmsitersimple;
            CLKernel  _clfill;
            CLKernel  _clgrad;
            CLKernel  _clvpmsdisparity;
            CLKernel  _clvpmswarpimg;
    };

}
#endif
