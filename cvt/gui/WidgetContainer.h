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

#ifndef CVT_WIDGETCONTAINER_H
#define CVT_WIDGETCONTAINER_H

#include <cvt/gui/Widget.h>
#include <cvt/gui/WidgetLayout.h>
#include <list>
#include <utility>

namespace cvt {
	class Window;

	class WidgetContainer : public Widget
	{
		friend class Window;

		public:
			WidgetContainer();
			~WidgetContainer();

			void    addWidget( Widget* w, const WidgetLayout& layout );
			void    addWidget( Widget* w );
			void    removeWidget( Widget* w );
			size_t	childrenCount() const;
			Widget* childAt( int x, int y );

			void resizeEvent( ResizeEvent& event );
			void moveEvent( MoveEvent& event );
			void paintEvent( PaintEvent& event, GFX& gfx );
			void mousePressEvent( MousePressEvent& event );
			void mouseMoveEvent( MouseMoveEvent& event );
			void mouseReleaseEvent( MouseReleaseEvent& event );
			void keyPressEvent( KeyEvent& event );
			void keyReleaseEvent( KeyEvent& event );

		protected:
			void paintChildren( GFX& gfx, const Recti& r );
			void resizeChildren( );
			void moveChildren( int dx, int dy );
			void raiseChild( Widget* c );
			void lowerChild( Widget* c );

		private:
			typedef std::list< std::pair<Widget*, WidgetLayout> > ChildList;

			WidgetContainer( bool toplevel );
			void setFocusWidget();

			ChildList _children;
			Widget* _activeWidget;
			Widget* _focusWidget;
	};

	inline size_t WidgetContainer::childrenCount() const
	{
		return _children.size();
	}


	inline void WidgetContainer::addWidget( Widget* w, const WidgetLayout& layout )
	{
		w->setParent( this );
		w->setVisible( true );
		_children.push_back( std::pair<Widget*, WidgetLayout>( w, layout ) );
		resizeChildren();
		setFocusWidget();
	}

	inline void WidgetContainer::addWidget( Widget* w )
	{
		w->setParent( this );
		w->setVisible( true );
		_children.push_front( std::pair<Widget*, WidgetLayout>( w, WidgetLayout() ) );
		resizeChildren();
		setFocusWidget();
	}


}

#endif
