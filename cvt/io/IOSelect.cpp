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

#include <cvt/io/IOSelect.h>
#include <cvt/io/IOHandler.h>
#include <cvt/math/Math.h>


namespace cvt {

	IOSelect::IOSelect()
	{
	}

	IOSelect::~IOSelect()
	{

	}

	int IOSelect::handleIO( ssize_t ms )
	{
		int maxfd = 0;
		int numfd, ret, active;

		if( _handlers.empty() )
			return 0;


		FD_ZERO( &_readfds );
		FD_ZERO( &_writefds );
		FD_ZERO( &_execeptfds );

		for( std::list<IOHandler*>::iterator it = _handlers.begin(), end = _handlers.end(); it != end; ++it ) {
			IOHandler* ioh = *it;
			if( ioh->_read || ioh->_write || ioh->_except ) {
				maxfd = Math::max( maxfd, ioh->_fd );
				if( ioh->_read )
					FD_SET( ioh->_fd, &_readfds );
				if( ioh->_write )
					FD_SET( ioh->_fd, &_writefds );
				if( ioh->_except )
					FD_SET( ioh->_fd, &_execeptfds );
			}
		}

		if( ms < 0 ) {
			ret = pselect( maxfd + 1, &_readfds, &_writefds, &_execeptfds, NULL, NULL );
		} else {
			msToTimespec( ms, _timeout );
			ret = pselect( maxfd + 1, &_readfds, &_writefds, &_execeptfds, &_timeout, NULL );
		}
		/* FIXME: do error handling */
		if( ret <= 0 )
			return ret;

		numfd = ret;

		for( std::list<IOHandler*>::iterator it = _handlers.begin(), end = _handlers.end(); it != end; ++it ) {
			IOHandler* ioh = *it;
			if( ioh->_read || ioh->_write || ioh->_except ) {
				active = 0;
				if( ioh->_read && FD_ISSET( ioh->_fd, &_readfds ) ) {
					ioh->onDataReadable();
					active = 1;
				}
				if( ioh->_write && FD_ISSET( ioh->_fd, &_writefds ) ) {
					ioh->onDataWriteable();
					active = 1;
				}
				if( ioh->_except && FD_ISSET( ioh->_fd, &_execeptfds ) ) {
					ioh->onException();
					active = 1;
				}
				numfd -= active;
				if( !numfd )
					break;
			}
		}

		return ret;
	}
}
