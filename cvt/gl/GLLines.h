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

#ifndef CVT_GLLINES_H
#define CVT_GLLINES_H

#include <cvt/gl/GLVertexArray.h>
#include <cvt/geom/scene/SceneLines.h>
#include <cvt/gfx/Color.h>

namespace cvt {
	class GLLines {
		public:
			GLLines();
			GLLines( const SceneLines& pts );
			GLLines( const Boxf& box, const Color& c );
			/*GLLines( const Vector3f* vertices, const Vector3f* normals, const Vector2f* texcoords, size_t vsize,
				    const unsigned int* indices, size_t isize, MeshType type );*/
			~GLLines();

			void draw( GLenum mode = GL_LINES );

		private:
			GLVertexArray _vao;
			GLBuffer _vertices;
			GLBuffer _colors;
			size_t	 _numPts;
	};

	inline GLLines::GLLines() :
		_vertices( GL_ARRAY_BUFFER ),
		_colors( GL_ARRAY_BUFFER ),
		_numPts( 0 )
	{
	}


	inline GLLines::GLLines( const Boxf& box, const Color& c )
	{
		Vector3f pos, size;
		box.getPosition( pos );
		box.getSize( size );
		float vertices[ 24 * 3 ] = { /* top */
								 pos.x, pos.y, pos.z,
								 pos.x + size.x, pos.y, pos.z,
								 pos.x + size.x, pos.y, pos.z,
								 pos.x + size.x, pos.y, pos.z + size.z,
								 pos.x + size.x, pos.y, pos.z + size.z,
								 pos.x, pos.y, pos.z + size.z,
								 pos.x, pos.y, pos.z + size.z,
								 pos.x, pos.y, pos.z,
								 /* bottom */
								 pos.x, pos.y + size.y, pos.z,
								 pos.x + size.x, pos.y + size.y, pos.z,
								 pos.x + size.x, pos.y + size.y, pos.z,
								 pos.x + size.x, pos.y + size.y, pos.z + size.z,
								 pos.x + size.x, pos.y + size.y, pos.z + size.z,
								 pos.x, pos.y + size.y, pos.z + size.z,
								 pos.x, pos.y + size.y, pos.z + size.z,
								 pos.x, pos.y + size.y, pos.z,
								 /* top to bottom */
								 pos.x, pos.y, pos.z,
								 pos.x, pos.y + size.y, pos.z,
								 pos.x + size.x, pos.y, pos.z,
								 pos.x + size.x, pos.y + size.y, pos.z,
								 pos.x + size.x, pos.y, pos.z + size.z,
								 pos.x + size.x, pos.y + size.y, pos.z + size.z,
								 pos.x, pos.y, pos.z + size.z,
								 pos.x, pos.y + size.y, pos.z + size.z
							  };
		_vertices.alloc( GL_STATIC_DRAW, sizeof( GLfloat ) * 3 * 24, vertices );
		_vao.setVertexData( _vertices, 3, GL_FLOAT );
		_vao.setColor( c );
		_numPts = 24;
	}

	inline GLLines::GLLines( const SceneLines& lines ) :
		_vertices( GL_ARRAY_BUFFER ),
		_colors( GL_ARRAY_BUFFER ),
		_numPts( lines.vertexSize() )
	{
		_vertices.alloc( GL_STATIC_DRAW, sizeof( GLfloat ) * 3 * lines.vertexSize(), lines.vertices() );
		_colors.alloc( GL_STATIC_DRAW, sizeof( GLfloat ) * 4 * lines.colorSize(), lines.colors() );

		_vao.setVertexData( _vertices, 3, GL_FLOAT );
		_vao.setColorData( _colors, 4, GL_FLOAT );
	}

	inline GLLines::~GLLines()
	{
	}

	inline void GLLines::draw( GLenum mode )
	{
		_vao.draw( mode, 0, _numPts );
	}
}

#endif
