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

#ifndef CVT_GLFORMAT_H
#define CVT_GLFORMAT_H

#include <iostream>

namespace cvt {
	class GLXContext;

	/**
	  \ingroup GL
	*/
	class GLFormat {
		friend class GLXContext;

		public:
			GLFormat();

			void defaultValues();

			void setRedSize( unsigned int size );
			unsigned int redSize() const;
			void setGreenSize( unsigned int size );
			unsigned int greenSize() const;
			void setBlueSize( unsigned int size );
			unsigned int blueSize() const;
			void setAlphaSize( unsigned int size );
			unsigned int alphaSize() const;
			void setDepthSize( unsigned int size );
			unsigned int depthSize() const;
			void setStencilSize( unsigned int size );
			unsigned int stencilSize() const;
			void setDoubleBuffer( bool v );
			bool doubleBuffer() const;
			void setDirect( bool v );
			bool direct() const;
			void setSamples( unsigned int size );
			unsigned int samples() const;

		private:
			unsigned int _red, _green, _blue, _alpha, _depth, _stencil, _samples;
			bool _db, _direct;
	};

	inline GLFormat::GLFormat()
	{
		defaultValues();
	}

	inline void GLFormat::defaultValues()
	{
		_red = _green = _blue = _alpha = 8;
		_depth = 16;
		_stencil = 8;
		_samples = 2;
		_db = true;
		_direct = true;
	}

	inline unsigned int GLFormat::redSize() const
	{
		return _red;
	}

	inline unsigned int GLFormat::greenSize() const
	{
		return _green;
	}

	inline unsigned int GLFormat::blueSize() const
	{
		return _blue;
	}

	inline unsigned int GLFormat::alphaSize() const
	{
		return _alpha;
	}

	inline unsigned int GLFormat::depthSize() const
	{
		return _depth;
	}

	inline unsigned int GLFormat::stencilSize() const
	{
		return _red;
	}

	inline bool GLFormat::doubleBuffer() const
	{
		return _db;
	}

	inline bool GLFormat::direct() const
	{
		return _direct;
	}

	inline void GLFormat::setRedSize( unsigned int v )
	{
		_red = v;
	}

	inline void GLFormat::setGreenSize( unsigned int v )
	{
		_green = v;
	}

	inline void GLFormat::setBlueSize( unsigned int v )
	{
		_blue = v;
	}

	inline void GLFormat::setAlphaSize( unsigned int v )
	{
		_alpha = v;
	}

	inline void GLFormat::setDepthSize( unsigned int v )
	{
		_depth = v;
	}

	inline void GLFormat::setStencilSize( unsigned int v )
	{
		_stencil = v;
	}

	inline void GLFormat::setDoubleBuffer( bool v )
	{
		_db = v;
	}

	inline void GLFormat::setDirect( bool v )
	{
		_direct = v;
	}

	inline void GLFormat::setSamples( unsigned int size )
	{
		_samples = size;
	}

	inline unsigned int GLFormat::samples() const
	{
		return _samples;
	}

	inline std::ostream& operator<<( std::ostream& out, const GLFormat& glformat )
	{
		out << "GLFormat:\n\tRED-Size: " << glformat.redSize()
			<< "\n\tGREEN-Size: " << glformat.greenSize()
			<< "\n\tBLUE-Size: " << glformat.blueSize()
			<< "\n\tALPHA-Size: " << glformat.alphaSize()
			<< "\n\tDEPTH-Size: " << glformat.depthSize()
			<< "\n\tSTENCIL-Size: " << glformat.stencilSize()
			<< "\n\tDoubleBuffer: " << glformat.doubleBuffer()
			<< "\n\tDirect: " << glformat.direct();
		return out;
	}

}

#endif
