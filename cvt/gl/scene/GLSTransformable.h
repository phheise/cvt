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

#ifndef CVT_GLSTRANSFORMABLE_H
#define CVT_GLSTRANSFORMABLE_H

#include <cvt/math/Matrix.h>

namespace cvt {
	class GLSTransformable
	{
		public:
			GLSTransformable( const GLSTransformable& t );
			GLSTransformable( );
			GLSTransformable( const Matrix4f& mat );
			virtual ~GLSTransformable();

			const Matrix4f& transformation() const;
			Matrix4f&		transformation();
			void			setTransformation( const Matrix4f& t );

		protected:
			Matrix4f	_transformation;
	};

	inline GLSTransformable::GLSTransformable( const GLSTransformable& t ) : _transformation( t._transformation )
	{
	}

	inline GLSTransformable::GLSTransformable( )
	{
		_transformation.setIdentity();
	}

	inline GLSTransformable::GLSTransformable( const Matrix4f& mat ) : _transformation( mat )
	{
	}

	inline GLSTransformable::~GLSTransformable()
	{
	}

	inline void GLSTransformable::setTransformation( const Matrix4f& t )
	{
		_transformation = t;
	}

	inline const Matrix4f& GLSTransformable::transformation() const
	{
		return _transformation;
	}

	inline Matrix4f& GLSTransformable::transformation()
	{
		return _transformation;
	}
}

#endif
