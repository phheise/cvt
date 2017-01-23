/*
   The MIT License (MIT)

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
#ifndef DECAFCL_H
#define DECAFCL_H

#include <cvt/gfx/Image.h>
#include <cvt/geom/scene/ScenePoints.h>
#include <cvt/math/Matrix.h>
#include <cvt/cl/CLKernel.h>
#include <cvt/cl/CLPyramid.h>

namespace cvt {

    enum DECAF_COSTFUNC {
        DECAF_L2 = 0,
        DECAF_HUBER = 1,
        DECAF_TUKEY = 2,
        DECAF_AD_CENSUS = 3
    };

    // DirECt Align Fusion
    class DECAFCL {
        public:
            DECAFCL( const Matrix3f& K, float focalLength, float baseLine, int levels, DECAF_COSTFUNC cf );
            ~DECAFCL();

            void init( const Image& rgb, const Image& disparityVariance );
            void initPropagate( const Image& rgb, const Image& disparityVariance, const Matrix4f& RT );

            void alignFuse( Matrix4f& RT, const Image& rgb, const Image& disparityVariance,
                            bool initialGuess = false, bool noFusion = false, float* costout = NULL, unsigned int* sizeout = NULL );

            void uniformDisparityVariance( Image& dst, const Image& src, float variance );
            void depthTouniformDisparityVariance( Image& dst, const Image& src, float scale, float variance );

            void drawDisparity( Image& dst, float scale = 1.0f );
            void drawDisparityVariance( Image& dst, float scale = 1.0f );

            void toScenePoints( ScenePoints& pts) const;
        private:
            unsigned int warp( const Matrix4f& RT, int level );
            float reduceSolve( Eigen::Matrix<float, 6, 1>& delta, unsigned int size, float alpha = 0.05f );
            void fuse( Image& varDispDst, const Image& varDispSrc0, const Image& varDispSrc1, float procNoise, float maxDiffRel );

            float reduceSolveCPU( Eigen::Matrix<float, 6, 1>& delta, unsigned int size );

            void fill( Image& output, const Vector4f& value );
            void pyrDownChecked( Image& output, const Image& input );

            void gummInit( Image& dst, const Image& input, float varianace );
            void gummInitDisparityVariance( Image& dst, const Image& input );
            void gummFuse( Image& dst, const Image& src, const Image& other, float procNoise );

            void drawWarp( Image& dst, const Matrix4f& RT, int level, bool clear = true );

            void drawDisparity( Image& output, const Image& input, float fB, float scale );
            void drawDisparityVariance( Image&, const Image& input, float scale );

            void warpDisparityVariance( Image& dst, const Image& varDispSrc, const Matrix4f& RT );

            void testReduce();

            Matrix3f  _K;
            Matrix3f  _Kinv;
            float     _fB; // focallength times baseline of the stereo setup
            int       _levels;

            CLPyramid _rgb;
            CLPyramid _dispVar;
            CLPyramid _rgbDest;
            CLPyramid _dispVarDest;
            CLBuffer* _warpBuffer;
            CLBuffer* _reduceBuffer;
            size_t    _warpStride;

            CLKernel  _warpcl;
            CLKernel  _reducecl;
            CLKernel  _warpdispvarcl;
            CLKernel  _fusecl;
            CLKernel  _drawwarpcl;
            CLKernel  _fillcl;
            CLKernel  _pyrdowncheckcl;
            CLKernel  _univariancecl;
            CLKernel  _depthunivarcl;
            CLKernel  _drawdispcl;
            CLKernel  _drawvarcl;
            CLKernel  _gumminit;
            CLKernel  _gumminitdispvar;
            CLKernel  _gummfuse;
    };

    inline void DECAFCL::drawDisparity( Image& dst, float scale )
    {
        drawDisparity( dst, _dispVar[ 0 ], _fB, scale );
    }

    inline void DECAFCL::drawDisparityVariance( Image& dst, float scale )
    {
        drawDisparityVariance( dst, _dispVar[ 0 ], scale );
    }

}

#endif
