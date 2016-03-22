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

#ifndef CVT_WIDGETIMPLWINGOSX_H
#define CVT_WIDGETIMPLWINGOSX_H

#include <cvt/gui/internal/WidgetImpl.h>
#include <cvt/gui/internal/OSX/CGLContext.h>
#include <cvt/gui/Window.h>
#include <cvt/gfx/GFX.h>
#include <cvt/math/Math.h>
#include <cvt/math/Vector.h>
#include <cvt/gl/GFXEngineGL.h>
#include <cvt/util/String.h>

namespace cvt {
	struct OSXDataImplWin;
	class ApplicationOSX;

	class WidgetImplWinGLOSX : public WidgetImpl
	{
		friend class ApplicationOSX;
		public:
			WidgetImplWinGLOSX( const CGLContext* shrctx, Widget* window );
			~WidgetImplWinGLOSX();

			virtual void setTitle( const String& title );
			virtual void setRect( const Recti& rect );
			virtual const Recti& rect( ) const { return _rect; };
			virtual void size( int& width, int& height ) const;
			virtual void setSize( int width, int height );
			virtual void position( int& x, int& y ) const;
			virtual void setPosition( int x, int y );
			virtual void setVisible( bool b );
			virtual bool isVisible( ) const { return _visible; };
			virtual void raise();
			virtual void lower();
			virtual void update();
			virtual void update( const Recti& rect );
			virtual void setMinimumSize( int w, int h );
			virtual void setMaximumSize( int w, int h );
			virtual void minimumSize( int& w, int& h );
			virtual void maximumSize( int& w, int& h );
			virtual void setParent( Widget* ) {};
			virtual Widget* parent( ) const { return NULL; };
			virtual GFXEngine* gfxEngine();

			void paintEvent( PaintEvent& event );
			void resizeEvent( ResizeEvent& event );
			void moveEvent( MoveEvent& event );
			void showEvent( ShowEvent& event );
			void hideEvent( HideEvent& event );
			void mousePressEvent( MousePressEvent& event ) { _widget->mousePressEvent( event ); }
			void mouseReleaseEvent( MouseReleaseEvent& event ) { _widget->mouseReleaseEvent( event ); }
			void mouseMoveEvent( MouseMoveEvent& event ) { _widget->mouseMoveEvent( event ); }
			void keyPressEvent( KeyEvent& ke ) { _widget->keyPressEvent( ke ); }
			void keyReleaseEvent( KeyEvent& ke ) { _widget->keyReleaseEvent( ke ); }

		private:
			Widget*		    _widget;
			bool			_visible;
			Recti			_rect;
			CGLContext*     _ctx;
			GFXEngineGL*    _gfxgl;
			Vector2<int>	_minSize;
			Vector2<int>	_maxSize;
			OSXDataImplWin* _osx;
	};
}

#endif
