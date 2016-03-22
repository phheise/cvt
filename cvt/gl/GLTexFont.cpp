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

#include <cvt/gl/GLTexFont.h>
#include <cvt/util/Exception.h>
#include <stdio.h>

namespace cvt {
	GLTexFont::GLTexFont()
	{
	}

	void GLTexFont::load( const char* path )
	{
		FILE* f;
		f = fopen( path, "r" );
		if( !f )
			throw CVTException( "Unable to open GL-font!" );

		if( fread( &_width, sizeof( int32_t ), 1, f ) != 1 )
			throw CVTException( "Unable to read GL-font!" );
		if( fread( &_height, sizeof( int32_t ), 1, f ) != 1 )
			throw CVTException( "Unable to read GL-font!" );
		if( fread( &_fontsize, sizeof( int32_t ), 1, f ) != 1 )
			throw CVTException( "Unable to read GL-font!" );
		if( fread( &_ascent, sizeof( int32_t ), 1, f ) != 1 )
			throw CVTException( "Unable to read GL-font!" );
		if( fread( &_descent, sizeof( int32_t ), 1, f ) != 1 )
			throw CVTException( "Unable to read GL-font!" );
		if( fread( &_spritesize, sizeof( int32_t ), 1, f ) != 1 )
			throw CVTException( "Unable to read GL-font!" );
		if( fread( &_offx, sizeof( int32_t ), 1, f ) != 1 )
			throw CVTException( "Unable to read GL-font!" );
		if( fread( &_offy, sizeof( int32_t ), 1, f ) != 1 )
			throw CVTException( "Unable to read GL-font!" );

		for( int i = 0; i <= 0xff; i++ ) {
			if( fread( _metric + i, sizeof( int32_t ), 5, f ) != 5 )
				throw CVTException( "Unable to read GL-font!" );
		}

		uint8_t* image = new uint8_t[ _width * _height ];
		if( fread( image, sizeof( uint8_t ), _width * _height, f ) != ( size_t ) _width * _height ) {
			delete[] image;
			throw CVTException( "Unable to read GL-font!" );
		}
		_tex.alloc( GL_RED, _width, _height, GL_RED, GL_UNSIGNED_BYTE, image );
		delete[] image;

		fclose( f );
	}
}
