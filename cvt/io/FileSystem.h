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

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <cvt/util/String.h>
#include <vector>
#include <cvt/util/Data.h>

namespace cvt {

	class FileSystem
	{
		public:
			static bool   exists( const String & path );
			static void   rename( const String & from, const String & to );
			static void   touch( const String& file );
			static bool   isFile( const String & file );
			static bool   isDirectory( const String & dir );
			static void   mkdir( const String & name );
			static void   ls( const String & path, std::vector<String> & entries );
			static void   filesWithExtension( const String & path, std::vector<String>& result, const String & ext = "" );
			static size_t size( const String& path );
			static bool   load( Data& d, const String& path, bool zerotermination = false );
			static bool   save( const String& path, const Data& d );
	};

}

#endif

