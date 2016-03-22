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

#include <cvt/vision/KLTPatch.h>
#include <cvt/util/CVTTest.h>
#include <cvt/io/Resources.h>

#include <cvt/math/GA2.h>
#include <cvt/math/SL3.h>
#include <cvt/math/Translation2D.h>

using namespace cvt;


static bool _dumpPatch( const ImagePyramid& gray,
                        const ImagePyramid& gx,
                        const ImagePyramid& gy )
{
    typedef Translation2D<float> PoseType;
    typedef KLTPatch<20, PoseType> Patch;

    Patch p( gray.octaves() );
    p.update( gray, gx, gy, Vector2f( 266, 266 ) );

    Image img;
    String name;
    for( size_t i = 0; i < gray.octaves(); i++ ){
        p.toImage( img, i );
        name.sprintf( "patch_octave_%02d.png", i );
        img.save( name );
    }

    return true;
}

template <size_t Size>
static bool _patchPoints()
{
    typedef Translation2D<float> PoseType;
    typedef KLTPatch<Size, PoseType> Patch;

    int half = Size >> 1;
    Vector2f pos( -half, -half );

    const Vector2f* points = Patch::patchPoints();

    size_t iter = 0;
    for( size_t r = 0; r < Size; r++ ){
        pos.x = -half;
        for( size_t r = 0; r < Size; r++ ){
            if( points[ iter ] != pos ){
                std::cout << "#" << iter << ": " << points[ iter ] << " -> " << pos << std::endl;
                return false;
            }
            pos.x += 1.0f;
            iter++;
        }
        pos.y+= 1.0f;
    }


    return true;
}

template<class PoseType>
static bool _trackPatch( const ImagePyramid& gray,
                         const ImagePyramid& gx,
                         const ImagePyramid& gy )
{
    typedef KLTPatch<16, PoseType> Patch;

    Vector2f truePos( 266, 266 ), estimated;

    Patch p( gray.octaves() );
    if( !p.update( gray, gx, gy, truePos ) ){
        std::cout << "UPDATE AT POSITION FAILED!" << std::endl;
    }

    Vector2f initial( truePos.x + 8, truePos.y - 8 );
    p.initPose( initial );
    p.currentCenter( initial );
    p.align( gray, 5 );
    p.currentCenter( estimated );

    if( ( truePos - estimated ).length() > 1e-1 ){
        std::cout << "Initial: " << initial;
        std::cout << " - True: " << truePos;
        std::cout << " -> Estimated: " << estimated << std::endl;
        return false;
    }

    return true;
}

BEGIN_CVTTEST( KLTPatch )

Resources resources;
String file = resources.find( "lena.png" );
Image lena( file );
Image lenagf;
lena.convert( lenagf, IFormat::GRAY_FLOAT );

ImagePyramid pyrf( 3, 0.5f );
ImagePyramid gx( 3, 0.5f );
ImagePyramid gy( 3, 0.5f );
pyrf.update( lenagf );
IKernel kx( IKernel::HAAR_HORIZONTAL_3 );
IKernel ky( IKernel::HAAR_VERTICAL_3 );
kx.scale( -0.5f );
ky.scale( -0.5f );

pyrf.convolve( gx, kx );
pyrf.convolve( gy, ky );


bool result = true;
bool b = true;

b = _dumpPatch( pyrf, gx, gy );
CVTTEST_PRINT( "Patch Dumping", b );
result &= b;

b = _patchPoints<16>();
CVTTEST_PRINT( "patchPoints<16>", b );
result &= b;

b = _patchPoints<17>();
CVTTEST_PRINT( "patchPoints<17>", b );
result &= b;
b = _patchPoints<20>();
CVTTEST_PRINT( "patchPoints<20>", b );
result &= b;
b = _patchPoints<31>();
CVTTEST_PRINT( "patchPoints<31>", b );
result &= b;

b = _trackPatch<Translation2D<float> >( pyrf, gx, gy );
CVTTEST_PRINT( "2D Translation Test: ", b );
result &= b;

b = _trackPatch<GA2<float> >( pyrf, gx, gy );
CVTTEST_PRINT( "2D General Affine Test: ", b );
result &= b;

return result;

END_CVTTEST
