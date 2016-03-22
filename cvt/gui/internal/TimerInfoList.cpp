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

#include <cvt/gui/internal/TimerInfoList.h>
#include <cvt/gui/internal/TimerInfo.h>

namespace cvt {
		TimerInfoList::TimerInfoList()
		{
		}

		TimerInfoList::~TimerInfoList()
		{
			// FIXME: do cleanup
		}


		void TimerInfoList::insertTimer( std::list<TimerInfo*>* list, TimerInfo* t )
		{
			for( std::list<TimerInfo*>::reverse_iterator rit = list->rbegin() ; rit != list->rend(); ++rit ) {
				if( ( *rit )->compare( t ) < 0  ) {
					_timers.insert( rit.base(), t );
					return;
				}
			}
			_timers.push_front( t );
		}

		uint32_t TimerInfoList::registerTimer( size_t intervalms, TimeoutHandler* th )
		{
			TimerInfo* ti = new TimerInfo( intervalms, th );
			insertTimer( &_timers, ti );
			return ti->id();
		}

		void TimerInfoList::unregisterTimer( uint32_t id )
		{
			for( std::list<TimerInfo*>::iterator it =_timers.begin() ; it != _timers.end(); ++it ) {
				if( ( *it )->id() == id  ) {
					TimerInfo* ti = *it;
					_timers.erase( it );
					delete ti;
					return;
				}
			}
		}

		int TimerInfoList::nextTimeout()
		{
			if( _timers.empty() )
				return -1;
			const TimerInfo* front = _timers.front();
			Time current;
			double t = ( front->timeout() - current );
			if( t > 0.0 )
				return ( int ) t;
			else
				return 0;
		}

		void TimerInfoList::handleTimers( )
		{
			Time now;
			std::list<TimerInfo*> tmplist;

			while( !_timers.empty() ) {
				if( ( _timers.front() )->timeout().compare( now ) < 0 ) {
					tmplist.splice( tmplist.begin(), _timers, _timers.begin() );
				} else
					break;
			}
			while( !tmplist.empty() ) {
					TimerInfo* tinfo = tmplist.back();
					tmplist.pop_back();
					tinfo->_th->onTimeout();
					tinfo->nextTimeout();
					insertTimer( &_timers, tinfo );
			}
		}

}
