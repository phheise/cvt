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

#include <cvt/gui/internal/OSX/WidgetImplWinGLOSX.h>
#include <cvt/gui/internal/OSX/OSXGLView.h>
#include <Cocoa/Cocoa.h>

namespace cvt {
	struct OSXDataImplWin {
		NSWindow* _win;
		NSOpenGLView* _view;
	};

	WidgetImplWinGLOSX::WidgetImplWinGLOSX( const CGLContext* shrctx, Widget* window ) : _widget( window ), _visible( false ), _rect( 0, 0, 1, 1 )
	{
		GLFormat format;
		_ctx = new CGLContext( format, shrctx );
		NSRect frame = NSMakeRect( _rect.x, _rect.y, _rect.width, _rect.height );
		_osx = new OSXDataImplWin;
		_osx->_win = [ [ NSWindow alloc ] initWithContentRect:frame
					     styleMask:NSTitledWindowMask | NSMiniaturizableWindowMask | NSClosableWindowMask | NSResizableWindowMask 
						 backing:NSBackingStoreBuffered
						 defer:NO ];
		_osx->_view = ( NSOpenGLView* )[[ OSXGLView alloc ] initWithFrame: frame CGLContextObj: ( _ctx->_cglctx ) WidgetImpl: ( WidgetImplWinGLOSX* )this ];
		[ _osx->_win setContentView: _osx->_view ];

		[ [ NSNotificationCenter defaultCenter] addObserver: _osx->_view selector:@selector(moveEvent:) name:NSWindowDidMoveNotification object:nil ];

		_ctx->makeCurrent();
		_gfxgl = new GFXEngineGL( _ctx );
	}

	WidgetImplWinGLOSX::~WidgetImplWinGLOSX()
	{
		[ _osx->_win release ];
		[ _osx->_view release ];
		setVisible( false );
		delete _gfxgl;
		delete _ctx;
		delete _osx;
	}

	void WidgetImplWinGLOSX::size( int& width, int& height ) const
	{
		width = _rect.width;
		height = _rect.height;
	}

	void WidgetImplWinGLOSX::setSize( int width, int height )
	{
		if( _rect.width == width && _rect.height == height )
			return;

		// FIXME: Do we get events or set the stuff ourselves ?
		//_rect.width = width;
		//_rect.height = height;
		[ _osx->_win setFrame: NSMakeRect( _rect.x, _rect.y, width, height ) display: true ];
	}

	void WidgetImplWinGLOSX::position( int& x, int& y ) const
	{
		x = _rect.x;
		y = _rect.y;
	}

	void WidgetImplWinGLOSX::setPosition( int x, int y )
	{
		if( _rect.x == x && _rect.y == y )
			return;

		// FIXME: Do we get events or set the stuff ourselves ?
		//_rect.x = x;
		//_rect.y = y;
		[ _osx->_win setFrameTopLeftPoint: NSMakePoint( x, y ) ];
	}

	void WidgetImplWinGLOSX::setTitle( const String& title )
	{
		[ _osx->_win setTitle: [ NSString stringWithUTF8String:title.c_str() ] ];
	}

	void WidgetImplWinGLOSX::setRect( const Recti& r )
	{
		if( _rect == r )
			return;
		// FIXME: Do we get events or set the stuff ourselves ?
		//_rect = r;
		[ _osx->_win setFrame: NSMakeRect( r.x, r.y, r.width, r.height ) display: true ];
	}

	void WidgetImplWinGLOSX::setVisible( bool visibility )
	{
		if( _visible == visibility )
			return;

		_visible = visibility;

		if( _visible )
			[ _osx->_win makeKeyAndOrderFront: nil ];
		else
			[ _osx->_win orderOut: nil ];
	}

	void WidgetImplWinGLOSX::update()
	{
		[ _osx->_view setNeedsDisplay: YES ];
	}

	void WidgetImplWinGLOSX::update( const Recti& )
	{
	    update();
	}

	void WidgetImplWinGLOSX::raise()
	{
		[ _osx->_win orderFront: nil ];
	}

	void WidgetImplWinGLOSX::lower()
	{
		[ _osx->_win orderBack: nil ];
	}


	void WidgetImplWinGLOSX::setMinimumSize( int w, int h )
	{
	    _minSize.set( Math::max( w, 0 ), Math::max( h, 0 ) );
		[ _osx->_win setMinSize: NSMakeSize( _minSize.x, _minSize.y ) ];
	}

	void WidgetImplWinGLOSX::setMaximumSize( int w, int h )
	{
	    _maxSize.set( Math::max( w, 0 ), Math::max( h, 0 ) );
		[ _osx->_win setMaxSize: NSMakeSize( _maxSize.x, _maxSize.y ) ];
	}

	void WidgetImplWinGLOSX::minimumSize( int& w, int& h )
	{
	    w = _minSize[ 0 ];
	    h = _minSize[ 1 ];
	}

	void WidgetImplWinGLOSX::maximumSize( int& w, int& h )
	{
	    w = _maxSize[ 0 ];
	    h = _maxSize[ 1 ];
	}


	void WidgetImplWinGLOSX::paintEvent( PaintEvent& event )
	{
		Recti viewport( 0, 0, _rect.width, _rect.height );
		_gfxgl->setViewport( viewport );
		GFX g( _gfxgl );
		_widget->paintEvent( event, g );
	}

	void WidgetImplWinGLOSX::resizeEvent( ResizeEvent& event )
	{
		event.getSize( _rect.width, _rect.height );
		_widget->resizeEvent( event );
	}

	void WidgetImplWinGLOSX::moveEvent( MoveEvent& event )
	{
		event.position( _rect.x, _rect.y );
		_widget->moveEvent( event );
	}

	void WidgetImplWinGLOSX::showEvent( ShowEvent& event )
	{
		_visible = true;
		_widget->showEvent( event );
	}

	void WidgetImplWinGLOSX::hideEvent( HideEvent& event )
	{
		_visible = false;
		_widget->hideEvent( event );
	}

	GFXEngine* WidgetImplWinGLOSX::gfxEngine()
	{
		return NULL;
	}

}
