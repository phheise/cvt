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

#ifndef CVT_VECTOR_H
#define CVT_VECTOR_H

#include <iostream>
#include <iomanip>

#include <cvt/util/String.h>
#include <cvt/math/Math.h>

#include <cvt/math/Vector2.h>
#include <cvt/math/Vector3.h>
#include <cvt/math/Vector4.h>
#include <cvt/math/Vector6.h>

namespace cvt {
	/**
	  \ingroup Math
	*/

	typedef Vector2<float> Vector2f;
	typedef Vector2<double> Vector2d;
	typedef Vector2<int32_t> Vector2i;

	typedef Vector3<float> Vector3f;
	typedef Vector3<double> Vector3d;

	typedef Vector4<float> Vector4f;
	typedef Vector4<double> Vector4d;

    typedef Vector6<float> Vector6f;
    typedef Vector6<double> Vector6d;

	typedef Vector2f Point2f;
	typedef Vector3f Point3f;

	template<int dim, typename T>
		struct Vector {
			typedef T TYPE;
		};

	template<typename T>
		struct Vector<2, T> {
			typedef Vector2<T> TYPE;
		};

	template<typename T>
		struct Vector<3, T> {
			typedef Vector3<T> TYPE;
		};

	template<typename T>
		struct Vector<4, T> {
			typedef Vector4<T> TYPE;
		};

    template<typename T>
        struct Vector<6, T> {
            typedef Vector6<T> TYPE;
        };

}

#endif
