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
 * File:   SIMDDebug.h
 * Author: sebi / heise
 *
 * Created on July 13, 2011, 12:52 PM
 */

#ifndef CVT_SIMDDEBUG_H
#define	CVT_SIMDDEBUG_H

#include <stdio.h>

namespace cvt {

    typedef union {
        __m128   m128;
        __m128i  m128i;
        float	 f[ 4 ];
        uint8_t  u8[ 16 ];
        int8_t	 i8[ 16 ];
        uint16_t u16[ 8 ];
        int16_t  i16[ 8 ];
        uint32_t u32[ 4 ];
        int32_t  i32[ 4 ];
    } SIMD128;

    static inline void SIMD_print_f( __m128 _x )
    {
		SIMD128 x;
		x.m128 = _x;

        printf( "%f %f %f %f\n", x.f[ 0 ], x.f[ 1 ], x.f[ 2 ], x.f[ 3 ] );
    }

    static inline void SIMD_print_u32hex( __m128i _x )
    {
		SIMD128 x;
		x.m128i = _x;

        printf( "0x%0X 0x%0X 0x%0X 0x%0X\n", x.u32[ 0 ], x.u32[ 1 ], x.u32[ 2 ], x.u32[ 3 ] );
    }

    static inline void SIMD_print_u32( __m128i _x )
    {
		SIMD128 x;
		x.m128i = _x;

        printf( "%u %u %u %u\n", x.u32[ 0 ], x.u32[ 1 ], x.u32[ 2 ], x.u32[ 3 ] );
    }

    static inline void SIMD_print_i32( __m128i _x )
    {
		SIMD128 x;
		x.m128i = _x;

        printf( "%d %d %d %d\n", x.i32[ 0 ], x.i32[ 1 ], x.i32[ 2 ], x.i32[ 3 ] );
    }

    static inline void SIMD_print_u16hex( __m128i _x )
    {
		SIMD128 x;
		x.m128i = _x;

        printf( "0x%0X 0x%0X 0x%0X 0x%0X 0x%0X 0x%0X 0x%0X 0x%0X\n", x.u16[ 0 ], x.u16[ 1 ], x.u16[ 2 ], x.u16[ 3 ],
                x.u16[ 4 ], x.u16[ 5 ], x.u16[ 6 ], x.u16[ 7 ] );
    }

    static inline void SIMD_print_u16( __m128i _x )
    {
		SIMD128 x;
		x.m128i = _x;

        printf( "%u %u %u %u %u %u %u %u\n", x.u16[ 0 ], x.u16[ 1 ], x.u16[ 2 ], x.u16[ 3 ],
                x.u16[ 4 ], x.u16[ 5 ], x.u16[ 6 ], x.u16[ 7 ] );
    }

    static inline void SIMD_print_i16( __m128i _x )
    {
		SIMD128 x;
		x.m128i = _x;

        printf( "%d %d %d %d %d %d %d %d\n", x.i16[ 0 ], x.i16[ 1 ], x.i16[ 2 ], x.i16[ 3 ],
                x.i16[ 4 ], x.i16[ 5 ], x.i16[ 6 ], x.i16[ 7 ] );
    }

    static inline void SIMD_print_i16( const char* str, __m128i _x )
    {
		SIMD128 x;
		x.m128i = _x;

        printf( "%s %d %d %d %d %d %d %d %d\n", str, x.i16[ 0 ], x.i16[ 1 ], x.i16[ 2 ], x.i16[ 3 ],
                x.i16[ 4 ], x.i16[ 5 ], x.i16[ 6 ], x.i16[ 7 ] );
    }

    static inline void SIMD_print_u8hex( __m128i _x )
    {
		SIMD128 x;
		x.m128i = _x;

        printf( "0x%0X 0x%0X 0x%0X 0x%0X 0x%0X 0x%0X 0x%0X 0x%0X 0x%0X 0x%0X 0x%0X 0x%0X 0x%0X 0x%0X 0x%0X 0x%0X\n",
                x.u8[ 0 ], x.u8[ 1 ], x.u8[ 2 ], x.u8[ 3 ],
                x.u8[ 4 ], x.u8[ 5 ], x.u8[ 6 ], x.u8[ 7 ],
                x.u8[ 8 ], x.u8[ 9 ], x.u8[ 10 ], x.u8[ 11 ],
                x.u8[ 12 ], x.u8[ 13 ], x.u8[ 14 ], x.u8[ 15 ] );
    }

    static inline void SIMD_print_u8( __m128i _x )
    {
		SIMD128 x;
		x.m128i = _x;

        printf( "%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u\n",
                x.u8[ 0 ], x.u8[ 1 ], x.u8[ 2 ], x.u8[ 3 ],
                x.u8[ 4 ], x.u8[ 5 ], x.u8[ 6 ], x.u8[ 7 ],
                x.u8[ 8 ], x.u8[ 9 ], x.u8[ 10 ], x.u8[ 11 ],
                x.u8[ 12 ], x.u8[ 13 ], x.u8[ 14 ], x.u8[ 15 ] );
    }

    static inline void SIMD_print_i8( __m128i _x )
    {
		SIMD128 x;
		x.m128i = _x;
        printf( "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                x.i8[ 0 ], x.i8[ 1 ], x.i8[ 2 ], x.i8[ 3 ],
                x.i8[ 4 ], x.i8[ 5 ], x.i8[ 6 ], x.i8[ 7 ],
                x.i8[ 8 ], x.i8[ 9 ], x.i8[ 10 ], x.i8[ 11 ],
                x.i8[ 12 ], x.i8[ 13 ], x.i8[ 14 ], x.i8[ 15 ] );
    }
}

#endif	/* SIMDDEBUG_H */

