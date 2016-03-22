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

#ifndef CVTRESIZEEVENT_H
#define CVTRESIZEEVENT_H

#include <cvt/gui/event/Event.h>

namespace cvt {
	class ResizeEvent : public Event
	{
		friend class ApplicationX11;
		public:
			ResizeEvent( int width, int height, int oldwidth, int oldheight ) : Event( EVENT_RESIZE ), _width( width ), _height( height ), _oldwidth( oldwidth ), _oldheight( oldheight ) { };
			void getSize( int& width, int& height ) const { width = _width; height = _height;  };
			void getOldSize( int& width, int& height ) const { width = _oldwidth; height = _oldheight;  };
			int width() const { return _width; };
			int height() const { return _height; };

		private:
			int _width, _height;
			int _oldwidth, _oldheight;
	};
};

#endif
