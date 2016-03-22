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

#include <cvt/math/Complex.h>
#include <cvt/util/CVTTest.h>

using namespace cvt;

BEGIN_CVTTEST( Complex )
	bool b = true;
	Complexf c1( 1.0f, 2.0f );
	Complexf c2( 3.0f, 4.0f );

	b &= ( c1 + c2 ) == Complexf( 4.0f, 6.0f );
	CVTTEST_PRINT( "+", b );

	b &= ( c1 - c2 ) == Complexf( -2.0f, -2.0f );
	CVTTEST_PRINT( "-", b );

	b &= ( c1 * c2 ) == Complexf( -5.0f, 10.0f );
	CVTTEST_PRINT( "*", b );

	b &= ( c1 / c2 ) == Complexf( 11.0f / 25.0f, 2.0f / 25.0f );
	CVTTEST_PRINT( "/", b );

	Complexf ctmp = c1;
	b &= ( ctmp += c2 ) == Complexf( 4.0f, 6.0f );
	CVTTEST_PRINT( "+=", b );

	ctmp = c1;
	b &= ( ctmp -= c2 ) == Complexf( -2.0f, -2.0f );
	CVTTEST_PRINT( "-=", b );

	ctmp = c1;
	b &= ( ctmp *= c2 ) == Complexf( -5.0f, 10.0f );
	CVTTEST_PRINT( "*=", b );

	ctmp = c1;
	b &= ( ctmp /= c2 ) == Complexf( 11.0f / 25.0f, 2.0f / 25.0f );
	CVTTEST_PRINT( "/=", b );


	b &= ( c1 + 3.0f ) == Complexf( 4.0f, 2.0f );
	CVTTEST_PRINT( "+", b );

	b &= ( c1 - 3.0f ) == Complexf( -2.0f, 2.0f );
	CVTTEST_PRINT( "-", b );

	b &= ( c1 * 3.0f ) == Complexf( 3.0f, 6.0f );
	CVTTEST_PRINT( "*", b );

	b &= ( c1 / 3.0f ) == Complexf( 1.0f / 3.0f, 2.0f / 3.0f );
	CVTTEST_PRINT( "/", b );

	ctmp = c1;
	b &= ( ctmp += 3.0f ) == Complexf( 4.0f, 2.0f );
	CVTTEST_PRINT( "+=", b );

	ctmp = c1;
	b &= ( ctmp -= 3.0f ) == Complexf( -2.0f, 2.0f );
	CVTTEST_PRINT( "-=", b );

	ctmp = c1;
	b &= ( ctmp *= 3.0f ) == Complexf( 3.0f, 6.0f );
	CVTTEST_PRINT( "*=", b );

	ctmp = c1;
	b &= ( ctmp /= 3.0f ) == Complexf( 1.0f / 3.0f, 2.0f / 3.0f );
	CVTTEST_PRINT( "/=", b );

	b &= c1.conj() == Complexf( c1.re, - c1.im );
	CVTTEST_PRINT( "conj()", b );

	return b;

END_CVTTEST
