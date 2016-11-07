/*
   The MIT License (MIT)

   Copyright (c) 2011 - 2013, Philipp Heise and Sebastian Klose
   Copyright (c) 2016, BMW Car IT GmbH, Philipp Heise (philipp.heise@bmw.de)

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
#include <cvt/math/Vector.h>
#include <cvt/container/List.h>
#include <stack>

namespace cvt {


    struct Face {
        Face( unsigned int i1, unsigned int i2, unsigned int i3 ) : idx1( i1 ), idx2( i2 ), idx3( i3 )
        {
        }

        bool isConnected( const std::vector<unsigned int>& indices )
        {
            for( size_t i = 0; i < indices.size(); i++ ) {
                if( isConnected( indices[ i ] ) )
                    return true;
            }
            return false;
        }

        bool isConnected( unsigned int idx )
        {
            return idx1 == idx ||  idx2 == idx || idx3 == idx;
        }

        bool isConnected( const Face& f )
        {
            return isConnected( f.idx1 ) || isConnected( f.idx2 ) || isConnected( f.idx3 );
        }

        unsigned int idx1, idx2, idx3;
    };

    class IndexHash {
        public:
            IndexHash( size_t N ) : _N( N )
            {
                _buckets = new std::vector<unsigned int>[ _N ];
            }

            ~IndexHash()
            {
                delete[] _buckets;
            }

            bool contains( unsigned int idx ) const
            {
                size_t bucket = idx % _N;
                for( size_t i = 0; i < _buckets[ bucket ].size(); i++ ) {
                    if( _buckets[ bucket ][ i ] == idx )
                        return true;
                }
                return false;
            }

            void add( unsigned int idx )
            {
                 size_t bucket = idx % _N;
                 _buckets[ bucket ].push_back( idx );
            }

        private:
            std::vector<unsigned int>* _buckets;
            size_t _N;
    };

    template<typename T>
    class VertexHash {
        public:

            VertexHash( size_t N ) : _N( N )
            {
                _buckets = new std::vector<Entry>[ _N ];
            }

            ~VertexHash()
            {
                delete[] _buckets;
            }

            void add( const Vector3f& k, T v )
            {
                size_t bucket = hash( k );
                _buckets[ bucket ].push_back( Entry( k, v ) );
            }

            const T* contains( const Vector3f& k ) const
            {
                size_t bucket = hash( k );
                Entry* e = bucketContains( bucket, k );
                if( e )
                    return &( e->value );
                return NULL;
            }

            size_t hash( const Vector3f& v ) const
            {
                const size_t PRIME1 = 3;
                const size_t PRIME2 = 7;
                const size_t PRIME3 = 11;
                size_t ret;

                Math::_flint32 flint;
                flint.f = v.x;
                ret  = PRIME1 * ( size_t ) ( ( flint.i & 0x7ffffff0 ) >> 4 );
                flint.f = v.y;
                ret += PRIME2 * ( size_t ) ( ( flint.i & 0x7ffffff0 ) >> 4 );
                flint.f = v.z;
                ret += PRIME3 * ( size_t ) ( ( flint.i & 0x7ffffff0 ) >> 4 );
                return ret % _N;
            }

        private:
            struct Entry {
                Entry( const Vector3f& k, const T& v ) : key( k ), value( v )
                {
                }

                Vector3f key;
                T        value;
            };

            Entry* bucketContains( size_t bucket, const Vector3f& v ) const
            {
                Vector3f tmp;
                size_t size = _buckets[ bucket ].size();
                for( size_t i = 0; i < size; i++ ) {
                    tmp = _buckets[ bucket ][ i ].key;
                    if( v.isEqual( tmp, Math::EPSILONF ) ) {
                        return &_buckets[ bucket ][ i ];
                    }
                }

                return NULL;
            }

            std::vector<Entry>* _buckets;
            size_t _N;
    };

    void SceneMesh::removeRedundancy( float vepsilon, float nepsilon, float tepsilon )
    {
        std::vector<Vector3f>       nvertices;
        std::vector<Vector3f>       nnormals;
        std::vector<Vector2f>       ntexcoords;
        std::vector<unsigned int>   indexold2new;
        std::vector<unsigned int>   nvindices;

        for( size_t idx = 0; idx < _vertices.size(); idx++ ) {
            Vector3f v( _vertices[ idx ] );
            bool added = false;

            for( size_t i = 0; i < nvertices.size() && !added; i++ ) {
                if( v.isEqual( nvertices[ i ], vepsilon ) ) {
                    if( hasNormals() ) {
                        Vector3f n( _normals[ idx ] );
                        if( !n.isEqual( nnormals[ i ], nepsilon ) )
                            continue;
                    }
                    if( hasTexcoords() ) {
                        Vector2f t = _texcoords[ idx ];
                        if( !t.isEqual( ntexcoords[ i ], tepsilon ) )
                            continue;
                    }
                    added = true;
                    indexold2new.push_back( i );
                }
            }
            if( !added ) {
                nvertices.push_back( v );
                if( hasNormals() )
                    nnormals.push_back( _normals[ idx ] );
                if( hasTexcoords() )
                    ntexcoords.push_back( _texcoords[ idx ] );
                indexold2new.push_back( nvertices.size() - 1 );
            }
        }


        for( size_t idx = 0; idx < _vindices.size(); idx++ ) {
            nvindices.push_back( indexold2new[ _vindices[ idx ] ] );
        }

        _vertices = nvertices;
        _normals = nnormals;
        _texcoords = ntexcoords;
        _vindices = nvindices;
    }


    void SceneMesh::removeDuplicateVertices()
    {
        VertexHash<size_t> vhash( 31357 ); //arbitrary huge prime - FIXME: consider number vertices when choosing prime number

        std::vector<Vector3f>       nvertices;
        std::vector<Vector3f>       nnormals;
        std::vector<Vector2f>       ntexcoords;
        std::vector<Vector3f>       ncolors;
        std::vector<Vector3f>       ntangents;
        std::vector<unsigned int>   indexold2new;
        std::vector<unsigned int>   nvindices;

        for( size_t idx = 0; idx < _vertices.size(); idx++ ) {
            const size_t* entry;

            bool found = false;
            size_t idxFound;
            // we only check one match
            if( ( entry = vhash.contains( _vertices[ idx ] ) ) != NULL ) {
                idxFound = *entry;
                found = true;

                /* check if also the other attributes match */
                if( hasNormals() ) {
                    Vector3f n( _normals[ idx ] );
                    if( !n.isEqual( nnormals[ idxFound ], Math::EPSILONF ) )
                        found = false;
                }
                if( hasTexcoords() ) {
                    Vector2f t( _texcoords[ idx ] );
                    if( !t.isEqual( ntexcoords[ idxFound ], Math::EPSILONF ) )
                        found = false;
                }
                if( hasColors() ) {
                    Vector3f col( _colors[ idx ] );
                    if( !col.isEqual( ncolors[ idxFound ], Math::EPSILONF ) )
                        found = false;
                }
                if( hasTangents() ) {
                    Vector3f tan( _tangents[ idx ] );
                    if( !tan.isEqual( ntangents[ idxFound ], Math::EPSILONF ) )
                        found = false;
                }
            }

            // already found so only push back the position
            if( found ) {
                indexold2new.push_back( idxFound );
            } else {
                nvertices.push_back( _vertices[ idx ] );
                if( hasNormals() )
                    nnormals.push_back( _normals[ idx ] );
                if( hasTexcoords() )
                    ntexcoords.push_back( _texcoords[ idx ] );
                if( hasColors() )
                    ncolors.push_back( _colors[ idx ] );
                if( hasTangents() )
                    ntangents.push_back( _tangents[ idx ] );
                indexold2new.push_back( nvertices.size() - 1 );
                vhash.add( _vertices[ idx ], nvertices.size() - 1 );
            }
        }

        for( size_t idx = 0; idx < _vindices.size(); idx++ ) {
            nvindices.push_back( indexold2new[ _vindices[ idx ] ] );
        }

        _vertices = nvertices;
        _normals = nnormals;
        _texcoords = ntexcoords;
        _colors = ncolors;
        _tangents = ntangents;
        _vindices = nvindices;
    }


    void SceneMesh::removeDegenerateFaces( float vepsilon )
    {
        if( _meshtype != SCENEMESH_TRIANGLES )
            return;

        std::vector<unsigned int>   nvindices;
        for( size_t i = 0; i < _vindices.size(); i += 3 ) {
            if( _vertices[ _vindices[ i ] ].isEqual( _vertices[ _vindices[ i + 1 ] ], vepsilon ) )
                continue;
            if( _vertices[ _vindices[ i ] ].isEqual( _vertices[ _vindices[ i + 2 ] ], vepsilon ) )
                continue;

            nvindices.push_back( _vindices[ i ] );
            nvindices.push_back( _vindices[ i + 1 ] );
            nvindices.push_back( _vindices[ i + 2 ] );
        }

        _vindices = nvindices;
    }

    void SceneMesh::removeIsolatedComponents()
    {
        if( _meshtype != SCENEMESH_TRIANGLES )
            return;

        List<Face> faces;
        std::vector<std::vector<unsigned int> > components;

        for( size_t i = 0; i < _vindices.size(); i += 3 ) {
            faces.append( Face ( _vindices[ i ], _vindices[ i + 1 ], _vindices[ i + 2 ] ) );
        }

        size_t maxcomp = 0, maxcompsize = 0;
        size_t curcomp = 0, curcompsize;
        while( !faces.isEmpty() ) {
            std::cout << "Faces left: " << faces.size() << std::endl;
            curcompsize = 0;
            IndexHash members( 31357 );

            components.push_back( std::vector<unsigned int>() );
            components.back().push_back( faces.front().idx1 );
            components.back().push_back( faces.front().idx2 );
            components.back().push_back( faces.front().idx3 );

            for( size_t i = 1; i <= 3; i++ ) {
                unsigned int idx = components.back()[ components.back().size() - i ];
                if( !members.contains( idx ) ) {
                    members.add( idx );
                }
            }

            // get next triangle of next component
            faces.remove( faces.begin() );

            List<Face>::Iterator it = faces.begin();
            while( it != faces.end() ) {
                if( members.contains( it->idx1 ) || members.contains( it->idx2 ) || members.contains( it->idx3 ) ) {
                    components.back().push_back( it->idx1 );
                    components.back().push_back( it->idx2 );
                    components.back().push_back( it->idx3 );
                    for( size_t i = 1; i <= 3; i++ ) {
                        unsigned int idx = components.back()[ components.back().size() - i ];
                        if( !members.contains( idx ) ) {
                            members.add( idx );
                        }
                    }
                    it = faces.remove( it );
                    curcompsize++;
                } else
                    it++;
            }

            // we want to keep the biggest component
            if( curcompsize > maxcompsize ) {
                maxcomp = curcomp;
                maxcompsize = curcompsize;
            }
            curcomp++;
        }

        _vindices = components[ maxcomp ];
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
