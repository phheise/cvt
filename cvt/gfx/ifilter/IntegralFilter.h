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

#ifndef CVT_INTEGRALFILTER_H
#define CVT_INTEGRALFILTER_H

#include <cvt/gfx/IFilter.h>
#include <cvt/cl/CLKernel.h>

namespace cvt {
	class GuidedFilter;
	class StereoGCVFilter;
	class IntegralFilter : public IFilter {
		friend class GuidedFilter;
		friend class StereoGCVFilter;
		public:
			IntegralFilter();
			void apply( Image& dst, const Image& src, const Image* src2 = NULL ) const;
			void apply( const ParamSet* set, IFilterType t = IFILTER_CPU ) const;

		private:
			void applyOuterRGB( Image& dst_RR_RG_RB, Image& dst_GG_GB_BB, const Image& src ) const;
			void applyShifted( Image& dst, const Image& src1, const Image& src2, float dx = 0, float dy = 0 ) const;

			CLKernel _clprefixsum_blockp;
			CLKernel _clprefixsum_blockp_sqr;
			CLKernel _clprefixsum_blockp_mul2;
			CLKernel _clprefixsum_blockp_mul2_shifted;
			CLKernel _clprefixsum_blockp_outerrgb;
			CLKernel _clprefixsum_horiz;
			CLKernel _clprefixsum_vert;
			CLKernel _clprefixsum_block2;
			size_t _blocksize;
	};
}

#endif
