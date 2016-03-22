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

#ifndef CVTTQUEUE_H
#define CVTTQUEUE_H

#include <deque>
#include <cvt/util/Mutex.h>
#include <cvt/util/Condition.h>

namespace cvt {
	template<typename T>
	class TQueue {
		public:
			TQueue() {};
			~TQueue() {};
			void enqueue( T a );
			T waitNext();

		private:
			std::deque<T> _queue;
			Mutex _mutex;
			Condition _cond;
	};

	template<typename T>
	void TQueue<T>::enqueue( T e )
	{
		_mutex.lock();
		_queue.push_back( e );
		_cond.notify();
		_mutex.unlock();
	}

	template<typename T>
	T TQueue<T>::waitNext()
	{
		T ret;
		_mutex.lock();
		while( _queue.empty() )
			_cond.wait( _mutex );
		ret = _queue.front();
		_queue.pop_front();
		_mutex.unlock();
		return ret;
	}

}



#endif
