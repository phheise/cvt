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

#ifndef CVT_TIMERINFO_H
#define CVT_TIMERINFO_H

#include <cvt/util/Time.h>
#include <cvt/gui/TimeoutHandler.h>
#include <cvt/util/IDGenerator.h>

namespace cvt {

	class TimerInfo {
		friend class TimerInfoList;

		public:
			TimerInfo( size_t msinterval, TimeoutHandler* handler );
			TimerInfo( const TimerInfo& ti );
			uint32_t id() const;
			int compare( const TimerInfo* t ) const;
			void nextTimeout();
			const Time& timeout() const;

		private:
			~TimerInfo();

			size_t _interval;
			uint32_t _id;
			Time _timeout;
			TimeoutHandler* _th;

			static IDGenerator _idgen;
	};

	inline TimerInfo::TimerInfo( size_t msinterval, TimeoutHandler* handler ) : _interval( msinterval ), _th( handler )
	{
		_id = _idgen.allocateID();
		nextTimeout();
	}

	inline void TimerInfo::nextTimeout()
	{
		_timeout.reset();
		_timeout += _interval;
	}

	inline TimerInfo::~TimerInfo()
	{
		_idgen.releaseID( _id );
	}

	inline uint32_t TimerInfo::id() const
	{
		return _id;
	}

	inline int TimerInfo::compare( const TimerInfo* t ) const
	{
		return _timeout.compare( t->_timeout );
	}

	inline const Time& TimerInfo::timeout() const
	{
		return _timeout;
	}
}

#endif
