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

#ifndef CL_CLPYRAMID_H
#define CL_CLPYRAMID_H

#include <cvt/cl/CLKernel.h>

#include <cassert>


namespace cvt {
    class CLPyramid {
        public:
            CLPyramid( float scaleFactor, int levels );
            ~CLPyramid();

            void update( const Image& image );
            void update( const Image& image, const IFormat& format );

            template<typename PyrDownFunc>
            void update( const Image& image, PyrDownFunc f );
            template<typename PyrDownFunc>
            void update( const Image& image, const IFormat& format, PyrDownFunc );

            const Image& operator[]( int index ) const;
            const Image& level( int index ) const;

            int levels() const;
            float scaleFactor() const;

        private:
            CLPyramid();
            CLPyramid( const CLPyramid& );
            CLPyramid& operator=( const CLPyramid& );

            void pyrDown( Image& output, const Image& input );

            float   _scale;
            int   _levels;
            Image**  _pyr;
            CLKernel _clpyrdown;
    };

    inline int CLPyramid::levels() const
    {
        return _levels;
    }

    inline float CLPyramid::scaleFactor() const
    {
        return _scale;
    }

    inline const Image& CLPyramid::operator[]( int index ) const
    {
        assert( index >= 0 && index < _levels);
        return *_pyr[ index ];
    }

    inline const Image& CLPyramid::level( int index ) const
    {
        return *_pyr[ index ];
    }

    template<typename PyrDownFunc>
    inline void CLPyramid::update( const Image& input, PyrDownFunc func )
    {
        _pyr[ 0 ]->assign( input, IALLOCATOR_CL );
        for( int lvl = 1; lvl < _levels; lvl++ )
        {
            _pyr[ lvl ]->reallocate( _pyr[ lvl - 1 ]->width() * _scale, _pyr[ lvl - 1 ]->height() * _scale, _pyr[ lvl - 1 ]->format(), IALLOCATOR_CL );
            func( *_pyr[ lvl ], *_pyr[ lvl - 1 ] );
        }
    }

    template<typename PyrDownFunc>
    inline void CLPyramid::update( const Image& input, const IFormat& format, PyrDownFunc func )
    {
         _pyr[ 0 ]->reallocate( input.width(), input.height(), format, IALLOCATOR_CL );
         input.convert( *_pyr[ 0 ] );

        for( int lvl = 1; lvl < _levels; lvl++ )
        {
            _pyr[ lvl ]->reallocate( _pyr[ lvl - 1 ]->width() * _scale, _pyr[ lvl - 1 ]->height() * _scale, _pyr[ lvl - 1 ]->format(), IALLOCATOR_CL );
            func( *_pyr[ lvl ], *_pyr[ lvl - 1 ] );
        }
    }

}

#endif
