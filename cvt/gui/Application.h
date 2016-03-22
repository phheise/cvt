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

#ifndef CVT_APPLICATION_H
#define CVT_APPLICATION_H

#include <stdlib.h>
#include <stdint.h>

#include <cvt/gui/TimeoutHandler.h>
#include <cvt/util/Signal.h>

namespace cvt {
	class WidgetImpl;
	class Widget;
	class GLContextImpl;
	class GLContext;

	class Application {
		friend class Widget;
		friend class GLContext;

		public:
			static void init() { instance(); }

			static void run() { ::atexit( Application::atexit ); instance()->runApp(); }
            static void exit() { instance()->exitApp(); }

			static uint32_t registerTimer( size_t interval, TimeoutHandler* t ) { return instance()->_registerTimer( interval, t ); }
			static void unregisterTimer( uint32_t id ) { instance()->_unregisterTimer( id ); }

			static bool hasGLSupport() { return instance()->_hasGLSupport(); }
			static bool hasCLSupport() { return instance()->_hasCLSupport(); }

			static GLContext* defaultGLContext() { return instance()->_defaultGLContext(); }


            static void addExitDelegate( const Delegate<void ()>& d )
            {
                instance()->exitDelegates.add( d );
            }

		protected:
			Application() {}
			Application( const Application& );
			virtual ~Application() {}

			static WidgetImpl* registerWindow( Widget* w ) { return instance()->_registerWindow( w ); }
			static void unregisterWindow( WidgetImpl* w ) { instance()->_unregisterWindow( w ); }

			virtual WidgetImpl* _registerWindow( Widget* widget ) = 0;
			virtual void _unregisterWindow( WidgetImpl* widget ) = 0;

			virtual uint32_t _registerTimer( size_t interval, TimeoutHandler* t ) = 0;
			virtual void _unregisterTimer( uint32_t id ) = 0;

			virtual bool _hasGLSupport() = 0;
			virtual bool _hasCLSupport() = 0;

			virtual GLContext* _defaultGLContext() = 0;

			virtual void runApp() = 0;
			virtual void exitApp() = 0;

			static void atexit();

			static Application* instance();
			static Application* _app;

            Signal<void>    exitDelegates;
	};
}

#endif
