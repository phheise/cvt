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

#ifndef CVTEVENT_H
#define CVTEVENT_H

namespace cvt {

	enum EventType {
		EVENT_NONE = 0,
		EVENT_RESIZE,
		EVENT_MOVE,
		EVENT_SHOW,
		EVENT_HIDE,
		EVENT_CLOSE,
		EVENT_PAINT,
		EVENT_MOUSEENTER,
		EVENT_MOUSELEAVE,
		EVENT_MOUSEMOVE,
		EVENT_MOUSEPRESS,
		EVENT_MOUSERELEASE,
		EVENT_MOUSEWHEEL,
		EVENT_KEY,
		EVENT_TIMER
	};

	class Event {
		public:
			Event( EventType t ) : _type( t ) {};
			virtual ~Event() {};
			EventType type() const { return _type; };

		private:
			EventType _type;
	};
}

#endif
