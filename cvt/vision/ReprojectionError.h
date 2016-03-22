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
#ifndef CVT_REPROJECTIONERROR_H
#define CVT_REPROJECTIONERROR_H

#include <cvt/math/Matrix.h>
#include <cvt/geom/PointSet.h>
#include <cvt/vision/RobustWeighting.h>

namespace cvt {

    template <class T>
    class ReprojectionError
    {
        public:
            ReprojectionError( const PointSet<3, T>& pts3D,
                               const PointSet<2, T>& pts2D,
                               size_t maxIterations = 20,
                               T errorThreshold = 0.0f );

            T minimize( cvt::Matrix4<T>& pose,
                        const Matrix3<T>& intrinsics,
                        const RobustEstimator<T>& estimator = NoWeighting<T>() ) const;

            T minimize( cvt::Matrix4<T>& pose,
                        const std::vector<size_t>& inliers,
                        const Matrix3<T>& intrinsics,
                        const RobustEstimator<T>& estimator = NoWeighting<T>() ) const;

        private:
            const PointSet<3, T>&   _pts3d;
            const PointSet<2, T>&   _pts2d;
            size_t                  _maxIters;
            T                       _errorThres;
    };

}

#endif // REPROJECTIONERROR_H
