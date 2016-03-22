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


#include <cvt/vision/ImagePyramid.h>
#include <cvt/util/CVTTest.h>
#include <cvt/io/Resources.h>
#include <cvt/gfx/IScaleFilter.h>

using namespace cvt;

static bool _filterTest( const cvt::Image& img, const cvt::IScaleFilter& filter )
{
    try {
        cvt::ImagePyramid pyr( 3, 0.5f );
        pyr.update( img, filter );
    } catch( const cvt::Exception& e ){
        return false;
    }

    return true;
}

static bool _convertTest( const cvt::Image& img )
{
    try {
        cvt::ImagePyramid pyr( 3, 0.8f );
        cvt::ImagePyramid pyrf( pyr.octaves(), pyr.scaleFactor() );
        pyr.update( img );
        pyr.convert( pyrf, IFormat::GRAY_FLOAT );
    } catch( const cvt::Exception& e ){
        return false;
    }

    return true;
}

static void _gradX( const Image& in, Image& out )
{
    out.reallocate( in );
    in.convolve( out, IKernel::HAAR_HORIZONTAL_3 );
}

static bool _funcTest( const cvt::Image& img )
{
    cvt::ImagePyramid pyr0( 10, 0.7f );
    cvt::ImagePyramid pyr1( pyr0.octaves(), pyr0.scaleFactor() );

    pyr0.update( img );
    pyr0.apply( pyr1, _gradX );

    pyr0.saveCombined( "lena_comb.png", Color::WHITE );

    return true;
}

BEGIN_CVTTEST( ImagePyramid )

cvt::Resources resources;
cvt::String file = resources.find( "lena.png" );
cvt::Image lena( file );
cvt::Image lenagf;

lena.convert( lenagf, IFormat::GRAY_FLOAT );

bool result = true;
bool b = true;

b = _filterTest( lena, cvt::IScaleFilterCubic() );
CVTTEST_PRINT( "ScaleFilterCubic", b );
result &= b;

b = _filterTest( lena, cvt::IScaleFilterBlackmanHarris( 5.0f ) );
CVTTEST_PRINT( "ScaleFilterBlackmanHarris", b );
result &= b;

b = _filterTest( lena, cvt::IScaleFilterGauss( 3.0f ) );
CVTTEST_PRINT( "ScaleFilterGauss", b );
result &= b;

b = _funcTest( lenagf );
CVTTEST_PRINT( "apply(...)", b );
result &= b;

return result;

END_CVTTEST
