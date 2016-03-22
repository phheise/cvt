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

#ifndef CVT_SCENEMESH_H
#define CVT_SCENEMESH_H

#include <cvt/util/SIMD.h>
#include <cvt/geom/scene/SceneGeometry.h>

#include <vector>

namespace cvt {
	enum SceneMeshType {
		SCENEMESH_TRIANGLES,
		SCENEMESH_QUADS
	};

	class SceneMesh : public SceneGeometry {
	public:
								SceneMesh( const String& name );
			virtual				~SceneMesh();

			SceneMeshType		meshType() const;

			void				clear();
			bool				isEmpty() const;

			size_t				vertexSize() const;
			size_t				normalSize() const;
			size_t				tangentSize() const;
			size_t				texcoordSize() const;
			size_t				faceSize() const;

			const Vector3f&		vertex( size_t i ) const;
			const Vector3f&		normal( size_t i ) const;
			const Vector3f&		tangent( size_t i ) const;
			const Vector2f&		texcoord( size_t i ) const;

			void				setVertices( const Vector3f* data, size_t size );
			void				setNormals( const Vector3f* data, size_t size );
			void				setTangents( const Vector3f* data, size_t size );
			void				setTexcoords( const Vector2f* data, size_t size );
			void				setFaces( const unsigned int* data, size_t size, SceneMeshType type );

			const Vector3f*		vertices() const;
			const Vector3f*		normals() const;
			const Vector3f*		tangents() const;
			const Vector2f*		texcoords() const;
			const unsigned int* faces() const;
			void				facesTriangles( std::vector<unsigned int>& output ) const;

			Vector3f			centroid() const;
			Boxf				boundingBox() const;

			void				transform( const Matrix3f& mat );
			void				transform( const Matrix4f& mat );
			void				translate( const Vector3f& translation );
			void				scale( float scale );

			void				calculateNormals( float angleweight = 0.0f, float areaweight = 0.0f );
			void				calculateTangents();
			void				calculateAdjacency();
			void				removeRedundancy( float vepsilon = 0.0f, float nepsilon = 0.0f, float tepsilon = 0.0f );
			void				simplify( float vepsilon );
			void				addNoise( float amount );
			void				flipNormals( );
			void				quadsToTriangles();
			void				subdivideCatmullClark();


		private:
			std::vector<Vector3f>		_vertices;
			std::vector<Vector3f>		_normals;
			std::vector<Vector3f>		_tangents;
			std::vector<Vector2f>		_texcoords;
			std::vector<unsigned int>	_vindices;
			SceneMeshType				_meshtype;
	};

	inline SceneMesh::SceneMesh( const String& name ) : SceneGeometry( name, SCENEGEOMETRY_MESH ), _meshtype( SCENEMESH_TRIANGLES )
	{
	}

	inline SceneMesh::~SceneMesh()
	{
	}

	inline SceneMeshType SceneMesh::meshType() const
	{
		return _meshtype;
	}

	inline void SceneMesh::clear()
	{
		_vertices.clear();
		_normals.clear();
		_texcoords.clear();
		_vindices.clear();
		_meshtype = SCENEMESH_TRIANGLES;
	}

	inline bool SceneMesh::isEmpty() const
	{
		return _vertices.empty() || _vindices.empty();
	}

	inline size_t SceneMesh::vertexSize() const
	{
		return _vertices.size();
	}

	inline size_t SceneMesh::normalSize() const
	{
		return _normals.size();
	}

	inline size_t SceneMesh::tangentSize() const
	{
		return _tangents.size();
	}

	inline size_t SceneMesh::texcoordSize() const
	{
		return _texcoords.size();
	}

	inline size_t SceneMesh::faceSize() const
	{
		size_t nface = _meshtype == SCENEMESH_TRIANGLES ? 3 : 4;
		return _vindices.size() / nface;
	}

	inline const Vector3f& SceneMesh::vertex( size_t i ) const
	{
		return _vertices[ i ];
	}

	inline const Vector3f& SceneMesh::normal( size_t i ) const
	{
		return _normals[ i ];
	}

	inline const Vector3f& SceneMesh::tangent( size_t i ) const
	{
		return _tangents[ i ];
	}

	inline const Vector2f& SceneMesh::texcoord( size_t i ) const
	{
		return _texcoords[ i ];
	}

	inline void SceneMesh::setVertices( const Vector3f* data, size_t size )
	{
		_vertices.assign( data, data + size );
	}

	inline void SceneMesh::setNormals( const Vector3f* data, size_t size )
	{
		_normals.assign( data, data + size );
	}

	inline void SceneMesh::setTangents( const Vector3f* data, size_t size )
	{
		_tangents.assign( data, data + size );
	}

	inline void SceneMesh::setTexcoords( const Vector2f* data, size_t size )
	{
		_texcoords.assign( data, data + size );
	}

	inline void SceneMesh::setFaces( const unsigned int* data, size_t size, SceneMeshType meshtype )
	{
		_meshtype = meshtype;
		_vindices.assign( data, data + size );
	}

	inline const Vector3f* SceneMesh::vertices() const
	{
		return &_vertices[ 0 ];
	}

	inline const Vector3f* SceneMesh::normals() const
	{
		return &_normals[ 0 ];
	}

	inline const Vector3f* SceneMesh::tangents() const
	{
		return &_tangents[ 0 ];
	}

	inline const Vector2f* SceneMesh::texcoords() const
	{
		return &_texcoords[ 0 ];
	}

	inline const unsigned int* SceneMesh::faces() const
	{
		return &_vindices[ 0 ];
	}

	inline Vector3f SceneMesh::centroid( ) const
	{
		size_t n = _vertices.size();
		const Vector3f* pt = &_vertices[ 0 ];
		Vector3f centroid;

		SIMD::instance()->sumPoints( centroid, pt, n );

		centroid /= ( float ) _vertices.size();
		return centroid;
	}


	inline Boxf SceneMesh::boundingBox() const
	{
		Vector3f min, max;
		Boxf bbox;

		if( !_vindices.size() )
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

	inline void SceneMesh::translate( const Vector3f& translation )
	{
		size_t n = _vertices.size();
		Vector3f* pt = &_vertices[ 0 ];

		SIMD::instance()->translatePoints( pt, pt, translation, n );
	}

	inline void SceneMesh::scale( float scale )
	{
		size_t n = _vertices.size();
		Vector3f* pt = &_vertices[ 0 ];

		SIMD::instance()->MulValue1f( ( float* ) pt, ( const float* ) pt, scale, n * 3 );
	}

	inline void SceneMesh::transform( const Matrix3f& mat )
	{
		size_t n = _vertices.size();
		Vector3f* pt = &_vertices[ 0 ];
		bool donormals = _normals.size();
		Vector3f* nt = &_normals[ 0 ];
		Matrix3f _nnmat( mat );
		_nnmat.transposeSelf();
		_nnmat.inverseSelf();

		// FIXME: also tangents

		if( !donormals ) {
			SIMD::instance()->transformPoints( pt, mat, pt, n );
		} else {
			SIMD::instance()->transformPoints( pt, mat, pt, n );
			SIMD::instance()->transformPoints( nt, _nnmat, nt, n );
		}
	}

	inline void SceneMesh::transform( const Matrix4f& mat )
	{
		size_t n = _vertices.size();
		Vector3f* pt = &_vertices[ 0 ];
		bool donormals = _normals.size();
		Vector3f* nt = &_normals[ 0 ];
		Matrix3f _nnmat( mat );
		_nnmat.transposeSelf();
		_nnmat.inverseSelf();

		// FIXME: also tangents

		if( mat[ 3 ] == Vector4f( 0, 0, 0, 1.0f ) ) {
			/* if last row is [ 0 0 0 1 ], split mat into 3 x 3 matrix and translation */
			if( !donormals ) {
				SIMD::instance()->transformPoints( pt, mat, pt, n );
			} else {
				SIMD::instance()->transformPoints( pt, mat, pt, n );
				SIMD::instance()->transformPoints( nt, _nnmat, nt, n );
			}
		} else {
			if( !donormals ) {
				SIMD::instance()->transformPointsHomogenize( pt, mat, pt, n );
			} else {
				SIMD::instance()->transformPointsHomogenize( pt, mat, pt, n );
				SIMD::instance()->transformPoints( nt, _nnmat, nt, n );
			}
		}
	}

	inline std::ostream& operator<<( std::ostream& out, const SceneMesh& mesh )
	{
		out << "Mesh: " << mesh.name() << "\n";
		out << "\tVertices: " << mesh.vertexSize();
		out << "\n\tNormals: " << mesh.normalSize();
		out << "\n\tTexCoords: " << mesh.texcoordSize();
		out << "\n\tFaces: " << mesh.faceSize() << "\n";
		return out;
	}

}

#endif
