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

#ifndef CVTTEST_H
#define CVTTEST_H

#include <iostream>

typedef bool (*CVTTestFunc)( void );

typedef struct {
    CVTTestFunc func;
    const char* name;
} CVTTest;


#define BEGIN_CVTTEST(x) extern "C" { bool x##_test( void ) {
#define END_CVTTEST } }

#define CVTTEST_PRINT( str, result  )	do { char c = 0x1B; \
							if( ( result ) ) \
								std::cerr << str << " : " << c << "[1;32mOK" << c << "[0;m" << std::endl; \
							else \
								std::cerr << str << " : " << c <<"[1;31mFAILED" << c << "[0;m" << std::endl; \
							} while( 0 )

#define CVTTEST_LOG( str)	do { \
								std::cerr << str << std::endl; \
							} while( 0 )

#endif
