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

#include <cvt/gfx/IDecompose.h>
#include <cvt/gfx/IMapScoped.h>
#include <cvt/util/SIMD.h>

namespace cvt {
	template<typename TYPE>
	static void decompose4Template( Image& c1, Image& c2, Image& c3, Image& c4, const Image& src,
                                    void ( SIMD::*decompose )( TYPE*, TYPE*, TYPE*, TYPE*, const TYPE*, size_t ) const
								 )
    {
        SIMD* simd = SIMD::instance();
		size_t w = src.width();
		size_t h = src.height();

        IMapScoped<TYPE> mapc1( c1 );
        IMapScoped<TYPE> mapc2( c2 );
        IMapScoped<TYPE> mapc3( c3 );
        IMapScoped<TYPE> mapc4( c4 );
		IMapScoped<const TYPE> mapsrc( src );

		for( size_t y = 0; y < h ; y++ ) {
			( simd->*decompose )( mapc1.ptr(), mapc2.ptr(), mapc3.ptr(), mapc4.ptr(), mapsrc.ptr(), w );
            mapc1++;
            mapc2++;
            mapc3++;
            mapc4++;
            mapsrc++;
        }
    }

	template<typename TYPE>
	static void decompose3Template( Image& c1, Image& c2, Image& c3, const Image& src,
                                    void ( SIMD::*decompose )( TYPE*, TYPE*, TYPE*, const TYPE*, size_t ) const
								 )
    {
        SIMD* simd = SIMD::instance();
		size_t w = src.width();
		size_t h = src.height();

        IMapScoped<TYPE> mapc1( c1 );
        IMapScoped<TYPE> mapc2( c2 );
        IMapScoped<TYPE> mapc3( c3 );
		IMapScoped<const TYPE> mapsrc( src );

		for( size_t y = 0; y < h ; y++ ) {
			( simd->*decompose )( mapc1.ptr(), mapc2.ptr(), mapc3.ptr(), mapsrc.ptr(), w );
            mapc1++;
            mapc2++;
            mapc3++;
            mapsrc++;
        }
    }

	template<typename TYPE>
	static void decompose2Template( Image& c1, Image& c2, const Image& src,
                                    void ( SIMD::*decompose )( TYPE*, TYPE*, const TYPE*, size_t ) const
								 )
    {
        SIMD* simd = SIMD::instance();
		size_t w = src.width();
		size_t h = src.height();

        IMapScoped<TYPE> mapc1( c1 );
        IMapScoped<TYPE> mapc2( c2 );
		IMapScoped<const TYPE> mapsrc( src );

		for( size_t y = 0; y < h ; y++ ) {
			( simd->*decompose )( mapc1.ptr(), mapc2.ptr(), mapsrc.ptr(), w );
            mapc1++;
            mapc2++;
            mapsrc++;
        }
    }

    void IDecompose::decompose( Image& chan1, Image& chan2, Image& chan3, Image& chan4, const Image& input )
    {
        if( input.channels() != 4 )
            throw CVTException( "Can't decompose iamge with less than 4 channels into 4 channels!" );

        if( input.format().type == IFORMAT_TYPE_FLOAT ) {
            chan1.reallocate( input.width(), input.height(), IFormat::GRAY_FLOAT, input.memType() );
            chan2.reallocate( input.width(), input.height(), IFormat::GRAY_FLOAT, input.memType() );
            chan3.reallocate( input.width(), input.height(), IFormat::GRAY_FLOAT, input.memType() );
            chan4.reallocate( input.width(), input.height(), IFormat::GRAY_FLOAT, input.memType() );
            decompose4Template<float>( chan1, chan2, chan3, chan4, input, &SIMD::Decompose_4f );
        } else if( input.format().type == IFORMAT_TYPE_UINT8 ) {
            chan1.reallocate( input.width(), input.height(), IFormat::GRAY_UINT8, input.memType() );
            chan2.reallocate( input.width(), input.height(), IFormat::GRAY_UINT8, input.memType() );
            chan3.reallocate( input.width(), input.height(), IFormat::GRAY_UINT8, input.memType() );
            chan4.reallocate( input.width(), input.height(), IFormat::GRAY_UINT8, input.memType() );
            decompose4Template<uint8_t>( chan1, chan2, chan3, chan4, input, &SIMD::Decompose_4u8 );
        } else
            throw CVTException( "Image format not supported for decomposition!" );

    }

    void IDecompose::decompose( Image& chan1, Image& chan2, Image& chan3, const Image& input )
    {
        if( input.channels() != 4 )
            throw CVTException( "Can't decompose iamge with less than 4 channels into 3 channels!" );

        if( input.format().type == IFORMAT_TYPE_FLOAT ) {
            chan1.reallocate( input.width(), input.height(), IFormat::GRAY_FLOAT, input.memType() );
            chan2.reallocate( input.width(), input.height(), IFormat::GRAY_FLOAT, input.memType() );
            chan3.reallocate( input.width(), input.height(), IFormat::GRAY_FLOAT, input.memType() );
            decompose3Template<float>( chan1, chan2, chan3, input, &SIMD::Decompose_4f_to_3f );
        } else if( input.format().type == IFORMAT_TYPE_UINT8 ) {
            chan1.reallocate( input.width(), input.height(), IFormat::GRAY_UINT8, input.memType() );
            chan2.reallocate( input.width(), input.height(), IFormat::GRAY_UINT8, input.memType() );
            chan3.reallocate( input.width(), input.height(), IFormat::GRAY_UINT8, input.memType() );
            decompose3Template<uint8_t>( chan1, chan2, chan3, input, &SIMD::Decompose_4u8_to_3u8 );
        } else
            throw CVTException( "Image format not supported for decomposition!" );
    }

    void IDecompose::decompose( Image& chan1, Image& chan2, const Image& input )
    {
        if( input.channels() != 2 )
            throw CVTException( "Can't decompose iamge with less than 2 channels into 2 channels!" );

        if( input.format().type == IFORMAT_TYPE_FLOAT ) {
            chan1.reallocate( input.width(), input.height(), IFormat::GRAY_FLOAT, input.memType() );
            chan2.reallocate( input.width(), input.height(), IFormat::GRAY_FLOAT, input.memType() );
            decompose2Template<float>( chan1, chan2, input, &SIMD::Decompose_2f );
        } else if( input.format().type == IFORMAT_TYPE_UINT8 ) {
            chan1.reallocate( input.width(), input.height(), IFormat::GRAY_UINT8, input.memType() );
            chan2.reallocate( input.width(), input.height(), IFormat::GRAY_UINT8, input.memType() );
            decompose2Template<uint8_t>( chan1, chan2, input, &SIMD::Decompose_2u8 );
        } else
            throw CVTException( "Image format not supported for decomposition!" );
    }

}
