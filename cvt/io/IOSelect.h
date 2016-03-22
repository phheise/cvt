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

#ifndef CVT_IOSELECT_H
#define CVT_IOSELECT_H

#include <stdlib.h>
#include <sys/select.h>
#include <time.h>

#include <list>

namespace cvt {
	class IOHandler;

	class IOSelect {
		public:
			IOSelect();
			~IOSelect();
			int handleIO( ssize_t timeout_ms );
			void registerIOHandler( IOHandler* ioh );
			void unregisterIOHandler( IOHandler* ion );

		private:
			void msToTimespec( size_t ms, struct timespec& ts ) const;

			fd_set _readfds;
			fd_set _writefds;
			fd_set _execeptfds;
			struct timespec _timeout;
			std::list<IOHandler*> _handlers;
	};


	inline void IOSelect::registerIOHandler( IOHandler* ioh )
	{
		_handlers.push_back( ioh );
	}

	inline void IOSelect::unregisterIOHandler( IOHandler* ion )
	{
		_handlers.remove( ion );
	}


	inline void IOSelect::msToTimespec( size_t ms, struct timespec& ts ) const
	{
		long ns;
		ldiv_t res;
		ns = ms * 1000000L;
		res = ldiv( ns, 1000000000L );
		ts.tv_sec = res.quot;
		ts.tv_nsec = res.rem;
	}

}

#endif
