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

#ifndef SIMDSSE42_H
#define SIMDSSE42_H

#include <cvt/util/SIMDSSE41.h>

namespace cvt {

	class SIMDSSE42 : public SIMDSSE41 {
		friend class SIMD;

		protected:
			SIMDSSE42()	{}

		public:
//			virtual size_t hammingDistance(const uint8_t* src1, const uint8_t* src2, size_t n) const;

			virtual std::string name() const;
			virtual SIMDType type() const;
	};

    inline std::string SIMDSSE42::name() const {
        return "SIMD-SSE42";
    }

    inline SIMDType SIMDSSE42::type() const {
        return SIMD_SSE42;
    }
}

#endif
