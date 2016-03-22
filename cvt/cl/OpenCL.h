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

#ifndef CVT_OPENCL_H
#define CVT_OPENCL_H

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/opencl.h>
#include <OpenCL/cl_gl.h>
#else
#include <CL/opencl.h>
#include <CL/cl_gl.h>
#endif

namespace cvt {
	class Application;
	class ApplicationX11;
	class CLContext;
	class CLCommandQueue;
	class CLDevice;
	class GLContext;

	/**
	  \ingroup CL
	  \defgroup CL OpenCL
	 */
	class CL {
		friend class Application;
		friend class ApplicationX11;
		friend class ApplicationOSX;

		public:
			static CLContext* defaultContext() { return _ctx; }
			static CLCommandQueue* defaultQueue() { return _queue; }
			static CLDevice* defaultDevice() { return _device; }
			static bool	defaultGLsharing() { return _glsharing; }
			static bool setDefaultDevice( const CLDevice& dev );
		private:
			static bool init( const GLContext* glctx );
			static void cleanup();

			// default context, device and queue
			static bool			   _glsharing;
			static CLContext*	   _ctx;
			static CLDevice*	   _device;
			static CLCommandQueue* _queue;
	};
}

#endif
