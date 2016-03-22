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

#include <cvt/vision/CameraCalibration.h>
#include <cvt/util/CVTTest.h>

#include "cvt/io/xml/XMLDocument.h"

namespace cvt {

BEGIN_CVTTEST( CameraCalibration )

bool result = true;
bool b;

CameraCalibration cc;

b = ( cc.hasExtrinsics() == false );

Matrix4f ext;
ext.setIdentity();
ext.setRotationXYZ( 0.4f, 0.2f, -2.7f );
ext.setTranslation( 10.5f, 20.6f, -342.7f );
cc.setExtrinsics( ext );

b &= ( cc.hasExtrinsics() == true );
result &= b;
CVTTEST_PRINT( "hasExtrinsics()", b );

b = ( cc.hasIntrinsics() == false );
Matrix3f K( 245.4f, 0.4f, 233.f, 0.0f, 233.0f, 342.0f, 0.0f, 0.0f, 1.0f );
cc.setIntrinsics( K );
b &= ( cc.hasIntrinsics() == true );
result &= b;
CVTTEST_PRINT( "hasIntrinsics()", b );

b = ( cc.hasDistortion() == false );
CVTTEST_PRINT( "hasDistortion()", b );
result &= b;

b = ( ext == cc.extrinsics() );
CVTTEST_PRINT( "setExtrinsics()", b );
result &= b;

b = ( K == cc.intrinsics() );
CVTTEST_PRINT( "setIntrinsics()", b );
result &= b;

cc.setWidth( 640 );
cc.setHeight( 480 );

cc.save( "camcalibtest.xml" );


CameraCalibration cc2;
cc2.load( "camcalibtest.xml" );

b  = ( cc.intrinsics() == cc2.intrinsics() );
b &= ( cc.extrinsics() == cc2.extrinsics() );
b &= ( cc.radialDistortion() == cc2.radialDistortion() );
b &= ( cc.tangentialDistortion() == cc2.tangentialDistortion() );
b &= ( cc.width() == cc2.width() );
b &= ( cc.height() == cc2.height() );

CVTTEST_PRINT( "serialize/deserialize", b );



return result;

END_CVTTEST

}
