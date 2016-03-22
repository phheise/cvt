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

#ifndef CVT_ASYNC_TCP_CLIENT_H
#define CVT_ASYNC_TCP_CLIENT_H

#include <cvt/com/TCPServer.h>
#include <cvt/com/TCPClient.h>

#include <cvt/util/Signal.h>
#include <cvt/io/IOHandler.h>

namespace cvt
{
	class AsyncTCPClient : public IOHandler
	{
		public:
			AsyncTCPClient( TCPClient* socket );
			AsyncTCPClient( const String & address, uint16_t port );
			~AsyncTCPClient();

			void onDataReadable();
			void onDataWriteable();
			void onException(){}

			size_t send( const uint8_t* buf, size_t maxLen );
			size_t receive( uint8_t* buf, size_t maxLen );
			
			Signal<void>	canSend;
			Signal<void>	canReceive;

		private:
			TCPClient*		_socket;
	};

	inline AsyncTCPClient::AsyncTCPClient( TCPClient* socket ) : IOHandler( socket->socketDescriptor() )
		,_socket( socket )
	{
		notifyReadable( true );
		notifyWriteable( true );
	}

	inline AsyncTCPClient::AsyncTCPClient( const String & address, uint16_t port ) : IOHandler( -1 )
		,_socket( new TCPClient() )
	{
		_socket->connect( address, port );
		_fd = _socket->socketDescriptor();
		notifyReadable( true );
		notifyWriteable( true );
	}

	inline AsyncTCPClient::~AsyncTCPClient()
	{
		if( _socket )
			delete _socket;
	}


	inline void AsyncTCPClient::onDataReadable()
	{
		canReceive.notify();
	}

	inline void AsyncTCPClient::onDataWriteable()
	{
		canSend.notify();
	}


	inline size_t AsyncTCPClient::send( const uint8_t* buf, size_t maxLen )
	{
		return _socket->send( buf, maxLen );
	}

	inline size_t AsyncTCPClient::receive( uint8_t* buf, size_t bufSize )
	{
		return _socket->receive( buf, bufSize );
	}
}

#endif
