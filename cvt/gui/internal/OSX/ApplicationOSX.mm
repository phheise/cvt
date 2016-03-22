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

#include <cvt/gui/internal/OSX/ApplicationOSX.h>
#include <cvt/gui/internal/OSX/WidgetImplWinGLOSX.h>
#include <Cocoa/Cocoa.h>

#include <cvt/cl/CLPlatform.h>

namespace cvt {
	struct OSXData {
		NSAutoreleasePool* _pool;
	};

	ApplicationOSX::ApplicationOSX()
	{
		/* transform to foreground app */
		//ProcessSerialNumber psn;
		//GetCurrentProcess( &psn );
		//TransformProcessType( &psn, kProcessTransformToForegroundApplication );
		//SetFrontProcess( &psn );

		/* setup cocoa stuff inside OSXData struct */
		_osx = new OSXData();
		_osx->_pool = [[ NSAutoreleasePool alloc ] init ];
		[ NSApplication sharedApplication ];
        [ NSApp setActivationPolicy:NSApplicationActivationPolicyRegular ];
        [ NSMenu setMenuBarVisible:NO ];

		/* replace default menu bar */
		id menubar = [[NSMenu new] autorelease];
		id appMenuItem = [[NSMenuItem new] autorelease];
		[menubar addItem:appMenuItem];
		[NSApp setMainMenu:menubar];
		id appMenu = [ [NSMenu new ] autorelease];
		id quitMenuItem = [[[NSMenuItem alloc] initWithTitle: @"Quit" action:@selector(terminate:) keyEquivalent:@"q"] autorelease ];
		[appMenu addItem:quitMenuItem];
		[appMenuItem setSubmenu:appMenu];
        [NSMenu setMenuBarVisible:YES];

		/* create default gl context */
		GLFormat format;
		_defaultctx = new CGLContext( format );
		_defaultctx->makeCurrent();
		GL::init();

		// OpenCL init, try to share resources with GL
		if( !CL::defaultContext() )
			_clsupport = CL::init( _defaultctx );

		_defaultctx->resetCurrent();
	}

	WidgetImpl* ApplicationOSX::_registerWindow( Widget* w )
	{
		WidgetImpl* ret;
		if( w->isToplevel() ) {
			WidgetImplWinGLOSX* impl = new WidgetImplWinGLOSX( _defaultctx, w );
			ret = impl;
		} else {
			ret = NULL;
		}
		return ret;
	}

	void ApplicationOSX::_unregisterWindow( WidgetImpl* )
	{
	}

	ApplicationOSX::~ApplicationOSX()
	{
		[ _osx->_pool release ];
		delete _osx;
	}

	void ApplicationOSX::runApp()
	{
        [ NSApp activateIgnoringOtherApps:YES ];
		[ NSApp run ];
	}

	void ApplicationOSX::exitApp()
	{
		[ NSApp stop: nil ];
	}
}
