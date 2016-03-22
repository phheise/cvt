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

#include <cvt/vision/ORB.h>
#include <cvt/util/CVTTest.h>
#include <cvt/io/Resources.h>
#include <cvt/math/Matrix.h>

namespace cvt {

    bool _centroidAngleTest()
    {
        cvt::Resources r;

        bool result = true;

        for( size_t i = 0; i < 360; i+=5 ){
            String fileIn( "bw" );
            fileIn += i;
            fileIn += ".png";
            //std::cout << "File : " << fileIn << std::endl;
            cvt::Image _img( r.find( fileIn ) );

            cvt::Image img;
            _img.convert( img, IFormat::GRAY_UINT8 );
            cvt::ORB orb( img, 1, 0.5f, 50 );

            cvt::Image ii;
            img.integralImage( ii );

            size_t stride;
            float * ptr = ii.map<float>( &stride );

            cvt::ORBFeature feature( img.width() / 2.0f, img.height() / 2.0f );

            orb.centroidAngle( feature, ptr, stride );

            //std::cout << "Feature Angle: " << Math::rad2Deg( feature.angle ) << std::endl;

            ii.unmap<float>( ptr );

        }

        return result;
    }
}

BEGIN_CVTTEST( orb )

	bool tmp, result = true;

	tmp = cvt::_centroidAngleTest();
	CVTTEST_PRINT( "centroidAngle(...)", tmp );
	result &= tmp;

	return result;

END_CVTTEST