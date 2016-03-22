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

#ifndef CVT_CLCONTEXT_H
#define CVT_CLCONTEXT_H

#include <cvt/cl/CLPlatform.h>
#include <cvt/cl/CLObject.h>
#include <cvt/cl/CLCommandQueue.h>
#include <cvt/cl/CLImageFormat.h>

#include <cvt/gl/GLContext.h>

namespace cvt {
	class CLContext : public CLObject<cl_context> {
		public:
			CLContext( cl_context ctx = NULL );
			CLContext( const CLContext& ctx );
			CLContext( const CLPlatform& platform, const std::vector<CLDevice>& devices, const GLContext* share = NULL, bool cerrlog = false );
			CLContext( const CLPlatform& platform, const CLDevice& device, const GLContext* share = NULL, bool cerrlog = false );
			CLContext( const CLPlatform& platform, cl_device_type type, const GLContext* share = NULL, bool cerrlog = false );
			~CLContext();

			void devices( std::vector<CLDevice>& devices ) const;

			CLUTIL_GETINFOTYPE( refCount, CL_CONTEXT_REFERENCE_COUNT, cl_uint, _object, ::clGetContextInfo )
			// Missing:
			// - CL_CONTEXT_NUM_DEVICES number of devs in context
			// - CL_CONTEX_DEVICES the devices in the context

			void supportedImage2DFormats( std::vector<CLImageFormat>& formats ) const;


		private:
			static void cerrlog( const char* errinfo, const void* pinfo, size_t pinfosize, void* );
	};

}

#endif
