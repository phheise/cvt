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

#include <cvt/com/Host.h>
#include <arpa/inet.h>

namespace cvt
{
	Host::Host( const String & addr, const cvt::String & service, ProtocolFamily pf ) :
		_addressLen( 0 )
	{
		lookup( addr, service, pf );
	}

	Host::Host( const String & address, uint16_t port, ProtocolFamily protFam ) :
		_addressLen( 0 )
	{
		String service;
		service.sprintf( "%d", port );
		lookup( address, service, protFam );
	}

	Host::Host()
	{
		memset( &_sockAddress, 0, sizeof( _sockAddress ) );
	}
	
	Host::~Host()
	{
	}


	void Host::lookup( const String & address, const String & service, ProtocolFamily protocolFamily )
	{
		// get the addressinfo
		struct addrinfo hints;
		memset( &hints, 0, sizeof( addrinfo ) );
		hints.ai_family = protocolFamily;

		struct addrinfo * tmp;
		int ret = getaddrinfo( address.c_str(), service.c_str(), &hints, &tmp );
		if( ret != 0 ){
			String msg( "getaddrinfo: " );
			msg += gai_strerror( ret );
			throw CVTException( msg.c_str() );
		}
		_addressLen = tmp->ai_addrlen;
		memcpy( &_sockAddress, tmp->ai_addr, _addressLen );
		freeaddrinfo( tmp );
	}

	uint16_t Host::port() const 
	{
		uint16_t port;
		if( _sockAddress.ss_family == AF_INET ){
			struct sockaddr_in * addr = ( struct sockaddr_in* )&_sockAddress;
			port = ntohs( addr->sin_port );
		} else {
			struct sockaddr_in6 * addr = ( struct sockaddr_in6* )&_sockAddress;
			port = ntohs( addr->sin6_port );
		}

		return port;
	}


	String Host::addressAsString()
	{
		char  inetString[ INET6_ADDRSTRLEN ];

		if( _sockAddress.ss_family == AF_INET ){
			struct sockaddr_in* ia = ( struct sockaddr_in* )&_sockAddress;
			inet_ntop( AF_INET, &ia->sin_addr, inetString, INET_ADDRSTRLEN );

		} else { 
			struct sockaddr_in6* ia = ( struct sockaddr_in6* )&_sockAddress;
			inet_ntop( AF_INET, &ia->sin6_addr, inetString, INET6_ADDRSTRLEN );

		}

		return inetString;	
	}
}
