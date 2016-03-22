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

#include <cvt/math/Polynomial.h>
#include <cvt/util/CVTTest.h>


using namespace cvt;

BEGIN_CVTTEST( Polynomial )
	bool b = true;
	Polynomialf p1( 2.0f, 1.0f );
	Polynomialf p2( 3.0f, 2.0f, 1.0f );

	Polynomialf tmp = p1 + p1;
	b &= tmp == 2.0f * p1;
	CVTTEST_PRINT( "+", b );
	CVTTEST_PRINT( "*", b );

	tmp = -1.0f * p1;
	b &= tmp == -p1;
	CVTTEST_PRINT( "-", b );

	tmp = p1 - p1;
	b &= tmp.degree() == 0;
	CVTTEST_PRINT( "-", b );

	Polynomialf tmp2;
	tmp2.setZero();
	b &= tmp == tmp2;
	CVTTEST_PRINT( "setZero()", b );

{
	std::vector<Complexf> roots;
	Polynomialf poly( 1.0f, -10.0f, 35.0f, -50.0f, 24.0f );
	poly.roots( roots );
	for( size_t i = 0; i < roots.size(); i++ )
		std::cout << roots[ i ] << std::endl;
}

{
	std::vector<Complexd> roots;
	Polynomiald poly( 1.0, -10.0, 35.0, -50.0, 24.0 );
	poly.roots( roots );
	for( size_t i = 0; i < roots.size(); i++ )
		std::cout << roots[ i ] << std::endl;
}

	return b;
END_CVTTEST
