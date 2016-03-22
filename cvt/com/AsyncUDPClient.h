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

#ifndef CVT_ASYNC_UDPCLIENT_H
#define CVT_ASYNC_UDPCLIENT_H

#include <cvt/com/UDPClient.h>
#include <cvt/util/Signal.h>
#include <cvt/io/IOHandler.h>

namespace cvt
{
	class AsyncUDPClient : public IOHandler
	{
		public:
			AsyncUDPClient();
			AsyncUDPClient( const String & address, uint16_t port );
			~AsyncUDPClient();

			void onDataReadable();
			void onDataWriteable();
			void onException(){}

			/* only notify observers when at leas s bytes have been read */
			size_t sendTo( const Host& receiver, const uint8_t* buf, size_t maxLen );
			size_t receiveFrom( Host& sender, uint8_t* buf, size_t bufSize );
			
			Signal<void>	canSend;
			Signal<void>	canReceive;

		private:
			UDPClient*		_socket;
	};

	inline AsyncUDPClient::AsyncUDPClient() : IOHandler( -1 )
		,_socket( new UDPClient() )
	{
		_fd = _socket->socketDescriptor();
		notifyReadable( true );
		notifyWriteable( true );
	}

	inline AsyncUDPClient::AsyncUDPClient( const String & address, uint16_t port ) : IOHandler( -1 )
		,_socket( new UDPClient( address, port ) )
	{
		_fd = _socket->socketDescriptor();
		notifyReadable( true );
		notifyWriteable( true );
	}

	inline AsyncUDPClient::~AsyncUDPClient()
	{
		if( _socket )
			delete _socket;
	}


	inline void AsyncUDPClient::onDataReadable()
	{
		canReceive.notify();
	}	

	inline void AsyncUDPClient::onDataWriteable()
	{
		canSend.notify();
	}

	inline size_t AsyncUDPClient::sendTo( const Host & h, const uint8_t* buf, size_t maxLen )
	{
		return _socket->sendTo( h, buf, maxLen );		
	}

	inline size_t AsyncUDPClient::receiveFrom( Host & h, uint8_t* buf, size_t bufSize )
	{
		return _socket->receiveFrom( h, buf, bufSize );		
	}
}

#endif
