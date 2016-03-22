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

#ifndef CVT_HOST_H
#define CVT_HOST_H

#include <cvt/util/String.h>
#include <netdb.h>

namespace cvt
{
	enum ProtocolFamily
	{
		IPV4 = PF_INET,
		IPV6 = PF_INET6,
		IPV_UNSPEC = PF_UNSPEC
	};

	class Host
	{
		public:
			friend class Socket;
			Host( const String & address, const String & service, ProtocolFamily protFam = IPV_UNSPEC );
			Host( const String & address, uint16_t port, ProtocolFamily protFam = IPV_UNSPEC );
			Host();
			~Host();

			void lookup( const String & address, const String & service, ProtocolFamily protocolFamily );
			uint16_t port() const;

			String addressAsString();

		private:
			struct sockaddr_storage	_sockAddress;
			socklen_t				_addressLen;

	};
}

#endif
