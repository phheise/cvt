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

#ifndef CVT_SCENELINES_H
#define CVT_SCENELINES_H

#include <cvt/geom/scene/SceneGeometry.h>

#include <vector>

namespace cvt {
	class SceneLines : public SceneGeometry {
		public:
			SceneLines( const String& name );
			virtual				~SceneLines();

			void				clear();
			bool				isEmpty() const;

			void				add( const SceneLines& spts );
			void				add( const Vector3f& pt, const Vector4f& color );

			float				lineSize() const;
			void				setLineSize( float linesize );

			size_t				vertexSize() const;
			//			size_t				texcoordSize() const;
			size_t				colorSize() const;

			const Vector3f&		vertex( size_t i ) const;
			const Vector4f&		color( size_t i ) const;

			void				setVertices( const Vector3f* data, size_t size );
			//			void				setTexcoords( const Vector2f* data, size_t size );
			void				setColors( const Vector4f* data, size_t size );

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
			float						_lineSize;
			std::vector<Vector3f>		_vertices;
			//			std::vector<Vector2f>		_texcoords;
			std::vector<Vector4f>		_colors;
	};

	inline SceneLines::SceneLines( const String& name ) : SceneGeometry( name, SCENEGEOMETRY_POINTS ), _lineSize( 1.0f )
	{
	}

	inline SceneLines::~SceneLines()
	{
	}

	inline void SceneLines::clear()
	{
		_vertices.clear();
		//		_texcoords.clear();
		_colors.clear();
	}

	inline bool SceneLines::isEmpty() const
	{
		return _vertices.empty();
	}

	inline void SceneLines::add( const SceneLines& spts )
	{
		_vertices.insert( _vertices.end(), &spts._vertices[ 0 ], ( &spts._vertices[ 0 ] ) + spts.vertexSize() );
		_colors.insert( _colors.end(), &spts._colors[ 0 ], ( &spts._colors[ 0 ] ) + spts.colorSize() );
	}

	inline void SceneLines::add( const Vector3f& pt, const Vector4f& color )
	{
		_vertices.push_back( pt );
		_colors.push_back( color );
	}

	inline float SceneLines::lineSize() const
	{
		return _lineSize;
	}

	inline void	SceneLines::setLineSize( float lineSize )
	{
		_lineSize = lineSize;
	}

	inline size_t SceneLines::vertexSize() const
	{
		return _vertices.size();
	}

	inline size_t SceneLines::colorSize() const
	{
		return _colors.size();
	}

	/*inline size_t SceneLines::texcoordSize() const
	  {
	  return _texcoords.size();
	  }*/


	inline const Vector3f& SceneLines::vertex( size_t i ) const
	{
		return _vertices[ i ];
	}

	inline const Vector4f& SceneLines::color( size_t i ) const
	{
		return _colors[ i ];
	}

	/*inline const Vector2f& SceneLines::texcoord( size_t i ) const
	  {
	  return _texcoords[ i ];
	  }*/

	inline void SceneLines::setVertices( const Vector3f* data, size_t size )
	{
		_vertices.assign( data, data + size );
	}

	inline void SceneLines::setColors( const Vector4f* data, size_t size )
	{
		_colors.assign( data, data + size );
	}


	/*inline void SceneLines::setTexcoords( const Vector2f* data, size_t size )
	  {
	  _texcoords.assign( data, data + size );
	  }*/

	inline const Vector3f* SceneLines::vertices() const
	{
		return &_vertices[ 0 ];
	}

	inline const Vector4f* SceneLines::colors() const
	{
		return &_colors[ 0 ];
	}

	/*inline const Vector2f* SceneLines::texcoords() const
	  {
	  return &_texcoords[ 0 ];
	  }*/

	inline Vector3f SceneLines::centroid( ) const
	{
		size_t n = _vertices.size();
		const Vector3f* pt = &_vertices[ 0 ];
		Vector3f centroid;

		centroid.setZero();
		while( n-- )
			centroid += *pt++;
		centroid /= ( float ) _vertices.size();
		return centroid;
	}


	inline Boxf SceneLines::boundingBox() const
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

	inline void SceneLines::translate( const Vector3f& translation )
	{
		size_t n = _vertices.size();
		Vector3f* pt = &_vertices[ 0 ];

		while( n-- )
			*pt++ += translation;
	}

	inline void SceneLines::scale( float scale )
	{
		size_t n = _vertices.size();
		Vector3f* pt = &_vertices[ 0 ];

		while( n-- )
			*pt++ *= scale;
	}

	inline void SceneLines::transform( const Matrix3f& mat )
	{
		size_t n = _vertices.size();
		Vector3f* pt = &_vertices[ 0 ];


		while( n-- ) {
			*pt = mat * *pt;
			pt++;
		}
	}

	inline void SceneLines::transform( const Matrix4f& mat )
	{
		size_t n = _vertices.size();
		Vector3f* pt = &_vertices[ 0 ];

		if( mat[ 3 ] == Vector4f( 0, 0, 0, 1.0f ) ) {
			/* if last row is [ 0 0 0 1 ], split mat into 3 x 3 matrix and translation */
			Vector3f trans( mat[ 0 ][ 3 ], mat[ 1 ][ 3 ], mat[ 2 ][ 3 ] );
			Matrix3f _mat( mat );
			while( n-- ) {
				*pt = _mat * *pt;
				*pt += trans;
				pt++;
			}
		} else {
			while( n-- ) {
				*pt = mat * *pt;
				pt++;
			}
		}
	}

	inline std::ostream& operator<<( std::ostream& out, const SceneLines& spts )
	{
		out << "SceneLines: " << spts.name() << "\n";
		out << "\tVertices: " << spts.vertexSize();
		return out;
	}

}

#endif
