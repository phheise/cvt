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

#include <cvt/vision/ReprojectionError.h>
#include <cvt/math/SE3.h>

namespace cvt {

    template <class T>
    ReprojectionError<T>::ReprojectionError( const PointSet<3, T>& pts3D,
                                             const PointSet<2, T>& pts2D,
                                             size_t maxIterations,
                                             T errorThreshold ) :
        _pts3d( pts3D ),
        _pts2d( pts2D ),
        _maxIters( maxIterations ),
        _errorThres( errorThreshold )
    {
    }

    template <class T>
    T ReprojectionError<T>::minimize( cvt::Matrix4<T>& cam2World, const Matrix3<T>& intrinsics, const RobustEstimator<T>& estimator ) const
    {
        SE3<T>  warp;
        warp.set( cam2World );
        size_t iters = 0;

        std::vector<Vector3<T> > ptsInCam;
        std::vector<Vector2<T> > projections;

        ptsInCam.resize( _pts3d.size() );
        projections.resize( _pts3d.size() );


        SIMD* simd = SIMD::instance();

        Matrix4<T>                           K44( intrinsics );
        Eigen::Matrix<T, 2, 1>               residual;
        typename SE3<T>::ScreenJacType       currJ;
        typename SE3<T>::ScreenHessType      H;
        typename SE3<T>::ParameterVectorType rhs, delta;

        // Gauss Newton
        T costs = 1000.0f;
        while( iters < _maxIters ){
            // transform points using current pose
            simd->transformPoints( &ptsInCam[ 0 ], cam2World, &_pts3d[ 0 ], _pts3d.size() );

            // project to screen
            simd->projectPoints( &projections[ 0 ], K44, &ptsInCam[ 0 ], _pts3d.size() );

            // build normal equations
            size_t n = _pts3d.size();
            Vector3<T>* p3D = &ptsInCam[ 0 ];
            Vector2<T>* p2D = &projections[ 0 ];
            const Vector2<T>* p2DOrig = &_pts2d[ 0 ];

            H = SE3<T>::ScreenHessType::Zero();
            rhs = SE3<T>::ParameterVectorType::Zero();
            T currCosts = 0.0f;
            T r, w;
            while( n-- ){
                // eval jac:
                SE3<T>::screenJacobian( currJ, *p3D, intrinsics );

                residual[ 0 ] = p2D->x - p2DOrig->x;
                residual[ 1 ] = p2D->y - p2DOrig->y;
                r = residual.squaredNorm();
                w = estimator.weight( r );

                H.noalias() += w * currJ.transpose() * currJ;
                rhs.noalias() += w * currJ.transpose() * residual;

                currCosts += r;

                p3D++;
                p2D++;
                p2DOrig++;
            }

            costs = currCosts / _pts2d.size();

            if( costs < _errorThres )
                break;

            delta = -H.inverse() * rhs;
            warp.apply( delta );

            // update cam2World
            cvt::EigenBridge::toCVT( cam2World, warp.transformation() );
            iters++;
        }

        return costs;
    }

    template <class T>
    T ReprojectionError<T>::minimize( cvt::Matrix4<T>& cam2World,
                                      const std::vector<size_t>& inliers,
                                      const Matrix3<T>& intrinsics,
                                      const RobustEstimator<T>& estimator ) const
    {
        SE3<T>  warp;
        warp.set( cam2World );
        size_t iters = 0;

        std::vector<Vector3<T> > ptsInCam;
        std::vector<Vector2<T> > projections;

        ptsInCam.resize( _pts3d.size() );
        projections.resize( _pts3d.size() );


        SIMD* simd = SIMD::instance();

        Matrix4<T>                           K44( intrinsics );
        Eigen::Matrix<T, 2, 1>               residual;
        typename SE3<T>::ScreenJacType       currJ;
        typename SE3<T>::ScreenHessType      H;
        typename SE3<T>::ParameterVectorType rhs, delta;

        // Gauss Newton
        T costs = 1000.0f;
        while( iters < _maxIters ){
            // transform points using current pose
            simd->transformPoints( &ptsInCam[ 0 ], cam2World, &_pts3d[ 0 ], _pts3d.size() );

            // project to screen
            simd->projectPoints( &projections[ 0 ], K44, &ptsInCam[ 0 ], _pts3d.size() );

            // build normal equations
            std::vector<size_t>::const_iterator it = inliers.begin();
            const std::vector<size_t>::const_iterator itEnd = inliers.end();

            H = SE3<T>::ScreenHessType::Zero();
            rhs = SE3<T>::ParameterVectorType::Zero();
            T currCosts = 0.0f;
            T r, w;
            while( it != itEnd ){
                // eval jac:
                SE3<T>::screenJacobian( currJ, ptsInCam[ *it ], intrinsics );

                residual[ 0 ] = projections[ *it ].x - _pts2d[ *it ].x;
                residual[ 1 ] = projections[ *it ].y - _pts2d[ *it ].y;
                r = residual.squaredNorm();
                w = estimator.weight( r );

                H.noalias() += w * currJ.transpose() * currJ;
                rhs.noalias() += w * currJ.transpose() * residual;

                currCosts += r;

                it++;
            }

            costs = currCosts / _pts2d.size();

            if( costs < _errorThres )
                break;

            delta = -H.inverse() * rhs;
            warp.apply( delta );

            // update cam2World
            cvt::EigenBridge::toCVT( cam2World, warp.transformation() );
            iters++;
        }

        return costs;
    }

    template class ReprojectionError<float>;
    template class ReprojectionError<double>;

}
