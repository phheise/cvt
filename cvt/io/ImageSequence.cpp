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

#include <cvt/io/ImageSequence.h>
#include <cvt/io/FileSystem.h>

namespace cvt {
    
    ImageSequence::ImageSequence( const String& basename,
                                  const String& ext ) :
	   _index( 0 )	
    {       
		std::vector<String> filenames;

		// get the folder:
		String folder;
		if( !extractFolder( folder, basename ) ){
			throw CVTException( "Could not extract folder from basename" );
		}

		FileSystem::filesWithExtension( folder, filenames, ext );
		for( size_t i = 0; i < filenames.size(); i++ ){
			if( filenames[ i ].hasPrefix( basename ) ){
				_files.push_back( filenames[ i ] );
			}
		}
		
		nextFrame();
    }
    
    bool ImageSequence::nextFrame( size_t )
    {
        // build the string and load the frame
		if( _index < _files.size() ){
			_current.load( _files[ _index ] );
			_index++;
			return true;
		} else {
			return false;
		}
    }

	bool ImageSequence::extractFolder( String& folder, const String& basename ) const
	{
		size_t pos = basename.length();
		
		while( pos-- ){
			if( basename[ pos ] == '/' ){
				break;
			}
		}

		if( pos == 0 )
			return false;

		folder = basename.substring( 0, pos+1 );
		return true;	
	}


	bool ImageSequence::hasNext() const
	{
		if( _index < ( _files.size() - 1 ) )
			return true;
		return false;
	}
    
}
