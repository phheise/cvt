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
 *  SL3Transform.h
 *  CVTools
 *
 *  Created by Sebastian Klose on 08.10.10.
 *  Copyright 2010. All rights reserved.
 *
 */

#ifndef CVT_SL3_H
#define CVT_SL3_H

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <cvt/math/Math.h>
#include <cvt/math/Matrix.h>

namespace cvt {
	template <typename T>
	class SL3
	{
		public:
			static const size_t NPARAMS	= 8;
			typedef Eigen::Matrix<T, 3, 3> MatrixType;
			typedef Eigen::Matrix<T, 3, NPARAMS> JacMatType;
			typedef Eigen::Matrix<T, 3*NPARAMS, NPARAMS> HessMatType;
			typedef Eigen::Matrix<T, 2, NPARAMS> ScreenJacType;
			typedef Eigen::Matrix<T, NPARAMS, NPARAMS> ScreenHessType;
			typedef Eigen::Matrix<T, NPARAMS, 1> ParameterVectorType;
			typedef Eigen::Matrix<T, 3, 1> PointType;

			SL3();
			~SL3(){}

			/* set: angles in radians! */
			void set( T alpha, T phi, T sx, T sy, T tx, T ty, T v0, T v1 );
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

			/* get the jacobian at a certain point */
			void jacobian( JacMatType & J, const PointType & p ) const;

			/* hessian of the pose */
			void hessian( HessMatType & H, const PointType & p ) const;

			/* sp = proj( transform( p ) ) */
			void project( Eigen::Matrix<T, 2, 1> & sp, const PointType & p ) const;

			/* sp = proj( transform( p ) ), J = d proj( transform( p ) ) / d params */
			void project( Eigen::Matrix<T, 2, 1> & sp, ScreenJacType & J, const PointType & p ) const;

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

			static const MatrixType& generator( size_t i ) { return _generators[ i ]; }

		private:
			MatrixType		_current;

			static MatrixType	_generators[ NPARAMS ];
	};

	template <typename T>
	static typename SL3<T>::MatrixType _initGenerator( size_t i )
	{
		typename SL3<T>::MatrixType ret;
		switch( i ){
			case 0: ret << 0, 0, 1, 0,  0, 0, 0, 0, 0; break;
			case 1: ret << 0, 0, 0, 0,  0, 1, 0, 0, 0; break;
			case 2: ret << 0, 1, 0, 0,  0, 0, 0, 0, 0; break;
			case 3: ret << 0, 0, 0, 1,  0, 0, 0, 0, 0; break;
			case 4: ret << 1, 0, 0, 0, -1, 0, 0, 0, 0; break;
			case 5: ret << 0, 0, 0, 0, -1, 0, 0, 0, 1; break;
			case 6: ret << 0, 0, 0, 0,  0, 0, 1, 0, 0; break;
			default: ret << 0, 0, 0, 0,  0, 0, 0, 1, 0; break;
		}
		return ret;
	}

	template <typename T>
	typename SL3<T>::MatrixType SL3<T>::_generators[] =
	{
		_initGenerator<T>( 0 ),
		_initGenerator<T>( 1 ),
		_initGenerator<T>( 2 ),
		_initGenerator<T>( 3 ),
		_initGenerator<T>( 4 ),
		_initGenerator<T>( 5 ),
		_initGenerator<T>( 6 ),
		_initGenerator<T>( 7 )
	};

	template < typename T >
	inline SL3<T>::SL3() : _current( MatrixType::Identity() )
	{
	}

	template < typename T>
	inline void SL3<T>::set( T alpha, T phi, T sx, T sy, T tx, T ty, T v0, T v1 )
	{
        Eigen::Transform<T, 2, Eigen::Affine> transform;
        Eigen::Matrix<T, 2, 1> s( sx, sy );

        transform.setIdentity();

        transform.rotate( alpha ).rotate( -phi ).scale( s ).rotate( phi );

        const Eigen::Matrix<T, 3, 3> & affine = transform.matrix();

		_current( 0, 0 ) = affine( 0, 0 ); _current( 0, 1 ) = affine( 0, 1 ); _current( 0, 2 ) = tx;
		_current( 1, 0 ) = affine( 1, 0 ); _current( 1, 1 ) = affine( 1, 1 ); _current( 1, 2 ) = ty;
		_current( 2, 0 ) = v0; _current( 2, 1 ) = v1; _current( 2, 2 ) = 1.0f;
	}
	
	template <typename T>
	inline void SL3<T>::set( const Matrix3<T> & mat )
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
	inline void SL3<T>::apply( const ParameterVectorType & delta )
	{
		MatrixType m;

		m( 0, 0 ) = delta[ 4 ];
		m( 0, 1 ) = delta[ 2 ];
		m( 0, 2 ) = delta[ 0 ];

		m( 1, 0 ) = delta[ 3 ];
		m( 1, 1 ) = -delta[ 4 ] - delta[5];
		m( 1, 2 ) = delta[ 1 ];

		m( 2, 0 ) = delta[ 6 ];
		m( 2, 1 ) = delta[ 7 ];
		m( 2, 2 ) = delta[ 5 ];

		/* m = exp( m ) */
		cvt::Math::exponential( m, m );

		/* update the current transformation */
		_current = m * _current;
		//_current *= m;
	}

	template <typename T>
	inline void SL3<T>::applyInverse( const ParameterVectorType & delta )
	{
		MatrixType m;

		m( 0, 0 ) = delta[ 4 ];
		m( 0, 1 ) = delta[ 2 ];
		m( 0, 2 ) = delta[ 0 ];

		m( 1, 0 ) = delta[ 3 ];
		m( 1, 1 ) = -delta[ 4 ] - delta[5];
		m( 1, 2 ) = delta[ 1 ];

		m( 2, 0 ) = delta[ 6 ];
		m( 2, 1 ) = delta[ 7 ];
		m( 2, 2 ) = delta[ 5 ];

		/* m = exp( m ) */
		cvt::Math::exponential( m, m );

//		std::cout << m << std::endl;

		/* update current in the inverse fashion */
		_current *= m;
	}

	template < typename T >
	inline void SL3<T>::transform( PointType & warped, const PointType & p ) const
	{
		warped = _current * p;
	}

	template < typename T >
	inline void SL3<T>::jacobianAroundT( JacMatType & J, const PointType & p ) const
	{
		J( 0, 0 ) = p[ 2 ];
		J( 1, 0 ) =    0  ;
		J( 2, 0 ) =    0  ;

		J( 0, 1 ) =	   0  ;
		J( 1, 1 ) = p[ 2 ];
		J( 2, 1 ) =    0  ;

		J( 0, 2 ) =	p[ 1 ];
		J( 1, 2 ) =    0  ;
		J( 2, 2 ) =    0  ;

		J( 0, 3 ) =	   0  ;
		J( 1, 3 ) = p[ 0 ];
		J( 2, 3 ) =    0  ;

		J( 0, 4 ) =	 p[ 0 ];
		J( 1, 4 ) = -p[ 1 ];
		J( 2, 4 ) =     0  ;

		J( 0, 5 ) =	    0  ;
		J( 1, 5 ) = -p[ 1 ];
		J( 2, 5 ) =  p[ 2 ];

		J( 0, 6 ) =	   0  ;
		J( 1, 6 ) =    0  ;
		J( 2, 6 ) = p[ 0 ];

		J( 0, 7 ) =	   0  ;
		J( 1, 7 ) =    0  ;
		J( 2, 7 ) = p[ 1 ];
	}

	template < typename T >
	inline void SL3<T>::jacobian( JacMatType & J, const PointType & p ) const
	{
		PointType pp = _current * p;
		jacobianAroundT( J, pp );
	}

	template < typename T >
	inline void SL3<T>::project( Eigen::Matrix<T, 2, 1> & sp, const PointType & p ) const
	{
		PointType pp = _current * p;

		sp[ 0 ] = pp[ 0 ] / pp[ 2 ];
		sp[ 1 ] = pp[ 1 ] / pp[ 2 ];
	}

	template < typename T >
	inline void SL3<T>::project( Eigen::Matrix<T, 2, 1> & sp, ScreenJacType & J, const PointType & p ) const
	{
		project( sp, p );
		screenJacobian( J, sp );
	}

	template < typename T >
	inline void SL3<T>::screenJacobian( ScreenJacType & J, const Eigen::Matrix<T, 2, 1> & sp ) const
	{
		J( 0, 0 ) = 1; 
		J( 0, 1 ) = 0; 
		J( 0, 2 ) = sp[ 1 ]; 
		J( 0, 3 ) = 0; 
		J( 0, 4 ) = sp[ 0 ]; 
		J( 0, 5 ) = -sp[ 0 ]; 
		J( 0, 6 ) = -sp[ 0 ] * sp[ 0 ];
		J( 0, 7 ) = -sp[ 0 ] * sp[ 1 ];

		J( 1, 0 ) = 0;
		J( 1, 1 ) = 1;
		J( 1, 2 ) = 0;
		J( 1, 3 ) = sp[ 0 ];
		J( 1, 4 ) = -sp[ 1 ];
		J( 1, 5 ) = -2 * sp[ 1 ];
		J( 1, 6 ) = -sp[ 0 ] * sp[ 1 ];
		J( 1, 7 ) = -sp[ 1 ] * sp[ 1 ];
	}


	template <typename T>
	inline void SL3<T>::hessian( HessMatType & H, const PointType & p ) const
	{
		H.setZero();

		// 0.5 * ( G_i * G_j + G_j * G_i )
		H( 0, 4 ) =  0.5 * p.z(); 
		H( 0, 5 ) =  0.5 * p.z(); 
		H( 0, 6 ) =  0.5 * p.x(); 
		H( 0, 7 ) =  0.5 * p.y(); 
		H( 1, 3 ) =  0.5 * p.z(); 
		H( 2, 6 ) =  0.5 * p.z(); 

		H( 3, 2 ) =  0.5 * p.z(); 
		H( 4, 4 ) = -0.5 * p.z(); 
		H( 4, 6 ) =  0.5 * p.x(); 
		H( 4, 7 ) =  0.5 * p.y(); 
		H( 5, 7 ) =  0.5 * p.z(); 
		
		H( 6, 1 ) =  0.5 * p.z(); 
		H( 6, 3 ) =  0.5 * p.x(); 
		H( 6, 5 ) = -0.5 * p.y(); 
		H( 7, 3 ) =  0.5 * p.y(); 
		H( 8, 6 ) =  0.5 * p.y(); 
		
		H( 9, 2 )  =  0.5 * p.x(); 
		H( 10, 0 ) =  0.5 * p.z(); 
		H( 10, 2 ) =  0.5 * p.y(); 
		H( 10, 5 ) = -0.5 * p.x(); 
		H( 11, 7 ) =  0.5 * p.x(); 
		
		H( 12, 0 ) =  0.5 * p.z(); 
		H( 12, 4 ) =		p.x(); 
		H( 13, 1 ) = -0.5 * p.z(); 
		H( 13, 4 ) =		p.y(); 
		H( 13, 5 ) =		p.y(); 
		H( 14, 6 ) =  0.5 * p.x(); 
		H( 14, 7 ) = -0.5 * p.y(); 
		
		H( 15, 0 ) =  0.5 * p.z();
		H( 15, 2 ) = -0.5 * p.y();
		H( 16, 3 ) = -0.5 * p.x();
		H( 16, 4 ) =        p.y();
		H( 16, 5 ) =        p.y();
		H( 17, 5 ) =        p.z();
		H( 17, 6 ) =  0.5 * p.x();
		
		H( 18, 0 ) =  0.5 * p.x();
		H( 19, 1 ) =  0.5 * p.x();
		H( 20, 0 ) =  0.5 * p.z();
		H( 20, 2 ) =  0.5 * p.y();
		H( 20, 4 ) =  0.5 * p.x();
		H( 20, 5 ) =  0.5 * p.x();
		
		H( 21, 0 ) =  0.5 * p.y();
		H( 22, 1 ) =  0.5 * p.y();
		H( 23, 1 ) =  0.5 * p.z();
		H( 23, 3 ) =  0.5 * p.x();
		H( 23, 4 ) = -0.5 * p.y();
		
	}

	template <typename T>
	inline void SL3<T>::screenHessian( ScreenHessType & wx, 
									   ScreenHessType & wy,
									   const Eigen::Matrix<T, 2, 1> & sp ) const
	{
		T x = sp[ 0 ];
		T y = sp[ 1 ];

		T xx = x * x;
		T xxx = xx * x;
		T xxy = xx * y;
		T xy = x * y;
		T yy = y * y;
		T yyy = yy * y;
		T yyx = yy * x;

		wx <<
		0,0,0,0,0.5,-0.5,-x,-0.5*y
		,0,0,0.5,0,0,0,0,-0.5*x
		,0,0.5,0,0.5*x,0,-1.5*y,-1.5*xy,-yy
		,0,0,0.5*x,0,0,0,0,-0.5*xx
		,0.5, 0, 0, 0,x, -x, -1.5*xx,-0.5*xy
		,-0.5,0,-1.5*y,0,-x,x,1.5*xx,2*xy
		,-x,0,-1.5*xy,0,-1.5*xx,1.5*xx,2*xxx,2*xxy
		,-0.5*y,-0.5*x,-yy,-0.5*xx,-0.5*xy,2*xy,2*xxy,2*yyx;

		wy <<
			0, 0, 0,   0.5,   0,  0, -0.5* y, 0,
			0, 0, 0,     0,-0.5, -1, -0.5* x,-y,
			0, 0, 0, 0.5*y,   0,  0, -0.5*yy, 0,
			0.5, 0, 0.5*y, 0, 0, -1.5*x, -xx, -1.5*xy,
			0, -0.5, 0, 0, y, 2*y, 0.5*xy, 1.5*yy,
			0, -1, 0, -1.5*x, 2*y, 4*y, 2.5*xy, 3*yy,
			-0.5*y, -0.5*x, -0.5*yy, -xx, 0.5*xy, 2.5*xy, 2*xxy, 2*yyx,
			0, -y, 0, -1.5*xy, 1.5*yy, 3*yy, 2*yyx, 2*yyy;

	}
}

#endif

