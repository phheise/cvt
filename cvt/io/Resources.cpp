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

#include "Resources.h"
#include "FileSystem.h"

#include <cvt/util/Exception.h>

#include <sstream>

namespace cvt {
	Resources::Resources()
	{
		searchFolders.push_back( CVT_DATA_FOLDER );
		searchFolders.push_back( "." );
	}

	Resources::~Resources()
	{
	}

	String Resources::find( const String& resource )
	{
		std::list<String>::iterator it  = searchFolders.begin();
		std::list<String>::iterator end = searchFolders.end();

        String rappend( "/" );
        rappend += resource;

		while (it != end) {
            String s( *it++ );
            s += rappend;

			if( FileSystem::exists( s ) ){
				return s;
			}
		}

		String s( "Resource not found: " );
        s += resource;
		throw CVTException( s.c_str() );
	}

	void Resources::addSearchLocation( const String& loc, bool prepend )
	{
		if( prepend )
			searchFolders.push_front( loc );
		else
			searchFolders.push_back( loc );
	}

}
