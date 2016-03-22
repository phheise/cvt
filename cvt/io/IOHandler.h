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

#ifndef CVT_IOHANDLER_H
#define CVT_IOHANDLER_H

#include <cvt/io/IOSelect.h>

namespace cvt {

	class IOHandler {
		friend class IOSelect;

		public:
			IOHandler( int fd = -1 );
			virtual ~IOHandler();
			virtual void onDataReadable();
			virtual void onDataWriteable();
			virtual void onException();
			void notifyReadable( bool b );
			void notifyWriteable( bool b );
			void notifyException( bool b );

		private:
			IOHandler( const IOHandler& );

			bool _read;
			bool _write;
			bool _except;
		protected:
			int _fd;
	};

	inline IOHandler::IOHandler( int fd ) : _read( false ), _write( false ), _except( false ), _fd( fd )
	{
	}

	inline IOHandler::~IOHandler()
	{
	}

	inline void IOHandler::notifyReadable( bool b )
	{
		if( _fd >= 0 )
			_read = b;
	}

	inline void IOHandler::notifyWriteable( bool b )
	{
		if( _fd >= 0 )
			_write = b;
	}

	inline void IOHandler::notifyException( bool b )
	{
		if( _fd >= 0 )
			_except = b;
	}

	inline void IOHandler::onDataReadable()
	{
		notifyReadable( false );
	}

	inline void IOHandler::onDataWriteable()
	{
		notifyWriteable( false );
	}

	inline void IOHandler::onException()
	{
		notifyException( false );
	}


}

#endif
