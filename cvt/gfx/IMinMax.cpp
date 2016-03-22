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

#include <cvt/gfx/IMinMax.h>
#include <cvt/gfx/IMapScoped.h>
#include <cvt/util/SIMD.h>

namespace cvt {

    void IMinMax::minmax( float& min, float& max, const Image& img, float mininit, float maxinit )
    {
		if( img.format() != IFormat::GRAY_FLOAT )
			throw CVTException( "Min/Max only implemented for single channel float images!" );

        min = mininit;
        max = maxinit;

        SIMD* simd = SIMD::instance();

        IMapScoped<const float> map( img );
        for( size_t y = 0; y < map.height(); y++ ) {
            simd->MinMax_1f( min, max, map.ptr(), map.width() );
            map++;
        }
    }

}
