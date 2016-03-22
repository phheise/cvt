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

#ifndef CVTGLVERTEXARRAY_H
#define CVTGLVERTEXARRAY_H

#include <cvt/gl/OpenGL.h>
#include <cvt/gl/GLBuffer.h>
#include <cvt/gl/GLProgram.h>
#include <cvt/gfx/Color.h>
#include <cvt/util/CVTAssert.h>

namespace cvt {

	/**
	  \ingroup GL
	*/
	class GLVertexArray {
		public:
			GLVertexArray();
			~GLVertexArray();

			void setVertexData( const GLBuffer& buffer, GLint size, GLenum type, GLsizei stride = 0, const GLvoid* offset = 0 );
			void setColorData( const GLBuffer& buffer, GLint size, GLenum type, GLsizei stride = 0, const GLvoid* offset = 0 );
			void setNormalData( const GLBuffer& buffer, GLint size, GLenum type, GLsizei stride = 0, const GLvoid* offset = 0 );
			void setTexCoordData( const GLBuffer& buffer, GLint size, GLenum type, GLsizei stride = 0, const GLvoid* offset = 0 );
			void setAttribData( GLuint index, const GLBuffer& buffer, GLint size, GLenum type, GLsizei stride = 0, const GLvoid* offset = 0 );

			void resetColor() { resetAttrib( GLSL_COLOR_IDX ); }
			void resetNormal() { resetAttrib( GLSL_NORMAL_IDX ); }
			void resetTexCoord() { resetAttrib( GLSL_TEXCOORD_IDX ); }
			void resetAttrib( GLuint index );

			void setColor( const Color& color );
// FIXME: Remove, the single attrib stuff is not part of the VAO state
//			void setAttribData( GLuint index, float v1 );
//			void setAttribData( GLuint index, float v1, float v2 );
//			void setAttribData( GLuint index, float v1, float v2, float v3 );
//			void setAttribData( GLuint index, float v1, float v2, float v3, float v4 );

			void draw( GLenum mode, GLint first, GLsizei count ) const;
			void drawIndirect( const GLBuffer& elembuf, GLenum type, GLenum mode, GLsizei count ) const;

		private:
			GLuint _vao;
			uint32_t _arrays;
			Color  _color;
	};

	inline void GLVertexArray::setColorData( const GLBuffer& buffer, GLint size, GLenum type, GLsizei stride, const GLvoid* offset )
	{
		setAttribData( GLSL_COLOR_IDX, buffer, size, type, stride, offset );
	}

	inline void GLVertexArray::setNormalData( const GLBuffer& buffer, GLint size, GLenum type, GLsizei stride, const GLvoid* offset )
	{
		setAttribData( GLSL_NORMAL_IDX, buffer, size, type, stride, offset );
	}

	inline void GLVertexArray::setTexCoordData( const GLBuffer& buffer, GLint size, GLenum type, GLsizei stride, const GLvoid* offset )
	{
		setAttribData( GLSL_TEXCOORD_IDX, buffer, size, type, stride, offset );
	}

}



#endif
