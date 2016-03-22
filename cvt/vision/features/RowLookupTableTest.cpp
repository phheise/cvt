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

#include <cvt/vision/features/RowLookupTable.h>
#include <cvt/vision/features/ORB.h>
#include <cvt/util/CVTTest.h>

using namespace cvt;
bool _featureSetTest()
{
	FeatureSet fset;
	Feature f;
	for( size_t i = 0; i < 5; i++ ){
			f.pt.y = 2 * i;
		for( size_t k = 0; k < 5; k++ ){
			f.pt.x = 2*k;
			fset.add( f );
		}
	}

	f.pt.y = 100;
	fset.add( f );

	RowLookupTable rlt( fset );

	bool b = true, result = true;

	b &= rlt.isValidRow( 0 );
	b &= rlt.isValidRow( 2 );
	b &= rlt.isValidRow( 4 );
	b &= rlt.isValidRow( 6 );
	b &= rlt.isValidRow( 8 );
	b &= rlt.isValidRow( 100 );
	b &= !rlt.isValidRow( 1 );
	b &= !rlt.isValidRow( 3 );
	b &= !rlt.isValidRow( 7 );
	b &= !rlt.isValidRow( 9 );
	b &= !rlt.isValidRow( 10 );
	b &= !rlt.isValidRow( -1 );
	if( !b ){
		CVTTEST_PRINT( "isValidRow()", b );
	}
	result &= b;

	b &= rlt.row( 0 ).start == 0;
	b &= rlt.row( 2 ).start == 5;
	b &= rlt.row( 4 ).start == 10;
	b &= rlt.row( 6 ).start == 15;
	b &= rlt.row( 8 ).start == 20;
	b &= rlt.row( 100 ).start == 25;
	b &= rlt.row( 0 ).len == 5;
	b &= rlt.row( 2 ).len == 5;
	b &= rlt.row( 4 ).len == 5;
	b &= rlt.row( 6 ).len == 5;
	b &= rlt.row( 8 ).len == 5;
	b &= rlt.row( 100 ).len == 1;

	if( !b ){
		CVTTEST_PRINT( "Row content", b );
	}
	result &= b;

	return result;
}

BEGIN_CVTTEST( RowLookupTable )

	bool testResult = true;
	bool b = false;

	b = _featureSetTest();
	CVTTEST_PRINT( "FeatureSet -> RowLookupTable", b );
	testResult &= b;

	return testResult;
END_CVTTEST
