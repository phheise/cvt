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
#include <cvt/vision/BHFlow.h>
#include <cvt/util/Exception.h>
#include <cvt/cl/CLBuffer.h>
#include <cvt/cl/kernel/median3.h>

#if 0
#include <cvt/cl/kernel/BHStereo512.h>
#define _BHStereo_source _BHStereo512_source
#define BINDESCTYPE cl_uint16
#else
#include <cvt/cl/kernel/BHFlow.h>
#define BINDESCTYPE cl_uint8
#endif


namespace cvt {

    struct BHBucket {
        cl_uint offset;
        cl_uint size;
    };


    BHFlow::BHFlow( const Image& image, size_t bits, size_t numhash, size_t hashwidth, size_t hashheight ) :
        _clbrief( _BHFlow_source, "bhflow_brief32_N" ),
        _clhashclear( _BHFlow_source, "bhflow_hash_clear" ),
        _clhashcount( _BHFlow_source, "bhflow_hash_count" ),
        _clhashcalcoffset( _BHFlow_source, "bhflow_hash_calcoffsets" ),
        _clhashmap( _BHFlow_source, "bhflow_hash" ),
        _clhashmatch( _BHFlow_source, "bhflow_hash_match" ),
        _clmedian3( _median3_source, "median3" ),
        _clbuckets( sizeof( BHBucket ) * ( ( image.width() + hashwidth - 1 ) / hashwidth ) *
                                         ( ( image.height() + hashheight - 1 ) / hashheight ) * ( 1 << bits ) * numhash ),
        _clmap( sizeof( cl_uint2 ) *( ( image.width() + hashwidth - 1 ) / hashwidth ) *
                                    ( ( image.height() + hashheight - 1 ) / hashheight ) * hashwidth * hashheight * numhash ),
        _clbindesc( sizeof( BINDESCTYPE ) * image.width() * image.height() ),
        _bits( bits ),
        _numhash( numhash ),
        _hashwidth( hashwidth ),
        _hashheight( hashheight )
    {
#define KX 32
#define KY 32

        brief( _clbindesc, image );

        clear( _clbuckets, ( ( image.width() + _hashwidth - 1 ) / _hashwidth ) *
                           ( ( image.height() + _hashheight - 1 ) / _hashheight ) * ( 1 << _bits ) * _numhash );

        count( _clbuckets, bits, numhash, hashwidth, hashheight, _clbindesc, image.width(), image.height() );

        calcoffsets( _clbuckets, bits, numhash, hashwidth, hashheight, image.width(), image.height() );
        hashmap( _clmap, _clbuckets, bits, numhash, hashwidth, hashheight, _clbindesc, image.width(), image.height() );

        //checkHashmap( _clbuckets, image.width(), image.height() );
    }

    void BHFlow::brief( CLBuffer& bindesc, const Image& image ) const
    {
#define BRIEFKX 16
#define BRIEFKY 16
        _clbrief.setArg( 0, bindesc );
        _clbrief.setArg<cl_int>( 1, image.width() );
        _clbrief.setArg<cl_int>( 2, image.height() );
        _clbrief.setArg( 3, image );
        _clbrief.run( CLNDRange( Math::pad( image.width(), BRIEFKX ), Math::pad( image.height(), BRIEFKY ), 4 ), CLNDRange( BRIEFKX, BRIEFKY, 4 ) );
    }

    void BHFlow::clear( CLBuffer& buckets, size_t size )
    {
        _clhashclear.setArg( 0, buckets );
        _clhashclear.setArg<cl_uint>( 1, size );
        _clhashclear.run( CLNDRange( Math::pad( size, KX * KY ) ), CLNDRange( KX * KY ) );
    }

    void BHFlow::count( CLBuffer& buckets, size_t bits, size_t numhash, size_t hashwidth, size_t hashheight, CLBuffer& bindesc, size_t width, size_t height )
    {
        _clhashcount.setArg( 0, buckets );
        _clhashcount.setArg<cl_uint>( 1, bits );
        _clhashcount.setArg<cl_uint>( 2, numhash );
        _clhashcount.setArg<cl_uint>( 3, hashwidth );
        _clhashcount.setArg<cl_uint>( 4, hashheight );
        _clhashcount.setArg( 5, bindesc );
        _clhashcount.setArg<cl_int>( 6, width );
        _clhashcount.setArg<cl_int>( 7, height );
        _clhashcount.run( CLNDRange( Math::pad( width, KX ), Math::pad( height, KY ) ), CLNDRange( KX, KY ) );
    }

    void BHFlow::calcoffsets( CLBuffer& buckets, size_t bits, size_t numhash, size_t hashwidth, size_t hashheight, size_t width, size_t height )
    {
        size_t bwidth = ( width + hashwidth - 1 ) / hashwidth;
        size_t bheight = ( height + hashheight - 1 ) / hashheight;

        _clhashcalcoffset.setArg( 0, buckets );
        _clhashcalcoffset.setArg<cl_uint>( 1, bits );
        _clhashcalcoffset.setArg<cl_uint>( 2, numhash );
        _clhashcalcoffset.setArg<cl_uint>( 3, hashwidth );
        _clhashcalcoffset.setArg<cl_uint>( 4, hashheight );
        _clhashcalcoffset.setArg<cl_uint>( 5, bwidth );
        _clhashcalcoffset.setArg<cl_uint>( 6, bheight );
        _clhashcalcoffset.run( CLNDRange( Math::pad( bwidth, 16 ), Math::pad( bheight, 16 ), Math::pad( numhash, 4 ) ), CLNDRange( 16, 16, 4 ) );
    }

    void BHFlow::hashmap( CLBuffer& map, CLBuffer& buckets, size_t bits, size_t numhash, size_t hashwidth, size_t hashheight, CLBuffer& bindesc, size_t width, size_t height )
    {
        _clhashmap.setArg( 0, map );
        _clhashmap.setArg( 1, buckets );
        _clhashmap.setArg<cl_uint>( 2, bits );
        _clhashmap.setArg<cl_uint>( 3, numhash );
        _clhashmap.setArg<cl_uint>( 4, hashwidth );
        _clhashmap.setArg<cl_uint>( 5, hashheight );
        _clhashmap.setArg( 6, bindesc );
        _clhashmap.setArg<cl_int>( 7, width );
        _clhashmap.setArg<cl_int>( 8, height );
        _clhashmap.run( CLNDRange( Math::pad( width, 32 ), Math::pad( height, 16 ), 2 ), CLNDRange( 32, 16, 2 ) );
    }


    void BHFlow::match( Image& flow, const Image& image, bool median ) const
    {
        flow.reallocate( image.width(), image.height(), IFormat::GRAYALPHA_FLOAT, IALLOCATOR_CL );

        CLBuffer brief2( sizeof( BINDESCTYPE ) * image.width() * image.height() );
        brief( brief2, image );

        _clhashmatch.setArg( 0, flow );
        _clhashmatch.setArg( 1, _clbuckets );
        _clhashmatch.setArg<cl_uint>( 2, _bits );
        _clhashmatch.setArg<cl_uint>( 3, _numhash );
        _clhashmatch.setArg<cl_uint>( 4, _hashwidth );
        _clhashmatch.setArg<cl_uint>( 5, _hashheight );
        _clhashmatch.setArg( 6, _clbindesc );
        _clhashmatch.setArg( 7, _clmap );
        _clhashmatch.setArg( 8, brief2 );
        _clhashmatch.setArg<cl_int>( 9, image.width() );
        _clhashmatch.setArg<cl_int>( 10, image.height() );
        _clhashmatch.run( CLNDRange( Math::pad( image.width(), KX ), Math::pad( image.height(), KY ) ), CLNDRange( KX, KY ) );

        if( median )
            median3( flow, flow );
    }

    void BHFlow::median3( Image& out, const Image& in ) const
    {
        out.reallocate( in.width(), in.height(), in.format(), IALLOCATOR_CL );
        _clmedian3.setArg( 0, out );
        _clmedian3.setArg( 1, in );
        _clmedian3.setArg( 2, CLLocalSpace( sizeof( cl_float4 ) * ( KX + 2 ) * ( KY + 2 ) ) );
        _clmedian3.runWait( CLNDRange( Math::pad( in.width(), KX ), Math::pad( in.height(), KY ) ), CLNDRange( KX, KY ) );
    }


    void BHFlow::checkHashmap( CLBuffer& map, int width, int height )
    {
        size_t maxoffset = ( ( width + _hashwidth - 1 ) / _hashwidth ) *
                           ( ( height + _hashheight - 1 ) / _hashheight ) * _hashwidth * _hashheight * _numhash;
        std::cout << "max. offset: " << maxoffset << std::endl;

        const BHBucket* buckets = ( const BHBucket* ) map.map();
        size_t end = ( ( width + _hashwidth - 1 ) / _hashwidth ) *
                     ( ( height + _hashheight - 1 ) / _hashheight ) * ( 1 << _bits ) * _numhash;


        std::cout << "Image: " << width << " x " << height << std::endl;

        size_t bwidth = ( width + _hashwidth - 1 ) / _hashwidth;
        size_t bheight = ( height + _hashheight - 1 ) / _hashheight;

        std::cout << "Buckets: " << bwidth << " x " << bheight << std::endl;

        for( size_t bx = 0; bx < bwidth; bx++ ) {
            for( size_t by = 0; by < bheight; by++ ) {
                for( size_t nhash = 0; nhash < _numhash; nhash++ ) {
                    size_t index = ( bwidth * by + bx ) * ( 1 << _bits ) * _numhash + ( 1 << _bits ) * nhash;
                    size_t sum = 0;
                    for( size_t i = 0 ; i < ( 1 << _bits ); i++ ) {

                        //std::cout << " ( " << bx << " " << by << " " << nhash << " " << i << " ) = " << buckets[ index + i ].offset << std::endl;
                        //std::cout << " ( " << bx << " " << by << " " << nhash << " " << i << " ) = " << buckets[ index + i ].size << std::endl;
                        sum += buckets[ index + i ].size;
                        if( buckets[ index + i ].size > _hashwidth * _hashheight ) {
                            std::cout << "Invalid bucket size!" << " ( " << bx << " " << by << " " << nhash << " " << i << " ) = " << buckets[ index + i ].size << std::endl;
                        }

                        if( buckets[ index + i ].offset > maxoffset ) {
                            std::cout << "Invalid offset!" << " ( " << bx << " " << by << " " << nhash << " " << i << " ) = " << buckets[ index + i ].size << std::endl;
                        }
                    }
                    if( sum != _hashwidth * _hashheight &&  bx + 1 != bwidth && by + 1 != bheight  ) {
                            std::cout << "Invalid sum!" << " ( " << bx << " " << by << " " << nhash << " ) = " << sum << std::endl;
                    }
                }
            }
        }

        //for( size_t i = 0; i < end; i++ ) {
        //    if( buckets[ i ].size > _hashwidth * _hashheight )
        //        std::cout << "Invalid bucket size" << std::endl;
        //    if( buckets[ i ].offset > maxoffset )
        //        std::cout << "Invalid offset" << std::endl;
        //}

        map.unmap( ( void* ) buckets );
    }
}
