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

#ifndef CVT_TVL1FLOW_H
#define CVT_TVL1FLOW_H

#include <cvt/gfx/IFilter.h>
//#include <cvt/gfx/ifilter/ROFFGPFilter.h>
//#include <cvt/gfx/ifilter/GuidedFilter.h>
#include <cvt/cl/CLKernel.h>

namespace cvt {
	class TVL1Flow : public IFilter {
		public:
			TVL1Flow( float scalefactor, size_t levels );
			~TVL1Flow();
			void apply( Image& flow, const Image& src1, const Image& src2 );
			void apply( const ParamSet* set, IFilterType t = IFILTER_CPU ) const {};

		private:
			void fillPyramidCL( const Image& img, size_t index );
			void solveTVL1( Image& flow, const Image& src1, const Image& src2, bool median );

			bool		 _toggle;
			float		 _scalefactor;
			size_t		 _levels;
			CLKernel	 _pyrup;
			CLKernel	 _pyrdown;
			CLKernel	 _tvl1;
			CLKernel	 _tvl1_warp;
//			CLKernel	 _tvl1_dataadd;
			CLKernel	 _clear;
			CLKernel	 _median3;
			float		 _lambda;
//			ROFFGPFilter _rof;
//			GuidedFilter _gf;
			Image*		 _pyr[ 2 ];
	};
}

#endif
