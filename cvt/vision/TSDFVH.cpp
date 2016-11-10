/*
   The MIT License (MIT)

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

#include <cvt/vision/TSDFVH.h>
#include <cvt/cl/kernel/TSDFVH/TSDFVH.h>
#include <cvt/cl/CLMatrix.h>

namespace cvt {
#define BUCKETS_MAX_SIZE 1000003 // Prime
#define BUCKET_OVF_MAX_SIZE 10000
#define BLOCKS_MAX_SIZE ( ( ( 1024 + 256 + 128 ) * 1024 * 1024 ) / sizeof( Block ) )
#define QUEUE_MAX_SIZE 100000

#include <cvt/vision/TSDFVH.inl>

    TSDFVH::TSDFVH( float truncation, float dmax ) :
        _trunc( truncation ),
        _dmax( dmax ),
        _clclearbuckets( _TSDFVH_source, "tsdfvh_clear_buckets" ),
        _clclearuint( _TSDFVH_source, "tsdfvh_clear_uint" ),
        _clfillstack( _TSDFVH_source, "stackui_fill_values" ),
        _clclearblockweight( _TSDFVH_source, "tsdfvh_clear_block" ),
        _clqueue( _TSDFVH_source, "tsdfvh_add_queue" ),
        _cltraverse( _TSDFVH_source, "tsdfvh_traverse_blocks" ),
        _hashtable( NULL ),
        _bucketovf_stack( NULL ),
        _blocks_stack( NULL ),
        _blocks( NULL ),
        _blocks_state( NULL ),
        _queue( NULL  )
    {
        _hashtable = new CLBuffer( sizeof( HashBucket ) * ( BUCKETS_MAX_SIZE + BUCKET_OVF_MAX_SIZE ) );
        _bucketovf_stack = new CLBuffer( sizeof( cl_uint ) * ( BUCKET_OVF_MAX_SIZE + 1 ) );
        _blocks_stack = new CLBuffer( sizeof( cl_uint ) * ( BLOCKS_MAX_SIZE + 1 ) );
        _blocks = new CLBuffer( sizeof( Block ) * ( BLOCKS_MAX_SIZE )  );
        _blocks_state = new CLBuffer( sizeof( cl_uint ) * ( BLOCKS_MAX_SIZE >> 5 ) );
        _queue = new CLBuffer( sizeof( cl_uint ) * ( QUEUE_MAX_SIZE + 1 ) );
        clear();

        std::cout << BUCKETS_MAX_SIZE << std::endl;
    }

    TSDFVH::~TSDFVH()
    {
        if( _hashtable ) delete _hashtable;
        if( _bucketovf_stack ) delete _bucketovf_stack;
        if( _blocks_stack ) delete _blocks_stack;
        if( _blocks ) delete _blocks;
        if( _blocks_state ) delete _blocks_state;
        if( _queue ) delete _queue;
    }

    void TSDFVH::add( const Matrix4f& KRT, const Image& depthmap, const Image& rgb )
    {
        Matrix4f KRTinv = KRT.inverse();

        queueBlocks( _queue, _blocks_stack, _blocks, _blocks_state, _hashtable, _bucketovf_stack, depthmap, KRTinv );
        //checkBlocks();
#if 0
        {
            cl_uint tmp;
            _queue->read( &tmp, sizeof( cl_uint ) );
            std::cout << "Queue: " << tmp - 1 << std::endl;
            //checkQueue( tmp );
        }
#endif
        traverseQueue( _queue, _blocks, _blocks_state, depthmap, rgb, KRT );

        //reset queue size
        cl_uint qsize = 1;
        _queue->write( &qsize, sizeof( cl_uint ) );

        //std::cout << "Blocks: " << blocksUsed() << std::endl;
        std::cout << "Blocks memory: " << blocksUsedSize() / ( 1024 * 1024 ) << " MB" << std::endl;
        //blocksToObj( "blocksvh.obj" );
        //
        //checkBlocks();
    }

    size_t TSDFVH::blocksUsed() const
    {
        if( _blocks_stack ) {
            cl_uint size;
            _blocks_stack->read( &size, sizeof( cl_uint ) );
            return size - 1;
        }
        return 0;
    }

    void TSDFVH::clear()
    {
        cl_uint size;

        // clear all the hash_entries
        clearBuckets( _hashtable, BUCKETS_MAX_SIZE + BUCKET_OVF_MAX_SIZE );
        // fill the stack with the bucket offsets
        // we start after the last bucket entry in the table
        stackFillValues( _bucketovf_stack, BUCKET_OVF_MAX_SIZE + 1, BUCKETS_MAX_SIZE - 1 );
        // fill the stack with the block offsets
        // leave out the dummy nil block
        stackFillValues( _blocks_stack, BLOCKS_MAX_SIZE, 0 );
        checkStack();
        // clear the blocks_state
        clear_uint( _blocks_state, BLOCKS_MAX_SIZE >> 5 );
        // set the queue_size to zero
        size = 1;
        _queue->write( &size, sizeof( cl_uint ) );
        // clear the blocks voxel weight
        clearBlock( _blocks, BLOCKS_MAX_SIZE );

    }

    void TSDFVH::clearBlock( CLBuffer* buffer, size_t size )
    {
        _clclearblockweight.setArg( 0, *buffer );
        _clclearblockweight.setArg<cl_uint>( 1, ( cl_uint ) size );
        _clclearblockweight.run( CLNDRange( size, 256 ), CLNDRange( 1, 256 ) );
    }

    void TSDFVH::clear_uint( CLBuffer* buffer, size_t size )
    {
        _clclearuint.setArg( 0, *buffer );
        _clclearuint.setArg<cl_uint>( 1, ( cl_uint ) size );
        _clclearuint.run( CLNDRange( Math::pad( size, 256 ) ), CLNDRange( 256 ) );
    }

    void TSDFVH::clearBuckets( CLBuffer* buffer, size_t size )
    {
        _clclearbuckets.setArg( 0, *buffer );
        _clclearbuckets.setArg<cl_uint>( 1, ( cl_uint ) size );
        _clclearbuckets.run( CLNDRange( Math::pad( size, 256 ) ), CLNDRange( 256 ) );
    }

    void TSDFVH::stackFillValues( CLBuffer* buffer, size_t size, ssize_t offset )
    {
        _clfillstack.setArg( 0, *buffer );
        _clfillstack.setArg<cl_uint>( 1, ( cl_uint ) size );
        _clfillstack.setArg<cl_int>( 2, ( cl_int ) offset );
        _clfillstack.run( CLNDRange( Math::pad( size, 256 ) ), CLNDRange( 256 ) );
    }

    void TSDFVH::queueBlocks( CLBuffer* queue,
                              CLBuffer* block_stack, CLBuffer* blocks, CLBuffer* block_state,
                             CLBuffer* hashtable, CLBuffer* bucketovf_stack,
                             const Image& depthmap, const Matrix4f& CAM2WORLD )
    {
        CLMatrix4f mat4f( CAM2WORLD );

        _clqueue.setArg( 0, *queue );
        _clqueue.setArg( 1, *block_stack );
        _clqueue.setArg( 2, *blocks );
        _clqueue.setArg( 3, *block_state );
        _clqueue.setArg( 4, *hashtable );
        _clqueue.setArg( 5, *bucketovf_stack );
        _clqueue.setArg( 6, depthmap );
        _clqueue.setArg( 7, sizeof( CLMatrix4f ), &mat4f );
        _clqueue.setArg( 8, _dmax );
        _clqueue.run( CLNDRange( Math::pad( depthmap.width(), 32 ), Math::pad( depthmap.height(), 32 ) ), CLNDRange( 32, 32 ) );
    }

    void TSDFVH::traverseQueue( CLBuffer* queue, CLBuffer* blocks, CLBuffer* blockState,
                               const Image& depthmap, const Image& rgb, const Matrix4f& KRT )
    {
        CLMatrix4f clKRT( KRT );

        cl_uint qsize;
        queue->read( &qsize, sizeof( cl_uint ) );

        _cltraverse.setArg( 0, *queue );
        _cltraverse.setArg( 1, *blocks );
        _cltraverse.setArg( 2, *blockState );
        _cltraverse.setArg( 3, depthmap );
        _cltraverse.setArg( 4, rgb );
        _cltraverse.setArg( 5, sizeof( CLMatrix4f ), &clKRT );
        _cltraverse.setArg( 6, _trunc );
        _cltraverse.setArg( 7, _dmax );
        _cltraverse.run( CLNDRange( qsize - 1, 256 ), CLNDRange( 1, 256 ) );
    }

    void TSDFVH::checkQueue( size_t size )
    {
        cl_uint stackid;
        cl_uint maxblock;

        _blocks_stack->read( &stackid, sizeof( cl_uint ) );
        _blocks_stack->read( &maxblock, sizeof( cl_uint ), sizeof( cl_uint ) * stackid );

        std::cout << "Blocks Stack-Pointer: " << stackid << std::endl;
        std::cout << "Blocks Stack-Pointer Value: " << maxblock << std::endl;

        std::cout << "Checking for valid queue" << std::endl;

        const cl_uint* queue = ( const cl_uint* ) _queue->map();
        for( size_t i = 1; i < size; i++ ) {
            if( queue[ i ] == 0 ) {
                std::cout << "Nil element queued at " << i << std::endl;
            }
            if( queue[ i ] > maxblock ) {
                std::cout << "Block element queued at " << i << " has higher value than stack-pointer " << queue[ i ] << " vs " << maxblock << std::endl;
            }
            for( size_t k = i + 1; k < size; k++ ) {
                if( queue[ i ] == queue[ k ] ) {
                    std::cout << "Double queued entry at " << i << " <> " << k << " with value " << queue[ i ] << std::endl;
                }
            }
        }
        _queue->unmap( ( const void* ) queue );
    }

    void TSDFVH::checkStack()
    {
        const cl_uint* stack = ( const cl_uint* ) _blocks_stack->map();
        if( stack[ 0 ] != 1 )
            std::cout << "Stack-Pointer not set to 1" << std::endl;
        for( size_t i = 1; i < BLOCKS_MAX_SIZE; i++ ) {
            if( stack[ i ] != i )
                std::cout << "Stack-Value " << i << " not valid: " << stack[ i ] << std::endl;
        }
        _blocks_stack->unmap( ( const void* ) stack );
    }

    void TSDFVH::checkBlocks()
    {
        /* check if block are initalized correctly */
#if 0
        std::cout << "Checking block values" << std::endl;
        const Block* blocks = ( const Block* ) _blocks->map();
        for( int i = 1; i < BLOCKS_MAX_SIZE; i++ ) {
            for( short z = 0; z < 7; z++  ) {
                for( short y = 0; y < 7; y++ ) {
                    for( short x = 0; x < 7; x++ ) {
                        if( Math::abs( blocks[ i ].v[ ( z * 8 +  y ) * 8 + x ].dist - 1.0f ) > Math::EPSILONF ) {
                            std::cout << "Block " << i << " dist not inialized to 1.0f" << std::endl;
                            return;
                        }
                        if( blocks[ i ].v[ ( z * 8 +  y ) * 8 + x ].weight != 0 ) {
                            std::cout << "Block " << i << " weight not inialized to 0" << std::endl;
                            //return;
                        }
                        if( blocks[ i ].v[ ( z * 8 +  y ) * 8 + x ].red != 0 ) {
                            std::cout << "Block " << i << " red not inialized to 0" << std::endl;
                        }
                        if( blocks[ i ].v[ ( z * 8 +  y ) * 8 + x ].green != 0 ) {
                            std::cout << "Block " << i << " green not inialized to 0" << std::endl;
                        }
                        if( blocks[ i ].v[ ( z * 8 +  y ) * 8 + x ].blue != 0 ) {
                            std::cout << "Block " << i << " blue not inialized to 0" << std::endl;
                        }

                    }
                }
            }
        }
        _blocks->unmap( ( void* ) blocks );
#endif

        /* check for double blocks*/
#if 0
        std::cout << "Checking for duplicate blocks" << std::endl;
        const Block* blocks = ( const Block* ) _blocks->map();
        size_t N = blocksUsed();
        for( size_t i = 1; i < N - 1; i++ ) {
            for( size_t k = i + 1; k < N; k++ ) {
                if( blocks[ i ].vposi.x == blocks[ k ].vposi.x &&
                    blocks[ i ].vposi.y == blocks[ k ].vposi.y &&
                    blocks[ i ].vposi.z == blocks[ k ].vposi.z ) {
                    std::cout << "Duplicate blocks with same vpos" << std::endl;
                }
            }
        }
        _blocks->unmap( ( void* ) blocks );
#endif
    }

    void TSDFVH::blocksToObj( const String& file )
    {
        int N = blocksUsed();
        const Block* blocks = ( const Block* ) _blocks->map();

        FILE* f = fopen( file.c_str(), "w" );
        // skip the dummy nil
        for( int i = 1; i <= N; i++ ) {
            cl_int3 vposi = blocks[ i ].vposi;

            float x = 8.0f * ( float ) ( vposi.x );
            float y = 8.0f * ( float ) ( vposi.y );
            float z = 8.0f * ( float ) ( vposi.z );
            fprintf( f, "v %f %f %f\n", x, y, z );
            fprintf( f, "v %f %f %f\n", x + 8, y, z );
            fprintf( f, "v %f %f %f\n", x + 8, y + 8, z );
            fprintf( f, "v %f %f %f\n", x, y + 8, z );
            fprintf( f, "v %f %f %f\n", x, y, z + 8 );
            fprintf( f, "v %f %f %f\n", x + 8, y, z + 8 );
            fprintf( f, "v %f %f %f\n", x + 8, y + 8, z + 8 );
            fprintf( f, "v %f %f %f\n", x, y + 8, z + 8 );


        }
#if 0
        for( int i = 0; i < N - 1; i++ ) {
            fprintf( f, "l %d %d\n", i * 8 + 1 + 0   , i * 8 + 1 + 1 );
            fprintf( f, "l %d %d\n", i * 8 + 1 + 1   , i * 8 + 1 + 2 );
            fprintf( f, "l %d %d\n", i * 8 + 1 + 2   , i * 8 + 1 + 3 );
            fprintf( f, "l %d %d\n", i * 8 + 1 + 3   , i * 8 + 1 + 0 );
            fprintf( f, "l %d %d\n", i * 8 + 1 + 0 + 4 , i * 8 + 1 + 1 + 4 );
            fprintf( f, "l %d %d\n", i * 8 + 1 + 1 + 4 , i * 8 + 1 + 2 + 4 );
            fprintf( f, "l %d %d\n", i * 8 + 1 + 2 + 4 , i * 8 + 1 + 3 + 4 );
            fprintf( f, "l %d %d\n", i * 8 + 1 + 3 + 4 , i * 8 + 1 + 0 + 4 );
            fprintf( f, "l %d %d\n", i * 8 + 1 + 0   , i * 8 + 1 + 0 + 4 );
            fprintf( f, "l %d %d\n", i * 8 + 1 + 1   , i * 8 + 1 + 1 + 4 );
            fprintf( f, "l %d %d\n", i * 8 + 1 + 2   , i * 8 + 1 + 2 + 4 );
            fprintf( f, "l %d %d\n", i * 8 + 1 + 3   , i * 8 + 1 + 3 + 4 );
            //fprintf( f, "f %d %d %d\n", i *4 + 0 , i * 4 + 1, i * 4 + 2 );
            //fprintf( f, "f %d %d %d\n", i *4 + 0 , i * 4 + 2, i * 4 + 3 );
            //fprintf( f, "f %d %d %d\n", i *4 + 0 , i * 4 + 3, i * 4 + 3 );
        }
#else
    for( int i = 0; i < N - 1; i++ ) {
            fprintf( f, "f %d %d %d\n", i * 8 + 1 , i * 8 + 2, i * 8 + 3 );
            fprintf( f, "f %d %d %d\n", i * 8 + 3 , i * 8 + 4, i * 8 + 1 );
            fprintf( f, "f %d %d %d\n", i * 8 + 2 , i * 8 + 6, i * 8 + 7 );
            fprintf( f, "f %d %d %d\n", i * 8 + 7 , i * 8 + 3, i * 8 + 2 );
            fprintf( f, "f %d %d %d\n", i * 8 + 4 , i * 8 + 3, i * 8 + 7 );
            fprintf( f, "f %d %d %d\n", i * 8 + 7 , i * 8 + 8, i * 8 + 4 );
            fprintf( f, "f %d %d %d\n", i * 8 + 1 , i * 8 + 4, i * 8 + 8 );
            fprintf( f, "f %d %d %d\n", i * 8 + 8 , i * 8 + 5, i * 8 + 1 );
            fprintf( f, "f %d %d %d\n", i * 8 + 5 , i * 8 + 6, i * 8 + 7 );
            fprintf( f, "f %d %d %d\n", i * 8 + 7 , i * 8 + 8, i * 8 + 5 );
            fprintf( f, "f %d %d %d\n", i * 8 + 1 , i * 8 + 2, i * 8 + 6 );
            fprintf( f, "f %d %d %d\n", i * 8 + 6 , i * 8 + 5, i * 8 + 1 );
        }

#endif
        fclose( f );

        _blocks->unmap( ( void* ) blocks );
    }


    ssize_t TSDFVH::findBlock( const HashBucket* buckets, const cl_int3& vposi ) const
    {
        cl_uint hashidx = hash( vposi, BUCKETS_MAX_SIZE );
        const HashBucket* bucket = buckets + hashidx;
        const cl_uint BUCKET_LOCK = 0x80000000;

#define FIND_CHECK( i ) do {\
    if( bucket->entries[ i ].block_ptr && \
        bucket->entries[ i ].vposi.x == vposi.x && \
        bucket->entries[ i ].vposi.y == vposi.y && \
        bucket->entries[ i ].vposi.z == vposi.z ) { \
        return bucket->entries[ i ].block_ptr; \
    } \
} while( 0 )

        FIND_CHECK( 0 );
        FIND_CHECK( 1 );
        FIND_CHECK( 2 );
        FIND_CHECK( 3 );

        // we have to check the chain
        while( bucket->bucket_lck_ptr & ( ~BUCKET_LOCK ) ) {
            bucket = buckets + ( bucket->bucket_lck_ptr & ( ~BUCKET_LOCK ) );
            FIND_CHECK( 0 );
            FIND_CHECK( 1 );
            FIND_CHECK( 2 );
            FIND_CHECK( 3 );
        }
#undef FIND_CHECK
        return -1;
    }

    static inline void vertexColorInterp( Vector3f& vtx, const Vector3f& p1, const Vector3f& p2,
                                          Vector3f& col, const Vector3f& col1, const Vector3f& col2,
                                          float val1, float val2 )
    {
        const float ISO_EPSILON = 1e-8f;
        float alpha;

        if( Math::abs( val1 ) <  ISO_EPSILON )
            alpha = 0.0f;
        else if( Math::abs( val2 ) <  ISO_EPSILON )
            alpha = 1.0f;
        else if( Math::abs( val1 - val2 ) <  ISO_EPSILON )
            alpha = 0.0f;
        else
            alpha = -val1 / ( val2 - val1 );

        col.mix( col1, col2, alpha );
        vtx.mix( p1, p2, alpha );
    }

    cl_float TSDFVH::volumeDistance( const Block* blocks, int x, int y, int z,
                                     BlockNeighbours& blknbrs ) const
    {
        int lx = 0 + ( x >> 3 );
        int ly = 0 + ( y >> 3 );
        int lz = 0 + ( z >> 3 );
        int idx = blknbrs.blocks[ lz ][ ly ][ lx ];
        if(  idx > 0 ) {
            return blocks[ idx ].v[ ( z & 0x7 ) * 64 + ( y & 0x7 ) * 8 + ( x & 0x7 ) ].gumm.mu;
        } else
            return 1.0f;
    }

    cl_uchar TSDFVH::volumeWeight( const Block* blocks, int x, int y, int z,
                                   BlockNeighbours& blknbrs ) const
    {
        int lx = 0 + ( x >> 3 );
        int ly = 0 + ( y >> 3 );
        int lz = 0 + ( z >> 3 );
        int idx = blknbrs.blocks[ lz ][ ly ][ lx ];
        if(  idx > 0 ) {
            return blocks[ idx ].v[ ( z & 0x7 ) * 64 + ( y & 0x7 ) * 8 + ( x & 0x7 ) ].gumm.accumall;
        } else
            return 0;
    }

    Vector3f TSDFVH::volumeColor( const Block* blocks, int x, int y, int z,
                                  BlockNeighbours& blknbrs ) const
    {
        float scale = 1.0f / 255.0f;
        int lx = 0 + ( x >> 3 );
        int ly = 0 + ( y >> 3 );
        int lz = 0 + ( z >> 3 );
        int idx = blknbrs.blocks[ lz ][ ly ][ lx ];
        if(  idx > 0 ) {
            float red   = ( float ) blocks[ idx ].v[ ( z & 0x7 ) * 64 + ( y & 0x7 ) * 8 + ( x & 0x7 ) ].red;
            float green = ( float ) blocks[ idx ].v[ ( z & 0x7 ) * 64 + ( y & 0x7 ) * 8 + ( x & 0x7 ) ].green;
            float blue  = ( float ) blocks[ idx ].v[ ( z & 0x7 ) * 64 + ( y & 0x7 ) * 8 + ( x & 0x7 ) ].blue;
            Vector3f ret( red, green, blue );
            return ret * scale;
        } else
            return Vector3f( 0.0f, 0.0f, 0.0f );
    }

    void TSDFVH::toSceneMesh( SceneMesh& mesh ) const
    {
        float   gridval[ 8 ];
        Vector3f gridvtx[ 8 ];
        Vector3f gridcol[ 8 ];
        int cubeindex;
        Vector3f vertlist[ 12 ];
        Vector3f collist[ 12 ];
        size_t vtxIdx = 0;
        std::vector<Vector3f> vertices;
        std::vector<Vector3f> colors;
        std::vector<unsigned int> faces;

        size_t N = blocksUsed();
        const Block* blocks = ( const Block* ) _blocks->map();
        const HashBucket* buckets = ( const HashBucket* ) _hashtable->map();

        BlockNeighbours blknbr;

        for( size_t idx = 1; idx <= N; idx++ ) {
            Vector3f base_pos( 8.0f * ( float ) ( blocks[ idx ].vposi.x ),
                               8.0f * ( float ) ( blocks[ idx ].vposi.y ),
                               8.0f * ( float ) ( blocks[ idx ].vposi.z ) );

            cl_int3 vposi;
            for( int z = 0; z <= 1; z++  ) {
                for( int y = 0; y <= 1; y++ ) {
                    for( int x = 0; x <= 1; x++ ) {
                        vposi = blocks[ idx ].vposi;
                        vposi.x += x;
                        vposi.y += y;
                        vposi.z += z;
                        blknbr.blocks[ z ][ y ][ x ] = findBlock( buckets, vposi );
                    }
                }
            }

            for( int z = 0; z < 8; z++  ) {
                for( int y = 0; y < 8; y++ ) {
                    for( int x = 0; x < 8; x++ ) {

#define VOL_DIST( a, b, c ) volumeDistance( blocks, a, b, c, blknbr )
#define VOL_COLOR( a, b, c ) volumeColor( blocks, a, b, c, blknbr )
#define VOL_WEIGHT( a, b, c ) volumeWeight( blocks, a, b, c, blknbr )

                        /* get the values of the current grid voxel */
#define WEIGHTMIN -0.1

                        if( VOL_WEIGHT( x, y, z ) <= WEIGHTMIN ) continue;
                        if( VOL_WEIGHT( x + 1, y, z ) <= WEIGHTMIN ) continue;
                        if( VOL_WEIGHT( x + 1, y + 1, z ) <= WEIGHTMIN ) continue;
                        if( VOL_WEIGHT( x, y + 1, z ) <= WEIGHTMIN ) continue;
                        if( VOL_WEIGHT( x, y, z + 1 ) <= WEIGHTMIN ) continue;
                        if( VOL_WEIGHT( x + 1, y, z + 1 ) <= WEIGHTMIN ) continue;
                        if( VOL_WEIGHT( x + 1, y + 1, z + 1 ) <= WEIGHTMIN ) continue;
                        if( VOL_WEIGHT( x, y + 1, z + 1 ) <= WEIGHTMIN ) continue;

                        gridval[ 0 ] = VOL_DIST( x  , y , z  );
                        gridval[ 1 ] = VOL_DIST( x + 1, y   , z  );
                        gridval[ 2 ] = VOL_DIST( x + 1, y + 1, z     );
                        gridval[ 3 ] = VOL_DIST( x  , y + 1, z   );
                        gridval[ 4 ] = VOL_DIST( x  , y , z + 1 );
                        gridval[ 5 ] = VOL_DIST( x + 1, y   , z + 1 );
                        gridval[ 6 ] = VOL_DIST( x + 1, y + 1, z + 1 );
                        gridval[ 7 ] = VOL_DIST( x  , y + 1, z + 1 );

                        /*
                           Determine the index into the edge table which
                           tells us which vertices are inside of the surface
                           */
                        cubeindex = 0;
                        if( gridval[ 0 ] < 0.0f ) cubeindex |= 1;
                        if( gridval[ 1 ] < 0.0f ) cubeindex |= 2;
                        if( gridval[ 2 ] < 0.0f ) cubeindex |= 4;
                        if( gridval[ 3 ] < 0.0f ) cubeindex |= 8;
                        if( gridval[ 4 ] < 0.0f ) cubeindex |= 16;
                        if( gridval[ 5 ] < 0.0f ) cubeindex |= 32;
                        if( gridval[ 6 ] < 0.0f ) cubeindex |= 64;
                        if( gridval[ 7 ] < 0.0f ) cubeindex |= 128;

                        /* Cube is entirely in/out of the surface */
                        if ( _edgeTable[ cubeindex ] == 0)
                            continue;

                        /* set the vertex positions of the current cell */
                        Vector3f vpos = base_pos + Vector3f( x, y, z );
                        gridvtx[ 0 ] = vpos + Vector3f( 0.0f, 0.0f, 0.0f );
                        gridvtx[ 1 ] = vpos + Vector3f( 1.0f, 0.0f, 0.0f );
                        gridvtx[ 2 ] = vpos + Vector3f( 1.0f, 1.0f, 0.0f );
                        gridvtx[ 3 ] = vpos + Vector3f( 0.0f, 1.0f, 0.0f );
                        gridvtx[ 4 ] = vpos + Vector3f( 0.0f, 0.0f, 1.0f );
                        gridvtx[ 5 ] = vpos + Vector3f( 1.0f, 0.0f, 1.0f );
                        gridvtx[ 6 ] = vpos + Vector3f( 1.0f, 1.0f, 1.0f);
                        gridvtx[ 7 ] = vpos + Vector3f( 0.0f, 1.0f, 1.0f );

                        gridcol[ 0 ] = VOL_COLOR( x , y , z  );
                        gridcol[ 1 ] = VOL_COLOR( x + 1, y  , z  );
                        gridcol[ 2 ] = VOL_COLOR( x + 1, y + 1, z    );
                        gridcol[ 3 ] = VOL_COLOR( x , y + 1, z   );
                        gridcol[ 4 ] = VOL_COLOR( x , y , z + 1 );
                        gridcol[ 5 ] = VOL_COLOR( x + 1, y  , z + 1 );
                        gridcol[ 6 ] = VOL_COLOR( x + 1, y + 1, z + 1 );
                        gridcol[ 7 ] = VOL_COLOR( x , y + 1, z + 1 );

                        /* Find the vertices where the surface intersects the cube */
                        if ( _edgeTable[ cubeindex ] & 1)
                            vertexColorInterp( vertlist[ 0 ], gridvtx[ 0 ], gridvtx[ 1 ],
                                              collist[ 0 ], gridcol[ 0 ], gridcol[ 1 ],
                                              gridval[ 0 ], gridval[ 1 ] );
                        if ( _edgeTable[ cubeindex ] & 2)
                            vertexColorInterp( vertlist[ 1 ], gridvtx[ 1 ], gridvtx[ 2 ],
                                              collist[ 1 ], gridcol[ 1 ], gridcol[ 2 ],
                                              gridval[ 1 ], gridval[ 2 ] );
                        if ( _edgeTable[ cubeindex ] & 4)
                            vertexColorInterp( vertlist[ 2 ], gridvtx[ 2 ], gridvtx[ 3 ],
                                              collist[ 2 ], gridcol[ 2 ], gridcol[ 3 ],
                                              gridval[ 2 ], gridval[ 3 ] );
                        if ( _edgeTable[ cubeindex ] & 8)
                            vertexColorInterp( vertlist[ 3 ], gridvtx[ 3 ], gridvtx[ 0 ],
                                              collist[ 3 ], gridcol[ 3 ], gridcol[ 0 ],
                                              gridval[ 3 ], gridval[ 0 ] );
                        if ( _edgeTable[ cubeindex ] & 16)
                            vertexColorInterp( vertlist[ 4 ], gridvtx[ 4 ], gridvtx[ 5 ],
                                              collist[ 4 ], gridcol[ 4 ], gridcol[ 5 ],
                                              gridval[ 4 ], gridval[ 5 ] );
                        if ( _edgeTable[ cubeindex ] & 32)
                            vertexColorInterp( vertlist[ 5 ], gridvtx[ 5 ], gridvtx[ 6 ],
                                              collist[ 5 ], gridcol[ 5 ], gridcol[ 6 ],
                                              gridval[ 5 ], gridval[ 6 ] );
                        if ( _edgeTable[ cubeindex ] & 64)
                            vertexColorInterp( vertlist[ 6 ], gridvtx[ 6 ], gridvtx[ 7 ],
                                              collist[ 6 ], gridcol[ 6 ], gridcol[ 7 ],
                                              gridval[ 6 ], gridval[ 7 ] );
                        if ( _edgeTable[ cubeindex ] & 128)
                            vertexColorInterp( vertlist[ 7 ], gridvtx[ 7 ], gridvtx[ 4 ],
                                              collist[ 7 ], gridcol[ 7 ], gridcol[ 4 ],
                                              gridval[ 7 ], gridval[ 4 ] );
                        if ( _edgeTable[ cubeindex ] & 256)
                            vertexColorInterp( vertlist[ 8 ], gridvtx[ 0 ], gridvtx[ 4 ],
                                              collist[ 8 ], gridcol[ 0 ], gridcol[ 4 ],
                                              gridval[ 0 ], gridval[ 4 ] );
                        if ( _edgeTable[ cubeindex ] & 512)
                            vertexColorInterp( vertlist[ 9 ], gridvtx[ 1 ], gridvtx[ 5 ],
                                              collist[ 9 ], gridcol[ 1 ], gridcol[ 5 ],
                                              gridval[ 1 ], gridval[ 5 ] );
                        if ( _edgeTable[ cubeindex ] & 1024)
                            vertexColorInterp( vertlist[ 10 ], gridvtx[ 2 ], gridvtx[ 6 ],
                                              collist[ 10 ], gridcol[ 2 ], gridcol[ 6 ],
                                              gridval[ 2 ], gridval[ 6 ] );
                        if ( _edgeTable[ cubeindex ] & 2048)
                            vertexColorInterp( vertlist[ 11 ], gridvtx[ 3 ], gridvtx[ 7 ],
                                              collist[ 11 ], gridcol[ 3 ], gridcol[ 7 ],
                                              gridval[ 3 ], gridval[ 7 ] );

                        /* Create the triangle */
                        for( int i = 0; _triTable[ cubeindex ][ i ] !=-1; i += 3 ) {
                            vertices.push_back( vertlist[ _triTable[ cubeindex ][ i  ] ] );
                            colors.push_back( collist[ _triTable[ cubeindex ][ i     ] ] );
                            faces.push_back( vtxIdx++ );

                            vertices.push_back( vertlist[ _triTable[ cubeindex ][ i + 1 ] ] );
                            colors.push_back( collist[ _triTable[ cubeindex ][ i + 1 ] ] );
                            faces.push_back( vtxIdx++ );

                            vertices.push_back( vertlist[ _triTable[ cubeindex ][ i + 2 ] ] );
                            colors.push_back( collist[ _triTable[ cubeindex ][ i + 2 ] ] );
                            faces.push_back( vtxIdx++ );
                        }

                    }
                }
            }

        }
        _blocks->unmap( ( void* ) blocks );
        _hashtable->unmap( ( void* ) buckets );

        mesh.clear();
        mesh.setVertices( &vertices[ 0 ], vertices.size() );
        mesh.setColors( &colors[ 0 ], colors.size() );
        mesh.setFaces( &faces[ 0 ], faces.size(), SCENEMESH_TRIANGLES );
    }

}
