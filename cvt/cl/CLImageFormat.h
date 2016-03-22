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

#ifndef CVT_CLIMAGEFORMAT_H
#define CVT_CLIMAGEFORMAT_H

#include <cvt/cl/OpenCL.h>
#include <iostream>

namespace cvt {
	/**
	  \ingroup CL
	 */
	class CLImageFormat
	{
		friend std::ostream& operator<<( std::ostream& out, const CLImageFormat& format );
		public:
			CLImageFormat( cl_image_format format );
			CLImageFormat( cl_channel_order order = CL_BGRA, cl_channel_type type = CL_UNORM_INT8 );
			CLImageFormat( const CLImageFormat& format );

			operator cl_image_format () const { return _format; }

			static const char* orderToString( cl_channel_order order );
			static const char* typeToString( cl_channel_type type  );
		private:
			cl_image_format _format;
	};


	inline CLImageFormat::CLImageFormat( cl_image_format format ) : _format( format )
	{
	}

	inline CLImageFormat::CLImageFormat( cl_channel_order order, cl_channel_type type )
	{
		 _format.image_channel_order = order;
		 _format.image_channel_data_type = type;
	}

	inline CLImageFormat::CLImageFormat( const CLImageFormat& format ) : _format( format._format )
	{
	}

	inline std::ostream& operator<<( std::ostream& out, const CLImageFormat& format )
	{
		out << "CLImageFormat:" << CLImageFormat::orderToString( format._format.image_channel_order )
			<< " " << CLImageFormat::typeToString( format._format.image_channel_data_type );
		return out;
	}
}


#endif
