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

/**
 * Special Euclidean Group Transformation:
 * 		- 3D Euclidean Transformation Parametrization using Lie Algebras
 */
#ifndef CVT_SE3_H
#define CVT_SE3_H

#include <cvt/math/Math.h>

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry>

#include <cvt/math/Matrix.h>
#include <cvt/util/EigenBridge.h>

#include <vector>

namespace cvt
{
    template<typename T>
    class SE3
        {
        public:
            typedef Eigen::Matrix<T, 4, 4>  MatrixType;
            typedef Eigen::Matrix<T, 3, 6>  JacMatType;
            typedef Eigen::Matrix<T, 24, 6> HessMatType;
            typedef Eigen::Matrix<T, 2, 6>  ScreenJacType;
            typedef Eigen::Matrix<T, 6, 6>  ScreenHessType;
            typedef Eigen::Matrix<T, 6, 1>  ParameterVectorType;
            typedef Eigen::Matrix<T, 3, 1>  PointType;
            typedef Eigen::Matrix<T, 4, 1>  PointTypeHom;

            /**
             *	angles in radians
             */
            SE3( T alpha = 0, T beta = 0, T gamma = 0, T tx = 0, T ty = 0, T tz = 0 );
            SE3( const ParameterVectorType & p );
            SE3( const SE3<T> & other );
            ~SE3();

            void set( T alpha, T beta, T gamma, T tx, T ty, T tz );
            void set( const ParameterVectorType & p );
            void set( const MatrixType & t ) { _current = t; }
            void set(const cvt::Matrix4<T> &t );
            void apply( const ParameterVectorType & delta );
                        void applyInverse( const ParameterVectorType & delta );
            void transform( PointType & warped, const PointType & p ) const;
            void jacobian( JacMatType & J, const PointType & p ) const;
            void jacobian( JacMatType & J, const PointTypeHom & p ) const;

            /**
             *  \brief  evaluates the screen jacobians for a given 3D point in camera coordinates
             *          (jacobian around current pose estimate!)
             *  \param  J               (output) the result
             *  \param  p               the 3D point given in camera coordinates
             *  \param  intrinsics      the intrinsic calibration matrix
             */
            static void screenJacobian( ScreenJacType & J, const PointType & p, const Eigen::Matrix<T, 3, 3> & intrinsics )
            {
                T zz = 1.0/Math::sqr( p.z() );
                T z = 1.0/p.z();
                T xx = Math::sqr( p.x() );
                T yy = Math::sqr( p.y() );

                J( 0, 0 ) = -intrinsics( 0, 0 ) * p.x() * p.y() * zz;
                J( 0, 1 ) =  intrinsics( 0, 0 ) * xx * zz + intrinsics( 0, 0 );
                J( 0, 2 ) = -intrinsics( 0, 0 ) * p.y() * z;
                J( 0, 3 ) =  intrinsics( 0, 0 ) * z;
                J( 0, 4 ) = 0.0;
                J( 0, 5 ) = -intrinsics( 0, 0 ) * p.x() * zz;

                J( 1, 0 ) = -intrinsics( 1, 1 ) * yy * zz - intrinsics( 1, 1 );
                J( 1, 1 ) =  intrinsics( 1, 1 ) * p.x() * p.y() * zz;
                J( 1, 2 ) =  intrinsics( 1, 1 ) * p.x() * z;
                J( 1, 3 ) =  0.0;
                J( 1, 4 ) =  intrinsics( 1, 1 ) * z;
                J( 1, 5 ) = -intrinsics( 1, 1 ) * p.y() * zz;
            }

            static void screenJacobian( ScreenJacType & J, const Vector3<T> & p, const Matrix3<T> & intrinsics )
            {
                T z = 1.0 / p.z;
                T zz = 1.0 / Math::sqr( p.z );
                T xx = Math::sqr( p.x );
                T yy = Math::sqr( p.y );

                J( 0, 0 ) = -intrinsics[ 0 ][ 0 ] * p.x * p.y * zz;
                J( 0, 1 ) =  intrinsics[ 0 ][ 0 ] * xx * zz + intrinsics[ 0 ][ 0 ];
                J( 0, 2 ) = -intrinsics[ 0 ][ 0 ] * p.y * z;
                J( 0, 3 ) =  intrinsics[ 0 ][ 0 ] * z;
                J( 0, 4 ) = 0.0;
                J( 0, 5 ) = -intrinsics[ 0 ][ 0 ] * p.x * zz;

                J( 1, 0 ) = -intrinsics[ 1 ][ 1 ] * yy * zz - intrinsics[ 1 ][ 1 ];
                J( 1, 1 ) =  intrinsics[ 1 ][ 1 ] * p.x * p.y * zz;
                J( 1, 2 ) =  intrinsics[ 1 ][ 1 ] * p.x * z;
                J( 1, 3 ) =  0.0;
                J( 1, 4 ) =  intrinsics[ 1 ][ 1 ] * z;
                J( 1, 5 ) = -intrinsics[ 1 ][ 1 ] * p.y * zz;
            }

            void hessian( HessMatType & h, const PointType & p ) const;

            /**
             *	\brief	Screen Hessian
             *	\param	wx	second partial derivatives of the x component
             *	\param	wy	second partial derivatives of the y component
             *	\param	p	the 3d point in camera coordinates for which to evaluate the hessian
             */
            void screenHessian( ScreenHessType & wx,
                                ScreenHessType & wy,
                                const PointType & p,
                                const Eigen::Matrix<T, 3, 3> & m ) const;

            /* p has to be pretransformed with the current T in this case! */
            void jacobianAroundT( JacMatType & J, const PointTypeHom & p ) const;


            const MatrixType & transformation() const
            {
                return _current;
            }

            void evalExp( MatrixType& expT, const ParameterVectorType& delta ) const;

            EIGEN_MAKE_ALIGNED_OPERATOR_NEW
        private:
            MatrixType				_current;
    };

    template <typename T>
    inline SE3<T>::SE3( T alpha, T beta, T gamma, T tx, T ty, T tz )
    {
        this->set( alpha, beta, gamma, tx, ty, tz );
    }

    template <typename T>
    inline SE3<T>::SE3( const ParameterVectorType & p )
    {
        this->set( p[ 0 ], p[ 1 ], p[ 2 ], p[ 3 ], p[ 4 ], p[ 5 ] );
    }

    template <typename T>
    inline SE3<T>::SE3( const SE3<T> & other ) :
        _current( other._current )
    {
    }

    template <typename T>
    inline SE3<T>::~SE3()
    {
    }

    template <typename T>
    void inline SE3<T>::set( T alpha, T beta, T gamma, T tx, T ty, T tz )
    {
        _current.block( 0, 0, 3, 3 ) = ( Eigen::AngleAxis<T>( alpha, Eigen::Matrix<T, 3, 1>::UnitX() ) *
                                         Eigen::AngleAxis<T>( beta, Eigen::Matrix<T, 3, 1>::UnitY() ) *
                                         Eigen::AngleAxis<T>( gamma, Eigen::Matrix<T, 3, 1>::UnitZ() ) ).toRotationMatrix();
        _current.coeffRef( 0, 3 ) = tx;
        _current.coeffRef( 1, 3 ) = ty;
        _current.coeffRef( 2, 3 ) = tz;
        _current.coeffRef( 3, 0 ) = 0;
        _current.coeffRef( 3, 1 ) = 0;
        _current.coeffRef( 3, 2 ) = 0;
        _current.coeffRef( 3, 3 ) = 1.0;
    }

    template<typename T>
    void inline SE3<T>::set( const ParameterVectorType & p )
    {
        this->set( p[ 0 ], p[ 1 ], p[ 2 ], p[ 3 ], p[ 4 ], p[ 5 ] );
    }

    template<typename T>
    void inline SE3<T>::set( const cvt::Matrix4<T>& t )
    {
        EigenBridge::toEigen( _current, t );
    }

    template <typename T>
    inline void SE3<T>::apply( const ParameterVectorType & delta )
    {
        MatrixType trans;
        evalExp( trans, delta );
        _current = trans * _current;
    }

    template <typename T>
    inline void SE3<T>::applyInverse( const ParameterVectorType & delta )
    {
        MatrixType trans;
        evalExp( trans, delta );
        _current *= trans;
    }

    template <typename T>
    inline void SE3<T>::evalExp( MatrixType& trans, const ParameterVectorType& delta ) const
    {
        // exponential can be evaluated in closed form in this case:
        T theta = Math::sqrt( Math::sqr( delta[ 0 ] ) + Math::sqr( delta[ 1 ] ) + Math::sqr( delta[ 2 ] ) );

        if( theta < 1e-7 ){
                trans( 0, 0 ) = 1.0; trans( 0, 1 ) = 0.0; trans( 0, 2 ) = 0.0; trans( 0, 3 ) = delta[ 3 ];
                trans( 1, 0 ) = 0.0; trans( 1, 1 ) = 1.0; trans( 1, 2 ) = 0.0; trans( 1, 3 ) = delta[ 4 ];
                trans( 2, 0 ) = 0.0; trans( 2, 1 ) = 0.0; trans( 2, 2 ) = 1.0; trans( 2, 3 ) = delta[ 5 ];
                trans( 3, 0 ) = 0.0; trans( 3, 1 ) = 0.0; trans( 3, 2 ) = 0.0; trans( 3, 3 ) =        1.0;
        } else {
                T a = Math::sin( theta );
                T thetaInv = 1.0f / theta;
                T b = ( 1 - Math::cos( theta ) );

                Eigen::Matrix<T, 3, 3> angleSkew;
                angleSkew( 0, 0 ) =         0  ; angleSkew( 0, 1 ) = -delta[ 2 ]; angleSkew( 0, 2 ) =  delta[ 1 ];
                angleSkew( 1, 0 ) =  delta[ 2 ]; angleSkew( 1, 1 ) =         0  ; angleSkew( 1, 2 ) = -delta[ 0 ];
                angleSkew( 2, 0 ) = -delta[ 1 ]; angleSkew( 2, 1 ) =  delta[ 0 ]; angleSkew( 2, 2 ) =         0  ;
                // normalize
                angleSkew *= thetaInv;

                Eigen::Matrix<T, 3, 3> angleSkewSqr = angleSkew * angleSkew;

                trans.template block<3, 3>( 0, 0 ) = Eigen::Matrix<T, 3, 3>::Identity() + a * angleSkew + b * angleSkewSqr;

                //trans.template block<3, 1>( 0, 3 ) = ( Eigen::Matrix<T, 3, 3>::Identity() * theta + b * angleSkew + ( theta - a ) * angleSkewSqr ) * delta.template segment<3>( 3 );
                trans.template block<3, 1>( 0, 3 ) = ( Eigen::Matrix<T, 3, 3>::Identity() + b * thetaInv * angleSkew + ( theta - a ) * thetaInv * angleSkewSqr ) * delta.template segment<3>( 3 );

                trans( 3, 0 ) = 0;
                trans( 3, 1 ) = 0;
                trans( 3, 2 ) = 0;
                trans( 3, 3 ) = 1;
        }
    }

    template <typename T>
    inline void SE3<T>::jacobian( JacMatType & J, const PointType & pp ) const
    {
        Eigen::Matrix<T, 4, 1> p;
        p.template segment<3>( 0 ) = pp;
        p[ 3 ] = 1.0;

        this->jacobian( J, p );
    }

    template <typename T>
    inline void SE3<T>::jacobian( JacMatType & J, const PointTypeHom & pp ) const
    {
        PointTypeHom p = _current * pp;
        jacobianAroundT( J, p );
    }

    template <typename T>
    inline void SE3<T>::jacobianAroundT( JacMatType & J, const PointTypeHom & p ) const
    {
        J( 0, 0 ) =	    0  ; J( 0, 1 ) =  p[ 2 ]; J( 0, 2 ) = -p[ 1 ]; J( 0, 3 ) = p[ 3 ]; J( 0, 4 ) =    0  ; J( 0, 5 ) = 0;
        J( 1, 0 ) = -p[ 2 ]; J( 1, 1 ) =	 0  ; J( 1, 2 ) =  p[ 0 ]; J( 1, 3 ) =    0  ; J( 1, 4 ) = p[ 3 ]; J( 1, 5 ) = 0;
        J( 2, 0 ) =  p[ 1 ]; J( 2, 1 ) = -p[ 0 ]; J( 2, 2 ) =	  0  ; J( 2, 3 ) =    0  ; J( 2, 4 ) =    0  ; J( 2, 5 ) = p[ 3 ];
    }

    template<typename T>
    inline void SE3<T>::transform( PointType & warped, const PointType & p ) const
    {
        warped = _current.template block<3, 3>( 0, 0 ) * p + _current.template block<3, 1>( 0, 3 );
    }

    template <typename T>
    inline void SE3<T>::hessian( HessMatType & h, const PointType & p ) const
    {
        h.setZero();
        h( 0, 1 ) = 0.5 * p.y();
        h( 0, 2 ) = 0.5 * p.z();
        h( 1, 0 ) = -p.y();
        h( 1, 1 ) = 0.5 * p.x();
        h( 1, 5 ) = -0.5;
        h( 2, 0 ) = -p.z();
        h( 2, 2 ) = 0.5 * p.x();
        h( 2, 4 ) = 0.5;

        h( 4, 0 ) = 0.5 * p.y();
        h( 4, 1 ) = -p.x();
        h( 4, 5 ) = 0.5;
        h( 5, 0 ) = 0.5 * p.x();
        h( 5, 2 ) = 0.5 * p.z();
        h( 6, 1 ) = -p.z();
        h( 6, 2 ) = 0.5 * p.y();
        h( 6, 3 ) = -0.5;

        h( 8, 0 ) = 0.5 * p.z();
        h( 8, 2 ) = -p.x();
        h( 8, 4 ) = -0.5;
        h( 9, 1 ) = 0.5 * p.z();
        h( 9, 2 ) = -p.y();
        h( 9, 3 ) = 0.5;
        h( 10, 0 ) = 0.5 * p.x();
        h( 10, 1 ) = 0.5 * p.y();

        h( 13, 2 ) = 0.5;
        h( 14, 1 ) = -0.5;

        h( 16, 2 ) = -0.5;
        h( 18, 0 ) = 0.5;

        h( 20, 1 ) = 0.5;
        h( 21, 0 ) = -0.5;
    }

    template <typename T>
    inline void SE3<T>::screenHessian( ScreenHessType & wx,
                                       ScreenHessType & wy,
                                       const PointType & p,
                                       const Eigen::Matrix<T, 3, 3>& intrinsics ) const
    {
        T x = p.x();
        T y = p.y();
        T xx = Math::sqr( x );
        T xxy = xx * y;
        T xy = x * y;
        T yy = Math::sqr( y );
        T iz = 1.0 / p.z();
        T izz = 1.0 / Math::sqr( p.z() );
        T izzz = 1.0 /( p.z() * Math::sqr( p.z() ) );
        T fx = intrinsics( 0, 0 );
        T fy = intrinsics( 1, 1 );

        wx( 0, 0 ) = fx*x*iz+2*fx*x*yy*izzz;
        wx( 0, 1 ) = y*(-fx*iz-(2*fx*xx)*izzz)+(0.5*fx*y)*iz;
        wx( 0, 2 ) = fx*yy*izz-(0.5*fx*xx)*izz+0.5*fx;
        wx( 0, 3 ) = -fx*y*izz;
        wx( 0, 4 ) = -0.5*fx*x*izz;
        wx( 0, 5 ) = 2*fx*xy*izzz;

        wx( 1, 0 ) = -0.5*fx*y*iz-2*fx*xxy*izzz;
        wx( 1, 1 ) = fx*x*iz-x*(-fx*iz-(2*fx*xx)*izzz);
        wx( 1, 2 ) = -1.5*fx*xy*izz;
        wx( 1, 3 ) = 1.5*fx*x*izz;
        wx( 1, 4 ) = 0;
        wx( 1, 5 ) = -0.5*fx*iz-2*fx*xx*izzz;

        wx( 2, 0 ) = fx*yy*izz-0.5*fx*xx*izz+0.5*fx;
        wx( 2, 1 ) = -1.5*fx*xy*izz;
        wx( 2, 2 ) = -fx*x*iz;
        wx( 2, 3 ) = 0;
        wx( 2, 4 ) = -0.5*fx*iz;
        wx( 2, 5 ) = fx*y*izz;

        wx( 3, 0 ) = -fx*y*izz;
        wx( 3, 1 ) = 1.5*fx*x*izz;
        wx( 3, 2 ) = 0;
        wx( 3, 3 ) = 0;
        wx( 3, 4 ) = 0;
        wx( 3, 5 ) = -fx*izz;

        wx( 4, 0 ) = -0.5*fx*x*izz;
        wx( 4, 1 ) = 0;
        wx( 4, 2 ) = -0.5*fx*iz;
        wx( 4, 3 ) = 0;
        wx( 4, 4 ) = 0;
        wx( 4, 5 ) = 0;

        wx( 5, 0 ) = 2*fx*xy*izzz;
        wx( 5, 1 ) = -0.5*fx*iz-2*fx*xx*izzz;
        wx( 5, 2 ) = fx*y*izz;
        wx( 5, 3 ) = -fx*izz;
        wx( 5, 4 ) = 0;
        wx( 5, 5 ) = 2*fx*x*izzz;

        wy( 0, 0 ) = y*(fy*iz+(2*fy*yy)*izzz)+fy*y*iz;
        wy( 0, 1 ) = -0.5*fy*x*iz-2*fy*x*yy*izzz;
        wy( 0, 2 ) = -1.5*fy*xy*izz;
        wy( 0, 3 ) = 0;
        wy( 0, 4 ) = -1.5*fy*y*izz;
        wy( 0, 5 ) = 0.5*fy*iz+2*fy*yy*izzz;

        wy( 1, 0 ) = 0.5*fy*x*iz-x*(fy*iz+(2*fy*yy)*izzz);
        wy( 1, 1 ) = fy*y*iz+2*fy*xxy*izzz;
        wy( 1, 2 ) = -0.5*fy*yy*izz+fy*xx*izz+0.5*fy;
        wy( 1, 3 ) = 0.5*fy*y*izz;
        wy( 1, 4 ) = fy*x*izz;
        wy( 1, 5 ) = -2*fy*xy*izzz;

        wy( 2, 0 ) = -1.5*fy*xy*izz;
        wy( 2, 1 ) = -0.5*fy*yy*izz+fy*xx*izz+0.5*fy;
        wy( 2, 2 ) = -fy*y*iz;
        wy( 2, 3 ) = 0.5*fy*iz;
        wy( 2, 4 ) = 0;
        wy( 2, 5 ) = -fy*x*izz;

        wy( 3, 0 ) = 0;
        wy( 3, 1 ) = 0.5*fy*y*izz;
        wy( 3, 2 ) = 0.5*fy*iz;
        wy( 3, 3 ) = 0;
        wy( 3, 4 ) = 0;
        wy( 3, 5 ) = 0;

        wy( 4, 0 ) = -1.5*fy*y*izz;
        wy( 4, 1 ) = fy*x*izz;
        wy( 4, 2 ) = 0;
        wy( 4, 3 ) = 0;
        wy( 4, 4 ) = 0;
        wy( 4, 5 ) = -fy*izz;

        wy( 5, 0 ) = 0.5*fy*iz+2*fy*yy*izzz;
        wy( 5, 1 ) = -2*fy*xy*izzz;
        wy( 5, 2 ) = -fy*x*izz;
        wy( 5, 3 ) = 0;
        wy( 5, 4 ) = -fy*izz;
        wy( 5, 5 ) = 2*fy*y*izzz;
    }
}

#endif
