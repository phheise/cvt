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

#ifndef CVT_AGAST_H
#define CVT_AGAST_H

#include <cvt/vision/features/FeatureDetector.h>

namespace cvt {
    class ASTDetector;

    class AGAST : public FeatureDetector
    {
        public:
            enum ASTType {
                AGAST_5_8,
                AGAST_7_12d,
                AGAST_7_12s,
                OAST_9_16
            };

            AGAST( ASTType astType, uint8_t threshold = 30, size_t border = 3 );
            ~AGAST();

            void detect( FeatureSet& features, const Image& image );
            void detect( FeatureSet& features, const ImagePyramid& image );

            void setThreshold( uint8_t threshold )	{ _threshold = threshold; }
            uint8_t threshold() const				{ return _threshold; }

            void setBorder( size_t border )			{ _border = Math::max<size_t>( border, 3 ); }
            size_t border() const					{ return _border; }

        private:
            ASTType         _astType;
            ASTDetector*    _astDetector;

            uint8_t _threshold;
            size_t	_border;
    };
}

#endif
