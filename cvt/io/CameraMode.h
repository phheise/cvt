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

#ifndef CVT_CAMERAMODE
#define CVT_CAMERAMODE

#include <cvt/gfx/IFormat.h>
#include <cvt/util/String.h>

namespace cvt {
	class CameraMode {
		public:
			CameraMode( size_t w = 0, size_t h = 0,
						size_t f = 0, const IFormat & pixFormat = IFormat::RGBA_UINT8 ) :
				width( w ), height( h ), fps( f ), format( pixFormat ), description( "" )
			{}

			CameraMode( const CameraMode & other ) :
				width( other.width ), height( other.height ),
				fps( other.fps ), format( other.format ), description( other.description )
			{
			}

			CameraMode & operator=( const CameraMode & other )
			{
				width = other.width;
				height = other.height;
				fps = other.fps;
				format = other.format;
				return *this;
			}

			bool operator==( const CameraMode& other ) const
			{
				if( width == other.width &&
					height == other.height &&
					fps == other.fps &&
					format == other.format &&
					description == other.description )
					return true;
				return false;
			}

		public:
			size_t  width;
			size_t  height;
			size_t  fps;
			IFormat format;
			String  description;
	};

	inline std::ostream& operator<<( std::ostream &out, const CameraMode &m )
	{
		out << m.format << " (" << m.width << " x " << m.height << ") @ " << m.fps << " fps " << m.description ;
		return out;
	}

}

#endif
