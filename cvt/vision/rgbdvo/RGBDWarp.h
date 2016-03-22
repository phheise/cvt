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


#ifndef RGBDWARP_H
#define RGBDWARP_H

#include <cvt/math/SE3.h>
#include <cvt/util/SIMD.h>

namespace cvt
{

    class StandardWarp
    {
        public:
            EIGEN_MAKE_ALIGNED_OPERATOR_NEW
            static const size_t NParams = 6;
            typedef float                               Type;
            typedef Eigen::Matrix<Type, 1, NParams>     JacobianType;
            typedef Eigen::Matrix<Type, 2, NParams>     ScreenJacType;
            typedef Eigen::Matrix<Type, NParams, NParams> HessianType;
            typedef Eigen::Matrix<Type, NParams, 1>     DeltaVectorType;

            StandardWarp() {}
            ~StandardWarp(){}

            StandardWarp( const StandardWarp& other ) :
                _pose( other._pose )
            {}

            void initialize( const Matrix4f& pose )
            {
                setPose( pose );
            }

            void setPose( const Matrix4f& pose )
            {
                Eigen::Matrix<float, 4, 4> pe;
                EigenBridge::toEigen( pe, pose );
                _pose.set( pe );
            }

            Matrix4f pose() const
            {
                Matrix4f p;
                EigenBridge::toCVT( p, _pose.transformation() );
                return p;
            }

            static void screenJacobian( ScreenJacType& j,
                                        const Vector3f& point,
                                        const Matrix3f& K )
            {
                SE3<float>::screenJacobian( j, point, K );
            }

            static void computeJacobian( JacobianType& j,
                                         const Vector3f& point,
                                         const Matrix3f& K,
                                         const Eigen::Matrix<float, 1, 2>& g,
                                         float pixval )
            {
                ScreenJacType J;
                screenJacobian( J, point, K );
                computeJacobian( j, J, g, pixval );
            }

            static void computeJacobian( JacobianType& j,
                                         const ScreenJacType& sj,
                                         const Eigen::Matrix<Type, 1, 2>& g,
                                         float /*pixval*/ )
            {
                j = g * sj;
            }

            float computeResidual( float templateValue, float warpedValue ) const
            {
                return templateValue - warpedValue;
            }

            void computeResiduals( float* residuals, const float* referenceValues, const float* warped, size_t n ) const
            {
                SIMD::instance()->Sub( residuals, referenceValues, warped, n );
            }

            float costs( const float* residuals, const std::vector<size_t>& indices ) const
            {
                float ssd = 0.0f;

                for( std::vector<size_t>::const_iterator it = indices.begin(), end = indices.end();
                     it != end;
                     ++it ){
                    ssd += Math::sqr( residuals[ *it ] );
                }

                if( indices.size() )
                    return ssd / indices.size();
                else
                    return 1.0f;
            }

            void updateParameters( const DeltaVectorType& v )
            {
                _pose.apply( v );
            }

            void updateParametersInv( const DeltaVectorType& v )
            {
                _pose.applyInverse( v );
            }

            static void zJacobian( JacobianType& J, const Vector3f& pCam )
            {
                J.setZero();
                J( 0, 0 ) =  pCam[ 1 ];
                J( 0, 1 ) = -pCam[ 0 ];
                J( 0, 5 ) =  1.0f;
            }

        private:
            SE3<Type>  _pose;
    };

    class AffineLightingWarp
    {
        public:
            EIGEN_MAKE_ALIGNED_OPERATOR_NEW
            static const size_t NParams = 8;
            static const size_t PoseParameters = 6;
            typedef float                                   Type;
            typedef Eigen::Matrix<Type, 1, NParams>         JacobianType;
            typedef Eigen::Matrix<Type, 2, PoseParameters>  ScreenJacType;
            typedef Eigen::Matrix<Type, NParams, NParams>   HessianType;
            typedef Eigen::Matrix<Type, NParams, 1>         DeltaVectorType;

            AffineLightingWarp() :
                _alpha( 0.0 ),
                _beta( 0.0 )
            {}

            ~AffineLightingWarp(){}

            AffineLightingWarp( const AffineLightingWarp& other ) :
                _pose( other._pose ),
                _alpha( other._alpha ),
                _beta( other._beta )
            {}

            AffineLightingWarp& operator= ( const AffineLightingWarp& other )
            {
                _pose = other._pose;
                _alpha = other._alpha;
                _beta = other._beta;
                return *this;
            }

            void initialize( const Matrix4<Type>& pose )
            {
                setPose( pose );
                _alpha = 0.0;
                _beta = 0.0;
            }

            void setPose( const Matrix4<Type>& pose )
            {
                Eigen::Matrix<Type, 4, 4> pe;
                EigenBridge::toEigen( pe, pose );
                _pose.set( pe );
            }

            Matrix4<Type> pose() const
            {
                Matrix4<Type> p;
                EigenBridge::toCVT( p, _pose.transformation() );
                return p;
            }

            void updateParameters( const DeltaVectorType& v )
            {
                _pose.apply( v.head<6>() );

                Type ta = 1.0 + v[ 6 ];
                _alpha = ( _alpha - v[ 6 ] ) / ta;
                _beta  = ( _beta  - v[ 7 ] ) / ta;
            }

            void updateParametersInv( const DeltaVectorType& v )
            {
                _pose.applyInverse( v.head<6>() );

                Type ta = 1.0 + v[ 6 ];
                _alpha = ( _alpha - v[ 6 ] ) / ta;
                _beta  = ( _beta  - v[ 7 ] ) / ta;
            }

            static void screenJacobian( ScreenJacType& j,
                                        const Vector3<Type>& point,
                                        const Matrix3<Type>& K )
            {
                SE3<Type>::screenJacobian( j, point, K );
            }

            static void computeJacobian( JacobianType& j,
                                         const Vector3f& point,
                                         const Matrix3f& K,
                                         const Eigen::Matrix<float, 1, 2>& g,
                                         float pixval )
            {
                ScreenJacType J;
                screenJacobian( J, point, K );
                computeJacobian( j, J, g, pixval );
            }

            static void computeJacobian( JacobianType& j,
                                         const ScreenJacType& sj,
                                         const Eigen::Matrix<Type, 1, 2>& g,
                                         float pixval )
            {
                j.head<6>() = g * sj;
                j[ 6 ] = pixval;
                j[ 7 ] = 1;
            }

            float computeResidual( float templateValue, float warpedValue ) const
            {
                return templateValue - ( 1.0f + _alpha ) * warpedValue - _beta;
            }

            void computeResiduals( float* residuals, const float* referenceValues, const float* warped, size_t n ) const
            {
                SIMD* simd = SIMD::instance();
                simd->SubValue1f( residuals, referenceValues, _beta, n );
                simd->MulSubValue1f( residuals, warped, ( 1.0f + _alpha ), n );
            }

            float costs( const float* residuals, const std::vector<size_t>& indices ) const
            {
                float ssd = 0.0f;

                for( std::vector<size_t>::const_iterator it = indices.begin(), end = indices.end();
                     it != end;
                     ++it ){
                    ssd += Math::sqr( residuals[ *it ] );
                }

                if( indices.size() )
                    return ssd / indices.size();
                else
                    return 1.0f;
            }

            static void zJacobian( JacobianType& J, const Vector3f& pCam )
            {
                J.setZero();
                J( 0, 0 ) =  pCam[ 1 ];
                J( 0, 1 ) = -pCam[ 0 ];
                J( 0, 5 ) =  1.0f;
            }

        private:
            SE3<Type>   _pose;
            Type        _alpha;
            Type        _beta;
    };
}

#endif // RGBDWARP_H
