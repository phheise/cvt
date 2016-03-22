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

#include "FloFile.h"
#include "util/Exception.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define TAG_FLOAT 202021.25  // check for this when READING the file
#define TAG_STRING "PIEH"    // use this when WRITING the file
#define UNKNOWN_FLOW_THRESH 1e9
#define UNKNOWN_FLOW 1e10 // value to use to represent unknown flow

// ".flo" file format used for optical flow evaluation
//
// Stores 2-band float image for horizontal (u) and vertical (v) flow components.
// Floats are stored in little-endian order.
// A flow value is considered "unknown" if either |u| or |v| is greater than 1e9.
//
//  bytes  contents
//
//  0-3     tag: "PIEH" in ASCII, which in little endian happens to be the float 202021.25
//          (just a sanity check that floats are represented correctly)
//  4-7     width as an integer
//  8-11    height as an integer
//  12-end  data (width*height*2*4 bytes total)
//          the float values for u and v, interleaved, in row order, i.e.,
//          u[row0,col0], v[row0,col0], u[row0,col1], v[row0,col1], ...
//


namespace cvt {
	namespace FloFile {

		static bool _unknown_flow( float u, float v ) {
#define UFLOW 1e9f
			return ( Math::abs(u) > UFLOW )	|| ( Math::abs(v) > UFLOW ) || std::isnan(u) || std::isnan(v);
#undef UFLOW
		}

		void FloReadFile( Image& flow, std::string const& filename, bool zerounknown )
		{

			if( filename.compare( filename.length() -4, 4, ".flo" ) != 0 )
				throw CVTException("FloReadFile " + filename + ": extension .flo expected");

			FILE *stream = fopen( filename.c_str(), "rb" );
			if (stream == 0)
				throw CVTException("FloReadFile: could not open " + filename );

			int32_t width, height;
			float tag;

			if ( ( int ) fread(&tag,    sizeof( float ), 1, stream) != 1 ||
				( int ) fread(&width,  sizeof( int32_t ),   1, stream) != 1 ||
				( int ) fread(&height, sizeof( int32_t ),   1, stream) != 1)
				throw CVTException("FloReadFile: problem reading file " + filename);

			if (tag != TAG_FLOAT) // simple test for correct endian-ness
				throw CVTException("FloReadFile(" + filename + "): wrong tag (possibly due to big-endian machine?)" );

			// another sanity check to see that integers were read correctly (99999 should do the trick...)
			if (width < 1 || width > 99999)
				throw CVTException("ReadFlowFile(" + filename + "): illegal width" );

			if (height < 1 || height > 99999)
				throw CVTException("ReadFlowFile(" + filename + "): illegal height" );


			flow.reallocate( width, height, IFormat::GRAYALPHA_FLOAT );
			size_t h = flow.height();
			size_t w2 = flow.width() * 2;
			size_t stride;
			uint8_t* ptr = flow.map( &stride );
			uint8_t* optr = ptr;
			while( h-- ) {
				if( fread( ptr, sizeof( float ), w2 , stream) != w2)
					throw CVTException("ReadFlowFile(" + filename + "): file is too short" );
				ptr += stride;
			}

			flow.unmap( optr );

			if( fgetc(stream) != EOF)
				throw CVTException("ReadFlowFile(" + filename + "): file is too long" );

			if( zerounknown ) {
				size_t w = flow.width();
				uint8_t* optr;
				h = flow.height();
				optr = ptr = flow.map( &stride );
				while( h-- ) {
					float* pptr = ( float* ) ptr;
					w2 = w;
					while( w2-- ) {
						if( _unknown_flow( *pptr, *( pptr + 1 ) ) ) {
							*pptr = 0.0f;
							*( pptr + 1 ) = 0.0f;
						}
						pptr += 2;
					}
					ptr += stride;
				}
				flow.unmap( optr );
			}

			fclose(stream);
		}


		void FloWriteFile( const Image& flow, std::string const & filename )
		{

			if( filename.compare( filename.length() -4, 4, ".flo" ) != 0 )
				throw CVTException("FloWriteFile " + filename + ": extension .flo expected");

			if( flow.format() != IFormat::GRAYALPHA_FLOAT )
				throw CVTException("FloWriteFile " + filename + ": illeagal image format");

			FILE *stream = fopen(filename.c_str(), "wb");
			if (stream == 0)
				throw CVTException("FloWriteFile (" + filename + "): could not open file");

			int32_t width, height;
			width = ( int32_t ) flow.width();
			height = ( int32_t ) flow.height();

			// write the header
			fprintf(stream, TAG_STRING);
			if ( fwrite( &width,  sizeof( int32_t ), 1, stream) != 1 ||
				fwrite( &height, sizeof( int32_t ), 1, stream) != 1)
				throw CVTException( "FloWriteFile (" + filename + "): problem writing header" );

			size_t h = flow.height();
			size_t w2 = flow.width() * 2;
			size_t stride;
			const uint8_t* ptr = flow.map( &stride );
			const uint8_t* optr = ptr;
			while( h-- ) {
				if( fwrite( ptr, sizeof( float ), w2 , stream ) != w2)
					throw CVTException("ReadFlowFile(" + filename + "): problem writing data" );
				ptr += stride;
			}
			flow.unmap( optr );
			fclose(stream);
		}

	}
}
