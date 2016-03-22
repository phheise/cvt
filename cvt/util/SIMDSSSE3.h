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

/*
 * File:   SIMDSSSE3.h
 * Author: sebi
 *
 * Created on August 1, 2011, 8:48 AM
 */

#ifndef CVT_SIMDSSSE3_H
#define	CVT_SIMDSSSE3_H

#include <cvt/util/SIMDSSE3.h>

namespace cvt {

    class SIMDSSSE3 : public SIMDSSE3 {

		friend class SIMD;

      protected:
        SIMDSSSE3() {}

      public:
		virtual void Conv_XYZAu8_to_ZYXAu8( uint8_t* dst, uint8_t const* src, const size_t n ) const;
		virtual size_t hammingDistance(const uint8_t* src1, const uint8_t* src2, size_t n) const;

        /* binary swap routines / endian conversion */
        virtual void BSwap16( uint16_t* dst, const uint16_t* src, size_t size ) const;
        virtual void BSwap32( uint32_t* dst, const uint32_t* src, size_t size ) const;
        virtual void BSwap64( uint64_t* dst, const uint64_t* src, size_t size ) const;

        virtual std::string name() const;

        virtual SIMDType type() const;

    };

    inline std::string SIMDSSSE3::name() const
	{
		return "SIMD-SSSE3";
	}

	inline SIMDType SIMDSSSE3::type() const
	{
		return SIMD_SSSE3;
	}

}

#endif	/* SIMDSSSE3_H */

