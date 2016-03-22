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

#ifndef CVT_APPLICATIONX11_H
#define CVT_APPLICATIONX11_H

#include <cvt/gui/Application.h>
#include <cvt/gui/internal/X11/X.h>
#include <cvt/gui/internal/X11/WidgetImplWinGLX11.h>
#include <cvt/gui/internal/X11/GLXContext.h>
#include <cvt/gui/event/Event.h>
#include <cvt/gui/TimeoutHandler.h>
#include <cvt/gui/internal/TimerInfoList.h>
#include <cvt/gl/OpenGL.h>
#include <cvt/io/IOSelect.h>
#include <map>
#include <deque>

namespace cvt {
	class ApplicationX11 : public Application
	{
		public:
			ApplicationX11();
			~ApplicationX11();

			virtual void runApp();
			virtual void exitApp() { run = false; };

			virtual uint32_t _registerTimer( size_t interval, TimeoutHandler* t ) { return _timers.registerTimer( interval, t ); };
			virtual void _unregisterTimer( uint32_t id ) { _timers.unregisterTimer( id ); };


		private:
			ApplicationX11( const Application& );

			virtual WidgetImpl* _registerWindow( Widget* w );
			virtual void _unregisterWindow( WidgetImpl* w );

			virtual bool _hasGLSupport() { return true; }
			virtual bool _hasCLSupport() { return _clsupport; }

			virtual GLContext* _defaultGLContext() { return _defaultctx; }

			::Display* dpy;
			GLXContext* _defaultctx;
			::Atom xatom_wmdelete;
			::Window _dummywin;
			bool run;
			std::map< ::Window, WidgetImplWinGLX11*> windows;
			std::deque< WidgetImplWinGLX11*> updates;
			IOSelect _ioselect;
			TimerInfoList _timers;
			bool _clsupport;
	};
}

#endif
