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

#ifndef CVT_BASICTIMER_H
#define CVT_BASICTIMER_H

#include <cvt/gui/TimeoutHandler.h>
#include <cvt/util/Signal.h>
#include <iostream>
#include <stdint.h>

namespace cvt {
	class BasicTimer : public TimeoutHandler {
		public:
			BasicTimer( size_t interval_ms );
			~BasicTimer();
			void start();
			void stop();
			Signal<BasicTimer*> timeout;
		private:
			void onTimeout();

			uint32_t _id;
			size_t _interval;
	};

	inline BasicTimer::BasicTimer( size_t ms ) : _id( 0 ), _interval( ms )
	{
	}

	inline BasicTimer::~BasicTimer( )
	{
		stop();
	}

	inline void BasicTimer::onTimeout()
	{
		timeout.notify( this );
	}
};


#endif
