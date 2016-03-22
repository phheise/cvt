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

#ifndef CVT_FAST_H
#define CVT_FAST_H

#include <cvt/vision/features/FeatureDetector.h>
#include <cvt/math/Math.h>
#include <cvt/util/CPU.h>
#include <cvt/util/Exception.h>
#include <cvt/gfx/Image.h>

#include <vector>

namespace cvt
{
	enum FASTSize {
		SEGMENT_9,
		SEGMENT_10,
		SEGMENT_11,
		SEGMENT_12
	};

	class FAST : public FeatureDetector
	{
		public:
			FAST( FASTSize size = SEGMENT_9, uint8_t threshold = 30, size_t border = 3 );
			~FAST();

			void detect( FeatureSet& features, const Image& image );
			void detect( FeatureSet& features, const ImagePyramid& image );

			void setThreshold( uint8_t threshold )	{ _threshold = threshold; }
			uint8_t threshold() const				{ return _threshold; }

			void setBorder( size_t border )			{ _border = Math::max<size_t>( border, 3 ); }
			size_t border() const					{ return _border; }

		private:
            FASTSize    _fastSize;
			uint8_t		_threshold;
            size_t		_border;

            static void make_offsets( int * offsets, size_t row_stride );

			static void detect9( const Image& img, uint8_t threshold, FeatureSetWrapper& features, size_t border = 3 );
			static void detect9cpu( const Image& img, uint8_t threshold, FeatureSetWrapper& features, size_t border = 3 );
			static void detect9simd( const Image& img, uint8_t threshold, FeatureSetWrapper& features, size_t border = 3 );
			static void detect10( const Image& img, uint8_t threshold, FeatureSetWrapper& features, size_t border = 3 );
			static void detect11( const Image& img, uint8_t threshold, FeatureSetWrapper& features, size_t border = 3 );
			static void detect12( const Image& img, uint8_t threshold, FeatureSetWrapper& features, size_t border = 3 );

            static int score9Pixel( const uint8_t* p, const int * offsets, uint8_t threshold );
            static int score10Pixel( const uint8_t* p, const int * offsets, uint8_t threshold );
            static int score11Pixel( const uint8_t* p, const int * offsets, uint8_t threshold );
            static int score12Pixel( const uint8_t* p, const int * offsets, uint8_t threshold );

            static bool isCorner9( const uint8_t * p, const int * offsets, uint8_t threshold );
            static bool isCorner10( const uint8_t * p, const int * offsets, uint8_t threshold );
            static bool isCorner11( const uint8_t * p, const int * offsets, uint8_t threshold );
            static bool isCorner12( const uint8_t * p, const int * offsets, uint8_t threshold );

	};

	inline void FAST::make_offsets( int* offsets, size_t row_stride )
	{
        offsets[ 0  ] =  0 + row_stride * 3;
		offsets[ 1  ] =  1 + row_stride * 3;
		offsets[ 2  ] =  2 + row_stride * 2;
		offsets[ 3  ] =  3 + row_stride * 1;
		offsets[ 4  ] =  3;
		offsets[ 5  ] =  3 - row_stride * 1;
		offsets[ 6  ] =  2 - row_stride * 2;
		offsets[ 7  ] =  1 - row_stride * 3;
		offsets[ 8  ] =    - row_stride * 3;
		offsets[ 9  ] = -1 - row_stride * 3;
		offsets[ 10 ] = -2 - row_stride * 2;
		offsets[ 11 ] = -3 - row_stride * 1;
		offsets[ 12 ] = -3;
		offsets[ 13 ] = -3 + row_stride;
		offsets[ 14 ] = -2 + row_stride * 2;
		offsets[ 15 ] = -1 + row_stride * 3;
    }

}

#endif
