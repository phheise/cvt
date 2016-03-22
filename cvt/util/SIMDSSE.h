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

#ifndef SIMDSSE_H
#define SIMDSSE_H

#include <cvt/util/SIMD.h>

namespace cvt {

	class SIMDSSE : public SIMD {
		friend class SIMD;
		friend class SIMDSSE2;

		protected:
			SIMDSSE() {};

		public:
			using SIMD::Add;
			using SIMD::Sub;
			using SIMD::Mul;
			using SIMD::Div;

			void Memcpy( uint8_t* dst, uint8_t const* src, const size_t n ) const;

			/* memory blocks */
			virtual void Add( float* dst, float const* src1, float const* src2, const size_t n ) const;
			virtual void Sub( float* dst, float const* src1, float const* src2, const size_t n ) const;
			virtual void Mul( float* dst, float const* src1, float const* src2, const size_t n ) const;
			virtual void Div( float* dst, float const* src1, float const* src2, const size_t n ) const;

			virtual void AddValue1f( float* dst, float const* src1, const float v, const size_t n ) const;
			virtual void SubValue1f( float* dst, float const* src1, const float v, const size_t n ) const;
			virtual void MulValue1f( float* dst, float const* src1, const float v, const size_t n ) const;
			virtual void DivValue1f( float* dst, float const* src1, const float v, const size_t n ) const;

			virtual void MulAddValue1f( float* dst, float const* src1, const float value, const size_t n ) const;
			virtual void MulSubValue1f( float* dst, float const* src1, const float value, const size_t n ) const;

			virtual void Conv_GRAYALPHAf_to_GRAYf( float* dst, const float* src, const size_t n ) const;
			/*shuffle*/
			virtual void Conv_XYZAf_to_ZYXAf( float* dst, float const* src, const size_t n ) const;

			virtual void warpLinePerspectiveBilinear4f( float* dst, const float* src, size_t srcStride, size_t srcWidth, size_t srcHeight,
													    const float* point, const float* normal, const size_t n ) const;

			virtual void boxFilterPrefixSum1_f_to_f( float* dst, size_t dstride, const float* src, size_t srcstride, size_t width, size_t height, size_t boxwidth, size_t boxheight ) const;

			virtual std::string name() const;
			virtual SIMDType type() const;
	};

	inline std::string SIMDSSE::name() const
	{
		return "SIMD-SSE";
	}

	inline SIMDType SIMDSSE::type() const
	{
		return SIMD_SSE;
	}
}

#endif
