/*
   The MIT License (MIT)

   Copyright (c) 2011 - 2014, Philipp Heise and Sebastian Klose
   Copyright (c) 2014, Brian Jensen <Jensen.J.Brian@gmail.com>

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

#ifndef CVT_CPU_H
#define CVT_CPU_H
#include <iostream>
#include <stdint.h>
#include <cvt/util/Flags.h>

namespace cvt {
	enum CPUFeatureFlags {
		CPU_BASE   =  0,
		CPU_MMX    = ( 1 << 1 ),
		CPU_SSE    = ( 1 << 2 ),
		CPU_SSE2   = ( 1 << 3 ),
		CPU_SSE3   = ( 1 << 4 ),
		CPU_SSSE3  = ( 1 << 5 ),
		CPU_SSE4_1 = ( 1 << 6 ),
		CPU_SSE4_2 = ( 1 << 7 ),
		CPU_AVX    = ( 1 << 8 ),
	};

	CVT_ENUM_TO_FLAGS( CPUFeatureFlags, CPUFeatures )

	static inline CPUFeatures cpuFeatures( void )
	{
		CPUFeatures ret = CPU_BASE;
		uint32_t eax, ebx, ecx, edx;

#ifdef ARCH_x86_64
		/* FIXME: what a clusterfuck - this works only for x86_64, we need to save ebx on x86
				  BUT cpuid on x86_64 destroys rbx and not only ebx
		 */
		asm volatile(
			"movl $1, %%eax;\n\t"
			"cpuid;\n\t"
				: "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
				:
				:
			);
#elif ARCH_x86
		asm volatile(
			"movl %%ebx, %%esi;\n\t"
			"movl $1, %%eax;\n\t"
			"cpuid;\n\t"
			"xchgl %%ebx, %%esi;\n\t"
				: "=a"(eax), "=S"(ebx), "=c"(ecx), "=d"(edx)
				:
				:
			);

#else
		eax = ebx = ecx = edx = 0;
#endif

		if( edx & ( 1 << 23 ) )
			ret |= CPU_MMX;
		if( edx & ( 1 << 25 ) )
			ret |= CPU_SSE;
		if( edx & ( 1 << 26 ) )
			ret |= CPU_SSE2;
		if( ecx & ( 1 <<  0 ) )
			ret |= CPU_SSE3;
		if( ecx & ( 1 <<  0 ) )
			ret |= CPU_SSE3;
		if( ecx & ( 1 <<  9 ) )
			ret |= CPU_SSSE3;
		if( ecx & ( 1 << 19 ) )
			ret |= CPU_SSE4_1;
		if( ecx & ( 1 << 20 ) )
			ret |= CPU_SSE4_2;
		if( ecx & ( 1 << 28 ) )
			ret |= CPU_AVX;
		return ret;
	}

	static inline void cpuFeaturesPrint()
	{
		CPUFeatures f = cpuFeatures();

		std::cout << "CPU features: ";
		if( f & CPU_MMX )
			std::cout << "MMX ";
		if( f & CPU_SSE )
			std::cout << "SSE ";
		if( f & CPU_SSE2 )
			std::cout << "SSE2 ";
		if( f & CPU_SSE3 )
			std::cout << "SSE3 ";
		if( f & CPU_SSSE3 )
			std::cout << "SSSE3 ";
		if( f & CPU_SSE4_1 )
			std::cout << "SSE4.1 ";
		if( f & CPU_SSE4_2 )
			std::cout << "SSE4.2 ";
		if( f & CPU_AVX )
			std::cout << "AVX ";
		std::cout << std::endl;
	}

    static inline bool isLittleEndian()
    {
        typedef union
        {
            int i;
            char c[ sizeof( int ) ];
        } endian_t;
        static const endian_t val = { 1 };
        return val.c[ 0 ] == 1;
    }

    static inline bool isBigEndian()
    {
        return !isLittleEndian();
    }
}

#endif
