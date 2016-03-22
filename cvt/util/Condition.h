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

#ifndef CVTCONDITION_H
#define CVTCONDITION_H

#include <cvt/util/Exception.h>
#include <pthread.h>
#include <errno.h>

namespace cvt {
	class Condition {
		public:
			Condition();
			~Condition();
			void wait( Mutex& mtx );
			void notify();
			void notifyAll();
		private:
			Condition( const Condition& c );
			pthread_cond_t _tcond;
	};

	inline Condition::Condition()
	{
		int err;
		err = pthread_cond_init( &_tcond, NULL );
		if( err )
			throw CVTException( err );
	}

	inline Condition::~Condition()
	{
		int err;
		err = pthread_cond_destroy( &_tcond );
		if( err )
			throw CVTException( err );
	}

	inline void Condition::wait( Mutex& mtx )
	{
		int err;
		err = pthread_cond_wait( &_tcond, &mtx._tmutex );
		if( err )
			throw CVTException( err );
	}

	inline void Condition::notify()
	{
		int err;
		err = pthread_cond_signal( &_tcond );
		if( err )
			throw CVTException( err );
	}

	inline void Condition::notifyAll()
	{
		int err;
		err =  pthread_cond_broadcast( &_tcond );
		if( err )
			throw CVTException( err );
	}
}

#endif
