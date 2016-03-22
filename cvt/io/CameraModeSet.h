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

#ifndef CVT_CAMERAMODESET
#define CVT_CAMERAMODESET

#include <vector>

#include <cvt/io/CameraMode.h>

namespace cvt {
	class CameraModeSet
	{
		public:
			CameraModeSet();
			CameraModeSet( const CameraModeSet& other );

			~CameraModeSet();
			CameraModeSet& operator=( const CameraModeSet& other );

			void add( const CameraMode & mode );
			CameraModeSet filterFormat( const IFormat & format ) const;
			CameraModeSet filterSize( size_t minWidth, size_t minHeight ) const;
			CameraModeSet filterFPS( size_t minFPS ) const;
			CameraModeSet filter( const IFormat & format, size_t minWidth, size_t minHeight, size_t minFPS  ) const;
			CameraMode bestMatching( const IFormat & format, size_t width = 0, size_t height = 0, size_t fps = 0  ) const;

			size_t size() const;
			const CameraMode & mode( size_t idx ) const;

		private:
			std::vector<CameraMode> _modes;
	};

	std::ostream& operator<<( std::ostream &out, const CameraModeSet & info );
}

#endif
