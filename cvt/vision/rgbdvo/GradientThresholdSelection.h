/*
   The MIT License (MIT)
   
   Copyright (c) 2011 - 2014, Philipp Heise and Sebastian Klose
   
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
#ifndef CVT_GRADIENT_THRESHOLD_SELECTION_H_
#define CVT_GRADIENT_THRESHOLD_SELECTION_H_

#include <cvt/gfx/Image.h>
#include <cvt/vision/rgbdvo/ApproxMedian.h>

namespace cvt
{
    class GradientThresholdSelection
    {
        public:
            GradientThresholdSelection( const Image& gradX, const Image& gradY, const Image& depth );
            ~GradientThresholdSelection();

            float threshold( float samplePercentage ) const;
            float selectBiggestN( size_t n ) const;

        private:
            ApproxMedian _gradHist;
    };

    inline GradientThresholdSelection::GradientThresholdSelection( const Image& gradX, const Image& gradY, const Image& depth ) :
        _gradHist( 0, 2.0, 0.02 )
    {
        // compute the gradient magnitude: 
        IMapScoped<const float> gx( gradX );
        IMapScoped<const float> gy( gradY );
        IMapScoped<const float> dMap( depth );

        size_t h = gradX.height();
        while( h-- )
        {
            const float* d = dMap.ptr();
            const float* x = gx.ptr();
            const float* y = gy.ptr();
            size_t w = gradX.width();
            while( w-- )
            {
                if( *d++ > 0 )
                {
                    float saliency = Math::abs( *x ) + Math::abs( *y );
                    _gradHist.add( saliency );
                }
                x++;
                y++;
            }
            gx++;
            gy++;
            dMap++;
        }
    }

    inline GradientThresholdSelection::~GradientThresholdSelection()
    {
    }
            
    inline float GradientThresholdSelection::threshold( float samplePercentage ) const
    {

        size_t nth = ( 1.0f - Math::min( samplePercentage, 1.0f ) ) * _gradHist.numSamples();
        return _gradHist.approximateNth( nth );
    }
    
    inline float GradientThresholdSelection::selectBiggestN( size_t n ) const
    {
        size_t N = _gradHist.numSamples();
        if( n >= N )
        {
            return 0.0f; // select all!
        }

        // selecting the nth biggest is the same as N-n-th smallest
        return _gradHist.approximateNth( N - n );
    }
}

#endif
