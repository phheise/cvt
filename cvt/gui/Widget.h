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

#ifndef CVTWIDGET_H
#define CVTWIDGET_H

#include <cvt/geom/Rect.h>
#include <cvt/gfx/GFX.h>
#include <cvt/gui/Events.h>
#include <cvt/util/String.h>

namespace cvt {
	class WidgetImpl;
	class GFXEngine;

	class Widget {
		friend class WidgetImplWinGLX11;
		friend class WidgetImplWinGLOSX;
		friend class WidgetImplDefault;
		friend class WidgetContainer;
		friend class Moveable;

		public:
			Widget( bool toplevel = false );
			virtual ~Widget();

			bool isToplevel() const { return _toplevel; }
			Widget* parent() const;
			void size( int& width, int& height ) const;
			void setSize( int width, int height );
			void position( int& x, int& y ) const;
			void setPosition( int x, int y );
			void setRect( const Recti& rect );
			const Recti& rect( ) const;
			void setVisible( bool b );
			bool isVisible() const;
			void show() { setVisible( true ); }
			void hide() { setVisible( false ); }
			void raise();
			void lower();
			void setTitle( const String& title );
			void setMinimumSize( int width, int height );
			void setMaximumSize( int width, int height );
			void minimumSize( int& w, int& h );
			void maximumSize( int& w, int& h );
			void update();
			void update( const Recti& rect );
			void mapGlobal( int& x, int& y ) const;
			void mapLocal( int& x, int& y ) const;
			virtual Widget* childAt( int , int ) { return NULL; }

		protected:
			virtual void resizeEvent( ResizeEvent& __attribute__ ((unused)) ) {}
			virtual void moveEvent( MoveEvent&  __attribute__ ((unused)) ) {}
			virtual void paintEvent( PaintEvent& __attribute__ ((unused)) , GFX& __attribute__ ((unused)) ) {}
			virtual void showEvent( ShowEvent& __attribute__ ((unused)) ) {}
			virtual void hideEvent( HideEvent& __attribute__ ((unused)) ) {}
			virtual void mouseMoveEvent( MouseMoveEvent& __attribute__ ((unused))) {}
			virtual void mousePressEvent( MousePressEvent& __attribute__ ((unused))) {}
			virtual void mouseReleaseEvent( MouseReleaseEvent& __attribute__ ((unused))) {}
			virtual void mouseWheelEvent() {}
			virtual void mouseEnterEvent() {}
			virtual void mouseLeaveEvent() {}
			virtual void keyPressEvent( KeyEvent& event __attribute__((unused)) ) {}
			virtual void keyReleaseEvent( KeyEvent& event __attribute__((unused)) ) {}

			virtual void raiseChild( Widget* ) {}
			virtual void lowerChild( Widget* ) {}

			void paintChild( Widget* w, GFX& gfx, const Recti& rect ) const;

		private:
			Widget( const Widget& w );

			void setParent( Widget* parent );

			bool _toplevel;
			WidgetImpl* impl;
	};
}

#endif
