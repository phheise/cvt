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

#ifndef CVT_PATCH_GENERATOR
#define CVT_PATCH_GENERATOR

#include <cvt/gfx/Image.h>
#include <cvt/math/Math.h>
#include <cvt/util/Range.h>
#include <cvt/util/RNG.h>
#include <Eigen/Core>


namespace cvt
{
	class PatchGenerator
	{
		public:
			PatchGenerator( const Rangef & angleRange, const Rangef & scaleRange, uint32_t patchSize = 32, double whiteNoiseSigma = 5.0 );
			~PatchGenerator();
			
			/* generate the next patch */
			void next( Image & outputPatch, const cvt::Image & inputImage, const Vector2f & patchCenter );			
			
		private:
			uint32_t			_patchSize;
			Rangef				_angleRange;
			Rangef				_scaleRange;
			double				_whiteNoiseSigma;			
			Eigen::Matrix2f		_affine;
			RNG					_rng;
			size_t				_inHandle, _outHandle;
			
			void randomizeAffine();		
	};	
}

#endif
