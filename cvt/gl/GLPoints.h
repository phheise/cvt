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

#ifndef CVT_GLPOINTS_H
#define CVT_GLPOINTS_H

#include <cvt/gl/GLVertexArray.h>
#include <cvt/geom/scene/ScenePoints.h>

namespace cvt {
    class GLPoints {
        public:
            GLPoints();
            GLPoints( const ScenePoints& pts );
            /*GLPoints( const Vector3f* vertices, const Vector3f* normals, const Vector2f* texcoords, size_t vsize,
                    const unsigned int* indices, size_t isize, MeshType type );*/
            ~GLPoints();

            void	setScenePoints( const ScenePoints& pts );
            void	draw();
			float	pointSize() const;
			void	setPointSize( float size );

        private:
            GLVertexArray _vao;
            GLBuffer _vertices;
            GLBuffer _colors;
            size_t	 _numPts;
            float    _pointSize;
    };

    inline GLPoints::GLPoints() :
        _vertices( GL_ARRAY_BUFFER ),
        _colors( GL_ARRAY_BUFFER ),
        _numPts( 0 )
    {
    }

    inline GLPoints::GLPoints( const ScenePoints& points ) :
        _vertices( GL_ARRAY_BUFFER ),
                _colors( GL_ARRAY_BUFFER )
    {
                setScenePoints( points );
    }

    inline GLPoints::~GLPoints()
    {
    }

	inline float GLPoints::pointSize() const
	{
		return _pointSize;
	}

	inline void	GLPoints::setPointSize( float size )
	{
		_pointSize = size;
	}


	inline void GLPoints::setScenePoints( const ScenePoints& pts )
	{
		_numPts = pts.vertexSize();
		_vertices.alloc( GL_STATIC_DRAW, sizeof( GLfloat ) * 3 * pts.vertexSize(), pts.vertices() );
		_colors.alloc( GL_STATIC_DRAW, sizeof( GLfloat ) * 4 * pts.colorSize(), pts.colors() );

		_vao.setVertexData( _vertices, 3, GL_FLOAT );
		_vao.setColorData( _colors, 4, GL_FLOAT );
		_pointSize = pts.pointSize();
	}

    inline void GLPoints::draw()
    {
        glPointSize( _pointSize );
        _vao.draw( GL_POINTS, 0, _numPts );
        glPointSize( 1.0f );
    }
}

#endif
