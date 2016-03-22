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

#ifndef CVT_MATRIX_H
#define CVT_MATRIX_H

#include <iostream>
#include <iomanip>

#include <cvt/util/String.h>
#include <cvt/math/Vector.h>
#include <cvt/math/Math.h>

#include <cvt/math/Matrix2.h>
#include <cvt/math/Matrix3.h>
#include <cvt/math/Matrix4.h>
#include <cvt/math/Matrix6.h>

namespace cvt
{
	/**
	   \ingroup Math
	 */
	typedef Matrix2<float> Matrix2f;
	typedef Matrix2<double> Matrix2d;

	typedef Matrix3<float> Matrix3f;
	typedef Matrix3<double> Matrix3d;

	typedef Matrix4<float> Matrix4f;
	typedef Matrix4<double> Matrix4d;

	typedef Matrix6<float> Matrix6f;
	typedef Matrix6<double> Matrix6d;

	template<int dim, typename T>
	struct Matrix {
		typedef T TYPE;
	};

	template<typename T>
	struct Matrix<2, T> {
		typedef Matrix2<T> TYPE;
	};

	template<typename T>
	struct Matrix<3, T> {
		typedef Matrix3<T> TYPE;
	};

	template<typename T>
	struct Matrix<4, T> {
		typedef Matrix4<T> TYPE;
	};

	template<typename T>
	struct Matrix<6, T> {
		typedef Matrix6<T> TYPE;
	};

}

#endif
