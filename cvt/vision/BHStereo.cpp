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

#include <cvt/vision/BHStereo.h>
#include <cvt/util/Exception.h>
#include <cvt/cl/CLBuffer.h>
#include <cvt/cl/kernel/median3.h>

#if 0
#include <cvt/cl/kernel/BHStereo512.h>
#define _BHStereo_source _BHStereo512_source
#define BINDESCTYPE cl_uint16
#else
#include <cvt/cl/kernel/BHStereo.h>
#define BINDESCTYPE cl_uint8
#endif


namespace cvt {

    struct BHBucket {
        cl_uint offset;
        cl_uint size;
    };


    BHStereo::BHStereo( const Image& image, size_t bits, size_t numhash ) :
        _clbrief( _BHStereo_source, "bhstereo_brief32_N" ),
        _clhashclear( _BHStereo_source, "bhstereo_hash_clear" ),
        _clhashcount( _BHStereo_source, "bhstereo_hash_count" ),
        _clhashcalcoffset( _BHStereo_source, "bhstereo_hash_calcoffsets" ),
        _clhashmap( _BHStereo_source, "bhstereo_hash" ),
        _clhashmatch( _BHStereo_source, "bhstereo_hash_match" ),
        _clmedian3( _median3_source, "median3" ),
        _clbuckets( sizeof( BHBucket ) * image.height() * ( 1 << bits ) * numhash ),
        _clmap( sizeof( cl_uint ) * image.width() * image.height() * numhash ),
        _clbindesc( sizeof( BINDESCTYPE ) * image.width() * image.height() ),
        _bits( bits ),
        _numhash( numhash )
    {
#define KX 32
#define KY 32

        brief( _clbindesc, image );

        clear( _clbuckets, ( 1 << bits ) * image.height() * numhash );
        count( _clbuckets, bits, numhash, _clbindesc, image.width(), image.height() );
        calcoffsets( _clbuckets, bits, image.height() * numhash );

        hashmap( _clmap, _clbuckets, bits, numhash, _clbindesc, image.width(), image.height() );

    }

    void BHStereo::brief( CLBuffer& bindesc, const Image& image )
    {
#define BRIEFKX 16
#define BRIEFKY 16
        _clbrief.setArg( 0, bindesc );
        _clbrief.setArg<cl_int>( 1, image.width() );
        _clbrief.setArg<cl_int>( 2, image.height() );
        _clbrief.setArg( 3, image );
        _clbrief.run( CLNDRange( Math::pad( image.width(), BRIEFKX ), Math::pad( image.height(), BRIEFKY ), 4 ), CLNDRange( BRIEFKX, BRIEFKY, 4 ) );
    }

    void BHStereo::clear( CLBuffer& buckets, size_t size )
    {
        _clhashclear.setArg( 0, buckets );
        _clhashclear.setArg<cl_uint>( 1, size );
        _clhashclear.run( CLNDRange( Math::pad( size, KX * KY ) ), CLNDRange( KX * KY ) );
    }

    void BHStereo::count( CLBuffer& buckets, size_t bits, size_t numhash, CLBuffer& bindesc, size_t width, size_t height )
    {
        _clhashcount.setArg( 0, buckets );
        _clhashcount.setArg<cl_uint>( 1, bits );
        _clhashcount.setArg<cl_uint>( 2, numhash );
        _clhashcount.setArg( 3, bindesc );
        _clhashcount.setArg<cl_int>( 4, width );
        _clhashcount.setArg<cl_int>( 5, height );
        _clhashcount.run( CLNDRange( Math::pad( width, KX ), Math::pad( height, KY ) ), CLNDRange( KX, KY ) );
    }

    void BHStereo::calcoffsets( CLBuffer& buckets, size_t bits, size_t height )
    {
        _clhashcalcoffset.setArg( 0, buckets );
        _clhashcalcoffset.setArg<cl_uint>( 1, bits );
        _clhashcalcoffset.setArg<cl_uint>( 2, height );
        _clhashcalcoffset.run( CLNDRange( Math::pad( height, KX ) ), CLNDRange( KX ) );
    }

    void BHStereo::hashmap( CLBuffer& map, CLBuffer& buckets, size_t bits, size_t numhash, CLBuffer& bindesc, size_t width, size_t height )
    {
        _clhashmap.setArg( 0, map );
        _clhashmap.setArg( 1, buckets );
        _clhashmap.setArg<cl_uint>( 2, bits );
        _clhashmap.setArg<cl_uint>( 3, numhash );
        _clhashmap.setArg( 4, bindesc );
        _clhashmap.setArg<cl_int>( 5, width );
        _clhashmap.setArg<cl_int>( 6, height );
        _clhashmap.run( CLNDRange( Math::pad( width, KX ), Math::pad( height, KY ) ), CLNDRange( KX, KY ) );
    }


    void BHStereo::match( Image& disparity, const Image& image, size_t max_disparity, bool median ) const
    {
        disparity.reallocate( image.width(), image.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );

        _clhashmatch.setArg( 0, disparity );
        _clhashmatch.setArg( 1, _clbuckets );
        _clhashmatch.setArg<cl_uint>( 2, _bits );
        _clhashmatch.setArg<cl_uint>( 3, _numhash );
        _clhashmatch.setArg( 4, _clbindesc );
        _clhashmatch.setArg( 5, _clmap );
        _clhashmatch.setArg( 6, image );
        _clhashmatch.setArg<cl_int>( 7, ( cl_int ) max_disparity );
        _clhashmatch.runWait( CLNDRange( Math::pad( image.width(), KX ), Math::pad( image.height(), KY ) ), CLNDRange( KX, KY ) );

        if( median )
            median3( disparity, disparity );
    }

    void BHStereo::median3( Image& out, const Image& in ) const
    {
        out.reallocate( in.width(), in.height(), in.format(), IALLOCATOR_CL );
        _clmedian3.setArg( 0, out );
        _clmedian3.setArg( 1, in );
        _clmedian3.setArg( 2, CLLocalSpace( sizeof( cl_float4 ) * ( KX + 2 ) * ( KY + 2 ) ) );
        _clmedian3.runWait( CLNDRange( Math::pad( in.width(), KX ), Math::pad( in.height(), KY ) ), CLNDRange( KX, KY ) );
    }
}
