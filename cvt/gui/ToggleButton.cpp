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

#include <cvt/gui/ToggleButton.h>

namespace cvt {
	ToggleButton::ToggleButton( const String& label, bool state, GFX::Icon off, GFX::Icon on ) : _label( label ), _state( state ), _iconon( on ), _iconoff( off )
	{
	}

	ToggleButton::ToggleButton( bool state, GFX::Icon off, GFX::Icon on ) : _state( state ), _iconon( on ), _iconoff( off )
	{
	}

	ToggleButton::~ToggleButton()
	{

	}

	void ToggleButton::paintEvent( PaintEvent& , GFX& g )
	{
		int w, h;
		size( w, h );
		if( _label.isEmpty() ) {
			w = ( w - 16 ) >> 1;
			h = ( h - 16 ) >> 1;
			g.color().set( 0.6f, 0.6f, 0.6f, 1.0f );
			g.drawIcon( w, h, _state? _iconon : _iconoff );
		} else {
			w = 2;
			h = ( h - 16 ) >> 1;
			g.color().set( 0.6f, 0.6f, 0.6f, 1.0f );
			g.drawIcon( w, h, _state? _iconon : _iconoff );
			g.color().set( 0.8f, 0.8f, 0.8f, 1.0f );
			size( w, h );
			g.drawText( 20, 0, w - 20 , h, ALIGN_LEFT | ALIGN_VCENTER , _label.c_str() );
		}
	}

	void ToggleButton::mouseReleaseEvent( MouseReleaseEvent& event )
	{
		Recti self;
		self.setPosition( 0, 0 );
		size( self.width, self.height );
		if( self.contains( event.x, event.y ) && event.button() == 1 ) {
			_state = !_state;
			toggled.notify( this );
		}
		update();
	}

}
