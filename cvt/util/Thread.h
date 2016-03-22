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

#ifndef CVTTHREAD_H
#define CVTTHREAD_H

#include <cvt/util/Exception.h>
#include <pthread.h>

namespace cvt {
	template<typename T>
	class Thread {
		public:
			Thread();
			virtual ~Thread() {}
			void run( T* );
			virtual void execute( T* arg ) = 0;
			void join();

		private:
			Thread( const Thread& t );
			static void* _run( Thread* t );

			pthread_t _tid;
			pthread_attr_t _tattr;
			T* _arg;
	};

	template<typename T>
	Thread<T>::Thread() : _arg( NULL )
	{
		int err;
		err = pthread_attr_init( &_tattr );
		if( err )
			throw CVTException( err );
		err = pthread_attr_setdetachstate(&_tattr, PTHREAD_CREATE_JOINABLE );
		if( err )
			throw CVTException( err );
	}

	template<typename T>
	void Thread<T>::run( T* arg )
	{
		int err;
		_arg = arg;
		err = pthread_create( &_tid, &_tattr, ( void* (*)( void* ) )  Thread::_run, this );
		if( err )
			throw CVTException( err );
	}

	template<typename T>
	void Thread<T>::join( )
	{
		int err;
		err = pthread_join( _tid, NULL );
		if( err )
			throw CVTException( err );
	}

	template<typename T>
	void* Thread<T>::_run( Thread* t )
	{
		t->execute( t->_arg );
		pthread_exit( NULL );
	}

}


#endif
