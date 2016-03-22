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

#ifndef CVT_SCENEPOINTS_H
#define CVT_SCENEPOINTS_H

#include <cvt/geom/scene/SceneGeometry.h>

#include <vector>

namespace cvt {
	class ScenePoints : public SceneGeometry {
		public:
			ScenePoints( const String& name );
			virtual				~ScenePoints();

			void				clear();
			bool				isEmpty() const;

			void				add( const ScenePoints& spts );

			float				pointSize() const;
			void				setPointSize( float ptsize );

			size_t				vertexSize() const;
			//			size_t				texcoordSize() const;
			size_t				colorSize() const;

			const Vector3f&		vertex( size_t i ) const;
			const Vector4f&		color( size_t i ) const;

			void				setVertices( const Vector3f* data, size_t size );
			//			void				setTexcoords( const Vector2f* data, size_t size );
                        void				setVerticesWithColor( const Vector3f* vertices, const Vector4f* colors, size_t size );

			const Vector3f*		vertices() const;
			//			const Vector2f*		texcoords() const;
			const Vector4f*		colors() const;

			Vector3f			centroid() const;
			Boxf				boundingBox() const;

			void				transform( const Matrix3f& mat );
			void				transform( const Matrix4f& mat );
			void				translate( const Vector3f& translation );
			void				scale( float scale );

		private:
			float						_ptsize;
			std::vector<Vector3f>		_vertices;
			//			std::vector<Vector2f>		_texcoords;
			std::vector<Vector4f>		_colors;
	};

	inline ScenePoints::ScenePoints( const String& name ) : SceneGeometry( name, SCENEGEOMETRY_POINTS ), _ptsize( 1.0f )
	{
	}

	inline ScenePoints::~ScenePoints()
	{
	}

	inline void ScenePoints::clear()
	{
		_vertices.clear();
		//		_texcoords.clear();
		_colors.clear();
	}

	inline bool ScenePoints::isEmpty() const
	{
		return _vertices.empty();
	}

	inline void ScenePoints::add( const ScenePoints& spts )
	{
		_vertices.insert( _vertices.end(), &spts._vertices[ 0 ], ( &spts._vertices[ 0 ] ) + spts.vertexSize() );
		_colors.insert( _colors.end(), &spts._colors[ 0 ], ( &spts._colors[ 0 ] ) + spts.colorSize() );
	}

	inline float ScenePoints::pointSize() const
	{
		return _ptsize;
	}

	inline void	ScenePoints::setPointSize( float ptsize )
	{
		_ptsize = ptsize;
	}

	inline size_t ScenePoints::vertexSize() const
	{
		return _vertices.size();
	}

	inline size_t ScenePoints::colorSize() const
	{
		return _colors.size();
	}

	/*inline size_t ScenePoints::texcoordSize() const
	  {
	  return _texcoords.size();
	  }*/


	inline const Vector3f& ScenePoints::vertex( size_t i ) const
	{
		return _vertices[ i ];
	}

	inline const Vector4f& ScenePoints::color( size_t i ) const
	{
		return _colors[ i ];
	}

	/*inline const Vector2f& ScenePoints::texcoord( size_t i ) const
	  {
	  return _texcoords[ i ];
	  }*/

	inline void ScenePoints::setVertices( const Vector3f* data, size_t size )
	{
		_vertices.assign( data, data + size );
	}

        inline void ScenePoints::setVerticesWithColor( const Vector3f* vertices, const Vector4f* colors, size_t size )
	{
                _vertices.assign( vertices, vertices + size );
                _colors.assign( colors, colors + size );
	}


	/*inline void ScenePoints::setTexcoords( const Vector2f* data, size_t size )
	  {
	  _texcoords.assign( data, data + size );
	  }*/

	inline const Vector3f* ScenePoints::vertices() const
	{
		return &_vertices[ 0 ];
	}

	inline const Vector4f* ScenePoints::colors() const
	{
		return &_colors[ 0 ];
	}

	/*inline const Vector2f* ScenePoints::texcoords() const
	  {
	  return &_texcoords[ 0 ];
	  }*/

	inline Vector3f ScenePoints::centroid( ) const
	{
		size_t n = _vertices.size();
		const Vector3f* pt = &_vertices[ 0 ];
		Vector3f centroid;

		SIMD::instance()->sumPoints( centroid, pt, n );

		centroid /= ( float ) _vertices.size();
		return centroid;
	}


	inline Boxf ScenePoints::boundingBox() const
	{
		Vector3f min, max;
		Boxf bbox;

		if( !_vertices.size() )
			return bbox;

		min = max = _vertices[ 0 ];
		size_t n = _vertices.size() - 1;
		const Vector3f* pt = &_vertices[ 1 ];

		while( n-- ) {
			min.x = Math::min( min.x, pt->x );
			min.y = Math::min( min.y, pt->y );
			min.z = Math::min( min.z, pt->z );
			max.x = Math::max( max.x, pt->x );
			max.y = Math::max( max.y, pt->y );
			max.z = Math::max( max.z, pt->z );
			pt++;
		}
		bbox.set( min, max );
		return bbox;
	}

	inline void ScenePoints::translate( const Vector3f& translation )
	{
		size_t n = _vertices.size();
		Vector3f* pt = &_vertices[ 0 ];

		SIMD::instance()->translatePoints( pt, pt, translation, n );
	}

	inline void ScenePoints::scale( float scale )
	{
		size_t n = _vertices.size();
		Vector3f* pt = &_vertices[ 0 ];

		SIMD::instance()->MulValue1f( ( float* ) pt, ( const float* ) pt, scale, n * 3 );
	}

	inline void ScenePoints::transform( const Matrix3f& mat )
	{
		size_t n = _vertices.size();
		Vector3f* pt = &_vertices[ 0 ];


		SIMD::instance()->transformPoints( pt, mat, pt, n );
	}

	inline void ScenePoints::transform( const Matrix4f& mat )
	{
		size_t n = _vertices.size();
		Vector3f* pt = &_vertices[ 0 ];

		if( mat[ 3 ] == Vector4f( 0, 0, 0, 1.0f ) ) {
			/* if last row is [ 0 0 0 1 ], split mat into 3 x 3 matrix and translation */
			SIMD::instance()->transformPoints( pt, mat, pt, n );
		} else {
			SIMD::instance()->transformPointsHomogenize( pt, mat, pt, n );
		}
	}

	inline std::ostream& operator<<( std::ostream& out, const ScenePoints& spts )
	{
		out << "ScenePoints: " << spts.name() << "\n";
		out << "\tVertices: " << spts.vertexSize();
		return out;
	}

}

#endif
