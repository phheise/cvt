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

#ifndef CVT_BSPLINE_H
#define CVT_BSPLINE_H

#include <cvt/math/Math.h>

namespace cvt {

	template<typename T>
	class BSpline {
		public:
			static T eval( T t );
			static T evalDerivative( T t );
			static T evalSecondDerivative( T t );
	};

	template<typename T>
	inline T BSpline<T>::eval( T t  )
	{
		t = Math::abs( t );
		if( t > 2.0f ) return 0.0f;
		if( t > 1.0f ) {
			t = ( 2.0f - t );
			return t * t * t / 6.0f;
		}
		return ( 4.0f - 6.0f * t * t + 3.0f * t * t *t ) / 6.0f;
	}

	template<typename T>
	inline T BSpline<T>::evalDerivative( T t  )
	{
		T z = Math::abs( t );
		if( z > 2.0f ) return 0.0f;
		if( t < -1.0f ) {
			t = ( 2.0f + t );
			return 0.5f * t * t;
		}
		if( t > 1.0f ) {
			t = ( 2.0f - t );
			return -0.5f * t * t;
		}
		if( t < 0.0f )
			return -1.5f * t * t - 2.0f * t;
		return 1.5f * t * t - 2.0f * t;
	}

	template<typename T>
	inline T BSpline<T>::evalSecondDerivative( T t  )
	{
		T z = Math::abs( t );
		if( z > 2.0f ) return 0.0f;
		if( z > 1.0f ) return -z + 2.0f;
		return 3.0f * z - 2.0f;
	}

	typedef BSpline<float> BSplinef;
	typedef BSpline<double> BSplined;
}

#endif
