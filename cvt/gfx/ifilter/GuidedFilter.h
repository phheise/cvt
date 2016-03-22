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

#ifndef CVT_GUIDEDFILTER_H
#define CVT_GUIDEDFILTER_H

#include <cvt/gfx/IFilter.h>
#include <cvt/util/Plugin.h>
#include <cvt/util/PluginManager.h>
#include <cvt/cl/CLKernel.h>

#include <cvt/gfx/ifilter/IntegralFilter.h>
#include <cvt/gfx/ifilter/BoxFilter.h>

namespace cvt {
	class GuidedFilter : public IFilter {
		public:
			GuidedFilter();
			~GuidedFilter() {};

			void apply( Image& dst, const Image& src, const Image& guide, const int radius, const float epsilon, bool rgbcovariance = false ) const;

			void apply( const ParamSet* attribs, IFilterType iftype ) const;

		private:
			void applyGC( Image& dst, const Image& src, const Image& guide, const int radius, const float epsilon ) const;
			void applyGC_COV( Image& dst, const Image& src, const Image& guide, const int radius, const float epsilon ) const;
			void applyCC( Image& dst, const Image& src, const Image& guide, const int radius, const float epsilon ) const;

			GuidedFilter( const GuidedFilter& t );

			CLKernel	   _clguidedfilter_calcab;
			CLKernel	   _clguidedfilter_calcab_outerrgb;
			CLKernel	   _clguidedfilter_applyab_gc;
			CLKernel	   _clguidedfilter_applyab_gc_outer;
			CLKernel	   _clguidedfilter_applyab_cc;
			IntegralFilter _intfilter;
			BoxFilter	   _boxfilter;
	};
}

#endif
