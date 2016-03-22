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

#ifndef CVT_GLPROGTEXMLS_H
#define CVT_GLPROGTEXMLS_H

#include <cvt/gl/OpenGL.h>
#include <cvt/gl/GLProgram.h>
#include <cvt/gl/GLVertexArray.h>
#include <cvt/gl/GLBuffer.h>
#include <cvt/gl/GLTexture.h>

namespace cvt {
	/**
	  \class GLTexMLSProg GLTexMLSProg.h
	  \brief OpenGL program for Moving Least Squares warping.
	*/
	class GLTexMLSProg : private GLProgram
	{
		public:
			GLTexMLSProg();
			~GLTexMLSProg();

			using GLProgram::bind;
			using GLProgram::unbind;
			void  setProjection( const Matrix4f& projection );
			void  setAlpha( float alpha );
			void  setGridSize( size_t gridwidth, size_t gridheight );
			void  setDisplacements( const Vector4f* disp, size_t size );
			void  drawImage( int x, int y, int width, int height, const Image& img );

		private:
			bool _glsl120;
			GLBuffer _vbo;
			GLVertexArray _vao;
			GLTexture _dptex;
			size_t _gridsize;
			Matrix4f _proj;
			GLint _mvploc;
			GLint _texloc;
			GLint _dptexloc;
			GLint _dptexsizeloc;
	};


}


#endif
