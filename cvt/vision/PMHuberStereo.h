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
#ifndef CVT_PMHUBERSTEREO_H
#define CVT_PMHUBERSTEREO_H

#include <cvt/gfx/Image.h>
#include <cvt/cl/CLKernel.h>

#include <cvt/vision/PDOptCL.h>
#include <cvt/gfx/PDROFInpaint.h>

namespace cvt {

    enum DisparityPostProcessing {
        DISPARITY_PP_NONE,
        DISPARITY_PP_LRCHECK,
        DISPARITY_PP_LRCHECK_FILL
    };

    class PMHuberStereo {
        public:
            PMHuberStereo();
            ~PMHuberStereo();

            void disparityMap( Image& dmap, const Image& left, const Image& right, size_t patchsize, float depthmax, size_t iterations,
                               DisparityPostProcessing pp = DISPARITY_PP_LRCHECK, Image* normalmap = NULL, const Image* initdisparity = NULL, const Image* initnormalmap = NULL );

            void depthMap( Image& dmap, const Image& left, const Image& right, size_t patchsize, float depthmax, size_t iterations,
                           float dscale = -1.0f, DisparityPostProcessing pp = DISPARITY_PP_LRCHECK, Image* normalmap = NULL, const Image* initdisparity = NULL, const Image* initnormalmap = NULL );

        private:
            void disparityMapAll( Image& matchesleft, Image& matchesright,
                                 const Image& left, const Image& right,
                                 int patchsize, float disparitymax, size_t iterations, float thetamin,
                                 const Image* initdisparityleft, const Image* initdisparityright,
                                 const Image* initnormalleft, const Image* initnormalright );

            void init( Image& output, const Image& first, const Image& second, const Image& firstgrad, const Image& secondgrad,
                       int patchsize, float depthmax, bool LR, const Image* initdisparity = NULL, const Image* initnormalmap = NULL );
            void propagate( Image& output, const Image& old, const Image& first, const Image& second, const Image& firstgrad, const Image& secondgrad, const Image& smooth,
                            float theta, int patchsize, float depthmax, bool LR, int iteration, CLBuffer& viewbufin, CLBuffer& viewbufout  );

            void occlusionMap( Image& occ, const Image& first, const Image& second, float maxdispdiff, float maxanglediff, bool LR ) const;

            void checkConsistencyLR( Image& output, const Image& first, const Image& second, float maxdispdiff, float maxanglediff, bool LR ) const;
            void convertFillNormalDisparity( Image& output, const Image& input, const Image& other, float dispscale, bool LR );
            void convertFillDisparity( Image& output, const Image& input, float dispscale );
            void convertDisparity( Image& output, const Image& input, float dispscale, bool LR );
            void convertFillNormal( Image& output, const Image& input, bool LR );

            void bilateralWeightToAlpha( Image& output, const Image& input, int patchsize ) const;

            void normalDepth( Image& output, const Image& input, float depthmax, bool LR );

            void visualize( const String& name, const Image& input, float dscale ) const;

            void gradient( Image& output, const Image& input );
            void clear( Image& output );

            void clearViewBuffer( CLBuffer& buf, int width, int height );

            void pyrUpMul( Image& output, const Image& input, const Vector4f& mul );

            CLKernel _clpmh_init;
            CLKernel _clpmh_init_disparity;
            CLKernel _clpmh_init_disparity_normal;
            CLKernel _clpmh_propagate;
            CLKernel _clpmh_depthmap;
            CLKernel _clpmh_viewbufclear;
            CLKernel _clpmh_fill;
            CLKernel _clpmh_consistency;
            CLKernel _clpmh_filldepthmap;
            CLKernel _clpmh_todisparity;
            CLKernel _clpmh_normaldepth;
            CLKernel _clpmh_fillnormalmap;
            CLKernel _clpmh_clear;
            CLKernel _clpmh_occmap;
            CLKernel _clpmh_gradxy;
            CLKernel _clpmh_weight;
            CLKernel _clpmh_bilateralweight;
            CLKernel _clpmh_visualize_depth_normal;
            CLKernel _clpyrupmul;
            PDOptCL  _pdopt;
            PDROFInpaint _pdrofinpaint;
    };

}
#endif
