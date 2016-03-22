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

#include <cvt/gui/Widget.h>
#include <cvt/gui/Application.h>
#include <cvt/gui/internal/WidgetImpl.h>
#include <cvt/gui/internal/WidgetImplDefault.h>

namespace cvt {
	Widget::Widget( bool toplevel ) : _toplevel( toplevel )
	{
		if( _toplevel )
			impl = Application::instance()->registerWindow( this );
		else
			impl = new WidgetImplDefault( this );
	}

	Widget::~Widget( )
	{
		if( _toplevel )
			Application::instance()->unregisterWindow( impl );
		delete impl;
	}

	Widget* Widget::parent() const
	{
		return impl->parent();
	}

	void Widget::setParent( Widget* w )
	{
		if( !_toplevel )
			impl->setParent( w );
	}

	void Widget::setSize( int width, int height )
	{
		impl->setSize( width, height );
	}

	void Widget::size( int& width, int& height ) const
	{
		impl->size( width, height );
	}

	void Widget::setPosition( int x, int y )
	{
		impl->setPosition( x, y );
	}

	void Widget::position( int& x, int& y ) const
	{
		impl->position( x, y );
	}

	void Widget::setRect( const Recti& rect )
	{
		impl->setRect( rect );
	}

	const Recti& Widget::rect( ) const
	{
		return impl->rect();
	}

	void Widget::setVisible( bool visibility )
	{
		impl->setVisible( visibility );
	}

	bool Widget::isVisible() const
	{
		return impl->isVisible();
	}

	void Widget::setTitle( const String& title )
	{
		impl->setTitle( title );
	}

	void Widget::setMinimumSize( int width, int height )
	{
		impl->setMinimumSize( width, height );
	}

	void Widget::setMaximumSize( int width, int height )
	{
		impl->setMaximumSize( width, height );
	}

	void Widget::minimumSize( int& w, int& h )
	{
		impl->minimumSize( w, h );
	}

	void Widget::maximumSize( int& w, int& h )
	{
		impl->maximumSize( w, h );
	}

	void Widget::update()
	{
		impl->update();
	}


	void Widget::update( const Recti& rect )
	{
		impl->update( rect );
	}

	void Widget::raise()
	{
		impl->raise();
	}

	void Widget::lower()
	{
		impl->lower();
	}

	void Widget::mapGlobal( int&x, int& y ) const
	{
		int gx, gy;

		if( isToplevel() )
			return;

		position( gx, gy );
		x += gx;
		y += gy;
	}

	void Widget::mapLocal( int& x, int& y ) const
	{
		int gx, gy;

		if( isToplevel() )
			return;

		position( gx, gy );
		x -= gx;
		y -= gy;
	}

	void Widget::paintChild( Widget* w, GFX& gfx, const Recti& rect ) const
	{
		if( w->parent() != this )
			return;

		/* get current cliprect and translation */
		Recti cliprect = gfx.clipRect();
		Vector2i gtransold;
		gfx.getTranslationGlobal( gtransold );
		/* get child rectangle in global coords */
		Recti newcliprect = w->rect();
		Recti childrect = newcliprect;

		newcliprect.intersect( cliprect );

		/* set new clipping  */
		gfx.setClipRect( newcliprect );
		/* do painting with default GFX, only paint currently visible part */
		newcliprect.intersect( rect );
		PaintEvent pe( newcliprect );
		gfx.setTranslationGlobal( Vector2i( childrect.x, childrect.y ) );
		gfx.setDefault();
		w->paintEvent( pe, gfx );
		/* restore old viewport */
		gfx.setTranslationGlobal( gtransold );
		gfx.setDefault();
		gfx.setClipRect( cliprect );
	}

}
