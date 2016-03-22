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

#ifndef CVT_RAWVIDEOWRITER_H
#define CVT_RAWVIDEOWRITER_H

#include <cvt/util/String.h>
#include <cvt/gfx/Image.h>

namespace cvt
{
	class RawVideoWriter
	{
		public:
			RawVideoWriter( const String & outname );
			~RawVideoWriter();

			void write( const Image & img );

		private:
			// file descriptor
			int		_fd;
			off_t	_currSize;
			off_t	_maxSize;

			// the total offset into the file
			off_t	_offsetInFile;

			/* current position in mapped region */
			void	* _map;
			size_t	  _mappedSize;
			uint8_t * _pos;
			size_t	  _pageSize;

			size_t	_width;
			size_t	_height;
			size_t	_stride;
			size_t	_formatID;
			size_t	_imgSize;

			/* change the filesize and update the mapping */
			void remapFile( size_t additionalBytes = 0 );

			/* write the header to the file */
			void writeHeader();

			/* resize to maxsize */
			void resizeFile();

	};
}

#endif
