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

#ifndef CVT_ASYNC_TCP_SERVER_H
#define CVT_ASYNC_TCP_SERVER_H

#include <cvt/com/Socket.h>
#include <cvt/io/IOHandler.h>
#include <cvt/util/Signal.h>
#include <cvt/util/String.h>

namespace cvt
{
	class AsyncTCPServer : public IOHandler  
	{
		public:
			AsyncTCPServer( const String & address, uint16_t port, int maxConnections = 10 );

			/* the server is only accepting new connections */
			void onDataReadable();
			void onDataWriteable(){}
			void onException(){}

			Signal<void>	canAccept;

			TCPClient*  accept();

		private:
			TCPServer	_socket;
			int			_maxConnections;
	};

	inline AsyncTCPServer::AsyncTCPServer( const String & address, uint16_t port, int maxConnections ) : IOHandler( -1 )
		,_socket( address, port )
		,_maxConnections( maxConnections )
	{
		_fd = _socket.socketDescriptor();
		notifyReadable( true );
		_socket.listen( _maxConnections );	
	}

	inline void AsyncTCPServer::onDataReadable()
	{
		// notify observers, that server has pending Connection request!
		canAccept.notify();
	}

	inline TCPClient* AsyncTCPServer::accept()
	{
		return _socket.accept();
	}
}

#endif
