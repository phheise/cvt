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

#ifndef CVT_PMHUBERSTEREO_H
#define CVT_PMHUBERSTEREO_H

#include <cvt/gfx/Image.h>
#include <cvt/cl/CLKernel.h>

#include <cvt/gfx/PDROF.h>
#include <cvt/gfx/PDROFInpaint.h>

namespace cvt {
	class PMHuberStereo {
		public:
			PMHuberStereo();
			~PMHuberStereo();

			void depthMap( Image& dmap, const Image& left, const Image& right, size_t patchsize, float depthmax, size_t iterations, size_t viewsamples, float dscale = -1.0f, Image* normalmap = NULL );
			void depthMapInpaint( Image& dmap, const Image& left, const Image& right, size_t patchsize, float depthmax, size_t iterations, size_t viewsamples );

		private:
			CLKernel _clpmh_init;
			CLKernel _clpmh_propagate;
			CLKernel _clpmh_depthmap;
			CLKernel _clpmh_viewbufclear;
			CLKernel _clpmh_fill;
			CLKernel _clpmh_consistency;
			CLKernel _clpmh_filldepthmap;
			CLKernel _clpmh_normaldepth;
			CLKernel _clpmh_fillnormalmap;
			CLKernel _clpmh_clear;
			CLKernel _clpmh_occmap;
			CLKernel _clpmh_gradxy;
			CLKernel _clpmh_weight;
			PDROF	 _pdrof;
			PDROFInpaint _pdrofinpaint;
	};

}

#endif
