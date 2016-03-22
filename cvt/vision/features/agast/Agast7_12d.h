/*
   The MIT License (MIT)

   Copyright (c) 2011 - 2013, Philipp Heise and Sebastian Klose

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

#ifndef CVT_AGAST7_12D_H
#define CVT_AGAST7_12D_H

#include <cvt/vision/features/agast/ASTDetector.h>

namespace cvt
{
    class Agast7_12d : public ASTDetector
    {
        public:
            Agast7_12d();
            virtual ~Agast7_12d();

            void detect( const Image& img, uint8_t threshold, FeatureSetWrapper& features, size_t border );

        private:
            ssize_t  _lastStride;
            ssize_t offset0, offset1, offset2, offset3, offset4, offset5, offset6, offset7;
            ssize_t offset8, offset9, offset10, offset11;

            int cornerScore( const uint8_t* p, uint8_t thresh ) const;
            void updateOffsets();
    };
}

#endif
