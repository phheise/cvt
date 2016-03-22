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

#ifndef EXCEPTION_H
#define EXCEPTION_H
#include <exception>
#include <sstream>
#include <iostream>
#include <cstring>
#include <string>

namespace cvt {

#define CVTException( s ) ( ::cvt::Exception( s, __FILE__, __LINE__, __PRETTY_FUNCTION__ ) )

	class Exception : std::exception
	{
		public:
			Exception() throw(): msg( "Unknown" )
            {
#ifdef DEBUG
                    std::cerr << "EXCEPTION: " << msg << std::endl;
#endif
            }
        
			Exception( const char* s ) throw() : msg( s ) {}
			~Exception() throw() {}
			const char* what() const throw() { return msg.c_str(); }

			Exception( int err, const char* file, size_t line, const char* func ) throw()
			{
				std::stringstream str;
				str << strerror( err );
				str << " ( FILE: ";
				str << file;
				str << " LINE: ";
				str << line;
				str << " FUNC: ";
				str << func;
				str << " )";
				msg = str.str();
                
#ifdef DEBUG
                std::cerr << "EXCEPTION: " << msg << std::endl;
#endif
			}


			Exception( const std::string & s, const char* file, size_t line, const char* func ) throw()
			{
				std::stringstream str;
				str << s;
				str << " ( FILE: ";
				str << file;
				str << " LINE: ";
				str << line;
				str << " FUNC: ";
				str << func;
				str << " )";
				msg = str.str();
#ifdef DEBUG
                std::cerr << "EXCEPTION: " << msg << std::endl;
#endif
			}

		protected:
			std::string msg;
	};

}

#endif
