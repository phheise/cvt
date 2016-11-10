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

#ifndef TSDFVH_CL
#define TSDFVH_CL

#import "cvt/cl/kernel/Matrix4.cl"
#import "cvt/cl/kernel/Stack.cl"
#import "cvt/cl/kernel/GUMM.cl"

#define BUCKETS_MAX_SIZE 1000003
#define QUEUE_MAX_SIZE 30000
// Block size
#define BLOCK_SIZE 8
#define NUM_MAX_TRIES 32

const sampler_t SAMPLER_NN       = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
const sampler_t SAMPLER_BILINEAR = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

typedef struct {
    //float  dist;
    GUMM   gumm;
    uchar  weight;
    uchar  red, green, blue;
} Voxel;

typedef struct {
    int3 vposi;
    Voxel v[ 8 * 8 * 8 ];
} Block;

typedef struct {
    uint  block_ptr;
    int3  vposi;
} HashEntry;

typedef struct {
    HashEntry entries[ 4 ];
    uint bucket_lck_ptr;
} HashBucket;

#define BUCKET_LOCK 0x80000000

/* general ops */
inline uint tsdfvh_hash( int3 vposi, const uint N )
{
    // See Teschner et al. and Nießner et al.
    const uint p0 = 73856093;
    const uint p1 = 19349669;
    const uint p2 = 83492791;
    uint hash = ( ( abs( vposi.x ) * p0 ) ^ ( abs( vposi.y ) * p1 ) ^ ( abs( vposi.z ) *  p2 ) ) % N;
    return hash;
}

inline int3 tsdfvh_world_to_block( float3 pos )
{
    // we assume here an resolution of 1mm per voxel and therefore 8mm per block
    // the input value is in milimeter
    // we divide and floor
    int3 vposi = convert_int3_rtn( pos / ( float ) BLOCK_SIZE );
    return vposi;
}

inline float3 tsdfvh_block_to_world( int3 vposi )
{
    float3 ret = convert_float3( vposi );
    return ret * ( float ) BLOCK_SIZE;
}

/* block operations */
kernel void tsdfvh_traverse_blocks( global read_only uint* queue, global Block* blocks, global uint* block_state,
                                    read_only image2d_t depthmap, read_only image2d_t colorimg,
                                    const Mat4f KRT, float truncation, float dmax )
{
    int id          = get_global_id( 0 );
    int width        = get_image_width( depthmap );
    int height      = get_image_height( depthmap );
    float2 offsethalf = ( float2 ) 0.5f;
    uint queue_size   = queue[ 0 ];
    local bool visit;

    if( id >= queue_size - 1 )
        return;

#define LINIDX_TO_XYZ( linidx ) ( int3 ) ( linidx & 0x07, ( linidx >> 3 ) & 0x07, ( linidx >> 6 ) & 0x07 )

    // get the pointer out of the queue
    uint block_ptr = queue[ id + 1 ];
    if( !block_ptr )
        return;
    // get the block itself
    global Block* block = blocks + block_ptr;
    // set the block state to not queued
    if( get_local_id( 1 ) == 0 ) {
        visit = atomic_and( block_state + ( block_ptr >> 5 ), ~( 1 << ( block_ptr & 0x1f ) ) ) & ( 1 << ( block_ptr & 0x1f ) );
    }
    barrier( CLK_LOCAL_MEM_FENCE );
    if( !visit )
        return;

    // get the base coords of the block
    float3 base = tsdfvh_block_to_world( block->vposi );

    for( uint linidx = get_local_id( 1 ); linidx < 512; linidx += get_local_size( 1 ) ) {
        int3 xyzidx = LINIDX_TO_XYZ( linidx );
        float3 pos3d = base + convert_float3( xyzidx );
        /* use the transformation to transform grid position to camera coordinates*/
        float3 gpos = mat4f_transform( &KRT, ( float4 ) ( pos3d, 1.0f ) ).xyz;
        /* project into camera */
        float2 ipos = gpos.xy / gpos.z;

        if( ipos.x < width && ipos.y < height && ipos.x >= 0 && ipos.y >= 0  && gpos.z > 0 ) {
            //float d = read_imagef( depthmap, SAMPLER_BILINEAR, ipos + offsethalf ).x;
            float d = read_imagef( depthmap, SAMPLER_NN, convert_int2_rte( ipos ) ).x;
            if( d > 0 && d < dmax && isfinite( d ) ) {
                float3 rgb = read_imagef( colorimg, SAMPLER_BILINEAR, ipos + offsethalf ).xyz;
                float sdf = d - gpos.z;
                float tsdf = clamp( sdf / truncation, -1.0f, 1.0f );
                if( sdf >= -truncation ) {
                    global Voxel* v = block->v + linidx;
                    float wn  = exp( pow( 1.0f - ( d / dmax ), 2.0f ) * 3.0f );
                    float weight = GUMM_update_global( &v->gumm, tsdf, wn, 0.5f );

                    float3 rgbold = ( float3 ) ( v->red, v->green, v->blue ) / 255.0f;

                    uchar3 rgbn = convert_uchar3_sat_rte( clamp( 255.0f * ( ( rgbold * ( v->gumm.accumg - weight ) + rgb * weight ) / ( v->gumm.accumg ) ), 0.0f, 255.0f ) );
                    v->red = rgbn.x;
                    v->green = rgbn.y;
                    v->blue = rgbn.z;
                }
            }
        }
    }
#undef LINIDX_TO_XYZ
}

/* entry operations */

inline void tsdfvh_add_entry( global HashBucket* buckets, const int3 vposi, const uint blk_pointer, global Stackui* bucket_stack )
{
    uint hash = tsdfvh_hash( vposi, BUCKETS_MAX_SIZE );
    global HashBucket* bucket = buckets + hash;

#define ADD_CHECK( i ) do { \
        if( !bucket->entries[ i ].block_ptr ) { \
            bucket->entries[ i ].vposi = vposi; \
            bucket->entries[ i ].block_ptr = blk_pointer; \
            return; \
        } \
    } while( 0 )

    ADD_CHECK( 0 );
    ADD_CHECK( 1 );
    ADD_CHECK( 2 );
    ADD_CHECK( 3 );

    // we have to check the chain
    while( bucket->bucket_lck_ptr & ( ~BUCKET_LOCK ) ) {
        bucket = buckets + ( bucket->bucket_lck_ptr & ( ~BUCKET_LOCK ) );
        ADD_CHECK( 0 );
        ADD_CHECK( 1 );
        ADD_CHECK( 2 );
        ADD_CHECK( 3 );
    }

    // we need to add element to the chain
    uint newbucket = stackui_pop( bucket_stack );
    global HashBucket* nbucket = buckets + newbucket;
    // the HashEntries and the bucket_lck_ptr are set to zero
    nbucket->bucket_lck_ptr = 0;
    nbucket->entries[ 0 ].vposi = vposi;
    nbucket->entries[ 0 ].block_ptr = blk_pointer;

    // we set bucket_lck_ptr to the newbucket and keep the lock if it is there
    // we use the atomic to have an consistent state for the readers who do not make use of the lock
    atomic_xchg( &bucket->bucket_lck_ptr, newbucket | ( bucket->bucket_lck_ptr & BUCKET_LOCK ) );
    //bucket->bucket_lck_ptr = newbucket | ( bucket->bucket_lck_ptr & BUCKET_LOCK );

    #undef ADD_CHECK
}

inline bool tsdfvh_find( global read_only HashBucket* buckets, uint* ret_block_ptr, const int3 vposi )
{
    uint hash = tsdfvh_hash( vposi, BUCKETS_MAX_SIZE );
    global read_only HashBucket* bucket = buckets + hash;

#define FIND_CHECK( i ) do {\
        if( bucket->entries[ i ].block_ptr && all( bucket->entries[ i ].vposi == vposi ) ) { \
            *ret_block_ptr = bucket->entries[ i ].block_ptr; /*( ( global const HashEntry* ) &buckets[ hash ].entries[ i ] ) - ( ( global const HashEntry* ) buckets )*/; \
            return true; \
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
    return false;
}

kernel void tsdfvh_clear_buckets( global HashBucket* buckets, uint size )
{
    int id = get_global_id( 0 );

    if( id >= size )
        return;

    // set all pointer/locks to zero
    buckets[ id ].entries[ 0 ].block_ptr = 0;
    buckets[ id ].entries[ 1 ].block_ptr = 0;
    buckets[ id ].entries[ 2 ].block_ptr = 0;
    buckets[ id ].entries[ 3 ].block_ptr = 0;
    buckets[ id ].bucket_lck_ptr = 0;
}

kernel void tsdfvh_clear_uint( global uint* array, uint size )
{
    int id = get_global_id( 0 );

    if( id >= size )
        return;

    array[ id ] = 0;
}

kernel void tsdfvh_clear_block( global Block* blocks, uint size )
{
    int id = get_global_id( 0 );

    if( id >= size )
        return;

    global Block* block = blocks + id;

    for( uint linidx = get_local_id( 1 ); linidx < 512; linidx += get_local_size( 1 ) ) {
//      block->v[ linidx ].dist = 1.0f;
//      block->v[ linidx ].weight = 0;
        GUMM_init_global( &block->v[ linidx ].gumm, 1.0f, 1.0f, 0.8f, 1.0f );
        block->v[ linidx ].red = 0;
        block->v[ linidx ].green = 0;
        block->v[ linidx ].blue = 0;
    }
}


kernel void tsdfvh_raycast( write_only image2d_t depthmap, global HashBucket* buckets, global Block* blocks, const Mat4f CAM2WORLD, float dmax, float truncation )
{
    const int width     = get_image_width( depthmap );
    const int height    = get_image_height( depthmap );
    const int2 coord    = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );

    if( coord.x >= width || coord.y >= height )
        return;

    float3 pt3d_start = mat4f_transform( &CAM2WORLD, ( float4 ) ( 0.0f, 0.0f, 0.0f, 1.0f ) ).xyz;
    float3 pt3d_end   = mat4f_transform( &CAM2WORLD, ( float4 ) ( dmax * ( float ) ( coord.x ), dmax * ( float ) ( coord.y ), dmax, 1.0f ) ).xyz;
    float3 dir      = normalize( pt3d_end - pt3d_start ) * truncation;

    int send = convert_int_rtp( length( pt3d_end - pt3d_start ) / truncation ) + 1;
    float dist_prev;
    for( int s = 0; s < send; s++ ) {

    }

}

kernel void tsdfvh_add_queue( global uint* queue,
                              global Stackui* block_stack, global Block* blocks, global uint* block_state,
                              global HashBucket* buckets, global Stackui* bucket_stack,
                              read_only image2d_t depthmap, const Mat4f CAM2WORLD, float dmax )
{
    const int width     = get_image_width( depthmap );
    const int height    = get_image_height( depthmap );
    const int2 coord    = ( int2 ) ( get_global_id( 0 ), get_global_id( 1 ) );
    const int lx        = get_local_id( 0 );
    const int ly        = get_local_id( 1 );
    uint hash_prev = ~0;
    uint block_ptr;
    bool skip = false;
    local uint hashes[ 32 ][ 32 ];
    uint valid = 0;

    if( coord.x >= width || coord.y >= height )
        valid = ~0;

    float depth = read_imagef( depthmap, SAMPLER_NN, coord ).x;
    if( depth <= 0.0f || depth > dmax )
        valid = ~0;

    // calculate 3d point
    float3 pt3do  = mat4f_transform( &CAM2WORLD, ( float4 ) ( 0.0f, 0.0f, 0.0f, 1.0f ) ).xyz;
    float3 pt3d  = mat4f_transform( &CAM2WORLD, ( float4 ) ( depth * ( float ) ( coord.x ), depth * ( float ) ( coord.y ), depth, 1.0f ) ).xyz;
    float3 dir = normalize( pt3d - pt3do ) * 8.5f;

    for( int i = -1; i <= 1; i++ ) {
        int3 vposi = tsdfvh_world_to_block( pt3d + dir * ( float ) i );
        uint hash  = tsdfvh_hash( vposi, BUCKETS_MAX_SIZE );

        skip = valid;
        skip = skip || ( hash == hash_prev );
        hash_prev = hash;

        hashes[ ly ][ lx ] = hash | valid;

        barrier( CLK_GLOBAL_MEM_FENCE );

        skip = skip || ( hashes[ ly ][ ( lx + 1 ) & 0x1f ] == hash ) || ( hashes[ ( ly + 1 ) & 0x1f ][ lx ] == hash ) || ( hashes[ ( ly + 1 ) & 0x1f ][ ( lx + 1 ) & 0x1f ] == hash );

        // first we try without a lock
        if( !skip ) {
            if( tsdfvh_find( buckets, &block_ptr, vposi ) ) {
                // we check the queueing state and queue if necessary
                uint bs_offset = block_ptr >> 5; // divide by 32
                uint bs_idx = block_ptr & 0x1f; // modulo 32
                // check if already queued or not
                if( ! ( atomic_or( block_state + bs_offset, 1 << bs_idx ) & ( 1 << bs_idx ) ) ) {
                    uint qidx = atomic_inc( queue );
                    queue[ qidx ] = block_ptr;
                }
                skip = true;
            }
        }
        barrier( CLK_GLOBAL_MEM_FENCE );

        if( !skip ) {
            while( ! ( atomic_or( &buckets[ hash ].bucket_lck_ptr, BUCKET_LOCK ) & BUCKET_LOCK ) ) {
                if( tsdfvh_find( buckets, &block_ptr, vposi ) ) {
                    // we check the queueing state and queue if necessary
                    uint bs_offset = block_ptr >> 5; // divide by 32
                    uint bs_idx = block_ptr & 0x1f; // modulo 32
                    // check if already queued or not
                    if( ! ( atomic_or( block_state + bs_offset, 1 << bs_idx ) & ( 1 << bs_idx ) ) ) {
                        uint qidx = atomic_inc( queue );
                        queue[ qidx ] = block_ptr;
                    }
                } else {
                    // we allocate the block
                    block_ptr = stackui_pop( block_stack );
                    // store the block coordinates
                    blocks[ block_ptr ].vposi = vposi;
                    // add the block to the hash-table
                    tsdfvh_add_entry( buckets, vposi, block_ptr, bucket_stack );
                    // mark the block as queued
                    uint bs_offset = block_ptr >> 5; // divide by 32
                    uint bs_idx = block_ptr & 0x1f; // modulo 32
                    // we also queue the new block if not already done so by one of the readers
                    if( ! ( atomic_or( block_state + bs_offset, 1 << bs_idx ) & ( 1 << bs_idx ) ) ) {
                        uint qidx = atomic_inc( queue );
                        queue[ qidx ] = block_ptr;
                    }
                }
                // we unlock the bucket
                atomic_and( &buckets[ hash ].bucket_lck_ptr, ~BUCKET_LOCK );
                break;
            }
        }
    }

    barrier( CLK_GLOBAL_MEM_FENCE );

    // queue everything in front of the point
    int steps = convert_int_rtp( length( pt3d - pt3do ) / 8.0f );
    for( int s = 1; s < steps; s++ ) {
        int3 vposi = tsdfvh_world_to_block( mix( pt3d, pt3do, ( ( float ) s ) / ( ( float ) ( steps - 1 ) ) ) );
        uint hash = tsdfvh_hash( vposi, BUCKETS_MAX_SIZE );

        skip = valid;
        skip = skip || ( hash == hash_prev );
        hash_prev = hash;

        hashes[ ly ][ lx ] = hash | valid;

        barrier( CLK_GLOBAL_MEM_FENCE );

        skip = skip || ( hashes[ ly ][ ( lx + 1 ) & 0x1f ] == hash ) || ( hashes[ ( ly + 1 ) & 0x1f ][ lx ] == hash ) || ( hashes[ ( ly + 1 ) & 0x1f ][ ( lx + 1 ) & 0x1f ] == hash );

        if( !skip ) {
            if( tsdfvh_find( buckets, &block_ptr, vposi ) ) {
                uint bs_offset = block_ptr >> 5; // divide by 32
                uint bs_idx = block_ptr & 0x1f; // modulo 32
                // check if already queued or not
                if( ! ( atomic_or( block_state + bs_offset, 1 << bs_idx ) & ( 1 << bs_idx ) ) ) {
                    uint qidx = atomic_inc( queue );
                    queue[ qidx ] = block_ptr;
                }
            }
        }
    }

}
#endif
