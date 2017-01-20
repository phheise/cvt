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
#ifndef CVT_BRIEFCL_H
#define CVT_BRIEFCL_H

#include <cvt/gfx/Image.h>
#include <cvt/cl/CLKernel.h>

namespace cvt {
    class BRIEFCL {
        public:
            BRIEFCL();
            ~BRIEFCL();

            void extract( CLBuffer& output, const CLBuffer& features, int size_features, const Image& image );

            void stereoMatch( CLBuffer& matches, int& matches_size, const CLBuffer& features0, const CLBuffer& brief0, int size_features0,
                              const CLBuffer& features1, const CLBuffer& brief1, int size_features1, const CLBuffer& sclidx1, int threshold = 100 );

        private:
            CLKernel _clbrief;
            CLKernel _clbriefstereomatch;
    };
}
#endif
