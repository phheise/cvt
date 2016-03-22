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

#ifndef IFILTER_GAUSSIIR_H
#define IFILTER_GAUSSIIR_H

#include <cvt/gfx/IFilter.h>
#include <cvt/cl/CLKernel.h>

namespace cvt
{
	/* gaussian filter with infinite impulse response */
	class GaussIIR : public IFilter {
		public:
					GaussIIR();
					~GaussIIR();
			void	applyOpenCL( Image& dst, const Image& src, const Vector4f & n, const Vector4f & m, const Vector4f & d ) const;
			void	applyCPUf( Image& dst, const Image& src, const Vector4f & n, const Vector4f & m, const Vector4f & d ) const;
			void	applyCPUu8( Image& dst, const Image& src, const Vector4f & n, const Vector4f & m, const Vector4f & d ) const;
			void	apply( const ParamSet* set, IFilterType t = IFILTER_CPU ) const;

		private:
			mutable CLKernel*	_kernelIIR;
			mutable CLKernel*	_kernelIIR2;
	};
}

#endif
