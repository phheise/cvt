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


#include <cvt/vision/features/FeatureSet.h>
#include <cvt/vision/features/NMSFilter.h>
#include <cvt/vision/features/GridFilter.h>
#include <algorithm>

namespace cvt {

    void FeatureSet::filterNMS( int radius, bool sort )
	{
		if( sort) {
			CmpPos cmp;
			std::sort( _features.begin(), _features.end(), cmp );
		}

		NMSFilter nms( _features );
		nms.filter( radius );
	}

	void FeatureSet::filterANMS( int radius, float threshold, bool sort )
	{
		if( sort) {
			sortPosition();
		}
	}

	void FeatureSet::filterBest( size_t n, bool sort )
	{
		if( sort ) {
			CmpScore cmp;
			std::sort( _features.begin(), _features.end(), cmp );
		}
		if( _features.size() > n )
			_features.resize( n );
	}

    void FeatureSet::filterGrid( size_t imageWidth, size_t imageHeight,
                                 size_t cellCountX, size_t cellCountY, size_t featuresPerCell )
	{
        GridFilter gridFilter( *this, featuresPerCell, imageWidth, imageHeight, cellCountX, cellCountY );
        gridFilter.filterGrid();
	}

	void FeatureSet::sortPosition()
	{
		CmpPosi cmp;
		std::sort( _features.begin(), _features.end(), cmp );
	}

}
