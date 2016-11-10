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
#ifndef CVT_TSDFVH_H
#define CVT_TSDFVH_H

#include <cvt/cl/CLKernel.h>
#include <cvt/cl/CLBuffer.h>
#include <cvt/cl/CLGUMM.h>
#include <cvt/math/Matrix.h>
#include <cvt/geom/scene/SceneMesh.h>

namespace cvt
{
    class TSDFVH
    {
        public:
            TSDFVH( float truncation = 24.0f, float dmax = 5000.0f );
            ~TSDFVH();

            void clear();

            void add( const Matrix4f& KRT, const Image& depthmap, const Image& rgb );
            void toSceneMesh( SceneMesh& mesh ) const;

            float truncation() const { return _trunc; }

            size_t blocksUsed() const;
            size_t blocksUsedSize() const;
            size_t blocksSize() const;
            size_t hashtableSize() const;

            void blocksToObj( const String& file );
        private:
            TSDFVH( const TSDFVH& );

            void clear_uint( CLBuffer* buffer, size_t size );
            void clearBuckets( CLBuffer* buffer, size_t size );
            void clearBlock( CLBuffer* buffer, size_t size );
            void stackFillValues( CLBuffer* buffer, size_t size, ssize_t offset = 0 );

            void queueBlocks( CLBuffer* queue,
                              CLBuffer* blockstack, CLBuffer* blocks, CLBuffer* block_state,
                             CLBuffer* hashtable, CLBuffer* bucketovf_stack,
                             const Image& depthmap, const Matrix4f& CAM2WORLD );

            void traverseQueue( CLBuffer* queue, CLBuffer* blocks, CLBuffer* blockState,
                                const Image& depthmap, const Image& rgb, const Matrix4f& KRT );

            void checkBlocks();
            void checkQueue( size_t size );
            void checkStack();

            typedef struct {
                //cl_float  dist;
                CLGUMM    gumm;
                cl_uchar  weight;
                cl_uchar  red, green, blue;
            } Voxel;

            typedef struct {
                cl_int3 vposi;
                Voxel v[ 8 * 8 * 8 ];
            } Block;

            typedef struct {
                cl_uint  block_ptr;
                cl_int3  vposi;
            } HashEntry;

            typedef struct {
                HashEntry entries[ 4 ];
                cl_uint bucket_lck_ptr;
            } HashBucket;

            typedef struct {
              cl_uint blocks[ 2 ][ 2 ][ 2 ];
            } BlockNeighbours;

            ssize_t findBlock( const HashBucket* buckets, const cl_int3& vposi ) const;
            cl_uint hash( const cl_int3& vposi, const cl_uint N ) const;
            cl_float volumeDistance( const Block* blocks, int x, int y, int z, BlockNeighbours& blknbrs ) const;
            cl_uchar volumeWeight( const Block* blocks, int x, int y, int z, BlockNeighbours& blknbrs ) const;
            Vector3f volumeColor( const Block* blocks, int x, int y, int z, BlockNeighbours& blknbrs ) const;

            float   _trunc;
            float   _dmax;
            CLKernel _clclearbuckets;
            CLKernel _clclearuint;
            CLKernel _clfillstack;
            CLKernel _clclearblockweight;
            CLKernel _clqueue;
            CLKernel _cltraverse;

            CLBuffer* _hashtable;
            CLBuffer* _bucketovf_stack;
            CLBuffer* _blocks_stack;
            CLBuffer* _blocks;
            CLBuffer* _blocks_state;
            CLBuffer* _queue;
    };

    inline cl_uint TSDFVH::hash( const cl_int3& vposi, const cl_uint N ) const
    {
        // See Teschner et al. and Nießner et al.
        const cl_uint p0 = 73856093;
        const cl_uint p1 = 19349669;
        const cl_uint p2 = 83492791;
        cl_uint hash = ( ( abs( vposi.x ) * p0 ) ^ ( abs( vposi.y ) * p1 ) ^ ( abs( vposi.z ) *  p2 ) ) % N;
        return hash;
    }

    inline size_t TSDFVH::blocksUsedSize() const
    {
            return blocksUsed() * sizeof( Block );
    }

    inline size_t TSDFVH::blocksSize() const
    {
        if( _blocks ) {
            return _blocks->size();
        }
        return 0;
    }

    inline size_t TSDFVH::hashtableSize() const
    {
        if( _hashtable ) {
            return _hashtable->size();
        }
        return 0;
    }

}
#endif
