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

#ifndef CVT_GLCONTEXT_H
#define CVT_GLCONTEXT_H

#include <cvt/gl/OpenGL.h>
#include <cvt/gl/GLFormat.h>
#include <cvt/cl/OpenCL.h>

namespace cvt {

	enum GLPlatform {
		GL_PLATFORM_X11,
		GL_PLATFORM_OSX
	};

	/**
	  \ingroup GL
	*/
	class GLContext {
		public:
			virtual ~GLContext();

			const GLFormat& format() const;
			virtual void makeCurrent() = 0;
			virtual void swapBuffers() = 0;
			virtual void resetCurrent() = 0;

			virtual void shareCL( cl_context_properties* props, int size, int* retsize ) const = 0;

			virtual GLPlatform platform() const = 0;

			static GLContext* currentContext();


		protected:
			GLContext( const GLFormat& format );
			GLFormat _format;
			static void setCurrent( GLContext* ctx );

		private:
			GLContext();
			GLContext( const GLContext& ctx );

			static GLContext* _current;
	};

	inline GLContext* GLContext::currentContext()
	{
		return _current;
	}

	inline const GLFormat& GLContext::format() const
	{
		return _format;
	}

	inline void GLContext::setCurrent( GLContext* c )
	{
		_current = c;
	}
}

#endif
