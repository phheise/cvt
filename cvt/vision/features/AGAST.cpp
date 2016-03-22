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

#include <cvt/vision/features/AGAST.h>

#include <cvt/vision/features/agast/OAST9_16.h>
#include <cvt/vision/features/agast/Agast5_8.h>
#include <cvt/vision/features/agast/Agast7_12d.h>
#include <cvt/vision/features/agast/Agast7_12s.h>

namespace cvt
{
    AGAST::AGAST( ASTType astType, uint8_t threshold, size_t border ) :
        _astType( astType ),
        _astDetector( 0 ),
        _threshold( threshold ),
        _border( border )
    {
        switch( _astType ){
            case AGAST_5_8:   _astDetector = new Agast5_8();   break;
            case AGAST_7_12d: _astDetector = new Agast7_12d(); break;
            case AGAST_7_12s: _astDetector = new Agast7_12s(); break;
            case OAST_9_16:   _astDetector = new OAST9_16();   break;
            default: throw CVTException( "unkown AST Type for AGAST!" );
        }
    }

    AGAST::~AGAST()
    {
    }

    void AGAST::detect( FeatureSet& features, const Image& img )
    {
        if( img.format() != IFormat::GRAY_UINT8 )
            throw CVTException( "Input Image format must be GRAY_UINT8" );

        FeatureSetWrapper fset( features );
        _astDetector->detect( img, _threshold, fset, _border );
    }

    void AGAST::detect( FeatureSet& featureSet, const ImagePyramid& imgpyr )
    {
        if( imgpyr[ 0 ].format() != IFormat::GRAY_UINT8 )
            throw CVTException( "Input Image format must be GRAY_UINT8" );

        for( size_t coctave = 0; coctave < imgpyr.octaves(); coctave++ ) {
            float cscale = Math::pow( imgpyr.scaleFactor(), -( float )coctave );
            FeatureSetWrapper features( featureSet, cscale, coctave );
            _astDetector->detect( imgpyr[ coctave ], _threshold, features, _border );
        }
    }

}
