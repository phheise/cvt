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

#include <iostream>
#include <cstring>
#include <exception>
#include <cvt/util/Exception.h>
#include <cvt/util/cvttestsproto.h>

static bool cvttest_run( CVTTestFunc func, const char* name )
{
    bool ret;

    std::cout <<"Running " << name << std::endl;
    std::cout << "-------------------------------------------------------------------------------" << std::endl;
    ret = func( );
    std::cout << "-------------------------------------------------------------------------------" << std::endl;
    std::cout << name << " returned: " << (ret?"TRUE":"FALSE") << std::endl;
    return ret;
}

int main( int argc, char** argv )
{
    size_t i = 0;
    size_t nsuccess = 0;

	if( argc == 1 ||  ( argc == 2 && strcmp( argv[1], "all" ) == 0 ) ) {
		while( _tests[i].func != NULL ) {
			try {
				if( cvttest_run( _tests[i].func, _tests[i].name ) )
					nsuccess++;
			} catch( cvt::Exception e ) {
				std::cerr << "Exception:" << std::endl;
				std::cerr << e.what() << std::endl;
			}

			i++;
		}
		std::cout << nsuccess << " PASSED and " << i - nsuccess << " FAILED of " << i << " TESTS" << std::endl;

	} else if( argc == 2 && strcmp( argv[1], "list" ) == 0 ) {
		while( _tests[i].func != NULL ) {
			std::cout << _tests[ i ].name << std::endl;
			i++;
		}
	} else {
        if( argc == 2 ) {
            size_t len;
            len = strlen( argv[ 1 ] );
            while( _tests[i].func != NULL ) {
                if( strncmp( _tests[i].name, argv[1], len ) == 0 ) {
					try {
						if( cvttest_run( _tests[i].func, _tests[i].name ) )
							nsuccess++;
					} catch( cvt::Exception e ) {
						std::cerr << "Exception:" << std::endl;
						std::cerr << e.what() << std::endl;
					}
                }
                i++;
            }
        }
    }

    return 0;
}

