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

#include <cvt/vision/slam/stereo/FeatureTracking.h>
#include <cvt/vision/Vision.h>

namespace cvt
{
	FeatureTracking::FeatureTracking()
	{
	}

	FeatureTracking::~FeatureTracking()
	{
	}

	void FeatureTracking::track( std::vector<size_t>& tracked,
								 const Image& currGray, // TODO directly hand in descriptors of
								 const std::vector<FeatureDescriptor*>& descriptors )
	{
		// match descriptors within window

		// refine with KLT

		// return successfully tracked indices
	}


	bool FeatureTracking::checkFeatureSAD( const Vector2f& point0,
										   const Vector2f& point1,
										   const Image & i0,
										   const Image & i1 ) const
	{
		size_t s0, s1;
		const uint8_t* ptr0 = i0.map( &s0 );
		const uint8_t* ptr1 = i1.map( &s1 );

		const uint8_t* p0 = ptr0 + ( (int)point0.y - 8 ) * s0 + ( (int)point0.x - 8 );
		const uint8_t* p1 = ptr1 + ( (int)point1.y - 8 ) * s1 + ( (int)point1.x - 8 );

		float sad = 0;
		for( size_t i = 0; i < 16; i++ ){
			sad += SIMD::instance()->SAD( p0, p1, 16 );
			p0 += s0;
			p1 += s1;
		}

		i0.unmap( ptr0 );
		i1.unmap( ptr1 );

		// average & normalize
		sad = 1.0f - ( sad / Math::sqr( 255.0 ) );

		if( sad > 0.7 /* TODO: make param */ ){
			return true;
		} else {
			return false;
		}
	}
}
