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

#include <cvt/util/SIMDSSE42.h>

#include <xmmintrin.h>
#include <smmintrin.h>

namespace cvt
{
    /*
	size_t SIMDSSE42::hammingDistance( const uint8_t* src1, const uint8_t* src2, size_t n ) const
    {
        size_t pcount = 0;

        size_t n8 = n >> 3;
        size_t r  = n - ( n8 << 3 );

        while ( n8-- ) {
            pcount += _mm_popcnt_u64( *(uint64_t*)src1 ^ *(uint64_t*)src2 );
            src1 += 8;
            src2 += 8;
        }

        if( r ){
            uint64_t  a = 0, b = 0;

            Memcpy( ( uint8_t* ) &a, src1, r );
            Memcpy( ( uint8_t* ) &b, src2, r );

            pcount += _mm_popcnt_u64( a ^ b );
        }

        return pcount;
    }*/

}
