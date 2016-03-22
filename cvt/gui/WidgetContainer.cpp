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

#include <cvt/gui/WidgetContainer.h>



namespace cvt {
	WidgetContainer::WidgetContainer() : Widget( false ), _activeWidget( NULL ), _focusWidget( NULL )
	{

	}

	WidgetContainer::WidgetContainer( bool toplevel ) : Widget( toplevel ), _activeWidget( NULL ), _focusWidget( NULL )
	{

	}

	WidgetContainer::~WidgetContainer()
	{
		_children.clear();
	}


	void WidgetContainer::setFocusWidget()
	{
		if( !childrenCount() ) {
			_focusWidget = NULL;
			return;
		}
		_focusWidget = _children.front().first;
	}

	Widget* WidgetContainer::childAt( int x, int y )
	{
		ChildList::iterator it = _children.begin();
		while( it != _children.end() )
		{
			Widget* w = it->first;
			if( w->parent() != this ) {
				ChildList::iterator it2 = it;
				++it;
				_children.erase( it2 );
				continue;
			}

			Recti r = w->rect();
			if( r.contains( x, y ) ) {
				return w;
			}
			++it;
		}
		return NULL;
	}

	void WidgetContainer::removeWidget( Widget* w  )
	{
		ChildList::iterator it = _children.begin();
		while( it != _children.end() )
		{
			Widget* widget = it->first;
			if( widget == w ) {
				_children.erase( it );
				setFocusWidget();
				return;
			}
			++it;
		}
	}

	void WidgetContainer::resizeEvent( ResizeEvent& )
	{
		resizeChildren();
	}

	void WidgetContainer::resizeChildren()
	{
		Recti r;
		Recti rself;

		rself = rect();
		if( isToplevel() )
			rself.setPosition( 0, 0 );

		ChildList::iterator it = _children.begin();
		while( it != _children.end() )
		{
			Widget* w = it->first;
			if( w->parent() != this ) {
				ChildList::iterator it2 = it;
				++it;
				_children.erase( it2 );
				continue;
			}

			r = w->rect( );
			it->second.rect( r, rself );
			w->setRect( r );
			++it;
		}
	}

	void WidgetContainer::moveEvent( MoveEvent& me )
	{
		int dx = me.x - me.oldx;
		int dy = me.y - me.oldy;
		moveChildren( dx, dy );
	}

	void WidgetContainer::moveChildren( int dx, int dy )
	{
		Recti r;
		int width, height;

		size( width, height );

		ChildList::iterator it = _children.begin();
		while( it != _children.end() )
		{
			Widget* w = it->first;
			if( w->parent() != this ) {
				ChildList::iterator it2 = it;
				++it;
				_children.erase( it2 );
				continue;
			}

			r = w->rect( );
			r.x += dx;
			r.y += dy;
			w->setRect( r );
			++it;
		}
	}

	void WidgetContainer::paintEvent( PaintEvent& , GFX& gfx )
	{
		Recti r = rect();
		paintChildren( gfx, r );
	}

	void WidgetContainer::paintChildren( GFX& gfx, const Recti& r )
	{

		ChildList::reverse_iterator it = _children.rbegin();
		while( it != _children.rend() )
		{
			Widget* w = it->first;
			if( w->parent() != this ) {
				ChildList::iterator it2 = it.base();
				++it;
				_children.erase( it2 );
				continue;
			}
			if( w->isVisible() ) {
				Recti rc;
				rc = w->rect();
				rc.intersect( r );
				paintChild( w, gfx, rc );
			}
			++it;
		}
	}

	void WidgetContainer::mousePressEvent( MousePressEvent& event )
	{
		int x, y;

		event.position( x, y );
		mapGlobal( x, y );
		_activeWidget = childAt( x, y );

		if( _activeWidget && _activeWidget->isVisible() ) {
			mapGlobal( event.x, event.y );
			_activeWidget->mapLocal( event.x, event.y );
			_activeWidget->mousePressEvent( event );
		} else {
			_activeWidget = NULL;
		}
	}

	void WidgetContainer::mouseMoveEvent( MouseMoveEvent& event )
	{
		if( _activeWidget ) {
			mapGlobal( event.x, event.y );
			_activeWidget->mapLocal( event.x, event.y );
			_activeWidget->mouseMoveEvent( event );
		}
	}

	void WidgetContainer::mouseReleaseEvent( MouseReleaseEvent& event )
	{
		if( _activeWidget ) {
			mapGlobal( event.x, event.y );
			_activeWidget->mapLocal( event.x, event.y );
			_activeWidget->mouseReleaseEvent( event );
		}
		_activeWidget = NULL;
	}

	void WidgetContainer::keyPressEvent( KeyEvent& event )
	{
		if( _focusWidget ) {
			_focusWidget->keyPressEvent( event );

		}
	}

	void WidgetContainer::keyReleaseEvent( KeyEvent& event )
	{
		if( _focusWidget ) {
			_focusWidget->keyReleaseEvent( event );
		}
	}


	void WidgetContainer::raiseChild( Widget* c )
	{
		if( c->parent() != this )
			return;

		for( ChildList::iterator it = _children.begin(), end = _children.end(); it != end; ++it  ) {
			if( it->first == c ) {
				if( _children.begin() != it ) {
					_children.splice( _children.begin(), _children, it );
					update();
				}
				return;
			}
		}
	}

	void WidgetContainer::lowerChild( Widget* c )
	{
		if( c->parent() != this )
			return;

		for( ChildList::iterator it = _children.begin(), end = _children.end(); it != end; ++it  ) {
			if( it->first == c ) {
				if( _children.rend().base() != it ) {
					_children.splice( _children.rend().base(), _children, it );
					update();
				}
				return;
			}
		}
	}

};
