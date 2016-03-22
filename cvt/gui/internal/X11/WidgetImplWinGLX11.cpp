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

#include <cvt/gui/internal/X11/WidgetImplWinGLX11.h>

namespace cvt {

	WidgetImplWinGLX11::WidgetImplWinGLX11( ::Display* display, const GLXContext* context, Widget* _window, std::deque<WidgetImplWinGLX11*>* updateq ) : _widget( _window ), dpy( display ), visible( false ), _rect( 0, 0, 1, 1 ), needsupdate( false ), _updateq( updateq )
	{
		::XSetWindowAttributes attr;
		GLFormat format;
		format.setStencilSize( 8 );
//		format.setSamples( 8 );
		_ctx = new GLXContext( dpy, format, context );

		const ::XVisualInfo* visinfo = _ctx->XVisualInfo();
		attr.colormap = ::XCreateColormap( dpy, RootWindow( dpy, DefaultScreen( dpy ) ), visinfo->visual, AllocNone);
		attr.event_mask = StructureNotifyMask | ButtonPressMask | ButtonReleaseMask | ButtonMotionMask | KeyPressMask | KeyReleaseMask | ExposureMask;
		attr.background_pixmap = None;
		attr.border_pixel = 0;

		unsigned long mask = CWColormap | CWEventMask | CWBackPixmap | CWBorderPixel;

		win = ::XCreateWindow( dpy, RootWindow( dpy, DefaultScreen( dpy ) ), 0, 0, _rect.width, _rect.height,
							  0, visinfo->depth, InputOutput,
							  visinfo->visual, mask, &attr );
		_ctx->setDrawable( win );
		_ctx->makeCurrent();
		gfxgl = new GFXEngineGL( _ctx );
	}

	WidgetImplWinGLX11::~WidgetImplWinGLX11()
	{
		setVisible( false );
		delete gfxgl;
		delete _ctx;
		::XDestroyWindow( dpy , win );
	}

	void WidgetImplWinGLX11::size( int& width, int& height ) const
	{
		width = _rect.width;
		height = _rect.height;
	}

	void WidgetImplWinGLX11::setSize( int width, int height )
	{
		if( _rect.width == width && _rect.height == height )
			return;

		::XResizeWindow( dpy, win, width, height );
	}

	void WidgetImplWinGLX11::position( int& x, int& y ) const
	{
		x = _rect.x;
		y = _rect.y;
	}

	void WidgetImplWinGLX11::setPosition( int x, int y )
	{
		if( _rect.x == x && _rect.y == y )
			return;

		::XMoveWindow( dpy, win, x, y );
	}

	void WidgetImplWinGLX11::setTitle( const String& title )
	{
//		char buffer[ 128 ];

//		title.copy( buffer, 128 );
		::XTextProperty windowName;
		windowName.value    = ( unsigned char* ) title.c_str();
		windowName.encoding = XA_STRING;
		windowName.format   = 8;
		windowName.nitems   = title.length();

		::XSetWMName( dpy, win, &windowName );
		::XFlush( dpy );
	}

	void WidgetImplWinGLX11::setRect( const Recti& r )
	{
		::XWindowChanges values;

		if( _rect == r )
			return;

		values.x = r.x;
		values.y = r.y;
		values.width = r.width;
		values.height = r.height;
		::XConfigureWindow( dpy , win, CWX | CWY | CWWidth | CWHeight , &values );
	}

	void WidgetImplWinGLX11::setVisible( bool visibility )
	{
		if( visible == visibility )
			return;

		visible = visibility;

		if( visible )
			::XMapWindow( dpy, win );
		else
			::XUnmapWindow( dpy, win );
	}

	void WidgetImplWinGLX11::update()
	{
		if( needsupdate || !visible )
			return;
		_updateq->push_back( this );
		needsupdate = true;
	}

	void WidgetImplWinGLX11::update( const Recti& )
	{
	    update();
	}

	void WidgetImplWinGLX11::raise()
	{
		XRaiseWindow( dpy, win );
	}

	void WidgetImplWinGLX11::lower()
	{
		XLowerWindow( dpy, win );
	}


	void WidgetImplWinGLX11::setMinimumSize( int w, int h )
	{
	    _minSize.set( Math::max( w, 0 ), Math::max( h, 0 ) );
	    XSizeHints hints;
	    hints.flags = PMinSize;
	    hints.min_width = ( uint32_t ) _minSize[ 0 ];
	    hints.min_height = ( uint32_t ) _minSize[ 1 ];
	    ::XSetWMNormalHints( dpy, win , &hints );
	}

	void WidgetImplWinGLX11::setMaximumSize( int w, int h )
	{
	    _maxSize.set( Math::max( w, 0 ), Math::max( h, 0 ) );
	    XSizeHints hints;
	    hints.flags = PMaxSize;
	    hints.max_width = ( uint32_t ) _maxSize[ 0 ];
	    hints.max_height = ( uint32_t ) _maxSize[ 1 ];
	    ::XSetWMNormalHints( dpy, win , &hints );
	}

	void WidgetImplWinGLX11::minimumSize( int& w, int& h )
	{
	    w = _minSize[ 0 ];
	    h = _minSize[ 1 ];
	}

	void WidgetImplWinGLX11::maximumSize( int& w, int& h )
	{
	    w = _maxSize[ 0 ];
	    h = _maxSize[ 1 ];
	}


	void WidgetImplWinGLX11::paintEvent( PaintEvent& event )
	{
		Recti viewport( 0, 0, _rect.width, _rect.height );
		gfxgl->setViewport( viewport );
		GFX g( gfxgl );
		_widget->paintEvent( event, g );
		needsupdate = false;
	}

	void WidgetImplWinGLX11::resizeEvent( ResizeEvent& event )
	{
		event.getSize( _rect.width, _rect.height );
		_widget->resizeEvent( event );
	}

	void WidgetImplWinGLX11::moveEvent( MoveEvent& event )
	{
		event.position( _rect.x, _rect.y );
		_widget->moveEvent( event );
	}

	void WidgetImplWinGLX11::showEvent( ShowEvent& event )
	{
		visible = true;
		_widget->showEvent( event );
	}

	void WidgetImplWinGLX11::hideEvent( HideEvent& event )
	{
		visible = false;
		_widget->hideEvent( event );
	}

	GFXEngine* WidgetImplWinGLX11::gfxEngine()
	{
		return NULL;
	}

}
