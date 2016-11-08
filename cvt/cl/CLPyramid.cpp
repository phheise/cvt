/*
   The MIT License (MIT)

   Copyright (c) 2011 - 2016, Philipp Heise
   Copyright (c) 2016, BMW Car IT GmbH, Philipp Heise (philipp.heise@bmw.de)

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

#include <cvt/cl/CLPyramid.h>
#include <cvt/cl/kernel/pyr/pyrdown.h>

namespace cvt {
#define KX 32
#define KY 32

    CLPyramid::CLPyramid( float scale, int levels ) :
        _scale( scale ),
        _levels( levels ),
        _clpyrdown( _pyrdown_source, "pyrdown" )
    {
        _pyr = new Image*[ levels ];
        for( int i = 0; i < _levels; i++ ) {
            _pyr[ i ] = new Image( 1, 1, IFormat::GRAY_UINT8 );
        }
    }

    CLPyramid::~CLPyramid()
    {
        for( int i = 0; i < _levels; i++ )
            delete _pyr[ i ];
        delete[] _pyr;
    }

    void CLPyramid::update( const Image& input )
    {
        _pyr[ 0 ]->assign( input, IALLOCATOR_CL );
        for( int lvl = 1; lvl < _levels; lvl++ )
        {
            _pyr[ lvl ]->reallocate( _pyr[ lvl - 1 ]->width() * _scale, _pyr[ lvl - 1 ]->height() * _scale, _pyr[ lvl - 1 ]->format(), IALLOCATOR_CL );
            pyrDown( *_pyr[ lvl ], *_pyr[ lvl - 1 ] );
        }
    }

    void CLPyramid::update( const Image& input, const IFormat& format )
    {
         _pyr[ 0 ]->reallocate( input.width(), input.height(), format, IALLOCATOR_CL );
         input.convert( *_pyr[ 0 ] );

        for( int lvl = 1; lvl < _levels; lvl++ )
        {
            _pyr[ lvl ]->reallocate( _pyr[ lvl - 1 ]->width() * _scale, _pyr[ lvl - 1 ]->height() * _scale, _pyr[ lvl - 1 ]->format(), IALLOCATOR_CL );
            pyrDown( *_pyr[ lvl ], *_pyr[ lvl - 1 ] );
        }
    }

    void CLPyramid::pyrDown( Image& output, const Image& input )
    {
        _clpyrdown.setArg( 0, output );
        _clpyrdown.setArg( 1, input );
        _clpyrdown.run( CLNDRange( Math::pad( output.width(), KX ), Math::pad( output.height(), KY ) ), CLNDRange( KX, KY ) );
    }

}
