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

#include <cvt/gui/Application.h>
#include <cvt/util/PluginManager.h>
#include <cvt/cl/OpenCL.h>
#include <cvt/util/SIMD.h>

#if defined( APPLE ) && !defined( APPLE_X11 )
#include <cvt/gui/internal/OSX/ApplicationOSX.h>
#else
#include <cvt/gui/internal/X11/ApplicationX11.h>
#endif

namespace cvt {

	Application* Application::_app = NULL;

	Application* Application::instance()
	{
		if( !_app ) {
#if defined( APPLE ) && !defined( APPLE_X11 )
			_app = new ApplicationOSX();
#else
			_app = new ApplicationX11();
//#else
//		#error "Platform for Application undefined"
#endif
		}
		return _app;
	}

	void Application::atexit()
	{
        instance()->exitDelegates.notify();
		PluginManager::cleanup();
		CL::cleanup();
		SIMD::cleanup();
		delete _app;
	}
}
