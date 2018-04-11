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
#ifndef CVT_CLWARP_H
#define CVT_CLWARP_H

#include <cvt/cl/CLKernel.h>

namespace cvt {
    class CLDebayer {
        public:
            CLDebayer();
            ~CLDebayer();

            void debayer( Image& dst, const Image& src );
            void debayerHQ( Image& dst, const Image& src );

        private:
            void debayer_RGGB_RGBA( Image& dst, const Image& src );
            void debayer_GBRG_RGBA( Image& dst, const Image& src );
            void debayer_GRBG_RGBA( Image& dst, const Image& src );
            void debayer_BGGR_RGBA( Image& dst, const Image& src );

            void debayerhq_RGGB_RGBA( Image& dst, const Image& src );
            void debayerhq_GBRG_RGBA( Image& dst, const Image& src );
            void debayerhq_GRBG_RGBA( Image& dst, const Image& src );
            void debayerhq_BGGR_RGBA( Image& dst, const Image& src );


            CLKernel _clrggb;
            CLKernel _clgbrg;
            CLKernel _clgrbg;
            CLKernel _clbggr;
            CLKernel _clrggbhq;
            CLKernel _clgbrghq;
            CLKernel _clgrbghq;
            CLKernel _clbggrhq;
    };
}

#endif

