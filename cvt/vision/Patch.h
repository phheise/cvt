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

#ifndef CVT_PATCH_H
#define CVT_PATCH_H

#include <cvt/gfx/Image.h>
#include <cvt/geom/Rect.h>
#include <cvt/util/Flags.h>

namespace cvt {
    
    enum PatchFlagTypes {
        PLAIN       = ( 1 << 0 ),
		NORMALIZE   = ( 1 << 1 )
	};
    
	CVT_ENUM_TO_FLAGS( PatchFlagTypes, PatchFlags )
    
    class Patch
    {
    public:
        Patch( const Image & img, const Recti & rect, PatchFlags computeFlags = PLAIN );
        ~Patch();
        
        void update( const Image & img, const Recti & rect );
        size_t width() const { return _patchData.width(); }
        size_t height() const { return _patchData.height(); }
        const Image & data() const { return _patchData; }
        
        float mean() const;
        float variance() const;
        
        void dump() const;
        
    private:
        PatchFlags  _computeFlags;
        Image       _patchData;
        
        float       _mean;
        float       _var;
        
        void calcMeanAndVariance();
        
        template <typename T> void meanAndVariance( const T* input, size_t stride )
        {
            float sum = 0.0f;
            float sqrSum = 0.0f;
            
            size_t height = _patchData.height();
            size_t width = _patchData.width();
            
            
            while( height-- ){
                for( size_t i = 0; i < width; i++ ){
                    sum += input[ i ];
                    sqrSum += ( float )Math::sqr( ( float )input[ i ] );
                }
                input += stride;
            }
            
            float normalizer = 1.0f / ( float )( width * this->height() );

            _mean = sum * normalizer;
            _var = sqrSum * normalizer - Math::sqr( _mean );
        }
    };
}

#endif