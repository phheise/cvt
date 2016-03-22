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

#ifndef CVT_X11HANDLER_H
#define CVT_X11HANDLER_H

#include <cvt/gui/internal/X11/X.h>
#include <cvt/gui/internal/X11/WidgetImplWinGLX11.h>

#include <cvt/io/IOHandler.h>

#include <map>

namespace cvt {
	class X11Handler : public IOHandler
	{
		public:
			X11Handler( ::Display* dpy, std::map< ::Window, WidgetImplWinGLX11*>* windows );
			~X11Handler();
			virtual void onDataReadable();
			void handleQueued();

		private:
			X11Handler( const X11Handler& );
			void handleEvents();

			::Display* _dpy;
			std::map< ::Window, WidgetImplWinGLX11*>* _windows;
			::Atom xatom_wmdelete;
			::Atom xatom_wmproto;
	};

	inline X11Handler::X11Handler( ::Display* dpy, std::map< ::Window, WidgetImplWinGLX11*>* windows ) : IOHandler(), _dpy( dpy ), _windows( windows )
	{
		_fd = ConnectionNumber( _dpy );
		notifyReadable( true );
		xatom_wmproto = XInternAtom( _dpy , "WM_PROTOCOLS", False);
		xatom_wmdelete = XInternAtom( _dpy, "WM_DELETE_WINDOW", False);
	}

	inline X11Handler::~X11Handler()
	{
	}
}

#endif
