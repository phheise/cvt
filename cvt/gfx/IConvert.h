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

#ifndef CVT_ICONVERT_H
#define CVT_ICONVERT_H

#include <cvt/gfx/IFormat.h>
#include <cvt/util/Flags.h>
#include <cvt/util/Exception.h>

namespace cvt
{
	class Image;

	enum IConvertFlagTypes {
		ICONVERT_DEBAYER_LINEAR = ( 1 << 0 ),
		ICONVERT_DEBAYER_HQLINEAR = ( 1 << 1 )
		/*
		 TODO: gamma treatment

		 ICONVERT_COLOR_SRGB_TO_LINEAR = ( 1 << 2 ),
		 ICONVERT_COLOR_LINEAR_TO_LINEAR = ( 1 << 3 ),
		 ICONVERT_COLOR_LINEAR_TO_SRGB = ( 1 << 4 ),
		 */
	};

	CVT_ENUM_TO_FLAGS( IConvertFlagTypes, IConvertFlags )

	typedef void (*ConversionFunction)( Image&, const Image&, IConvertFlags flags );

	class IConvert
	{
		public:
			/* conversion from source format to dst format */
			static void convert( Image& dst, const Image& src, IConvertFlags flags = ICONVERT_DEBAYER_LINEAR );

			static const IConvert & instance();

		private:
			IConvert();
			IConvert( const IConvert& ) {}

			static IConvert * _instance;
			ConversionFunction * _convertFuncs;

			void initTable();
	};
}

#endif
