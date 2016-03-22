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

#ifndef CVTMUTEX_H
#define CVTMUTEX_H

#include <cvt/util/Exception.h>
#include <pthread.h>
#include <errno.h>

namespace cvt {
	class Mutex {
		friend class Condition;
		public:
			Mutex();
			~Mutex();
			void lock();
			void unlock();
			bool trylock();
		private:
			Mutex( const Mutex& t );

			pthread_mutex_t _tmutex;
	};

	inline Mutex::Mutex()
	{
		int err;
		err = pthread_mutex_init( &_tmutex, NULL );
		if( err )
			throw CVTException( err );
	}

	inline Mutex::~Mutex()
	{
		int err;
		err = pthread_mutex_destroy( &_tmutex );
		if( err )
			throw CVTException( err );
	}

	inline void Mutex::lock()
	{
		pthread_mutex_lock( &_tmutex );
	}

	inline bool Mutex::trylock()
	{
		int ret = pthread_mutex_trylock( &_tmutex );
		if( ret && ret != EBUSY )
			throw CVTException( ret );
		return ( ret == EBUSY );
	}

	inline void Mutex::unlock()
	{
		int err;
		err = pthread_mutex_unlock( &_tmutex );
		if( err )
			throw CVTException( err );
	}

	class ScopeLock
	{
		public:
			ScopeLock( Mutex* mtx );
			~ScopeLock();

		private:
			ScopeLock( const ScopeLock& sl );

			Mutex* _mtx;
	};

	inline ScopeLock::ScopeLock( Mutex* mtx ) : _mtx( mtx )
	{
		_mtx->lock();
	}

	inline ScopeLock::~ScopeLock()
	{
		_mtx->unlock();
	}
}

#endif
