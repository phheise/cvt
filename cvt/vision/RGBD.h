/*
   The MIT License (MIT)

   Copyright (c) 2011 - 2013, Philipp Heise and Sebastian Klose
   Copyright (c) 2016, BMW Car IT GmbH, Philipp Heise (philipp.heise@bmw.de)

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
#ifndef CVT_RGBD_H
#define CVT_RGBD_H

#include <cvt/math/Matrix.h>
#include <cvt/gfx/Image.h>
#include <cvt/geom/PointSet.h>
#include <cvt/vision/CameraCalibration.h>
#include <cvt/geom/scene/ScenePoints.h>

namespace cvt {

    class RGBD {
        public:
            static void unprojectToScenePoints( ScenePoints& pts, const Image& rgb, const Image& depth, const Matrix3f& K, float dscale = 1.0f );

            static void unprojectToScenePoints( ScenePoints& pts, const Image& rgb, const Image& depth, float dscale = 1.0f );

            static void unprojectToXYZ( PointSet3f& pts, const Image& depth, const Matrix3f& K, float depthScale );

            static void warpForward( Image& dst, const Image& rgb, const Image& depth, const Matrix3f& K, const Matrix4f RT, float dscale = 1.0f );

            static void warpBackward( Image& out, const Image& depth, const Image& rgbdst, const Matrix3f& K, const Matrix4f RT, float dscale = 1.0f );

            static void warpDepthForward( Image& dst, const Image& depth, const Matrix3f& K, const Matrix4f RT, float dscale = 1.0f );

        private:
            RGBD();
            ~RGBD();
            RGBD( const RGBD& );
    };

}

#endif
