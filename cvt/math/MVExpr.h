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

#ifndef CVT_MVEXPR_H
#define CVT_MVEXPR_H

#include <cvt/math/VectorN.h>
#include <cvt/math/MatrixNM.h>

namespace cvt {

	enum MVExprType {
		MV_EXPR_ADD = 0,
		MV_EXPR_SUB,
		MV_EXPR_MUL
	};

	template<MVExprType type, typename T>
	struct MVExprOp {
		T operator()( T a, T b );
	};

	template<>
	struct MVExprOp<MV_EXPR_ADD, float> {
		float operator()( float a, float b ) { return a + b; }
	};

	template<>
	struct MVExprOp<MV_EXPR_SUB, float> {
		float operator()( float a, float b ) { return a - b; }
	};

	template<>
	struct MVExprOp<MV_EXPR_MUL, float> {
		float operator()( float a, float b ) { return a * b; }
	};


	template<typename XPR, size_t N, size_t M, typename T>
	class MVExpr
	{
		public:
			MVExpr( const XPR& op ) : _op( op ) {}

			T	   operator()( size_t row, size_t col ) const { return _op( row, col ); }


		private:
			const XPR _op;
	};


	template<typename T1, typename T2, size_t N, size_t M, typename T>
	class MVExprMul
	{
		public:
			MVExprMul( const T1& op1, const T2& op2 ) : _op1( op1 ), _op2( op2 ) {}

			T operator()( size_t row, size_t col ) const
			{
				T ret = 0;

				for( size_t i = 0; i < M; i++ )
					ret += _op1( row, i ) * _op2( i, col );

				return ret;
			}


		private:
			const T1  _op1;
			const T2  _op2;

	};

	template<typename T1, size_t N, size_t M, typename T>
	class MVExprTranspose
	{
		public:
			MVExprTranspose( const T1& op ) : _op( op ) {}

			T	   operator()( size_t row, size_t col ) const { return _op( col, row ); }


		private:
			const T1 _op;
	};


	template<typename T1, typename T2, size_t N, size_t M, typename T, MVExprType optype>
	class MVExprCWise
	{
		public:
			MVExprCWise( const T1& op1, const T2& op2 ) : _op1( op1 ), _op2( op2 ) {}

			T	operator()( size_t row, size_t col ) const
			{
				MVExprOp<optype,T> op;
				return op( _op1( row, col ), _op2( row, col ) );
			}


		private:
			const T1  _op1;
			const T2  _op2;
	};

	template<size_t N, size_t M, typename T>
	class MVExprMatrix
	{
		public:
			MVExprMatrix( const Matrix<N,M,T>& m ) : _mat( m ) {}
			T		operator()( size_t row, size_t col ) const { return _mat( row, col ); }

		private:
			const Matrix<N,M,T>& _mat;
	};

	template<size_t N, typename T>
	class MVExprVector
	{
		public:
			MVExprVector( const Vector<N,T>& v ) : _vec( v ) {}
			T		operator()( size_t row, size_t col ) const { return _vec[ col ]; }

		private:
			const Vector<N,T>&	_vec;
	};

	template<typename T>
	class MVExprScalar
	{
		public:
			MVExprScalar( T s ) : _scalar( s ) {}
			T		operator()( size_t row, size_t col ) const { return _scalar; }

		private:
			T	_scalar;
	};


	/*
		Matrix * Matrix
	 */
	template<size_t N, size_t M, size_t O, typename T>
	inline MVExpr<MVExprMul<MVExprMatrix<N,O,T>, MVExprMatrix<O,M,T>, N,M,T>,N,M,T> operator*( const Matrix<N,O,T>& m1, const Matrix<O,M,T>& m2 )
	{
		return  MVExpr<MVExprMul<MVExprMatrix<N,O,T>, MVExprMatrix<O,M,T>, N,M,T>,N,M,T>( MVExprMul<MVExprMatrix<N,O,T>, MVExprMatrix<O,M,T>, N,M,T>( MVExprMatrix<N,O,T>(m1), MVExprMatrix<O,M,T>(m2) ) );
	}


	/* Assignment */
	template<size_t N, size_t M, typename T>
	template<typename XPR>
	Matrix<N,M,T>& Matrix<N,M,T>::operator=( const MVExpr<XPR, N, M, T>& expr )
	{
		for( size_t m = 0; m < M; m++ ) {
			for( size_t n = 0; n < M; n++ ) {
				this->operator()(n,m) = expr( n, m );
			}
		}
		return *this;
	}



}

#endif
