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

#ifndef CVT_STEREOGCV_H
#define CVT_STEREOGCV_H

#include <cvt/gfx/IFilter.h>
#include <cvt/gfx/ifilter/IntegralFilter.h>
#include <cvt/gfx/ifilter/BoxFilter.h>
#include <cvt/cl/CLKernel.h>

namespace cvt {
	class StereoGCVFilter : public IFilter {
		public:
					StereoGCVFilter();
					~StereoGCVFilter();

			void	apply( Image& dst, const Image& cam0, const Image& cam1, float dmin, float dmax, float dt = 1.0f ) const;
			void	apply( const ParamSet* attribs, IFilterType iftype ) const {}

		private:
			void	depthmap( Image& dst, const Image& cam0, const Image& cam1, float dmin, float dmax, float dt = 1.0f ) const;

			CLKernel		_cldepthcost;
			CLKernel		_cldepthcostgrad;
			CLKernel		_cldepthcostncc;
			CLKernel		_cldepthmin;
			CLKernel		_clfill;
			CLKernel		_clcdconv;
			CLKernel		_clgrad;
//			CLKernel		_cldepthrefine;
			CLKernel	   _clguidedfilter_calcab_outerrgb;
			CLKernel	   _clguidedfilter_applyab_gc_outer;
			CLKernel	   _clocclusioncheck;
			IntegralFilter _intfilter;
			BoxFilter	   _boxfilter;

	};

	inline StereoGCVFilter::~StereoGCVFilter()
	{
	}
}

#endif
