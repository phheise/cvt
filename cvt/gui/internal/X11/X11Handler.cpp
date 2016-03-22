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

#include <cvt/gui/internal/X11/X11Handler.h>
#include <cvt/gui/internal/X11/X11KeyMap.h>

namespace cvt {

	void X11Handler::handleEvents()
	{
		XEvent xevent;
		WidgetImplWinGLX11* win;

		XNextEvent( _dpy, &xevent );
		switch( xevent.type ) {
			case ConfigureNotify:
				{
					win = (*_windows)[ xevent.xconfigure.window ];

					::Window wp;
					int oldx = win->_rect.x;
					int oldy = win->_rect.y;
					unsigned int oldwidth = win->_rect.width;
					unsigned int oldheight = win->_rect.height;
					int nx = 0, ny = 0;
				    unsigned int nw, nh, b, d;

					while( XCheckTypedWindowEvent( _dpy, xevent.xconfigure.window, ConfigureNotify, &xevent ) )
						;

					XGetGeometry( _dpy, xevent.xconfigure.window, &wp, &nx, &ny, &nw, &nh, &b, &d );
					XTranslateCoordinates( _dpy, xevent.xconfigure.window, RootWindow( _dpy, DefaultScreen( _dpy ) ) , 0, 0, &nx, &ny, &wp);

					if( oldx != nx  || oldy != ny ) {
						MoveEvent me( nx, ny, oldx, oldy );
						win->moveEvent( me );
					}

					if( oldwidth != nw || oldheight != nh ) {
						ResizeEvent re( nw, nh, oldwidth, oldheight );
						win->resizeEvent( re );
					}
				}
				break;
			case ReparentNotify:
				{
					win = (*_windows)[ xevent.xreparent.window ];
					::Window child;
					int nx, ny;
					int oldx = win->_rect.x;
					int oldy = win->_rect.y;

					XTranslateCoordinates( _dpy, xevent.xreparent.window, RootWindow( _dpy, DefaultScreen( _dpy ) ), 0, 0, &nx, &ny, &child );
					if( oldx != nx  || oldy != ny ) {
						MoveEvent me( nx, ny, oldx, oldy );
						win->moveEvent( me );
					}
				}
				break;
			case MapNotify:
				{
					ShowEvent se;

					win = (*_windows)[ xevent.xmap.window ];
					win->showEvent( se );
				}
				break;
			case UnmapNotify:
				{
					HideEvent he;

					win = (*_windows)[ xevent.xmap.window ];
					win->hideEvent( he );
				}
				break;
			case Expose:
				{
					win = (*_windows)[ xevent.xexpose.window ];

					// Compress resize/motions before sending expose events
#if 1
					if( XCheckTypedWindowEvent( _dpy, xevent.xconfigure.window, ConfigureNotify, &xevent ) ) {
						::Window wp;
						int oldx = win->_rect.x;
						int oldy = win->_rect.y;
						unsigned int oldwidth = win->_rect.width;
						unsigned int oldheight = win->_rect.height;
						int nx = 0, ny = 0;
						unsigned int nw, nh, b, d;

						while( XCheckTypedWindowEvent( _dpy, xevent.xconfigure.window, ConfigureNotify, &xevent ) )
							;

						XGetGeometry( _dpy, xevent.xconfigure.window, &wp, &nx, &ny, &nw, &nh, &b, &d );
						XTranslateCoordinates( _dpy, xevent.xconfigure.window, RootWindow( _dpy, DefaultScreen( _dpy ) ), 0, 0, &nx, &ny, &wp);

						if( oldx != nx  || oldy != ny ) {
							MoveEvent me( nx, ny, oldx, oldy );
							win->moveEvent( me );
						}

						if( oldwidth != nw || oldheight != nh ) {
							ResizeEvent re( nw, nh, oldwidth, oldheight );
							win->resizeEvent( re );
						}
					}
#endif
					// Compress expose
					// FIXME: fix the rect information
					while( XCheckTypedWindowEvent( _dpy, xevent.xexpose.window, Expose, &xevent ) )
						;
					PaintEvent pe( 0, 0, win->_rect.width, win->_rect.height );
					win->paintEvent( pe );
				}
				break;
			case ButtonPress:
				{
					win = ( *_windows )[ xevent.xbutton.window ];
					MousePressEvent mp( xevent.xbutton.x, xevent.xbutton.y, xevent.xbutton.button );
					( ( Window* ) win->_widget )->mousePressEvent( mp );
				}
				break;
			case ButtonRelease:
				{
					win = ( *_windows )[ xevent.xbutton.window ];
					MouseReleaseEvent mr( xevent.xbutton.x, xevent.xbutton.y, xevent.xbutton.button );
					( ( Window* ) win->_widget )->mouseReleaseEvent( mr );
				}
				break;
			case KeyPress:
				{
					win = ( *_windows )[ xevent.xkey.window ];
					KeyEvent ke;
					X11KeyMap::mapToKeyEvent( ke, &xevent.xkey );
					( ( Window* ) win->_widget )->keyPressEvent( ke );
				}
				break;
			case KeyRelease:
				{
					win = ( *_windows )[ xevent.xkey.window ];
					KeyEvent ke;
					X11KeyMap::mapToKeyEvent( ke, &xevent.xkey );
					( ( Window* ) win->_widget )->keyReleaseEvent( ke );
				}
				break;
			case MotionNotify:
				{
					win = ( *_windows )[ xevent.xmotion.window ];
					//FIXME: what about the state ...
					while( XCheckTypedWindowEvent( _dpy, xevent.xmotion.window, MotionNotify, &xevent ) )
						;
					MouseMoveEvent mme( xevent.xmotion.x, xevent.xmotion.y, ( xevent.xmotion.state >> 8 ) & 0x1F );
					( ( Window* ) win->_widget )->mouseMoveEvent( mme );
				}
				break;
			case ClientMessage:
				{
					if( xevent.xclient.message_type == xatom_wmproto && ( ::Atom ) xevent.xclient.data.l[0] == xatom_wmdelete ) {
						CloseEvent ce;
						win = ( *_windows )[ xevent.xclient.window ];
						( ( Window* ) win->_widget )->closeEvent( ce );
					}
				}
				break;
			default:
				break;
		}
	}


	void X11Handler::handleQueued()
	{
		while( XEventsQueued( _dpy, QueuedAlready ) )
			handleEvents();
	}

	void X11Handler::onDataReadable()
	{
		if ( !XEventsQueued( _dpy, QueuedAfterFlush) )
			XSync( _dpy, 0 );
		while( XPending( _dpy ) )
			handleEvents();
	}

}
