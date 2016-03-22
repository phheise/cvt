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

#include <cvt/vision/rgbdvo/InformationSelection.h>
#include <Eigen/Core>

#include <vector>

namespace cvt
{
    bool testJ6()
    {
        typedef Eigen::Matrix<float, 6, 1> JacType;

        std::vector<JacType> all( 100 );

        for( size_t x = 0; x < all.size(); x++ ){
            all[ x ].fill( x );
        }

        size_t n = 30;
        InformationSelection<JacType> selector( n );

        std::vector<size_t> expectedIds;
        for( size_t i = 0; i < n; i++ ){
            expectedIds.push_back( all.size() - n + i );
        }

        const std::set<size_t>& ids = selector.selectInformation( &all[ 0 ], all.size() );

        std::set<size_t>::const_iterator it = ids.begin();
        const std::set<size_t>::const_iterator itEnd = ids.end();
        int i = 0;

        while( it != itEnd ){
            if( expectedIds[ i++ ] != *it ) return false;
            it++;
        }

        return true;
    }

BEGIN_CVTTEST( RGBDInfoSelection )

bool result = true;
bool b;

b = testJ6();
CVTTEST_PRINT( "incremental data", b );
result &= b;

return result;

END_CVTTEST

}



