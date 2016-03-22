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

#ifndef CVT_SOCKET_H
#define CVT_SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <cvt/com/Host.h>
#include <cvt/util/String.h>


namespace cvt
{
	class Socket 
	{
		public:
			enum SocketType {
				TCP_SOCKET = SOCK_STREAM,
				UDP_SOCKET = SOCK_DGRAM
			};

			void close();
			
			/* get the socket file descriptor (for select) */
			int socketDescriptor() const { return _sockfd; }

			size_t bytesAvailableForReading();

		protected:
			Socket( SocketType type, int fd = -1 );
			virtual ~Socket();
			/* bind this socket to a certain address */
			void bind( const String & addr, uint16_t port );

			/* create connection to another socket */
			void connect( const String & addr, uint16_t port );

			/* listen for incoming connection requests */
			void listen( int backlog = 10 );

			/* send data to connected peer */
			size_t send( const uint8_t* data, size_t len );

			/* reveive data from connected peer */
			size_t receive( uint8_t* data, size_t maxSize );

			/* send data to a specific host (UDP) */
			size_t sendTo( const Host & host, const uint8_t* data, size_t len );

			/* reveive data (UDP), host tells the peer that sent the data  */
			size_t receiveFrom( Host & host, uint8_t* data, size_t maxLen );

			static void fillAdressInfo( struct addrinfo & info, 
									    const String & address, 
									    const String & service, 
									    SocketType type );

			int				_sockfd;
			SocketType		_sockType;

			void setReusable();
	};
}

#endif
