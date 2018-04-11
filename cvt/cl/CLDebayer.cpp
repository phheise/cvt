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

#include <cvt/cl/CLDebayer.h>
#include <cvt/cl/kernel/debayer.h>
#include <cvt/cl/kernel/debayerhq.h>

namespace cvt {
#define KX 16
#define KY 16

    CLDebayer::CLDebayer() :
        _clrggb( _debayer_source, "debayer_RGGB_RGBA" ),
        _clgbrg( _debayer_source, "debayer_GBRG_RGBA" ),
        _clgrbg( _debayer_source, "debayer_GRBG_RGBA" ),
        _clbggr( _debayer_source, "debayer_BGGR_RGBA" ),
        _clrggbhq( _debayerhq_source, "debayerhq_RGGB_RGBA" ),
        _clgbrghq( _debayerhq_source, "debayerhq_GBRG_RGBA" ),
        _clgrbghq( _debayerhq_source, "debayerhq_GRBG_RGBA" ),
        _clbggrhq( _debayerhq_source, "debayerhq_BGGR_RGBA" )
    {
    }

    CLDebayer::~CLDebayer()
    {
    }

    void CLDebayer::debayer( Image& dst, const Image& src )
    {
        switch( src.format().formatID ) {
            case IFORMAT_BAYER_RGGB_UINT8: debayer_RGGB_RGBA( dst, src );
                                           break;
            case IFORMAT_BAYER_GRBG_UINT8: debayer_GRBG_RGBA( dst, src );
                                           break;
            case IFORMAT_BAYER_GBRG_UINT8: debayer_GBRG_RGBA( dst, src );
                                           break;
            case IFORMAT_BAYER_BGGR_UINT8: debayer_BGGR_RGBA( dst, src );
                                           break;
            default:
                throw CVTException( "Conversion not implemented!" );
        }
    }

    void CLDebayer::debayerHQ( Image& dst, const Image& src )
    {
        switch( src.format().formatID ) {
            case IFORMAT_BAYER_RGGB_UINT8: debayerhq_RGGB_RGBA( dst, src );
                                           break;
            case IFORMAT_BAYER_GRBG_UINT8: debayerhq_GRBG_RGBA( dst, src );
                                           break;
            case IFORMAT_BAYER_GBRG_UINT8: debayerhq_GBRG_RGBA( dst, src );
                                           break;
            case IFORMAT_BAYER_BGGR_UINT8: debayerhq_BGGR_RGBA( dst, src );
                                           break;
            default:
                throw CVTException( "Conversion not implemented!" );
        }
    }



    void CLDebayer::debayer_RGGB_RGBA( Image& dst, const Image& src )
    {
        dst.reallocate( src.width(), src.height(), IFormat::RGBA_UINT8, IALLOCATOR_CL );

        _clrggb.setArg( 0, dst );
        _clrggb.setArg( 1, src );
        _clrggb.setArg( 2, CLLocalSpace( sizeof( cl_float ) * ( KX + 2 ) * ( KY + 2 ) ) );
        _clrggb.run( CLNDRange( Math::pad( src.width(), KX ), Math::pad( src.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void CLDebayer::debayer_GBRG_RGBA( Image& dst, const Image& src )
    {
        dst.reallocate( src.width(), src.height(), IFormat::RGBA_UINT8, IALLOCATOR_CL );

        _clgbrg.setArg( 0, dst );
        _clgbrg.setArg( 1, src );
        _clgbrg.setArg( 2, CLLocalSpace( sizeof( cl_float ) * ( KX + 2 ) * ( KY + 2 ) ) );
        _clgbrg.run( CLNDRange( Math::pad( src.width(), KX ), Math::pad( src.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void CLDebayer::debayer_GRBG_RGBA( Image& dst, const Image& src )
    {
        dst.reallocate( src.width(), src.height(), IFormat::RGBA_UINT8, IALLOCATOR_CL );

        _clgrbg.setArg( 0, dst );
        _clgrbg.setArg( 1, src );
        _clgrbg.setArg( 2, CLLocalSpace( sizeof( cl_float ) * ( KX + 2 ) * ( KY + 2 ) ) );
        _clgrbg.run( CLNDRange( Math::pad( src.width(), KX ), Math::pad( src.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void CLDebayer::debayer_BGGR_RGBA( Image& dst, const Image& src )
    {
        dst.reallocate( src.width(), src.height(), IFormat::RGBA_UINT8, IALLOCATOR_CL );

        _clbggr.setArg( 0, dst );
        _clbggr.setArg( 1, src );
        _clbggr.setArg( 2, CLLocalSpace( sizeof( cl_float ) * ( KX + 2 ) * ( KY + 2 ) ) );
        _clbggr.run( CLNDRange( Math::pad( src.width(), KX ), Math::pad( src.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void CLDebayer::debayerhq_RGGB_RGBA( Image& dst, const Image& src )
    {
        dst.reallocate( src.width(), src.height(), IFormat::RGBA_UINT8, IALLOCATOR_CL );

        _clrggbhq.setArg( 0, dst );
        _clrggbhq.setArg( 1, src );
        _clrggbhq.setArg( 2, CLLocalSpace( sizeof( cl_float ) * ( KX + 4 ) * ( KY + 4 ) ) );
        _clrggbhq.run( CLNDRange( Math::pad( src.width(), KX ), Math::pad( src.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void CLDebayer::debayerhq_GBRG_RGBA( Image& dst, const Image& src )
    {
        dst.reallocate( src.width(), src.height(), IFormat::RGBA_UINT8, IALLOCATOR_CL );

        _clgbrghq.setArg( 0, dst );
        _clgbrghq.setArg( 1, src );
        _clgbrghq.setArg( 2, CLLocalSpace( sizeof( cl_float ) * ( KX + 4 ) * ( KY + 4 ) ) );
        _clgbrghq.run( CLNDRange( Math::pad( src.width(), KX ), Math::pad( src.height(), KY ) ), CLNDRange( KX, KY ) );
    }


    void CLDebayer::debayerhq_GRBG_RGBA( Image& dst, const Image& src )
    {
        dst.reallocate( src.width(), src.height(), IFormat::RGBA_UINT8, IALLOCATOR_CL );

        _clgrbghq.setArg( 0, dst );
        _clgrbghq.setArg( 1, src );
        _clgrbghq.setArg( 2, CLLocalSpace( sizeof( cl_float ) * ( KX + 4 ) * ( KY + 4 ) ) );
        _clgrbghq.run( CLNDRange( Math::pad( src.width(), KX ), Math::pad( src.height(), KY ) ), CLNDRange( KX, KY ) );
    }

    void CLDebayer::debayerhq_BGGR_RGBA( Image& dst, const Image& src )
    {
        dst.reallocate( src.width(), src.height(), IFormat::RGBA_UINT8, IALLOCATOR_CL );

        _clbggrhq.setArg( 0, dst );
        _clbggrhq.setArg( 1, src );
        _clbggrhq.setArg( 2, CLLocalSpace( sizeof( cl_float ) * ( KX + 4 ) * ( KY + 4 ) ) );
        _clbggrhq.run( CLNDRange( Math::pad( src.width(), KX ), Math::pad( src.height(), KY ) ), CLNDRange( KX, KY ) );
    }
}

