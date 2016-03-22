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

#ifndef CVTWIDGETIMPL_H
#define CVTWIDGETIMPL_H

#include <cvt/geom/Rect.h>
#include <cvt/gui/Widget.h>
#include <cvt/gui/Window.h>
#include <cvt/util/String.h>

namespace cvt {
	class WidgetImpl {
		public:
			virtual				~WidgetImpl() {};
			virtual void	    setSize( int width, int height ) = 0;
			virtual void	    size( int& width, int& height ) const = 0;
			virtual void	    setPosition( int width, int height ) = 0;
			virtual void	    position( int& width, int& height ) const = 0;
			virtual void	    setRect( const Recti& rect ) = 0;
			virtual const Recti&  rect() const = 0;
			virtual void	    setVisible( bool visibility ) = 0;
			virtual bool	    isVisible() const = 0;
			virtual void	    setTitle( const String& ) {};
			virtual void		raise() = 0;
			virtual void		lower() = 0;
			virtual void	    update() = 0;
			virtual void	    update( const Recti& rect ) = 0;
			virtual void	    setMinimumSize( int w, int h ) = 0;
			virtual void	    setMaximumSize( int w, int h ) = 0;
			virtual void	    minimumSize( int& w, int& h ) = 0;
			virtual void	    maximumSize( int& w, int& h ) = 0;
			virtual void	    setParent( Widget* w ) = 0;
			virtual Widget*	    parent() const = 0;
			virtual GFXEngine* gfxEngine() = 0;
	};
}

#endif
