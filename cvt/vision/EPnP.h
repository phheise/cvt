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

#ifndef CVT_EPNP_H
#define CVT_EPNP_H

#include <cvt/math/PCA.h>
#include <cvt/geom/PointSet.h>
#include <cvt/math/Matrix.h>
#include <cvt/math/Vector.h>

namespace cvt {

    template <typename T>
    class EPnP {
        public:
            EPnP( const PointSet<3, T> & pointSet );

            /**
             * Compute pose from 2D-3D matches
             * @param transform	Output transformation (Rotation and Translation)
             * @param pointSet	The 2D correspondences
             * @param K			Intrinsic Matrix
             */
            void solve( Matrix4<T> & transform, const PointSet<2, T> & pointSet, const Matrix3<T> & K ) const;

        private:
            const PointSet<3, T>&	_points3D;
            // the control points
            PointSet<3, T>			_controlPoints;
            Matrix3<T>				_pcInv;

            // the barycentric coords of the 3D pointset pts
            std::vector<Vector4<T> > _barycentricCoords;

            void computeControlPoints( const PointSet<3, T> & ptSet );
            void computeBarycentricCoords( const PointSet<3, T> & ptSet );
            void buildSystem( Eigen::Matrix<T, 12, 12> & A, const PointSet<2, T> & points2D, const Matrix3<T> & K ) const;

            void computeControlPointsDelta( Eigen::Matrix<T, 6, 1> & cpDelta ) const;

            void solveBetaN2( Eigen::Matrix<T, 4,  1> & betas,
                              const Eigen::Matrix<T, 6, 10> & C,
                              const Eigen::Matrix<T, 6,  1> & dSqr ) const;

            void solveBetaN3( Eigen::Matrix<T, 4,  1> & betas,
                              const Eigen::Matrix<T, 6, 10> & C,
                              const Eigen::Matrix<T, 6,  1> & dSqr ) const;

            void solveBetaN4( Eigen::Matrix<T, 4,  1> & betas,
                              const Eigen::Matrix<T, 6, 10> & C,
                              const Eigen::Matrix<T, 6,  1> & dSqr ) const;

            void computePose( Matrix4<T> & transform,
                              const Eigen::Matrix<T, 12, 1> & estimatedCoords,
                              const PointSet<3, T> & controlPoints ) const;

			T	reprojectionError( const Matrix4<T> & transform,
								   const Matrix4<T> & K44,
								   const PointSet<3, T> & p3d,
								   const PointSet<2, T> & p2d ) const;

            void fillConstraintMatrix( Eigen::Matrix<T, 6, 10> & C,
                                       const Eigen::Matrix<T, 12, 1> & v0,
                                       const Eigen::Matrix<T, 12, 1> & v1,
                                       const Eigen::Matrix<T, 12, 1> & v2,
                                       const Eigen::Matrix<T, 12, 1> & v3 ) const;
    };

    typedef EPnP<float>  EPnPf;
    typedef EPnP<double> EPnPd;
}

#endif
