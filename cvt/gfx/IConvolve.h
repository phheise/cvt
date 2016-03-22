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

#ifndef CVT_ICONVOLVE_H
#define CVT_ICONVOLVE_H

#include <cvt/gfx/IBorder.h>
#include <cvt/gfx/Color.h>

namespace cvt
{
	class Image;
	class IKernel;

	class IConvolve {
		public:
			static void convolve( Image& dst, const Image& src, const IKernel& kernel, IBorderType btype = IBORDER_CLAMP, const Color& = Color::BLACK );
			static void convolve( Image& dst, const Image& src, const IKernel& hkernel, const IKernel& vkernel, IBorderType btype = IBORDER_CLAMP, const Color& = Color::BLACK );

		private:
			IConvolve() {}
			IConvolve( const IConvolve& ) {}
	};
}

#endif
