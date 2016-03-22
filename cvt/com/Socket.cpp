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

#include <cvt/com/Socket.h>

#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace cvt
{
	Socket::Socket( SocketType type, int fd ) : 
		_sockfd( fd )
		,_sockType( type )
	{
	}
	
	Socket::~Socket()
	{
		close();
	}

	void Socket::bind( const String & address, uint16_t port )
	{
		struct addrinfo info;
		String service;
		
		service.sprintf( "%d", port );
		Socket::fillAdressInfo( info, address, service, _sockType );

		// create the socket from the filled information
		_sockfd = socket( info.ai_family, info.ai_socktype, info.ai_protocol );
		setReusable();

		if( _sockfd == -1 ){
			throw CVTException( "Could not create Socket!" );
		}

		// bind it to the requested address
		if( ::bind( _sockfd, info.ai_addr, info.ai_addrlen ) != 0 ){
			close();
			String msg( "Bind: " );
			msg += strerror( errno );
			throw CVTException( msg.c_str() );
		}
	}

	void Socket::listen( int backlog )
	{
		if( ::listen( _sockfd, backlog ) != 0 ){
			close();
			String msg( "Listen: " );
			msg += strerror( errno );
			throw CVTException( msg.c_str() );
		}
	}

	void Socket::setReusable()
	{
		int yes = 1;
		if( setsockopt( _sockfd, 
					    SOL_SOCKET,
						SO_REUSEADDR,
						&yes,
						sizeof( int ) ) == -1 ){
			String msg( "setsockopt:" );
			msg += strerror( errno );
			throw CVTException( msg.c_str() );	
		} 
	}


	void Socket::connect( const String & addr, uint16_t port )
	{
		struct addrinfo info;
		String service;
		service.sprintf( "%d", port );
		Socket::fillAdressInfo( info, addr, service, _sockType );

		// create the socket from the filled information
		if( _sockfd == -1 ){
			_sockfd = socket( info.ai_family, info.ai_socktype, info.ai_protocol );
			setReusable();
		}

		if( _sockfd == -1 ){
			throw CVTException( "Could not create Socket!" );
		}

		// connect
		if( ::connect( _sockfd, info.ai_addr, info.ai_addrlen ) == -1 ) {
			close();
			String error( "Connect: " );
			error += strerror( errno );
			throw CVTException( error.c_str() );
		}
	}

	size_t Socket::send( const uint8_t* data, size_t len )
	{
		ssize_t numSend = ::send( _sockfd, data, len, 0 );

		if( numSend == -1 ){
			String msg( "Send: " );
			msg += strerror( errno );
			throw CVTException( msg.c_str() );
		}

		return ( size_t )numSend;
	}

	size_t Socket::sendTo( const Host & host, const uint8_t* data, size_t len )
	{
		ssize_t numSend = ::sendto( _sockfd, data, len, 0, 
								    ( struct sockaddr* )&host._sockAddress, 
									host._addressLen );

		if( numSend == -1 ){
			String msg( "SendTo: " );
			msg += strerror( errno );
			throw CVTException( msg.c_str() );
		}

		return ( size_t )numSend;
	}

	size_t Socket::receiveFrom( Host & host, uint8_t* data, size_t maxLen )
	{
		ssize_t numRecveived = recvfrom( _sockfd, data, maxLen, 0,
									     ( struct sockaddr* )&host._sockAddress, 
										 &host._addressLen );

		if( numRecveived == -1 ){
			String msg( "ReveiveFrom: " );
			msg += strerror( errno );
			throw CVTException( msg.c_str() );
		}

		return ( size_t )numRecveived;
	}

	size_t Socket::receive( uint8_t* data, size_t maxSize )
	{
		ssize_t numRecvd = ::recv( _sockfd, data, maxSize, 0 );

		if( numRecvd == 0 ){
			throw CVTException( "Connection closed by peer" );
		}
		if( numRecvd < 0 ){
			String msg( "Receive:" );
			msg += strerror( errno );
			throw CVTException( msg.c_str() );
		}

		return ( size_t )numRecvd;
	}

	void Socket::close()
	{
		if( _sockfd != -1 )
			::close( _sockfd );
	}

	size_t	Socket::bytesAvailableForReading()
	{
		if( _sockfd == -1 )
			return 0;

		int n;
		int ret = ioctl( _sockfd, FIONREAD, &n );

		if( ret == -1 )
			return 0;
		return n;
	}

	void Socket::fillAdressInfo( struct addrinfo & info, 
								 const String & address, 
								 const String & service, 
								 SocketType type )
	{	
		struct addrinfo hints;
		memset( &hints, 0, sizeof( hints ) );
		hints.ai_family		= IPV_UNSPEC;
		hints.ai_socktype	= type;
		hints.ai_flags		= AI_PASSIVE;	// fill in IP for me

		struct addrinfo* results;
		int status;
		const char * addr = NULL;

		if( address != "" ){
			addr = address.c_str();
		}

		if ( ( status = getaddrinfo( addr, service.c_str(), &hints, &results ) ) != 0 ){
			String msg;
			msg.sprintf( "Error in getaddrinfo: %s", gai_strerror( status ) );
			throw CVTException( msg.c_str() );
		}


		memcpy( &info, results, sizeof( struct addrinfo ) );
		info.ai_next = NULL;

		// free the linked list results
		freeaddrinfo( results );
	}
}
