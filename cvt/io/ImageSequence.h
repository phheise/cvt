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

#ifndef CVT_IMAGESEQUENCE_H
#define CVT_IMAGESEQUENCE_H

#include <cvt/io/VideoInput.h>
#include <cvt/util/String.h>
#include <vector>

namespace cvt
{
    class ImageSequence : public VideoInput
    {
        public:
            ImageSequence( const String& basename,
                           const String& ext );
        
            ~ImageSequence(){}
        
            size_t  width() const { return _current.width(); }
            size_t  height() const { return _current.height(); }
            const   IFormat & format() const { return _current.format(); }
            const   Image & frame() const { return _current; }
            bool    nextFrame( size_t timeout = 0 );
			bool	hasNext() const;
            
        
        private:
            Image					_current;   
			std::vector<String>		_files;
            size_t					_index;
	
			bool extractFolder( String& folder, const String& basename ) const;
    };
}

#endif
