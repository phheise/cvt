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

#ifndef CVT_TIMERINFOLIST_H
#define CVT_TIMERINFOLIST_H

#include <cvt/gui/TimeoutHandler.h>

#include <stdint.h>
#include <iostream>
#include <list>
#include <utility>

namespace cvt {
	class TimerInfo;

	class TimerInfoList {
		public:
			TimerInfoList();
			~TimerInfoList();
			void handleTimers();
			uint32_t registerTimer( size_t intervalms, TimeoutHandler* t );
			void unregisterTimer( uint32_t id );
			int nextTimeout();

		private:
			void insertTimer( std::list<TimerInfo*>* list, TimerInfo* t );
			TimerInfoList( const TimerInfoList& tl );

			std::list<TimerInfo*> _timers;
	};


}

#endif
