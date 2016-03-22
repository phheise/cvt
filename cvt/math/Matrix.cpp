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

#include <cvt/math/Matrix.h>
#include <cvt/util/CVTTest.h>

#include <Eigen/Core>
#include <Eigen/SVD>

#include <iostream>

#include <cvt/util/DataIterator.h>

namespace cvt
{

	template<typename T>
	bool Matrix2<T>::inverseSelf( )
	{
		T D, invD, tmp;

		D = mat[ 0 ][ 0 ] * mat[ 1 ][ 1 ] - mat[ 1 ][ 0 ] * mat[ 0 ][ 1 ];

		if( Math::abs( D ) < Math::epsilon<T>( ) )
			return false;

		invD = ( T ) 1 / D;

		tmp = mat[ 0 ][ 0 ];
		mat[ 0 ][ 0 ] =   mat[ 1 ][ 1 ] * invD;
		mat[ 0 ][ 1 ] = -mat[ 0 ][ 1 ] * invD;
		mat[ 1 ][ 0 ] = -mat[ 1 ][ 0 ] * invD;
		mat[ 1 ][ 1 ] =   tmp * invD;

		return true;
	}

	template bool Matrix2<float>::inverseSelf( );

	template bool Matrix2<double>::inverseSelf( );

	template<typename T>
	bool Matrix3<T>::inverseSelf( )
	{
		Matrix3<T> cofactor;
		T D, invD;

		cofactor[ 0 ][ 0 ] = mat[ 1 ][ 1 ] * mat[ 2 ][ 2 ] - mat[ 2 ][ 1 ] * mat[ 1 ][ 2 ];
		cofactor[ 1 ][ 0 ] = mat[ 2 ][ 0 ] * mat[ 1 ][ 2 ] - mat[ 1 ][ 0 ] * mat[ 2 ][ 2 ];
		cofactor[ 2 ][ 0 ] = mat[ 1 ][ 0 ] * mat[ 2 ][ 1 ] - mat[ 2 ][ 0 ] * mat[ 1 ][ 1 ];

		D = mat[ 0 ][ 0 ] * cofactor[ 0 ][ 0 ] + mat[ 0 ][ 1 ] * cofactor[ 1 ][ 0 ] + mat[ 0 ][ 2 ] * cofactor[ 2 ][ 0 ];

		if( Math::abs( D ) < Math::epsilon<T>( ) )
			return false;

		invD = ( ( T ) 1 ) / D;

		cofactor[ 0 ][ 1 ] = mat[ 2 ][ 1 ] * mat[ 0 ][ 2 ] - mat[ 0 ][ 1 ] * mat[ 2 ][ 2 ];
		cofactor[ 0 ][ 2 ] = mat[ 0 ][ 1 ] * mat[ 1 ][ 2 ] - mat[ 1 ][ 1 ] * mat[ 0 ][ 2 ];

		cofactor[ 1 ][ 1 ] = mat[ 0 ][ 0 ] * mat[ 2 ][ 2 ] - mat[ 2 ][ 0 ] * mat[ 0 ][ 2 ];
		cofactor[ 1 ][ 2 ] = mat[ 1 ][ 0 ] * mat[ 0 ][ 2 ] - mat[ 0 ][ 0 ] * mat[ 1 ][ 2 ];

		cofactor[ 2 ][ 1 ] = mat[ 2 ][ 0 ] * mat[ 0 ][ 1 ] - mat[ 0 ][ 0 ] * mat[ 2 ][ 1 ];
		cofactor[ 2 ][ 2 ] = mat[ 0 ][ 0 ] * mat[ 1 ][ 1 ] - mat[ 1 ][ 0 ] * mat[ 0 ][ 1 ];

		mat[ 0 ][ 0 ] =  cofactor[ 0 ][ 0 ] * invD;
		mat[ 0 ][ 1 ] =  cofactor[ 0 ][ 1 ] * invD;
		mat[ 0 ][ 2 ] =  cofactor[ 0 ][ 2 ] * invD;

		mat[ 1 ][ 0 ] =  cofactor[ 1 ][ 0 ] * invD;
		mat[ 1 ][ 1 ] =  cofactor[ 1 ][ 1 ] * invD;
		mat[ 1 ][ 2 ] =  cofactor[ 1 ][ 2 ] * invD;

		mat[ 2 ][ 0 ] =  cofactor[ 2 ][ 0 ] * invD;
		mat[ 2 ][ 1 ] =  cofactor[ 2 ][ 1 ] * invD;
		mat[ 2 ][ 2 ] =  cofactor[ 2 ][ 2 ] * invD;

		return true;
	}

	template bool Matrix3<float>::inverseSelf( );

	template bool Matrix3<double>::inverseSelf( );

	template<typename T>
	bool Matrix4<T>::inverseSelf( )
	{
		T D, invD;
		T det2_23_01, det2_23_02, det2_23_03,
		  det2_23_12, det2_23_13, det2_23_23;
		T det3_123_123, det3_123_023, det3_123_013, det3_123_012;

		det2_23_01 = mat[ 2 ][ 0 ] * mat[ 3 ][ 1 ] - mat[ 3 ][ 0 ] * mat[ 2 ][ 1 ];
		det2_23_02 = mat[ 2 ][ 0 ] * mat[ 3 ][ 2 ] - mat[ 3 ][ 0 ] * mat[ 2 ][ 2 ];
		det2_23_03 = mat[ 2 ][ 0 ] * mat[ 3 ][ 3 ] - mat[ 3 ][ 0 ] * mat[ 2 ][ 3 ];
		det2_23_12 = mat[ 2 ][ 1 ] * mat[ 3 ][ 2 ] - mat[ 3 ][ 1 ] * mat[ 2 ][ 2 ];
		det2_23_13 = mat[ 2 ][ 1 ] * mat[ 3 ][ 3 ] - mat[ 3 ][ 1 ] * mat[ 2 ][ 3 ];
		det2_23_23 = mat[ 2 ][ 2 ] * mat[ 3 ][ 3 ] - mat[ 3 ][ 2 ] * mat[ 2 ][ 3 ];

		det3_123_123  =  mat[ 1 ][ 1 ] * det2_23_23 - mat[ 1 ][ 2 ] * det2_23_13 + mat[ 1 ][ 3 ] * det2_23_12;
		det3_123_023  =  mat[ 1 ][ 0 ] * det2_23_23 - mat[ 1 ][ 2 ] * det2_23_03 + mat[ 1 ][ 3 ] * det2_23_02;
		det3_123_013  =  mat[ 1 ][ 0 ] * det2_23_13 - mat[ 1 ][ 1 ] * det2_23_03 + mat[ 1 ][ 3 ] * det2_23_01;
		det3_123_012  =  mat[ 1 ][ 0 ] * det2_23_12 - mat[ 1 ][ 1 ] * det2_23_02 + mat[ 1 ][ 2 ] * det2_23_01;

		D = mat[ 0 ][ 0 ] * det3_123_123 - mat[ 0 ][ 1 ] * det3_123_023 + mat[ 0 ][ 2 ] * det3_123_013 - mat[ 0 ][ 3 ] * det3_123_012;

		if( Math::abs( D ) < Math::epsilon<T>( ) )
			return false;

		invD = ( T ) 1 / D;

		T det3_023_123, det3_023_023, det3_023_013, det3_023_012;

		det3_023_123  =  mat[ 0 ][ 1 ] * det2_23_23 - mat[ 0 ][ 2 ] * det2_23_13 + mat[ 0 ][ 3 ] * det2_23_12;
		det3_023_023  =  mat[ 0 ][ 0 ] * det2_23_23 - mat[ 0 ][ 2 ] * det2_23_03 + mat[ 0 ][ 3 ] * det2_23_02;
		det3_023_013  =  mat[ 0 ][ 0 ] * det2_23_13 - mat[ 0 ][ 1 ] * det2_23_03 + mat[ 0 ][ 3 ] * det2_23_01;
		det3_023_012  =  mat[ 0 ][ 0 ] * det2_23_12 - mat[ 0 ][ 1 ] * det2_23_02 + mat[ 0 ][ 2 ] * det2_23_01;

		T det2_13_01, det2_13_02, det2_13_03,
		  det2_13_12, det2_13_13, det2_13_23;
		T det3_013_123, det3_013_023, det3_013_013, det3_013_012;

		det2_13_01 = mat[ 1 ][ 0 ] * mat[ 3 ][ 1 ] - mat[ 3 ][ 0 ] * mat[ 1 ][ 1 ];
		det2_13_02 = mat[ 1 ][ 0 ] * mat[ 3 ][ 2 ] - mat[ 3 ][ 0 ] * mat[ 1 ][ 2 ];
		det2_13_03 = mat[ 1 ][ 0 ] * mat[ 3 ][ 3 ] - mat[ 3 ][ 0 ] * mat[ 1 ][ 3 ];
		det2_13_12 = mat[ 1 ][ 1 ] * mat[ 3 ][ 2 ] - mat[ 3 ][ 1 ] * mat[ 1 ][ 2 ];
		det2_13_13 = mat[ 1 ][ 1 ] * mat[ 3 ][ 3 ] - mat[ 3 ][ 1 ] * mat[ 1 ][ 3 ];
		det2_13_23 = mat[ 1 ][ 2 ] * mat[ 3 ][ 3 ] - mat[ 3 ][ 2 ] * mat[ 1 ][ 3 ];

		det3_013_123  =  mat[ 0 ][ 1 ] * det2_13_23 - mat[ 0 ][ 2 ] * det2_13_13 + mat[ 0 ][ 3 ] * det2_13_12;
		det3_013_023  =  mat[ 0 ][ 0 ] * det2_13_23 - mat[ 0 ][ 2 ] * det2_13_03 + mat[ 0 ][ 3 ] * det2_13_02;
		det3_013_013  =  mat[ 0 ][ 0 ] * det2_13_13 - mat[ 0 ][ 1 ] * det2_13_03 + mat[ 0 ][ 3 ] * det2_13_01;
		det3_013_012  =  mat[ 0 ][ 0 ] * det2_13_12 - mat[ 0 ][ 1 ] * det2_13_02 + mat[ 0 ][ 2 ] * det2_13_01;

		T det2_12_01, det2_12_02, det2_12_03,
		  det2_12_12, det2_12_13, det2_12_23;
		T det3_012_123, det3_012_023, det3_012_013, det3_012_012;

		det2_12_01 = mat[ 1 ][ 0 ] * mat[ 2 ][ 1 ] - mat[ 2 ][ 0 ] * mat[ 1 ][ 1 ];
		det2_12_02 = mat[ 1 ][ 0 ] * mat[ 2 ][ 2 ] - mat[ 2 ][ 0 ] * mat[ 1 ][ 2 ];
		det2_12_03 = mat[ 1 ][ 0 ] * mat[ 2 ][ 3 ] - mat[ 2 ][ 0 ] * mat[ 1 ][ 3 ];
		det2_12_12 = mat[ 1 ][ 1 ] * mat[ 2 ][ 2 ] - mat[ 2 ][ 1 ] * mat[ 1 ][ 2 ];
		det2_12_13 = mat[ 1 ][ 1 ] * mat[ 2 ][ 3 ] - mat[ 2 ][ 1 ] * mat[ 1 ][ 3 ];
		det2_12_23 = mat[ 1 ][ 2 ] * mat[ 2 ][ 3 ] - mat[ 2 ][ 2 ] * mat[ 1 ][ 3 ];

		det3_012_123  =  mat[ 0 ][ 1 ] * det2_12_23 - mat[ 0 ][ 2 ] * det2_12_13 + mat[ 0 ][ 3 ] * det2_12_12;
		det3_012_023  =  mat[ 0 ][ 0 ] * det2_12_23 - mat[ 0 ][ 2 ] * det2_12_03 + mat[ 0 ][ 3 ] * det2_12_02;
		det3_012_013  =  mat[ 0 ][ 0 ] * det2_12_13 - mat[ 0 ][ 1 ] * det2_12_03 + mat[ 0 ][ 3 ] * det2_12_01;
		det3_012_012  =  mat[ 0 ][ 0 ] * det2_12_12 - mat[ 0 ][ 1 ] * det2_12_02 + mat[ 0 ][ 2 ] * det2_12_01;

		mat[ 0 ][ 0 ] =   det3_123_123 * invD;
		mat[ 0 ][ 1 ] = -det3_023_123 * invD;
		mat[ 0 ][ 2 ] =   det3_013_123 * invD;
		mat[ 0 ][ 3 ] = -det3_012_123 * invD;

		mat[ 1 ][ 0 ] = -det3_123_023 * invD;
		mat[ 1 ][ 1 ] =   det3_023_023 * invD;
		mat[ 1 ][ 2 ] = -det3_013_023 * invD;
		mat[ 1 ][ 3 ] =   det3_012_023 * invD;

		mat[ 2 ][ 0 ] =   det3_123_013 * invD;
		mat[ 2 ][ 1 ] = -det3_023_013 * invD;
		mat[ 2 ][ 2 ] =   det3_013_013 * invD;
		mat[ 2 ][ 3 ] = -det3_012_013 * invD;

		mat[ 3 ][ 0 ] = -det3_123_012 * invD;
		mat[ 3 ][ 1 ] =   det3_023_012 * invD;
		mat[ 3 ][ 2 ] = -det3_013_012 * invD;
		mat[ 3 ][ 3 ] =   det3_012_012 * invD;

		return true;
	}

	template bool Matrix4<float>::inverseSelf( );

	template bool Matrix4<double>::inverseSelf( );

	template<typename T>
	bool Matrix6<T>::inverseSelf( )
	{
		// this is from the doom3-BFG and unverified
		// FIXME: evaluate using sub-matrices

		T* mat = ptr( );
		T di, s, d;

		di = mat[ 0 ];
		s = di;
		mat[ 0 ] = d = 1.0f / di;
		mat[ 1 ] *= d;
		mat[ 2 ] *= d;
		mat[ 3 ] *= d;
		mat[ 4 ] *= d;
		mat[ 5 ] *= d;
		d = -d;
		mat[ 6 ] *= d;
		mat[ 12 ] *= d;
		mat[ 18 ] *= d;
		mat[ 24 ] *= d;
		mat[ 30 ] *= d;
		d = mat[ 6 ] * di;
		mat[ 7 ] += mat[ 1 ] * d;
		mat[ 8 ] += mat[ 2 ] * d;
		mat[ 9 ] += mat[ 3 ] * d;
		mat[ 10 ] += mat[ 4 ] * d;
		mat[ 11 ] += mat[ 5 ] * d;
		d = mat[ 12 ] * di;
		mat[ 13 ] += mat[ 1 ] * d;
		mat[ 14 ] += mat[ 2 ] * d;
		mat[ 15 ] += mat[ 3 ] * d;
		mat[ 16 ] += mat[ 4 ] * d;
		mat[ 17 ] += mat[ 5 ] * d;
		d = mat[ 18 ] * di;
		mat[ 19 ] += mat[ 1 ] * d;
		mat[ 20 ] += mat[ 2 ] * d;
		mat[ 21 ] += mat[ 3 ] * d;
		mat[ 22 ] += mat[ 4 ] * d;
		mat[ 23 ] += mat[ 5 ] * d;
		d = mat[ 24 ] * di;
		mat[ 25 ] += mat[ 1 ] * d;
		mat[ 26 ] += mat[ 2 ] * d;
		mat[ 27 ] += mat[ 3 ] * d;
		mat[ 28 ] += mat[ 4 ] * d;
		mat[ 29 ] += mat[ 5 ] * d;
		d = mat[ 30 ] * di;
		mat[ 31 ] += mat[ 1 ] * d;
		mat[ 32 ] += mat[ 2 ] * d;
		mat[ 33 ] += mat[ 3 ] * d;
		mat[ 34 ] += mat[ 4 ] * d;
		mat[ 35 ] += mat[ 5 ] * d;
		di = mat[ 7 ];
		s *= di;
		mat[ 7 ] = d = 1.0f / di;
		mat[ 6 ] *= d;
		mat[ 8 ] *= d;
		mat[ 9 ] *= d;
		mat[ 10 ] *= d;
		mat[ 11 ] *= d;
		d = -d;
		mat[ 1 ] *= d;
		mat[ 13 ] *= d;
		mat[ 19 ] *= d;
		mat[ 25 ] *= d;
		mat[ 31 ] *= d;
		d = mat[ 1 ] * di;
		mat[ 0 ] += mat[ 6 ] * d;
		mat[ 2 ] += mat[ 8 ] * d;
		mat[ 3 ] += mat[ 9 ] * d;
		mat[ 4 ] += mat[ 10 ] * d;
		mat[ 5 ] += mat[ 11 ] * d;
		d = mat[ 13 ] * di;
		mat[ 12 ] += mat[ 6 ] * d;
		mat[ 14 ] += mat[ 8 ] * d;
		mat[ 15 ] += mat[ 9 ] * d;
		mat[ 16 ] += mat[ 10 ] * d;
		mat[ 17 ] += mat[ 11 ] * d;
		d = mat[ 19 ] * di;
		mat[ 18 ] += mat[ 6 ] * d;
		mat[ 20 ] += mat[ 8 ] * d;
		mat[ 21 ] += mat[ 9 ] * d;
		mat[ 22 ] += mat[ 10 ] * d;
		mat[ 23 ] += mat[ 11 ] * d;
		d = mat[ 25 ] * di;
		mat[ 24 ] += mat[ 6 ] * d;
		mat[ 26 ] += mat[ 8 ] * d;
		mat[ 27 ] += mat[ 9 ] * d;
		mat[ 28 ] += mat[ 10 ] * d;
		mat[ 29 ] += mat[ 11 ] * d;
		d = mat[ 31 ] * di;
		mat[ 30 ] += mat[ 6 ] * d;
		mat[ 32 ] += mat[ 8 ] * d;
		mat[ 33 ] += mat[ 9 ] * d;
		mat[ 34 ] += mat[ 10 ] * d;
		mat[ 35 ] += mat[ 11 ] * d;
		di = mat[ 14 ];
		s *= di;
		mat[ 14 ] = d = 1.0f / di;
		mat[ 12 ] *= d;
		mat[ 13 ] *= d;
		mat[ 15 ] *= d;
		mat[ 16 ] *= d;
		mat[ 17 ] *= d;
		d = -d;
		mat[ 2 ] *= d;
		mat[ 8 ] *= d;
		mat[ 20 ] *= d;
		mat[ 26 ] *= d;
		mat[ 32 ] *= d;
		d = mat[ 2 ] * di;
		mat[ 0 ] += mat[ 12 ] * d;
		mat[ 1 ] += mat[ 13 ] * d;
		mat[ 3 ] += mat[ 15 ] * d;
		mat[ 4 ] += mat[ 16 ] * d;
		mat[ 5 ] += mat[ 17 ] * d;
		d = mat[ 8 ] * di;
		mat[ 6 ] += mat[ 12 ] * d;
		mat[ 7 ] += mat[ 13 ] * d;
		mat[ 9 ] += mat[ 15 ] * d;
		mat[ 10 ] += mat[ 16 ] * d;
		mat[ 11 ] += mat[ 17 ] * d;
		d = mat[ 20 ] * di;
		mat[ 18 ] += mat[ 12 ] * d;
		mat[ 19 ] += mat[ 13 ] * d;
		mat[ 21 ] += mat[ 15 ] * d;
		mat[ 22 ] += mat[ 16 ] * d;
		mat[ 23 ] += mat[ 17 ] * d;
		d = mat[ 26 ] * di;
		mat[ 24 ] += mat[ 12 ] * d;
		mat[ 25 ] += mat[ 13 ] * d;
		mat[ 27 ] += mat[ 15 ] * d;
		mat[ 28 ] += mat[ 16 ] * d;
		mat[ 29 ] += mat[ 17 ] * d;
		d = mat[ 32 ] * di;
		mat[ 30 ] += mat[ 12 ] * d;
		mat[ 31 ] += mat[ 13 ] * d;
		mat[ 33 ] += mat[ 15 ] * d;
		mat[ 34 ] += mat[ 16 ] * d;
		mat[ 35 ] += mat[ 17 ] * d;
		di = mat[ 21 ];
		s *= di;
		mat[ 21 ] = d = 1.0f / di;
		mat[ 18 ] *= d;
		mat[ 19 ] *= d;
		mat[ 20 ] *= d;
		mat[ 22 ] *= d;
		mat[ 23 ] *= d;
		d = -d;
		mat[ 3 ] *= d;
		mat[ 9 ] *= d;
		mat[ 15 ] *= d;
		mat[ 27 ] *= d;
		mat[ 33 ] *= d;
		d = mat[ 3 ] * di;
		mat[ 0 ] += mat[ 18 ] * d;
		mat[ 1 ] += mat[ 19 ] * d;
		mat[ 2 ] += mat[ 20 ] * d;
		mat[ 4 ] += mat[ 22 ] * d;
		mat[ 5 ] += mat[ 23 ] * d;
		d = mat[ 9 ] * di;
		mat[ 6 ] += mat[ 18 ] * d;
		mat[ 7 ] += mat[ 19 ] * d;
		mat[ 8 ] += mat[ 20 ] * d;
		mat[ 10 ] += mat[ 22 ] * d;
		mat[ 11 ] += mat[ 23 ] * d;
		d = mat[ 15 ] * di;
		mat[ 12 ] += mat[ 18 ] * d;
		mat[ 13 ] += mat[ 19 ] * d;
		mat[ 14 ] += mat[ 20 ] * d;
		mat[ 16 ] += mat[ 22 ] * d;
		mat[ 17 ] += mat[ 23 ] * d;
		d = mat[ 27 ] * di;
		mat[ 24 ] += mat[ 18 ] * d;
		mat[ 25 ] += mat[ 19 ] * d;
		mat[ 26 ] += mat[ 20 ] * d;
		mat[ 28 ] += mat[ 22 ] * d;
		mat[ 29 ] += mat[ 23 ] * d;
		d = mat[ 33 ] * di;
		mat[ 30 ] += mat[ 18 ] * d;
		mat[ 31 ] += mat[ 19 ] * d;
		mat[ 32 ] += mat[ 20 ] * d;
		mat[ 34 ] += mat[ 22 ] * d;
		mat[ 35 ] += mat[ 23 ] * d;
		di = mat[ 28 ];
		s *= di;
		mat[ 28 ] = d = 1.0f / di;
		mat[ 24 ] *= d;
		mat[ 25 ] *= d;
		mat[ 26 ] *= d;
		mat[ 27 ] *= d;
		mat[ 29 ] *= d;
		d = -d;
		mat[ 4 ] *= d;
		mat[ 10 ] *= d;
		mat[ 16 ] *= d;
		mat[ 22 ] *= d;
		mat[ 34 ] *= d;
		d = mat[ 4 ] * di;
		mat[ 0 ] += mat[ 24 ] * d;
		mat[ 1 ] += mat[ 25 ] * d;
		mat[ 2 ] += mat[ 26 ] * d;
		mat[ 3 ] += mat[ 27 ] * d;
		mat[ 5 ] += mat[ 29 ] * d;
		d = mat[ 10 ] * di;
		mat[ 6 ] += mat[ 24 ] * d;
		mat[ 7 ] += mat[ 25 ] * d;
		mat[ 8 ] += mat[ 26 ] * d;
		mat[ 9 ] += mat[ 27 ] * d;
		mat[ 11 ] += mat[ 29 ] * d;
		d = mat[ 16 ] * di;
		mat[ 12 ] += mat[ 24 ] * d;
		mat[ 13 ] += mat[ 25 ] * d;
		mat[ 14 ] += mat[ 26 ] * d;
		mat[ 15 ] += mat[ 27 ] * d;
		mat[ 17 ] += mat[ 29 ] * d;
		d = mat[ 22 ] * di;
		mat[ 18 ] += mat[ 24 ] * d;
		mat[ 19 ] += mat[ 25 ] * d;
		mat[ 20 ] += mat[ 26 ] * d;
		mat[ 21 ] += mat[ 27 ] * d;
		mat[ 23 ] += mat[ 29 ] * d;
		d = mat[ 34 ] * di;
		mat[ 30 ] += mat[ 24 ] * d;
		mat[ 31 ] += mat[ 25 ] * d;
		mat[ 32 ] += mat[ 26 ] * d;
		mat[ 33 ] += mat[ 27 ] * d;
		mat[ 35 ] += mat[ 29 ] * d;
		di = mat[ 35 ];
		s *= di;
		mat[ 35 ] = d = 1.0f / di;
		mat[ 30 ] *= d;
		mat[ 31 ] *= d;
		mat[ 32 ] *= d;
		mat[ 33 ] *= d;
		mat[ 34 ] *= d;
		d = -d;
		mat[ 5 ] *= d;
		mat[ 11 ] *= d;
		mat[ 17 ] *= d;
		mat[ 23 ] *= d;
		mat[ 29 ] *= d;
		d = mat[ 5 ] * di;
		mat[ 0 ] += mat[ 30 ] * d;
		mat[ 1 ] += mat[ 31 ] * d;
		mat[ 2 ] += mat[ 32 ] * d;
		mat[ 3 ] += mat[ 33 ] * d;
		mat[ 4 ] += mat[ 34 ] * d;
		d = mat[ 11 ] * di;
		mat[ 6 ] += mat[ 30 ] * d;
		mat[ 7 ] += mat[ 31 ] * d;
		mat[ 8 ] += mat[ 32 ] * d;
		mat[ 9 ] += mat[ 33 ] * d;
		mat[ 10 ] += mat[ 34 ] * d;
		d = mat[ 17 ] * di;
		mat[ 12 ] += mat[ 30 ] * d;
		mat[ 13 ] += mat[ 31 ] * d;
		mat[ 14 ] += mat[ 32 ] * d;
		mat[ 15 ] += mat[ 33 ] * d;
		mat[ 16 ] += mat[ 34 ] * d;
		d = mat[ 23 ] * di;
		mat[ 18 ] += mat[ 30 ] * d;
		mat[ 19 ] += mat[ 31 ] * d;
		mat[ 20 ] += mat[ 32 ] * d;
		mat[ 21 ] += mat[ 33 ] * d;
		mat[ 22 ] += mat[ 34 ] * d;
		d = mat[ 29 ] * di;
		mat[ 24 ] += mat[ 30 ] * d;
		mat[ 25 ] += mat[ 31 ] * d;
		mat[ 26 ] += mat[ 32 ] * d;
		mat[ 27 ] += mat[ 33 ] * d;
		mat[ 28 ] += mat[ 34 ] * d;

        // FIXME: check if the inversion is correct
        return true;
	}

	template bool Matrix6<float>::inverseSelf( );

	template bool Matrix6<double>::inverseSelf( );


	template<>
	Matrix2<float> Matrix2<float>::fromString( const String& s )
	{
		Matrix2<float> m;

		DataIterator it( s );
		String token;
		String deliminators( "\n\r\t " );
		for( size_t i = 0; i < 2; i++ ) {
			for( size_t k = 0; k < 2; k++ ) {
				if( !it.nextToken( token, deliminators ) )
					throw CVTException( "Could not create Matrix from String!" );
				m[ i ][ k ] = token.toFloat( );
			}
		}

		return m;
	}

	template<>
	Matrix2<double> Matrix2<double>::fromString( const String& s )
	{
		Matrix2<double> m;

		DataIterator it( s );
		String token;
		String deliminators( "\n\r\t " );
		for( size_t i = 0; i < 2; i++ ) {
			for( size_t k = 0; k < 2; k++ ) {
				if( !it.nextToken( token, deliminators ) )
					throw CVTException( "Could not create Matrix from String!" );
				m[ i ][ k ] = token.toDouble( );
			}
		}

		return m;
	}

	template<>
	Matrix2<int> Matrix2<int>::fromString( const String& s )
	{
		Matrix2<int> m;

		DataIterator it( s );
		String token;
		String deliminators( "\n\r\t " );
		for( size_t i = 0; i < 2; i++ ) {
			for( size_t k = 0; k < 2; k++ ) {
				if( !it.nextToken( token, deliminators ) )
					throw CVTException( "Could not create Matrix from String!" );
				m[ i ][ k ] = token.toInteger( );
			}
		}

		return m;
	}

	template<>
	Matrix3<float> Matrix3<float>::fromString( const String& s )
	{
		Matrix3<float> m;

		DataIterator it( s );
		String token;
		String deliminators( "\n\r\t " );
		for( size_t i = 0; i < 3; i++ ) {
			for( size_t k = 0; k < 3; k++ ) {
				if( !it.nextToken( token, deliminators ) )
					throw CVTException( "Could not create Matrix from String!" );
				m[ i ][ k ] = token.toFloat( );
			}
		}

		return m;
	}

	template<>
	Matrix3<double> Matrix3<double>::fromString( const String& s )
	{
		Matrix3<double> m;

		DataIterator it( s );
		String token;
		String deliminators( "\n\r\t " );
		for( size_t i = 0; i < 3; i++ ) {
			for( size_t k = 0; k < 3; k++ ) {
				if( !it.nextToken( token, deliminators ) )
					throw CVTException( "Could not create Matrix from String!" );
				m[ i ][ k ] = token.toDouble( );
			}
		}

		return m;
	}

	template<>
	Matrix3<int> Matrix3<int>::fromString( const String& s )
	{
		Matrix3<int> m;

		DataIterator it( s );
		String token;
		String deliminators( "\n\r\t " );
		for( size_t i = 0; i < 3; i++ ) {
			for( size_t k = 0; k < 3; k++ ) {
				if( !it.nextToken( token, deliminators ) )
					throw CVTException( "Could not create Matrix from String!" );
				m[ i ][ k ] = token.toInteger( );
			}
		}

		return m;
	}

	template<>
	Matrix4<float> Matrix4<float>::fromString( const String& s )
	{
		Matrix4<float> m;

		DataIterator it( s );
		String token;
		String deliminators( "\n\r\t " );
		for( size_t i = 0; i < 4; i++ ) {
			for( size_t k = 0; k < 4; k++ ) {
				if( !it.nextToken( token, deliminators ) )
					throw CVTException( "Could not create Matrix from String!" );
				m[ i ][ k ] = token.toFloat( );
			}
		}

		return m;
	}

	template<>
	Matrix4<double> Matrix4<double>::fromString( const String& s )
	{
		Matrix4<double> m;

		DataIterator it( s );
		String token;
		String deliminators( "\n\r\t " );
		for( size_t i = 0; i < 4; i++ ) {
			for( size_t k = 0; k < 4; k++ ) {
				if( !it.nextToken( token, deliminators ) )
					throw CVTException( "Could not create Matrix from String!" );
				m[ i ][ k ] = token.toDouble( );
			}
		}

		return m;
	}

	template<>
	Matrix4<int> Matrix4<int>::fromString( const String& s )
	{
		Matrix4<int> m;

		DataIterator it( s );
		String token;
		String deliminators( "\n\r\t " );
		for( size_t i = 0; i < 4; i++ ) {
			for( size_t k = 0; k < 4; k++ ) {
				if( !it.nextToken( token, deliminators ) )
					throw CVTException( "Could not create Matrix from String!" );
				m[ i ][ k ] = token.toInteger( );
			}
		}

		return m;
	}

#define ROTAPPLYLEFT3( m, c, s, i, k ) \
	do { \
		T tmp1, tmp2; \
		tmp1 = s * m[ k ][ i ] + m[ i ][ i ] * c; \
		tmp2 = c * m[ k ][ i ] - m[ i ][ i ] * s; \
		m[ i ][ i ] = tmp1;	\
		m[ k ][ i ] = tmp2;	\
		tmp1 = s * m[ k ][ k ] + m[ i ][ k ] * c; \
		tmp2 = c * m[ k ][ k ] - m[ i ][ k ] * s; \
		m[ i ][ k ] = tmp1;	\
		m[ k ][ k ] = tmp2;	\
		int h = 3 - i - k; \
		tmp1 = s * m[ k ][ h ] + m[ i ][ h ] * c; \
		tmp2 = c * m[ k ][ h ] - m[ i ][ h ] * s; \
		m[ i ][ h ] = tmp1;	\
		m[ k ][ h ] = tmp2;	\
	} while( 0 )

#define ROTAPPLYRIGHT3( m, c, s, i, k )	\
	do { \
		T tmp1, tmp2; \
		tmp1 = c * m[ i ][ i ] - m[ i ][ k ] * s; \
		tmp2 = s * m[ i ][ i ] + m[ i ][ k ] * c; \
		m[ i ][ i ] = tmp1;	\
		m[ i ][ k ] = tmp2;	\
		tmp1 = c * m[ k ][ i ] - m[ k ][ k ] * s; \
		tmp2 = s * m[ k ][ i ] + m[ k ][ k ] * c; \
		m[ k ][ i ] = tmp1;	\
		m[ k ][ k ] = tmp2;	\
		int h = 3 - i - k; \
		tmp1 = c * m[ h ][ i ] - m[ h ][ k ] * s; \
		tmp2 = s * m[ h ][ i ] + m[ h ][ k ] * c; \
		m[ h ][ i ] = tmp1;	\
		m[ h ][ k ] = tmp2;	\
	} while( 0 )

#define JACOBIAPPLY3( m, c, s, i, k ) \
	do { \
		T tmp1, tmp2; \
		tmp1 = c * c * m[ i ][ i ] + s * s * m[ k ][ k ] - 2.0f * s * c * m[ i ][ k ]; \
		tmp2 = s * s * m[ i ][ i ] + c * c * m[ k ][ k ] + 2.0f * s * c * m[ i ][ k ]; \
		m[ i ][ i ] = tmp1;	\
		m[ k ][ k ] = tmp2;	\
		m[ i ][ k ] = 0; \
		m[ k ][ i ] = 0; \
		int h = 3 - i - k; \
		tmp1 = c * m[ i ][ h ] - s * m[ k ][ h ]; \
		tmp2 = s * m[ i ][ h ] + c * m[ k ][ h ]; \
		m[ i ][ h ] = tmp1;	\
		m[ k ][ h ] = tmp2;	\
		tmp1 = c * m[ h ][ i ] - s * m[ h ][ k ]; \
		tmp2 = s * m[ h ][ i ] + c * m[ h ][ k ]; \
		m[ h ][ i ] = tmp1;	\
		m[ h ][ k ] = tmp2;	\
	} while( 0 )

#define ROTAPPLYRIGHT4( m, c, s, i, k )	\
	do { \
		T tmp1, tmp2; \
		tmp1 = c * m[ 0 ][ i ] - m[ 0 ][ k ] * s; \
		tmp2 = s * m[ 0 ][ i ] + m[ 0 ][ k ] * c; \
		m[ 0 ][ i ] = tmp1;	\
		m[ 0 ][ k ] = tmp2;	\
		tmp1 = c * m[ 1 ][ i ] - m[ 1 ][ k ] * s; \
		tmp2 = s * m[ 1 ][ i ] + m[ 1 ][ k ] * c; \
		m[ 1 ][ i ] = tmp1;	\
		m[ 1 ][ k ] = tmp2;	\
		tmp1 = c * m[ 2 ][ i ] - m[ 2 ][ k ] * s; \
		tmp2 = s * m[ 2 ][ i ] + m[ 2 ][ k ] * c; \
		m[ 2 ][ i ] = tmp1;	\
		m[ 2 ][ k ] = tmp2;	\
		tmp1 = c * m[ 3 ][ i ] - m[ 3 ][ k ] * s; \
		tmp2 = s * m[ 3 ][ i ] + m[ 3 ][ k ] * c; \
		m[ 3 ][ i ] = tmp1;	\
		m[ 3 ][ k ] = tmp2;	\
	} while( 0 )

#define JACOBIAPPLY4( m, c, s, i, k ) \
	do { \
		static int _other[ 4 ][ 4 ] = { {-1, 2, 1, 1}, {2, -1, 0, 0}, {1, 0, -1, 0}, {1, 0, 0, -1} }; \
		T tmp1, tmp2; \
		tmp1 = c * c * m[ i ][ i ] + s * s * m[ k ][ k ] - 2.0f * s * c * m[ i ][ k ]; \
		tmp2 = s * s * m[ i ][ i ] + c * c * m[ k ][ k ] + 2.0f * s * c * m[ i ][ k ]; \
		m[ i ][ i ] = tmp1;	\
		m[ k ][ k ] = tmp2;	\
		m[ i ][ k ] = 0; \
		m[ k ][ i ] = 0; \
		int h = _other[ i ][ k ]; \
		tmp1 = c * m[ i ][ h ] - s * m[ k ][ h ]; \
		tmp2 = s * m[ i ][ h ] + c * m[ k ][ h ]; \
		m[ i ][ h ] = tmp1;	\
		m[ k ][ h ] = tmp2;	\
		tmp1 = c * m[ h ][ i ] - s * m[ h ][ k ]; \
		tmp2 = s * m[ h ][ i ] + c * m[ h ][ k ]; \
		m[ h ][ i ] = tmp1;	\
		m[ h ][ k ] = tmp2;	\
		h = 6 - i - k - h; \
		tmp1 = c * m[ i ][ h ] - s * m[ k ][ h ]; \
		tmp2 = s * m[ i ][ h ] + c * m[ k ][ h ]; \
		m[ i ][ h ] = tmp1;	\
		m[ k ][ h ] = tmp2;	\
		tmp1 = c * m[ h ][ i ] - s * m[ h ][ k ]; \
		tmp2 = s * m[ h ][ i ] + c * m[ h ][ k ]; \
		m[ h ][ i ] = tmp1;	\
		m[ h ][ k ] = tmp2;	\
	} while( 0 )

	template<typename T>
	void Matrix2<T>::svd( Matrix2<T>& u, Matrix2<T>& mat,  Matrix2<T>& v ) const
	{
		T c, s;

		mat = *this;

		/* diagonalize */

		/* make 2 x 2 symmetric */
		Math::givens( c, s, mat[ 0 ][ 0 ] + mat[ 1 ][ 1 ], mat[ 1 ][ 0 ] - mat[ 0 ][ 1 ] );
		mat *= Matrix2<T>( c, s, -s, c );

		/* apply the givens rotation to V^T */
		v = Matrix2<T>( c, s, -s, c );

		/* make 2 x 2 diagonal, apply jacobi */
		Math::jacobi( c, s, mat[ 0 ][ 0 ], mat[ 1 ][ 0 ], mat[ 1 ][ 1 ] );

		Matrix2<T> jl( c, -s, s, c );
		Matrix2<T> jr( c,  s, -s, c );
		mat = jl * mat * jr;

		/* apply the jacobi rotation to V^T */
		v *= Matrix2<T>( c, s, -s, c );

		/* apply the jacobi rotation to U */
		u = Matrix2<T>( c, s, -s, c );

		/* make singular values positive */
		for( int i = 0; i < 2; i++ ) {
			if( mat[ i ][ i ] < 0 ) {
				mat[ i ][ i ] = -mat[ i ][ i ];
				for( int k = 0; k < 2; k++ )
					u[ k ][ i ] = -u[ k ][ i ];
			}
		}

		/* sort singular values */
		/* bring largest singular value to position 0 */
		if( mat[ 0 ][ 0 ] < mat[ 1 ][ 1 ] ) {
			T tmp;
			for( int i = 0; i < 2; i++ ) {
				tmp = u[ i ][ 0 ];
				u[ i ][ 0 ] = u[ i ][ 1 ];
				u[ i ][ 1 ] = tmp;

				tmp = v[ i ][ 0 ];
				v[ i ][ 0 ] = v[ i ][ 1 ];
				v[ i ][ 1 ] = tmp;
			}
			tmp = mat[ 0 ][ 0 ];
			mat[ 0 ][ 0 ] = mat[ 1 ][ 1 ];
			mat[ 1 ][ 1 ] = tmp;
		}
	}

	template void Matrix2f::svd( Matrix2f&, Matrix2f&, Matrix2f& ) const;

	template void Matrix2d::svd( Matrix2d&, Matrix2d&, Matrix2d& ) const;

	template<typename T>
	void Matrix3<T>::svd( Matrix3<T>& u, Matrix3<T>& mat,  Matrix3<T>& v ) const
	{
		T c, s;
		bool finished;
		T eps = Math::epsilon<T>( );

		mat = *this;
		u.setIdentity( );
		v.setIdentity( );


		/* diagonalize */
		do {
			finished = true;
			for( int i = 0; i < 2; i++ ) {
				for( int k = i + 1; k < 3; k++ ) {
					if( Math::abs( mat[ i ][ k ] ) >= eps || Math::abs( mat[ k ][ i ] ) >= eps  ) {
						finished = false;
						/* make 2 x 2 symmetric */
						Math::givens( c, s, mat[ i ][ i ] + mat[ k ][ k ], mat[ k ][ i ] - mat[ i ][ k ] );
						ROTAPPLYRIGHT3( mat, c, s, i, k );

						/* apply the givens rotation to V^T */
						ROTAPPLYRIGHT3( v, c, s, i, k );

						/* make 2 x 2 diagonal, apply jacobi */
						Math::jacobi( c, s, mat[ i ][ i ], mat[ k ][ i ], mat[ k ][ k ] );
						JACOBIAPPLY3( mat, c, s, i, k );

						/* apply the jacobi rotation to V^T */
						ROTAPPLYRIGHT3( v, c, s, i, k );

						/* apply the jacobi rotation to U */
						ROTAPPLYRIGHT3( u, c, s, i, k );
					}
				}
			}
		} while( !finished );

		/* make singular values positive */
		for( int i = 0; i < 3; i++ ) {
			if( mat[ i ][ i ] < 0 ) {
				mat[ i ][ i ] = -mat[ i ][ i ];
				for( int k = 0; k < 3; k++ )
					u[ k ][ i ] = -u[ k ][ i ];
			}
		}

		/* sort singular values */
		int imax;
		imax = ( mat[ 0 ][ 0 ] > mat[ 1 ][ 1 ] ) ? 0 : 1;
		if( mat[ imax ][ imax ] < mat[ 2 ][ 2 ] ) imax = 2;
		/* bring largest singular value to position 0 */
		if( imax != 0 ) {
			T tmp;
			for( int i = 0; i < 3; i++ ) {
				tmp = u[ i ][ 0 ];
				u[ i ][ 0 ] = u[ i ][ imax ];
				u[ i ][ imax ] = tmp;

				tmp = v[ i ][ 0 ];
				v[ i ][ 0 ] = v[ i ][ imax ];
				v[ i ][ imax ] = tmp;
			}
			tmp = mat[ 0 ][ 0 ];
			mat[ 0 ][ 0 ] = mat[ imax ][ imax ];
			mat[ imax ][ imax ] = tmp;
		}
		/* swap singular values 1 and 2 if necessary */
		if( mat[ 1 ][ 1 ] < mat[ 2 ][ 2 ] ) {
			T tmp;
			for( int i = 0; i < 3; i++ ) {
				tmp = u[ i ][ 1 ];
				u[ i ][ 1 ] = u[ i ][ 2 ];
				u[ i ][ 2 ] = tmp;

				tmp = v[ i ][ 1 ];
				v[ i ][ 1 ] = v[ i ][ 2 ];
				v[ i ][ 2 ] = tmp;
			}
			tmp = mat[ 1 ][ 1 ];
			mat[ 1 ][ 1 ] = mat[ 2 ][ 2 ];
			mat[ 2 ][ 2 ] = tmp;
		}
	}

	template void Matrix3f::svd( Matrix3f&, Matrix3f&, Matrix3f& ) const;

	template void Matrix3d::svd( Matrix3d&, Matrix3d&, Matrix3d& ) const;


	template<typename T>
	void Matrix4<T>::svd( Matrix4<T>& u, Matrix4<T>& mat,  Matrix4<T>& v ) const
	{
		T c, s;
		bool finished;
		T eps = Math::epsilon<T>( );

		mat = *this;
		u.setIdentity( );
		v.setIdentity( );

		/* diagonalize */
		do {
			finished = true;
			for( int i = 0; i < 3; i++ ) {
				for( int k = i + 1; k < 4; k++ ) {
					if( Math::abs( mat[ i ][ k ] ) >= eps || Math::abs( mat[ k ][ i ] ) >= eps  ) {
						finished = false;
						/* make 2 x 2 symmetric */
						Math::givens( c, s, mat[ i ][ i ] + mat[ k ][ k ], mat[ k ][ i ] - mat[ i ][ k ] );
						ROTAPPLYRIGHT4( mat, c, s, i, k );

						/* apply the givens rotation to V^T */
						ROTAPPLYRIGHT4( v, c, s, i, k );

						/* make 2 x 2 diagonal, apply jacobi */
						Math::jacobi( c, s, mat[ i ][ i ], mat[ k ][ i ], mat[ k ][ k ] );
						JACOBIAPPLY4( mat, c, s, i, k );

						/* apply the jacobi rotation to V^T */
						ROTAPPLYRIGHT4( v, c, s, i, k );

						/* apply the jacobi rotation to U */
						ROTAPPLYRIGHT4( u, c, s, i, k );
					}
				}
			}
		} while( !finished );

		/* make singular values positive */
		for( int i = 0; i < 4; i++ ) {
			if( mat[ i ][ i ] < 0 ) {
				mat[ i ][ i ] = -mat[ i ][ i ];
				for( int k = 0; k < 4; k++ )
					u[ k ][ i ] = -u[ k ][ i ];
			}
		}

		/* sort singular values */
		int imax;
		imax = ( mat[ 0 ][ 0 ] > mat[ 1 ][ 1 ] ) ? 0 : 1;
		if( mat[ imax ][ imax ] < mat[ 2 ][ 2 ] ) imax = 2;
		if( mat[ imax ][ imax ] < mat[ 3 ][ 3 ] ) imax = 3;

		/* bring largest singular value to position 0 */
		if( imax != 0 ) {
			T tmp;
			for( int i = 0; i < 4; i++ ) {
				tmp = u[ i ][ 0 ];
				u[ i ][ 0 ] = u[ i ][ imax ];
				u[ i ][ imax ] = tmp;

				tmp = v[ i ][ 0 ];
				v[ i ][ 0 ] = v[ i ][ imax ];
				v[ i ][ imax ] = tmp;
			}
			tmp = mat[ 0 ][ 0 ];
			mat[ 0 ][ 0 ] = mat[ imax ][ imax ];
			mat[ imax ][ imax ] = tmp;
		}

		imax = ( mat[ 1 ][ 1 ] > mat[ 2 ][ 2 ] ) ? 1 : 2;
		if( mat[ imax ][ imax ] < mat[ 3 ][ 3 ] ) imax = 3;
		/* bring second largest singular value to position 1 */
		if( imax != 1 ) {
			T tmp;
			for( int i = 0; i < 4; i++ ) {
				tmp = u[ i ][ 1 ];
				u[ i ][ 1 ] = u[ i ][ imax ];
				u[ i ][ imax ] = tmp;

				tmp = v[ i ][ 1 ];
				v[ i ][ 1 ] = v[ i ][ imax ];
				v[ i ][ imax ] = tmp;
			}
			tmp = mat[ 1 ][ 1 ];
			mat[ 1 ][ 1 ] = mat[ imax ][ imax ];
			mat[ imax ][ imax ] = tmp;
		}
		/* swap singular values 2 and 3 if necessary */
		if( mat[ 2 ][ 2 ] < mat[ 3 ][ 3 ] ) {
			T tmp;
			for( int i = 0; i < 4; i++ ) {
				tmp = u[ i ][ 2 ];
				u[ i ][ 2 ] = u[ i ][ 3 ];
				u[ i ][ 3 ] = tmp;

				tmp = v[ i ][ 2 ];
				v[ i ][ 2 ] = v[ i ][ 3 ];
				v[ i ][ 3 ] = tmp;
			}
			tmp = mat[ 2 ][ 2 ];
			mat[ 2 ][ 2 ] = mat[ 3 ][ 3 ];
			mat[ 3 ][ 3 ] = tmp;
		}
	}

	template void Matrix4f::svd( Matrix4f&, Matrix4f&, Matrix4f& ) const;

	template void Matrix4d::svd( Matrix4d&, Matrix4d&, Matrix4d& ) const;


	BEGIN_CVTTEST( Matrix2 )
	Matrix2f mat, m2;
	bool b = true;

	mat.setIdentity( );
	mat *= 2.0f;
	m2 = mat.inverse( );
	mat *= m2;

	b = b && mat.isIdentity( );
	CVTTEST_PRINT( "inverse()", b );

	String s = m2.toString( );
	Matrix2f m2p = Matrix2f::fromString( s );
	b = ( m2p == m2 );
	CVTTEST_PRINT( "fromString()", b );

	{
		srand( time( NULL ) );

		Eigen::Matrix2f et;
		Eigen::Matrix2f mat = Eigen::Matrix2f::Random( );
		Eigen::JacobiSVD<Eigen::Matrix2f> svd( mat, Eigen::ComputeFullU | Eigen::ComputeFullV );

		/* Eigen SVD */
		et = svd.matrixU( ) * svd.singularValues( ).asDiagonal( ) * svd.matrixV( ).transpose( );

		std::cout << "U:\n" << svd.matrixU( ) << std::endl << std::endl;
		std::cout << "D:\n" << svd.singularValues( ) << std::endl << std::endl;
		std::cout << "V:\n" << svd.matrixV( ) << std::endl << std::endl;

		/* Copy mat to CVT matrix */
		Matrix2f m, u, d, v, t, t2;
		for( int y = 0; y < 2; y++ )
			for( int x = 0; x < 2; x++ )
				m[ y ][ x ] = mat( y, x );

		/* CVT SVD */
		m.svd( u, d, v );
		t = u * d * v.transpose( );

		std::cout << "U:\n" << u << std::endl << std::endl;
		std::cout << "D:\n" << d << std::endl << std::endl;
		std::cout << "V:\n" << v << std::endl << std::endl;

		for( int y = 0; y < 2; y++ )
			for( int x = 0; x < 2; x++ )
				t2[ y ][ x ] = et( y, x );

		std::cout << t2 << std::endl;
		std::cout << t << std::endl;

		CVTTEST_PRINT( "SVD", t.isEqual( t2, 100.0f * Math::EPSILONF ) );
		CVTTEST_PRINT( "SVD", t.isEqual( m, 100.0 * Math::EPSILONF ) );
	}

	return true;
	END_CVTTEST

	BEGIN_CVTTEST( Matrix3 )
	Matrix3f mat, m2;
	bool b = true;

	mat.setIdentity( );
	mat *= 2.0f;
	m2 = mat.inverse( );
	mat *= m2;

	b = b && mat.isIdentity( );
	CVTTEST_PRINT( "inverse()", b );


	mat.setIdentity( );
	mat *= 2.0f;
	mat.inverseSelf( );
	b = b && mat == Matrix3f(  0.5f, 0.0f,  0.0f,
		0.0f, 0.5f,  0.0f,
		0.0f, 0.0f,  0.5f );
	CVTTEST_PRINT( "inverse()", b );

	mat[ 0 ][ 0 ] =  1.0f;
	mat[ 0 ][ 1 ] =  0.0f;
	mat[ 0 ][ 2 ] =  4.0f;

	mat[ 1 ][ 0 ] =  1.0f;
	mat[ 1 ][ 1 ] =  1.0f;
	mat[ 1 ][ 2 ] =  6.0f;

	mat[ 2 ][ 0 ] = -3.0f;
	mat[ 2 ][ 1 ] =  0.0f;
	mat[ 2 ][ 2 ] = -10.0f;

	mat.inverseSelf( );
	b = b && mat == Matrix3f( -5.0f, 0.0f, -2.0f,
		-4.0f, 1.0f, -1.0f,
		1.5f, 0.0f,  0.5f );
	CVTTEST_PRINT( "inverse()", b );


	mat[ 0 ][ 0 ] = -2.0f;
	mat[ 0 ][ 1 ] =  2.0f;
	mat[ 0 ][ 2 ] = -3.0f;

	mat[ 1 ][ 0 ] = -1.0f;
	mat[ 1 ][ 1 ] =  1.0f;
	mat[ 1 ][ 2 ] =  3.0f;

	mat[ 2 ][ 0 ] =  2.0f;
	mat[ 2 ][ 1 ] =  0.0f;
	mat[ 2 ][ 2 ] = -1.0f;

	b = b && mat.determinant( ) == 18.0f;
	CVTTEST_PRINT( "determinant()", b );


	String s( " 20.0 34.5  56.0 \n \t 23.0 34 22 \n 12 12 33" );
	Matrix3f mf = Matrix3f::fromString( s );
	b  = ( mf[ 0 ][ 0 ] == 20.0f );
	b &= ( mf[ 0 ][ 1 ] == 34.5f );
	b &= ( mf[ 0 ][ 2 ] == 56.0f );
	b &= ( mf[ 1 ][ 0 ] == 23.0f );
	b &= ( mf[ 1 ][ 1 ] == 34.0f );
	b &= ( mf[ 1 ][ 2 ] == 22.0f );
	b &= ( mf[ 2 ][ 0 ] == 12.0f );
	b &= ( mf[ 2 ][ 1 ] == 12.0f );
	b &= ( mf[ 2 ][ 2 ] == 33.0f );
	CVTTEST_PRINT( "fromString()", b );

	{
		srand( time( NULL ) );

		Eigen::Matrix3f et;
		Eigen::Matrix3f mat = Eigen::Matrix3f::Random( );
		Eigen::JacobiSVD<Eigen::Matrix3f> svd( mat, Eigen::ComputeFullU | Eigen::ComputeFullV );

		/* Eigen SVD */
		et = svd.matrixU( ) * svd.singularValues( ).asDiagonal( ) * svd.matrixV( ).transpose( );

		/* Copy mat to CVT matrix */
		Matrix3f m, u, d, v, t, t2;
		for( int y = 0; y < 3; y++ )
			for( int x = 0; x < 3; x++ )
				m[ y ][ x ] = mat( y, x );

		/* CVT SVD */
		m.svd( u, d, v );
		t = u * d * v.transpose( );


		for( int y = 0; y < 3; y++ )
			for( int x = 0; x < 3; x++ )
				t2[ y ][ x ] = et( y, x );

		CVTTEST_PRINT( "SVD", t.isEqual( t2, 100.0f * Math::EPSILONF ) );
		CVTTEST_PRINT( "SVD", t.isEqual( m, 100.0 * Math::EPSILONF ) );
	}

	return true;
	END_CVTTEST

	BEGIN_CVTTEST( Matrix4 )
	Matrix4f mat, m2;
	bool b = true;

	mat.setIdentity( );
	mat *= 2.0f;
	m2 = mat.inverse( );
	mat *= m2;

	b = b && mat.isIdentity( );
	CVTTEST_PRINT( "inverse()", b );


	mat.setIdentity( );
	mat *= 2.0f;
	mat.inverseSelf( );
	b = b && mat == Matrix4f( 0.5f, 0.0f,  0.0f, 0.0f,
		0.0f, 0.5f,  0.0f, 0.0f,
		0.0f, 0.0f,  0.5f, 0.0f,
		0.0f, 0.0f,  0.0f, 0.5f );

	CVTTEST_PRINT( "inverse()", b );

	mat[ 0 ][ 0 ] =  3.0f;
	mat[ 0 ][ 1 ] =  2.0f;
	mat[ 0 ][ 2 ] =  0.0f;
	mat[ 0 ][ 3 ] =  1.0f;

	mat[ 1 ][ 0 ] =  4.0f;
	mat[ 1 ][ 1 ] =  0.0f;
	mat[ 1 ][ 2 ] =  1.0f;
	mat[ 1 ][ 3 ] =  2.0f;

	mat[ 2 ][ 0 ] =  3.0f;
	mat[ 2 ][ 1 ] =  0.0f;
	mat[ 2 ][ 2 ] =  2.0f;
	mat[ 2 ][ 3 ] =  1.0f;

	mat[ 3 ][ 0 ] =  9.0f;
	mat[ 3 ][ 1 ] =  3.0f;
	mat[ 3 ][ 2 ] =  2.0f;
	mat[ 3 ][ 3 ] =  1.0f;

	b = b && mat.determinant( ) == 24.0f;
	CVTTEST_PRINT( "determinant()", b );

	mat[ 0 ][ 0 ] =  2.0f;
	mat[ 0 ][ 1 ] =  5.0f;
	mat[ 0 ][ 2 ] = -2.0f;
	mat[ 0 ][ 3 ] = -1.0f;

	mat[ 1 ][ 0 ] =  3.0f;
	mat[ 1 ][ 1 ] =  4.0f;
	mat[ 1 ][ 2 ] =  1.0f;
	mat[ 1 ][ 3 ] =  2.0f;

	mat[ 2 ][ 0 ] = -1.0f;
	mat[ 2 ][ 1 ] =  9.0f;
	mat[ 2 ][ 2 ] =  6.0f;
	mat[ 2 ][ 3 ] =  7.0f;

	mat[ 3 ][ 0 ] =  8.0f;
	mat[ 3 ][ 1 ] =  1.0f;
	mat[ 3 ][ 2 ] =  3.0f;
	mat[ 3 ][ 3 ] =  2.0f;

	b = b && mat.determinant( ) == 301.0f;
	CVTTEST_PRINT( "determinant()", b );

	String s = mat.toString( );
	Matrix4f m2p = Matrix4f::fromString( s );
	b = ( m2p == mat );
	CVTTEST_PRINT( "fromString()", b );

	{
		srand( time( NULL ) );

		Eigen::Matrix4f et;
		Eigen::Matrix4f mat = Eigen::Matrix4f::Random( );
		Eigen::JacobiSVD<Eigen::Matrix4f> svd( mat, Eigen::ComputeFullU | Eigen::ComputeFullV );

		/* Eigen SVD */
		et = svd.matrixU( ) * svd.singularValues( ).asDiagonal( ) * svd.matrixV( ).transpose( );

		/* Copy mat to CVT matrix */
		Matrix4f m, u, d, v, t, t2;
		for( int y = 0; y < 4; y++ )
			for( int x = 0; x < 4; x++ )
				m[ y ][ x ] = mat( y, x );

		/* CVT SVD */
		m.svd( u, d, v );
		t = u * d * v.transpose( );


		for( int y = 0; y < 4; y++ )
			for( int x = 0; x < 4; x++ )
				t2[ y ][ x ] = et( y, x );

		CVTTEST_PRINT( "SVD", t.isEqual( t2, 100.0f * Math::EPSILONF ) );
		CVTTEST_PRINT( "SVD", t.isEqual( m, 100.0 * Math::EPSILONF ) );
	}

	return true;
	END_CVTTEST

}
