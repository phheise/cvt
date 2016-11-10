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

#ifndef CVT_BHSTEREO_H
#define CVT_BHSTEREO_H

#include <cvt/gfx/Image.h>
#include <cvt/cl/CLKernel.h>

namespace cvt {

    class BHStereo
    {
        public:
            BHStereo( const Image& image, size_t bits, size_t numhash );

            void match( Image& disparity, const Image& image, size_t max_disparity, bool median = false ) const;

        private:
            void brief( CLBuffer& brief, const Image& image );

            void clear( CLBuffer& buckets, size_t size );
            void count( CLBuffer& buckets, size_t bits, size_t numhash, CLBuffer& brief, size_t width, size_t height );
            void calcoffsets( CLBuffer& buckets, size_t size, size_t height );
            void hashmap( CLBuffer& map, CLBuffer& buckets, size_t bits, size_t numhash, CLBuffer& bindesc, size_t width, size_t height );
            void median3( Image& out, const Image& in ) const;

            CLKernel _clbrief;
            CLKernel _clhashclear;
            CLKernel _clhashcount;
            CLKernel _clhashcalcoffset;
            CLKernel _clhashmap;
            CLKernel _clhashmatch;
            CLKernel _clmedian3;

            CLBuffer _clbuckets;
            CLBuffer _clbindesc;
            CLBuffer _clmap;

            size_t   _bits;
            size_t   _numhash;
    };

}

#endif
