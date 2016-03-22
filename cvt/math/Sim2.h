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
 *  Sim2.h
 *
 *	Similarity in 2D 
 *
 *  Created by Sebastian Klose 
 *  Copyright 2011. All rights reserved.
 *
 */

#ifndef CVT_SIM2_H
#define CVT_SIM2_H

#include <Eigen/Core>
#include <cvt/math/Math.h>
#include <cvt/math/Matrix.h>

namespace cvt {
	/**
	 *	\class Sim2	Similarity in 2D
	 *	parameters:
	 *	p0:	tx
	 *	p1:	ty
	 *	p2:	alpha (rotation)
	 *	p3:	scale (exp(scale))
	 */
	template <typename T>
	class Sim2
	{
		public:
			static const size_t NPARAMS	= 4;
			typedef Eigen::Matrix<T, 3, 3> MatrixType;
			typedef Eigen::Matrix<T, 3, NPARAMS> JacMatType;
			typedef Eigen::Matrix<T, 3*NPARAMS, NPARAMS> HessMatType;
			typedef Eigen::Matrix<T, 2, NPARAMS> ScreenJacType;
			typedef Eigen::Matrix<T, NPARAMS, NPARAMS> ScreenHessType;
			typedef Eigen::Matrix<T, NPARAMS, 1> ParameterVectorType;
			typedef Eigen::Matrix<T, 3, 1> PointType;
			typedef Eigen::Matrix<T, 2, 1> SPType;

			Sim2();
			~Sim2(){};

			/* set: angles in radians! */
			void set( T scale, T alpha, T tx, T ty );
			void set( const Matrix3<T> & mat );
			
			/**
			 *	\brief apply delta parameters 
			 *	\param	delta	the delta to apply
			 *	\desc	forward update of pose: T( delta ) * T( current )
			 */
			void apply( const ParameterVectorType & delta );

			/**
			 *	\brief	apply delta parameters in inverse compositional way 
			 *	\param	delta	the delta to apply
			 *	\desc	This is meant for usage in inverse compositional approaches,
			 *			where the current pose represents the transformation from the
			 *			model to the object, but the delta parameters have been estimated
			 *			in the model coords:
			 *			T(delta) * p_model = inv(T(current)) * p_current
			 *			To update T(current), we need the following:
			 *			p_model = inv( T( delta ) ) * inv(T( current ))
			 *			so the update becomes T(current) = T(current) * T(delta)
			 */
			void applyInverse( const ParameterVectorType & delta );

			/* transform the point */
			void transform( PointType & warped, const PointType & p ) const;
			void transform( SPType & warped, const SPType & p ) const;
			
			/* transform the point: warped = current^-1 * p */
			void transformInverse( SPType & warped, const SPType & p ) const;

			/* get the jacobian at a certain point */
			void jacobian( JacMatType & J, const PointType & p ) const;

			/* hessian of the pose */
			void hessian( HessMatType & H, const PointType & p ) const;

			/* sp = proj( transform( p ) ) */
			void project( SPType & sp, const PointType & p ) const;

			/* sp = proj( transform( p ) ), J = d proj( transform( p ) ) / d params */
			void project( SPType & sp, ScreenJacType & J, const PointType & p ) const;

			/* p is already transformed with the current T in this case, but not yet projected! */
			void jacobianAroundT( JacMatType & J, const PointType & p ) const;

			/* sp is a screen point (transformed with current) */
			void screenJacobian( ScreenJacType & J, const Eigen::Matrix<T, 2, 1> & sp ) const;

			void screenHessian( ScreenHessType & wx, 
								ScreenHessType & wy,
							    const Eigen::Matrix<T, 2, 1> & sp ) const;

			/* get back the currently stored transformation matrix */
			const MatrixType & transformation() const { return _current; }
			MatrixType & transformation() { return _current; }

			void scale( T s );

		private:
			MatrixType		_current;
	};

	template < typename T >
	inline Sim2<T>::Sim2() : _current( MatrixType::Identity() )
	{
	}

	template < typename T>
	inline void Sim2<T>::set( T scale, T alpha, T tx, T ty )
	{
		T c = scale * Math::cos( alpha );
		T s = scale * Math::sin( alpha );
		_current( 0, 0 ) = c; 
		_current( 0, 1 ) = -s;
		_current( 0, 2 ) = tx;
		_current( 1, 0 ) = s; 
		_current( 1, 1 ) = c;
	   	_current( 1, 2 ) = ty;
		_current( 2, 0 ) = 0; 
		_current( 2, 1 ) = 0; 
		_current( 2, 2 ) = 1;
	}

	template <typename T>
	inline void Sim2<T>::set( const Matrix3<T> & mat )
	{
		_current( 0, 0 ) = mat[ 0 ][ 0 ];
		_current( 0, 1 ) = mat[ 0 ][ 1 ];
		_current( 0, 2 ) = mat[ 0 ][ 2 ];
		_current( 1, 0 ) = mat[ 1 ][ 0 ];
		_current( 1, 1 ) = mat[ 1 ][ 1 ];
		_current( 1, 2 ) = mat[ 1 ][ 2 ];
		_current( 2, 0 ) = mat[ 2 ][ 0 ];
		_current( 2, 1 ) = mat[ 2 ][ 1 ];
		_current( 2, 2 ) = mat[ 2 ][ 2 ];
	}

	template < typename T >
	inline void Sim2<T>::apply( const ParameterVectorType & delta )
	{
		MatrixType m;

		m( 0, 0 ) =  delta[ 3 ];
		m( 0, 1 ) = -delta[ 2 ];
		m( 0, 2 ) =  delta[ 0 ];

		m( 1, 0 ) =  delta[ 2 ];
		m( 1, 1 ) =  delta[ 3 ];
		m( 1, 2 ) =  delta[ 1 ];

		m( 2, 0 ) = 0;
		m( 2, 1 ) = 0;
		m( 2, 2 ) = 0;

		/* m = exp( m ) */
		cvt::Math::exponential( m, m );

		/* update the current transformation */
		_current = m * _current;
		//_current *= m;
	}

	template <typename T>
	inline void Sim2<T>::applyInverse( const ParameterVectorType & delta )
	{
		MatrixType m;

		m( 0, 0 ) =  delta[ 3 ];
		m( 0, 1 ) = -delta[ 2 ];
		m( 0, 2 ) =  delta[ 0 ];

		m( 1, 0 ) =  delta[ 2 ];
		m( 1, 1 ) =  delta[ 3 ];
		m( 1, 2 ) =  delta[ 1 ];

		m( 2, 0 ) = 0;
		m( 2, 1 ) = 0;
		m( 2, 2 ) = 0;

		/* m = exp( m ) */
		cvt::Math::exponential( m, m );

		/* update current in the inverse fashion */
		_current *= m;
	}

	template < typename T >
	inline void Sim2<T>::transform( PointType & warped, const PointType & p ) const
	{
		warped = _current * p;
	}

	template < typename T >
	inline void Sim2<T>::transform( Eigen::Matrix<T, 2, 1> & warped, const Eigen::Matrix<T, 2, 1> & p ) const
	{
		warped  = _current.template block<2, 2>( 0, 0 ) * p;
		warped += _current.template block<2, 1>( 0, 2 );
	}

	template < typename T >
	inline void Sim2<T>::transformInverse( Eigen::Matrix<T, 2, 1> & warped, const Eigen::Matrix<T, 2, 1> & p ) const
	{
		warped = p - _current.template block<2, 1>( 0, 2 );
		warped = _current.template block<2, 2>( 0, 0 ).transpose() * warped;
	}

	template < typename T >
	inline void Sim2<T>::jacobianAroundT( JacMatType & J, const PointType & p ) const
	{
		J( 0, 0 ) = p[ 2 ];
		J( 1, 0 ) =    0  ;
		J( 2, 0 ) =    0  ;

		J( 0, 1 ) =	   0  ;
		J( 1, 1 ) = p[ 2 ];
		J( 2, 1 ) =    0  ;

		J( 0, 2 ) =	-p[ 1 ];
		J( 1, 2 ) =  p[ 0 ];
		J( 2, 2 ) =     0  ;

		J( 0, 3 ) = p[ 0 ];
		J( 1, 3 ) = p[ 1 ];
		J( 2, 3 ) =    0  ;
	}

	template < typename T >
	inline void Sim2<T>::jacobian( JacMatType & J, const PointType & p ) const
	{
		PointType pp = _current * p;
		jacobianAroundT( J, pp );
	}

	template < typename T >
	inline void Sim2<T>::project( Eigen::Matrix<T, 2, 1> & sp, const PointType & p ) const
	{
		PointType pp = _current * p;

		sp[ 0 ] = pp[ 0 ] / pp[ 2 ];
		sp[ 1 ] = pp[ 1 ] / pp[ 2 ];
	}

	template < typename T >
	inline void Sim2<T>::project( Eigen::Matrix<T, 2, 1> & sp, ScreenJacType & J, const PointType & p ) const
	{
		project( sp, p );
		screenJacobian( J, sp );
	}

	template < typename T >
	inline void Sim2<T>::screenJacobian( ScreenJacType & J, const Eigen::Matrix<T, 2, 1> & sp ) const
	{
		J( 0, 0 ) = 1; 
		J( 0, 1 ) = 0; 
		J( 0, 2 ) = -sp[ 1 ]; 
		J( 0, 3 ) =  sp[ 0 ]; 

		J( 1, 0 ) = 0;
		J( 1, 1 ) = 1;
		J( 1, 2 ) = sp[ 0 ];
		J( 1, 3 ) = sp[ 1 ];
	}


	template <typename T>
	inline void Sim2<T>::hessian( HessMatType & H, const PointType & p ) const
	{
		H.setZero();

		// 0.5 * ( G_i * G_j + G_j * G_i )
		H( 0, 3 ) =  0.5 * p.z(); 
		H( 1, 2 ) =  0.5 * p.z(); 
		H( 3, 2 ) = -0.5 * p.z(); 
		H( 4, 3 ) =  0.5 * p.z(); 
		H( 6, 1 ) = -0.5 * p.z(); 
		H( 6, 2 ) =		  -p.x(); 
		H( 6, 3 ) =		  -p.y(); 
		H( 7, 0 ) =	 0.5 * p.z(); 
		H( 7, 2 ) =		  -p.y(); 
		H( 7, 3 ) =		   p.x(); 
		H( 9, 0 ) =	 0.5 * p.z(); 
		H( 9, 2 ) =		  -p.y(); 
		H( 9, 3 ) =		   p.x(); 
		H(10, 1 ) =	 0.5 * p.z(); 
		H(10, 2 ) =		   p.x(); 
		H(10, 3 ) =		   p.y(); 
	}

	template <typename T>
	inline void Sim2<T>::screenHessian( ScreenHessType & wx, 
									   ScreenHessType & wy,
									   const Eigen::Matrix<T, 2, 1> & sp ) const
	{
		T x = sp[ 0 ];
		T y = sp[ 1 ];

		wx << 0,    0,    0, 0.5,
			  0,    0, -0.5,   0,
			  0, -0.5,   -x,  -y,
			  0.5,  0,   -y,   x;

		wy << 0,   0, 0.5,   0,
			  0,   0,   0, 0.5,
			0.5,   0,  -y,   x,
			  0, 0.5,   x,   y;
	}
	
	template <typename T>
	inline void Sim2<T>::scale( T s )
	{
		_current.template block<2, 3>( 0, 0 ) *= s;
	}
}

#endif

