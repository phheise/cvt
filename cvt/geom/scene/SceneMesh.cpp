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

#include <cvt/geom/scene/SceneMesh.h>
#include <set>

namespace cvt {


	void SceneMesh::removeRedundancy( float vepsilon, float nepsilon, float tepsilon )
	{
		std::vector<Vector3f>		nvertices;
		std::vector<Vector3f>		nnormals;
		std::vector<Vector2f>		ntexcoords;
		std::vector<unsigned int>	nvindices;

		for( size_t idx = 0; idx < _vertices.size(); idx++ ) {
			Vector3f v( _vertices[ idx ] );
			bool added = false;

			for( size_t i = 0; i < nvertices.size() && !added; i++ ) {
				if( v.isEqual( nvertices[ i ], vepsilon ) ) {
					if( normalSize() ) {
						Vector3f n( _normals[ idx ] );
						if( !n.isEqual( nnormals[ i ], nepsilon ) )
							continue;
					}
					if( texcoordSize() ) {
						Vector2f t = _texcoords[ idx ];
						if( !t.isEqual( ntexcoords[ i ], tepsilon ) )
							continue;
					}
					added = true;
					nvindices.push_back( i );
				}
			}
			if( !added ) {
				nvertices.push_back( v );
				if( normalSize() )
					nnormals.push_back( _normals[ idx ] );
				if( texcoordSize() )
					ntexcoords.push_back( _texcoords[ idx ] );
				nvindices.push_back( nvertices.size() - 1 );
			}
		}

		_vertices = nvertices;
		_normals = nnormals;
		_texcoords = ntexcoords;
		_vindices = nvindices;
	}

	void SceneMesh::quadsToTriangles()
	{
		const unsigned int table[] = { 0, 1, 2, 2, 3, 0 };

		if( _meshtype != SCENEMESH_QUADS || _vindices.size() % 4 != 0 )
			return;

		std::vector<unsigned int> nvindices;
		size_t size = _vindices.size();
		for( size_t n = 0; n < size; n += 4 ) {
				// Convert quad to triangle
				for( int i = 0; i < 6; i++ ) {
					nvindices.push_back( _vindices[ n + table[ i ] ] );
				}
		}

		_vindices = nvindices;
		_meshtype = SCENEMESH_TRIANGLES;
	}


	void SceneMesh::facesTriangles( std::vector<unsigned int>& output ) const
	{
		const unsigned int table[] = { 0, 1, 2, 2, 3, 0 };

		if( _meshtype != SCENEMESH_QUADS ) {
			output = _vindices;
			return;
		};

		output.clear();
		size_t size = _vindices.size();
		for( size_t n = 0; n < size; n += 4 ) {
				// Convert quad to triangle
				for( int i = 0; i < 6; i++ ) {
					output.push_back( _vindices[ n + table[ i ] ] );
				}
		}
	}

	void SceneMesh::calculateNormals( float angleweight, float areaweight )
	{
		_normals.clear();
		_normals.resize( _vertices.size(), Vector3f( 0.0f, 0.0f, 0.0f ) );

		if( _meshtype == SCENEMESH_TRIANGLES ) {
			size_t size = _vindices.size();
			for( size_t n = 0; n < size; n += 3 ) {
				Vector3f v[ 3 ];
				v[ 0 ] = _vertices[ _vindices[ n + 0 ] ];
				v[ 1 ] = _vertices[ _vindices[ n + 1 ] ];
				v[ 2 ] = _vertices[ _vindices[ n + 2 ] ];
				Vector3f normal;
				normal.cross( v[ 1 ] - v[ 0 ], v[ 2 ] - v[ 0 ] );
				float area = 1.0f;//normal.normalize();
				area = Math::sqrt( area ) * 0.5f;
				for (int k = 0; k < 3; k++ ) {
					Vector3f a = v[ ( k + 1 ) % 3 ] - v[ k ];
					Vector3f b = v[ ( k + 2 ) % 3 ] - v[ k ];
					float angle = 1.0f;// Math::acos( ( a * b ) / ( a.length() * b.length() ) );
					_normals[ _vindices[ n + k ] ] += ( 1.0f +  angleweight * angle + areaweight * area ) * normal;
				}
			}
		} else if( _meshtype == SCENEMESH_QUADS ) {
			size_t size = _vindices.size();
			for( size_t n = 0; n < size; n += 4 ) {
				Vector3f v[ 4 ];
				v[ 0 ] = _vertices[ _vindices[ n + 0 ] ];
				v[ 1 ] = _vertices[ _vindices[ n + 1 ] ];
				v[ 2 ] = _vertices[ _vindices[ n + 2 ] ];
				v[ 3 ] = _vertices[ _vindices[ n + 3 ] ];
				Vector3f normal;
				normal.cross( v[ 1 ] - v[ 0 ], v[ 2 ] - v[ 0 ] );
				float area = normal.normalize();
				area = Math::sqrt( area );
				area = 0.5f * ( area + ( v[ 1 ] - v[ 3 ] ).cross( v[ 2 ] - v[ 3 ] ).length() );
				for (int k = 0; k < 4; k++ ) {
					int lower = ( k - 1 );
					if( lower < 0 )
						lower = 3;
					Vector3f a = v[ lower ] - v[ k ];
					Vector3f b = v[ ( k + 1 ) % 4 ] - v[ k ];
					float angle = Math::acos( ( a * b ) / ( a.length() * b.length() ) );
					_normals[ _vindices[ n + k ] ] += ( 1.0f +  angleweight * angle + areaweight * area ) * normal;
				}
			}
		}
		size_t size = _normals.size();
		for( size_t n = 0; n < size; n++ )
			_normals[ n ].normalize();
	}

	void SceneMesh::calculateTangents()
	{
	}

}
