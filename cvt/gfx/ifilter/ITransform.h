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

#ifndef CVT_ITRANSFORM_H
#define CVT_ITRANSFORM_H

#include <cvt/gfx/IFilter.h>
#include <cvt/util/Plugin.h>
#include <cvt/math/Function.h>
#include <cvt/util/PluginManager.h>

namespace cvt {
	class ITransform : public IFilter {
		public:
			ITransform();
			~ITransform();
			void apply( const ParamSet* attribs, IFilterType iftype ) const;

			/**
			 *	@param	dst			the destination image
			 *	@param	src			the source image
			 *	@param	transform	transformation from src to dst!
			 */
			static void apply( Image& dst, const Image& src, const Matrix3f& transform, size_t width = 0, size_t height = 0 );
			static void apply( Image& dst, const Image& src, const Matrix3f& transform, const Matrix3f& itransform, size_t width = 0, size_t height = 0 );
		
			static void apply( Image& dst, const Image& src, const Function<Vector2f, Vector2f>& warpFunc, size_t width = 0, size_t height = 0 );

		private:
			static void applyFC1( Image& dst, const Image& src, const Matrix3f& T, const Matrix3f& Tinv );
			static void applyFC4( Image& dst, const Image& src, const Matrix3f& T, const Matrix3f& Tinv );
			static void applyU8C1( Image& dst, const Image& src, const Matrix3f& T, const Matrix3f& Tinv );
			static void applyU8C4( Image& dst, const Image& src, const Matrix3f& T, const Matrix3f& Tinv );

			/* function versions */
			static void applyFC1( Image& dst, const Image& src, const Function<Vector2f, Vector2f>& warp );
			static void applyFC4( Image& dst, const Image& src, const Function<Vector2f, Vector2f>& warp );
			static void applyU8C1( Image& dst, const Image& src, const Function<Vector2f, Vector2f>& warp );
			static void applyU8C4( Image& dst, const Image& src, const Function<Vector2f, Vector2f>& warp );

			ITransform( const ITransform& t );
	};
}

#endif
