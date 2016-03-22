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

#ifndef CVT_SIGNAL_H
#define CVT_SIGNAL_H

#include <cvt/util/Delegate.h>
#include <list>
#include <stddef.h>

namespace cvt {
	template<typename T> class Signal;

	template<typename T>
	class Signal
	{
		typedef typename std::list<Delegate<void ( T )> > ListType;
		typedef typename std::list<Delegate<void ( T )> >::iterator ListTypeIterator;

		public:
			Signal() {}
			~Signal() {}
			void add( const Delegate<void ( T )>& d ) { _delegates.push_back( d ); }
			void remove( const Delegate<void ( T )>& d ) { _delegates.remove( d ); }
			void notify( T arg );
			void notify( );
			size_t numDelegates() { return _delegates.size(); }

		private:
			ListType _delegates;
	};

	template<typename T>
	inline void Signal<T>::notify( T arg )
	{
		for( ListTypeIterator it = _delegates.begin(); it != _delegates.end(); ++it ) {
			it->operator()( arg );
		}
	}

	template<>
	class Signal<void>
	{
		typedef std::list<Delegate<void ( )> > ListType;
		typedef std::list<Delegate<void ( )> >::iterator ListTypeIterator;

		public:
			Signal() {}
			~Signal() {}
			void add( const Delegate<void ( )>& d ) { _delegates.push_back( d ); }
			void remove( const Delegate<void ( )>& d ) { _delegates.remove( d ); }
			void notify( void );

		private:
			ListType _delegates;
	};


	inline void Signal<void>::notify( void )
	{
		for( Signal<void>::ListTypeIterator it = _delegates.begin(); it != _delegates.end(); ++it ) {
			it->operator()( );
		}
	}

}


#endif
