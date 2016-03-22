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

#include <cvt/util/CVTTest.h>
#include <cvt/util/String.h>
#include <iostream>
#include <stdio.h>

namespace cvt {

	BEGIN_CVTTEST( string )
		String str( "Hallo Welt" );
		std::cout << str << std::endl;

		size_t len = str.length();
		for( size_t i = 0; i <= len; i++ )
			printf("0x%0X ", str[ i ] );
		printf("\n");

		std::cout << "Length: " << str.length() << std::endl;


		String substr( str, 0, 5 );
		std::cout << substr << std::endl;

		String substr2( str, 6, 4 );
		std::cout << substr2 << std::endl;
		std::cout << "== : " << ( substr2 == "Welt" ) << std::endl;


		substr += " blub";
		std::cout << substr << std::endl;

		std::cout << "Length: " << substr.length() << std::endl;

		std::cout << "Prefix: " << str.hasPrefix( "Hallo" ) << std::endl;
		std::cout << "Prefix: " << substr.hasPrefix( "Hallo" ) << std::endl;

		std::cout << "Suffix: " << str.hasSuffix( "Welt" ) << std::endl;
		std::cout << "Suffix: " << substr.hasSuffix( "blub" ) << std::endl;

		str.sprintf("%d ", 10 );
		std::cout << str << std::endl;
		std::cout << str.length() << std::endl;

		str += 15.5;

		str.sprintfConcat(" %d %s", 20, "Hallo Welt" );
		std::cout << str << std::endl;
		std::cout << str.length() << std::endl;

		/* find/rfind testing */
		str = "Hallo";
		ssize_t idx = str.find( 'l' );
		std::cout << "l at " << idx << " == " << str[ idx ] << std::endl;
		idx = str.rfind( 'l' );
		std::cout << "l at " << idx << " == " << str[ idx ] << std::endl;
		idx = str.rfind( 'H' );
		std::cout << "H at " << idx << " == " << str[ idx ] << std::endl;
		idx = str.find( 'o' );
		std::cout << "o at " << idx << " == " << str[ idx ] << std::endl;
		idx = str.find( 'l', 3 );
		std::cout << "l at " << idx << " == " << str[ idx ] << std::endl;
		idx = str.rfind( 'l', 3 );
		std::cout << "l at " << idx << " == " << str[ idx ] << std::endl;
		idx = str.find( 'x' );
		if( idx > 0 )
			std::cout << "Error" << std::endl;
		idx = str.rfind( 'x' );
		if( idx > 0 )
			std::cout << "Error" << std::endl;
		idx = str.find( 'H', 10 );
		if( idx > 0 )
			std::cout << "Error" << std::endl;
		idx = str.find( 'H', 2 );
		if( idx > 0 )
			std::cout << "Error" << std::endl;
		idx = str.rfind( 'x', 2 );
		if( idx > 0 )
			std::cout << "Error" << std::endl;


		String a = "a";
		String b = "b";
		std::cout << ( a < b ) << std::endl;

		return true;
	END_CVTTEST

}
