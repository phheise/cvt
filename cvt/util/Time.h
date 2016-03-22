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

#ifndef CVT_TIME_H
#define CVT_TIME_H

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>

#ifdef APPLE
	#include <mach/mach_time.h>
#else
	#ifndef _POSIX_TIMERS
		#error "Posix timers not supported"
	#endif
	#ifndef _POSIX_MONOTONIC_CLOCK
		#error "Posix monotonic clock not supported"
	#endif
#endif

namespace cvt {

	class Time {
		public:
			Time();
			Time( uint32_t sec, uint32_t nsecs );
			Time( const Time& t );
			~Time();
			void reset();
			double elapsedSeconds() const;
			double elapsedMilliSeconds() const;
			double elapsedMicroSeconds() const;
			double ms() const;
			double operator+( const Time& t ) const;
			double operator-( const Time& t ) const;
			double operator+( double ms ) const;
			double operator-( double ms ) const;
			const Time& operator+=( size_t ms );
			int compare( const Time& t ) const;

		private:
			double timespecToMS( const struct timespec& ts ) const;
			double timespecToUS( const struct timespec& ts ) const;
			double timespecToS( const struct timespec& ts ) const;
			void updateTimespec( struct timespec& ts ) const;

			struct timespec _ts;

#ifdef APPLE
			static mach_timebase_info_data_t _machTimebase;
#endif
	};

	inline Time::Time()
	{
#ifdef APPLE
		if( _machTimebase.denom == 0 )
               mach_timebase_info( &_machTimebase );
#endif
		reset();
	}

	inline Time::Time( const Time& t )
	{
		_ts.tv_sec = t._ts.tv_sec;
		_ts.tv_nsec = t._ts.tv_nsec;
	}

	inline Time::Time( uint32_t sec, uint32_t nsecs )
	{
		_ts.tv_sec = sec;
		_ts.tv_nsec = nsecs;
	}

	inline Time::~Time() {}

	inline void Time::reset()
	{
		updateTimespec( _ts );
	}

	inline void Time::updateTimespec( struct timespec& ts ) const
	{
#ifdef APPLE
		uint64_t t = mach_absolute_time();
		uint64_t ns = t * ( _machTimebase.numer / _machTimebase.denom );
		ts.tv_sec = ns / 1000000000L;
        ts.tv_nsec = ns - (ts.tv_sec * 1000000000L);
#else
		clock_gettime( CLOCK_MONOTONIC, &ts );
#endif
	}

	inline double Time::timespecToS( const struct timespec& ts ) const
	{
		return ( ( double ) ts.tv_sec ) + ( ( double ) ts.tv_nsec ) * 0.000000001;
	}

	inline double Time::timespecToMS( const struct timespec& ts ) const
	{
		return ( ( double ) ts.tv_sec ) * 1000.0 + ( ( double ) ts.tv_nsec ) * 0.000001;
	}

	inline double Time::timespecToUS( const struct timespec& ts ) const
	{
		return ( ( double ) ts.tv_sec ) * 1000000.0 + ( ( double ) ts.tv_nsec ) * 0.001;
	}

	inline double Time::elapsedSeconds() const
	{
		struct timespec ts2;
		updateTimespec( ts2 );
		return timespecToS( ts2 ) - timespecToS( _ts );
	}

	inline double Time::elapsedMilliSeconds() const
	{
		struct timespec ts2;
		updateTimespec( ts2 );
		return timespecToMS( ts2 ) - timespecToMS( _ts );
	}

	inline double Time::elapsedMicroSeconds() const
	{
		struct timespec ts2;
		updateTimespec( ts2 );
		return timespecToUS( ts2 ) - timespecToUS( _ts );
	}

	inline double Time::operator-( const Time& t ) const
	{
		return timespecToMS( _ts ) - timespecToMS( t._ts );
	}

	inline double Time::operator+( const Time& t ) const
	{
		return timespecToMS( _ts ) + timespecToMS( t._ts );
	}

	inline double Time::operator-( double ms ) const
	{
		return timespecToMS( _ts ) - ms;
	}

	inline double Time::operator+( double ms ) const
	{
		return timespecToMS( _ts ) + ms;
	}

	inline double Time::ms() const
	{
		return timespecToMS( _ts );
	}

	inline const Time& Time::operator+=( size_t ms )
	{
		long ns;
		ldiv_t res;
		ns = ms * 1000000L + _ts.tv_nsec;
		res = ldiv( ns, 1000000000L );
		_ts.tv_sec += res.quot;
		_ts.tv_nsec = res.rem;
		return *this;
	}

	inline int Time::compare( const Time& t ) const
	{
		if ( _ts.tv_sec < t._ts.tv_sec)
			return -1;
		if ( _ts.tv_sec > t._ts.tv_sec)
			return 1;
		return ( int ) ( _ts.tv_nsec - t._ts.tv_nsec );
	}
}

#endif
