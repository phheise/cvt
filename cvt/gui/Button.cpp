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

#include <cvt/gui/Button.h>

namespace cvt {
	Button::Button( const String& label ) : _label( label )
	{
	}

	Button::~Button()
	{

	}

	void Button::paintEvent( PaintEvent& , GFX& g )
	{
		int w, h;
		g.color().set( 0.2f, 0.2f, 0.2f, 1.0f );
		size( w, h );
		g.fillRoundRect( 0, 0, w, h, 0.5f * ( float ) h );
		g.color().set( 0.8f, 0.8f, 0.8f, 1.0f );
		Recti bounds( 0, 0, w, h );
		g.drawText( bounds, ALIGN_CENTER | ALIGN_VCENTER, _label.c_str() );
	}

	void Button::mouseReleaseEvent( MouseReleaseEvent& event )
	{
		Recti self;
		self.setPosition( 0, 0 );
		size( self.width, self.height );
		if( self.contains( event.x, event.y ) && event.button() == 1 )
			clicked.notify();
	}


}
