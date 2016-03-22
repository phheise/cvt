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
#ifndef CVT_IDECOMPOSE_H
#define CVT_IDECOMPOSE_H

#include <cvt/gfx/Image.h>

namespace cvt {

    class IDecompose {
        public:
            /**
              @brief Decompose RGBA/BGRA image to seperate images containing the single channels
             */
            static void decompose( Image& chan1, Image& chan2, Image& chan3, Image& chan4, const Image& input );

            /**
              @brief Decompose RGBA/BGRA image to seperate images containing the single channels except
                     for the alpha channel which is skipped.
             */
            static void decompose( Image& chan1, Image& chan2, Image& chan3, const Image& input );

            /**
              @brief Decompose GRAY-ALPHA image to sperate image conating the GRAY and the ALPHA channel only
             */
            static void decompose( Image& chan1, Image& chan2, const Image& input );

        private:
            IDecompose();
            ~IDecompose();
    };

}

#endif
