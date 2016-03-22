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

#ifndef CVTGLPROGRAM_H
#define CVTGLPROGRAM_H

#include <cvt/gl/OpenGL.h>
#include <cvt/gl/GLException.h>
#include <cvt/math/Matrix.h>
#include <cvt/gfx/Color.h>

namespace cvt {

#define GLSL_VERTEX_IDX   0
#define GLSL_COLOR_IDX    1
#define GLSL_NORMAL_IDX   2
#define GLSL_TEXCOORD_IDX 3

	/**
	  \ingroup GL
	*/
	class GLProgram {
		public:
			GLProgram();
			~GLProgram();
			void build( const char* vertsrc, const char* fragsrc, const char* geomsrc = NULL );
            void build( const char** vertsrc, int vertcount, const char** fragsrc, int fragcount, const char** geomsrc = NULL, int geomcount = 0 );

			void bind();
			void unbind();
			void bindAttribLocation( const char *name, unsigned int location );
			unsigned int getAttribLocation( const char* name ) const;
			GLint uniformLocation( const char* name ) const;

			void setArg( const char* name, int i );
			void setArg( const char* name, float f );
			void setArg( const char* name, float f1, float f2 );
			void setArg( const char* name, const Color & c );
			void setArg( const char* name, const Vector3f & c );
			void setArg( const char* name, const Vector4f & c );
			void setArg( const char* name, const Matrix3f & m );
			void setArg( const char* name, const Matrix4f & m );

		private:
			GLuint program;
	};
}

#endif
