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

#include <cvt/gui/internal/X11/ApplicationX11.h>
#include <cvt/gui/internal/X11/WidgetImplWinGLX11.h>
#include <cvt/gui/internal/X11/X11Handler.h>
#include <cvt/util/Exception.h>
#include <cvt/gui/Window.h>

#include <cvt/cl/CLPlatform.h>

namespace cvt {
	ApplicationX11::ApplicationX11()
	{
		// Leaks memory ...
		//XInitThreads();

		dpy = ::XOpenDisplay( NULL );
		if( !dpy )
			throw CVTException( "Error: Couldn't connect to X-Server\n" );

		GLFormat format;
		_defaultctx = new GLXContext( dpy, format );

		xatom_wmdelete = ::XInternAtom( dpy, "WM_DELETE_WINDOW", False);

		const ::XVisualInfo* visinfo = _defaultctx->XVisualInfo();
		::XSetWindowAttributes attr;
		attr.background_pixmap = None;
		attr.border_pixel = 0;
		attr.colormap = ::XCreateColormap( dpy, RootWindow( dpy, DefaultScreen( dpy ) ), visinfo->visual, AllocNone);
		unsigned long mask = CWBackPixmap | CWBorderPixel | CWColormap;

		// OpenGL init
		_dummywin = ::XCreateWindow( dpy, RootWindow( dpy, DefaultScreen( dpy ) ), 0, 0, 1, 1,
							  0, visinfo->depth, InputOutput, visinfo->visual, mask, &attr );

		_defaultctx->setDrawable( _dummywin );
		_defaultctx->makeCurrent();
		GL::init();

		// OpenCL init, try to share resources with GL
		if( !CL::defaultContext() )
			_clsupport = CL::init( _defaultctx );

		_defaultctx->resetCurrent();
	}

	ApplicationX11::~ApplicationX11()
	{
		delete _defaultctx;
		XDestroyWindow( dpy, _dummywin );
		::XCloseDisplay( dpy );
	}


	WidgetImpl* ApplicationX11::_registerWindow( Widget* w )
	{
		WidgetImpl* ret;
		if( w->isToplevel() ) {
			WidgetImplWinGLX11* impl = new WidgetImplWinGLX11( dpy, _defaultctx, w, &updates );
			XSetWMProtocols(dpy, impl->win, &xatom_wmdelete, 1 );
			windows.insert( std::pair< ::Window, WidgetImplWinGLX11*>( impl->win, impl ) );
			ret = impl;
		} else {
			ret = NULL;
		}
		return ret;
	};

	void ApplicationX11::_unregisterWindow( WidgetImpl* impl )
	{
		    windows.erase( ( ( WidgetImplWinGLX11* ) impl )->win );
	};

	void ApplicationX11::runApp()
	{
		int timeout;
		X11Handler x11handler( dpy, &windows );
		_ioselect.registerIOHandler( &x11handler );

		run = true;

		XSync( dpy, false );

		while( run ) {
			_timers.handleTimers();
			timeout = _timers.nextTimeout();

			x11handler.handleQueued();
			_ioselect.handleIO( timeout );

			if( !updates.empty() ) {
				PaintEvent pe( 0, 0, 0, 0 );
				WidgetImplWinGLX11* win;

				while( !updates.empty() ) {
					win = updates.front();
					updates.pop_front();
					if( win->needsupdate )
						win->paintEvent( pe );
				}
			}
		}

		_ioselect.unregisterIOHandler( &x11handler );

		/* FIXME: do cleanup afterwards */
	}
}

