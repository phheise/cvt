/*
   The MIT License (MIT)

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
#ifndef CVT_DISPARITY_H
#define CVT_DISPARITY_H

#include <cvt/gfx/Image.h>

namespace cvt {
    class Disparity {
        public:
            static void interpolateBackground( Image& dst, const Image& disparity );
            static void errorImage( Image& dst, const Image& disparity, const Image& disparity_gt_noc /*, const Image& disparity_gt_occ*/ );
            static void colorCode( Image& dst, const Image& disparity, float* max_disparity = NULL );
            static void evaluateError( float& error, float& density, const Image& disparity, const Image& dispgt, float abs_thres, float rel_thres );
            static void evaluateError( size_t& error, size_t& valid, size_t& all, const Image& disparity, const Image& dispgt, float abs_thres, float rel_thres );

            static void histogramErrorAbsolute( const Image& gt, const Image& test, std::vector< std::pair< float, float> >& histogram );
            static void histogramErrorRelative( const Image& gt, const Image& test, std::vector< std::pair< float, float> >& histogram );

            static void defaultHistogramAbsolute( std::vector< std::pair< float, float> >& histogram );
            static void defaultHistogramRelative( std::vector< std::pair< float, float> >& histogram );
        private:
            Disparity();
            ~Disparity();
            Disparity( const Disparity& disparity );
    };

    inline void Disparity::evaluateError( float& error, float& density, const Image& disparity, const Image& dispgt, float abs_thres, float rel_thres )
    {
        size_t wrong, valid, all;

        evaluateError( wrong, valid, all, disparity, dispgt, abs_thres, rel_thres );
        error   = ( float ) wrong / ( float ) all;
        density = ( float ) valid / ( float ) all;
    }

}
#endif
