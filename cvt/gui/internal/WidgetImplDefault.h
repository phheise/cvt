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

#ifndef CVT_WIDGETIMPLDEFAULT_H
#define CVT_WIDGETIMPLDEFAULT_H

#include <cvt/gui/internal/WidgetImpl.h>
#include <cvt/math/Vector.h>


namespace cvt {
	class WidgetImplDefault : public WidgetImpl
	{
		public:
			WidgetImplDefault( Widget* w );
			~WidgetImplDefault();

			virtual void	    setSize( int width, int height );
			virtual void	    size( int& width, int& height ) const;
			virtual void	    setPosition( int width, int height );
			virtual void	    position( int& width, int& height ) const;
			virtual void	    setRect( const Recti& rect );
			virtual const Recti& rect() const;
			virtual void	    setVisible( bool visibility );
			virtual bool	    isVisible() const;
			virtual void		raise();
			virtual void		lower();
			virtual void	    update();
			virtual void	    update( const Recti& rect );
			virtual void	    setMinimumSize( int w, int h );
			virtual void	    setMaximumSize( int w, int h );
			virtual void	    minimumSize( int& w, int& h );
			virtual void	    maximumSize( int& w, int& h );
			virtual void	    setParent( Widget* w );
			virtual Widget*	    parent() const;
			virtual GFXEngine* gfxEngine();

		private:
			WidgetImplDefault( const WidgetImpl& wi );

			Widget*	 _widget;
			Widget*	 _parent;
			Recti	 _rect;
			bool	 _visible;
			Vector2<int> _minSize;
			Vector2<int> _maxSize;
	};


	inline WidgetImplDefault::WidgetImplDefault( Widget* w ) : _widget( w ), _parent( NULL ), _rect( 0, 0, 1, 1), _visible( false ), _minSize( 0, 0 ), _maxSize( 32768, 32768 )
	{
	}

	inline WidgetImplDefault::~WidgetImplDefault( )
	{
	}


	inline void WidgetImplDefault::setSize( int width, int height )
	{
		if( _rect.width == width && _rect.height == height )
			return;

		width  = Math::min( Math::max( width, _minSize[ 0 ] ), _maxSize[ 0 ] );
		height = Math::min( Math::max( height,_minSize[ 1 ] ), _maxSize[ 1 ] );
		ResizeEvent re( width, height, _rect.width, _rect.height );
		_rect.width  = width;
		_rect.height = height;
		_widget->resizeEvent( re );
	}

	inline void WidgetImplDefault::size( int& width, int& height ) const
	{
		width  = _rect.width;
		height = _rect.height;
	}

	inline void WidgetImplDefault::setPosition( int x, int y )
	{
		if( _rect.x == x && _rect.y == y )
			return;
		MoveEvent me( x, y, _rect.x, _rect.y );
		_rect.x  = x;
		_rect.y  = y;
		_widget->moveEvent( me );
	}

	inline void WidgetImplDefault::position( int& x, int& y ) const
	{
		x  = _rect.x;
		y  = _rect.y;
	}

	inline void WidgetImplDefault::setRect( const Recti& rect )
	{
		setPosition( rect.x, rect.y );
		setSize( rect.width, rect.height );
	}

	inline const Recti& WidgetImplDefault::rect( ) const
	{
		return _rect;
	}

	inline void WidgetImplDefault::setVisible( bool visibility )
	{
		if( _visible == visibility )
			return;
		_visible = visibility;
		if( _visible ) {
			ShowEvent se;
			_widget->showEvent( se );
		} else {
			HideEvent he;
			_widget->hideEvent( he );
		}
	}

	inline bool WidgetImplDefault::isVisible() const
	{
		return _visible;
	}

	inline void WidgetImplDefault::update()
	{
		if( _parent )
			_parent->update( _rect );
	}

	inline void WidgetImplDefault::update( const Recti& rect )
	{
		if( _parent ) {
			Recti r = _rect;
			r.intersect( rect );
			if( !r.isEmpty() )
				_parent->update( r );
		}
	}

	inline void WidgetImplDefault::setMinimumSize( int w, int h )
	{
		_minSize.set( w, h );
	}

	inline void WidgetImplDefault::setMaximumSize( int w, int h )
	{
		_maxSize.set( w, h );
	}

	inline void WidgetImplDefault::minimumSize( int& w, int& h )
	{
		w = _minSize[ 0 ];
		h = _minSize[ 1 ];
	}

	inline void WidgetImplDefault::maximumSize( int& w, int& h )
	{
		w = _maxSize[ 0 ];
		h = _maxSize[ 1 ];
	}

	inline void WidgetImplDefault::setParent( Widget* w )
	{
		_parent = w;
	}

	inline Widget* WidgetImplDefault::parent() const
	{
		return _parent;
	}

	inline void WidgetImplDefault::raise()
	{
		if( _parent )
			_parent->raiseChild( _widget );
	}

	inline void WidgetImplDefault::lower()
	{
		if( _parent )
			_parent->lowerChild( _widget );
	}

	inline GFXEngine* WidgetImplDefault::gfxEngine()
	{
		return NULL;
	}
}
#endif
