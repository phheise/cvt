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

#ifndef CVTMOUSEMOVEEVENT_H
#define CVTMOUSEMOVEEVENT_H

#include <cvt/gui/event/Event.h>

namespace cvt {
	class MouseMoveEvent : public Event
	{
		public:
			MouseMoveEvent( int _x, int _y, unsigned int buttonMask ) : Event( EVENT_MOUSEMOVE ), x( _x ), y( _y ), _buttonMask( buttonMask ) { };
			void position( int& x, int& y ) { x = this->x; y = this->y;  };
			unsigned int buttonMask() { return _buttonMask; };

			int x, y;
		private:
			unsigned int _buttonMask;
	};
};

#endif
