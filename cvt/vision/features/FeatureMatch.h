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

/*
 * File:   FeatureMatch.h
 * Author: Sebastian Klose
 *
 * Created on June 28, 2011
 */

#ifndef CVT_FEATUREMATCH_H
#define	CVT_FEATUREMATCH_H

#include <cvt/vision/features/Feature.h>

namespace cvt {

    struct FeatureMatch {

        FeatureMatch() :
            feature0( 0 ), feature1( 0 ), distance( -1.0f )
        {
        }

        FeatureMatch( const FeatureMatch & other ) :
            feature0( other.feature0 ),
            feature1( other.feature1 ),
            distance( other.distance )
        {
        }

        FeatureMatch& operator=( const FeatureMatch& other )
        {
            feature0 = other.feature0;
            feature1 = other.feature1;
            distance = other.distance;
            return *this;
        }

        const Feature* 	feature0;
        const Feature* 	feature1;
        float			distance;
	};

	struct MatchingIndices {
		size_t	srcIdx;
		size_t	dstIdx;
		float	distance;
	};

}

#endif

