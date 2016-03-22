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

#ifndef CVT_GLVIEW_H
#define CVT_GLVIEW_H

#include <cvt/gui/Widget.h>

namespace cvt
{
	class GLView : public Widget
	{
		public:
			GLView() :
				_viewport( rect() ),
				_allowViewportChange( false )
			{
			}

			virtual ~GLView(){}

			void setViewport( const Recti& rect );
			void setViewport( int x, int y, size_t w, size_t h );

		protected:
			void paintEvent( PaintEvent& ev, GFX& g );
			virtual void paintGLEvent( PaintEvent& ev ) = 0;

			Recti	_viewport;
			bool	_allowViewportChange;
	};

	inline void GLView::setViewport( int x, int y, size_t w, size_t h )
	{
		if( _allowViewportChange ) {
			Recti r = rect();
			Recti view( x + r.x, y + r.y, w, h );
			r.intersect( view );
			glViewport( r.x, _viewport.height - ( r.y + r.height ), r.width, r.height );
		}
	}

	inline void GLView::setViewport( const Recti& vrect )
	{
		if( _allowViewportChange ) {
			Recti r = rect();
			r.intersect( vrect );
			glViewport( r.x, _viewport.height - ( r.y + r.height ), r.width, r.height );
		}
	}
}

#endif
