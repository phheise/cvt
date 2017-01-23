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
#include <cvt/vision/RGBD.h>
#include <cvt/gfx/IMapScoped.h>

namespace cvt {

    void RGBD::unprojectToScenePoints( ScenePoints& scenepts, const Image& texture, const Image& depthmap, const Matrix3f& K, float dscale )
    {
        Matrix3f Kinv = K.inverse();
        std::vector<Vector3f> pts;
        std::vector<Vector4f> colors;

        scenepts.clear();

        if( texture.format() != IFormat::RGBA_FLOAT || depthmap.format() != IFormat::GRAY_FLOAT || texture.width() != depthmap.width() || texture.height() != depthmap.height() )
            throw CVTException( "unprojectToScenePoints: invalid texture or depth-map!" );

        IMapScoped<const float> tex( texture );
        IMapScoped<const float> dmap( depthmap );
        size_t w = depthmap.width();
        size_t h = depthmap.height();
        for( size_t y = 0; y < h; y++ ) {
            const float* dmapptr = dmap.ptr();
            const float* texptr = tex.ptr();

            for( size_t x = 0; x < w; x++ ) {
                if( dmapptr[ x ] > 0 ) {
                    Vector3f pt = Kinv * Vector3f( x, y, 1.0f );
                    pts.push_back( pt * dmapptr[ x ] * dscale );
                    colors.push_back( Vector4f( texptr[ x * 4 + 0 ], texptr[ x * 4 + 1 ], texptr[ x * 4 + 2 ], texptr[ x * 4 + 3 ] ) );
                }
            }

            dmap++;
            tex++;
        }

        scenepts.setVerticesWithColor( &pts[ 0 ], &colors[ 0 ], pts.size() );
    }

    void RGBD::unprojectToScenePoints( ScenePoints& scenepts, const Image& texture, const Image& depthmap, float dscale )
    {
        std::vector<Vector3f> pts;
        std::vector<Vector4f> colors;

        scenepts.clear();

        if( texture.format() != IFormat::RGBA_FLOAT || depthmap.format() != IFormat::GRAY_FLOAT || texture.width() != depthmap.width() || texture.height() != depthmap.height() )
            throw CVTException( "unprojectToScenePoints: invalid texture or depth-map!" );

        IMapScoped<const float> tex( texture );
        IMapScoped<const float> dmap( depthmap );
        size_t w = depthmap.width();
        size_t h = depthmap.height();
        float dx = 0.5f * depthmap.width();
        float dy = 0.5f * depthmap.height();

        for( size_t y = 0; y < h; y++ ) {
            const float* dmapptr = dmap.ptr();
            const float* texptr = tex.ptr();

            for( size_t x = 0; x < w; x++ ) {
                if( dmapptr[ x ] > 0 ) {
                    Vector3f pt = Vector3f( ( x - dx ) / ( float ) w , ( y - dy ) / ( float ) h, dmapptr[ x ] * dscale );
                    pts.push_back( pt );
                    colors.push_back( Vector4f( texptr[ x * 4 + 0 ], texptr[ x * 4 + 1 ], texptr[ x * 4 + 2 ], texptr[ x * 4 + 3 ] ) );
                }
            }

            dmap++;
            tex++;
        }

        scenepts.setVerticesWithColor( &pts[ 0 ], &colors[ 0 ], pts.size() );
    }

    void RGBD::unprojectToXYZ( PointSet3f& pts, const Image& depth, const Matrix3f& K, float depthScale )
    {
        if( depth.format() == IFormat::GRAY_UINT16 ){
            IMapScoped<const uint16_t> depthMap( depth );
            float invFx = 1.0f / K[ 0 ][ 0 ];
            float invFy = 1.0f / K[ 1 ][ 1 ];
            float cx    = K[ 0 ][ 2 ];
            float cy    = K[ 1 ][ 2 ];

            // temp vals
            std::vector<float> tmpx( depth.width() );
            std::vector<float> tmpy( depth.height() );

            for( size_t i = 0; i < tmpx.size(); i++ ){
                tmpx[ i ] = ( i - cx ) * invFx;
            }
            for( size_t i = 0; i < tmpy.size(); i++ ){
                tmpy[ i ] = ( i - cy ) * invFy;
            }

            Vector3f p3d;
            for( size_t y = 0; y < depth.height(); y++ ){
                const uint16_t* dptr = depthMap.ptr();
                for( size_t x = 0; x < depth.width(); x++ ){
                    float d = dptr[ x ] * depthScale;
                    p3d[ 0 ] = tmpx[ x ] * d;
                    p3d[ 1 ] = tmpy[ y ] * d;
                    p3d[ 2 ] = d;
                    pts.add( p3d );
                }
                // next line in depth image
                depthMap++;
            }
        } else {
            throw CVTException( "Unproject not implemented for given format" );
        }
    }


    void RGBD::warpForward( Image& dst, const Image& rgb, const Image& depth, const Matrix3f& K, const Matrix4f RT, float dscale )
    {
        if( rgb.format() != IFormat::RGBA_FLOAT || depth.format() != IFormat::GRAY_FLOAT || rgb.width() != depth.width() || rgb.height() != depth.height() )
            throw CVTException( "invalid rgb or depth!" );

        dst.reallocate( rgb.width(), rgb.height(), IFormat::RGBA_FLOAT );

        // fill the background black
        dst.fill( Color::BLACK );


        Matrix3f Kinv = K.inverse();
        Matrix4f KRT  = K.toMatrix4() * RT;
        IMapScoped<const float> rgbmap( rgb );
        IMapScoped<const float> dmap( depth );
        IMapScoped<float> dstmap( dst );
        size_t w = depth.width();
        size_t h = depth.height();
        for( size_t y = 0; y < h; y++ ) {
            const float* dmapptr = dmap.ptr();
            const float* rgbptr = rgbmap.ptr();

            for( size_t x = 0; x < w; x++ ) {
                if( dmapptr[ x ] > 0 ) {
                    Vector3f pt     = Kinv * Vector3f( x, y, 1.0f ) * dmapptr[ x ] * dscale;
                    Vector4f ptproj = KRT * Vector4f( pt.x, pt.y, pt.z, 1.0f );
                    ptproj.x        = ptproj.x / ptproj.z;
                    ptproj.y        = ptproj.y / ptproj.z;

                    int xi = Math::round( ptproj.x );
                    int yi = Math::round( ptproj.y );

                    if( xi > 0 && xi < dst.width() &&
                        yi > 0 && yi < dst.height() ) {
                        dstmap( xi * 4 + 0, yi ) = rgbptr[ x * 4 + 0 ];
                        dstmap( xi * 4 + 1, yi ) = rgbptr[ x * 4 + 1 ];
                        dstmap( xi * 4 + 2, yi ) = rgbptr[ x * 4 + 2 ];
                    }
                }
            }
            dmap++;
            rgbmap++;
        }

    }

    void RGBD::warpBackward( Image& out, const Image& depth, const Image& rgbdst, const Matrix3f& K, const Matrix4f RT, float dscale )
    {
        if( rgbdst.format() != IFormat::RGBA_FLOAT || depth.format() != IFormat::GRAY_FLOAT || rgbdst.width() != depth.width() || rgbdst.height() != depth.height() )
            throw CVTException( "invalid rgb or depth!" );

        out.reallocate( depth.width(), depth.height(), IFormat::RGBA_FLOAT );

        // fill the background black
        out.fill( Color::BLACK );

        Matrix3f Kinv = K.inverse();
        Matrix4f KRT  = K.toMatrix4() * RT;
        IMapScoped<const float> rgbmap( rgbdst );
        IMapScoped<const float> dmap( depth );
        IMapScoped<float> dstmap( out );
        size_t w = depth.width();
        size_t h = depth.height();
        for( size_t y = 0; y < h; y++ ) {
            const float* dmapptr = dmap.ptr();
            float* dstptr = dstmap.ptr();

            for( size_t x = 0; x < w; x++ ) {
                if( dmapptr[ x ] > 0.0f ) {
                    Vector3f pt     = Kinv * Vector3f( x + 0.5f, y + 0.5f, 1.0f ) * dmapptr[ x ] * dscale;
                    Vector4f ptproj = KRT * Vector4f( pt.x, pt.y, pt.z, 1.0f );
                    ptproj.x        = ( ptproj.x / ptproj.z );
                    ptproj.y        = ( ptproj.y / ptproj.z );

                    if( ptproj.x > 0.0f && ptproj.x < ( float ) ( out.width() - 1 ) &&
                        ptproj.y > 0.0f && ptproj.y < ( float ) ( out.height() - 1 ) && ptproj.z > 0.0f ) {
                        dstptr[ x * 4 + 0 ] = rgbmap( ptproj.x, ptproj.y, 0 );
                        dstptr[ x * 4 + 1 ] = rgbmap( ptproj.x, ptproj.y, 1 );
                        dstptr[ x * 4 + 2 ] = rgbmap( ptproj.x, ptproj.y, 2 );
                    }
                }
            }
            dmap++;
            dstmap++;
        }

    }

    void RGBD::warpDepthForward( Image& dst, const Image& depth, const Matrix3f& K, const Matrix4f RT, float dscale )
    {
        if( depth.format() != IFormat::GRAY_FLOAT )
            throw CVTException( "invalid depth!" );

        dst.reallocate( depth.width(), depth.height(), IFormat::GRAY_FLOAT );

        // fill the background black
        dst.fill( Color::BLACK );


        Matrix3f Kinv = K.inverse();
        Matrix4f KRT  = K.toMatrix4() * RT;
        IMapScoped<const float> dmap( depth );
        IMapScoped<float> dstmap( dst );
        size_t w = depth.width();
        size_t h = depth.height();
        for( size_t y = 0; y < h; y++ ) {
            const float* dmapptr = dmap.ptr();

            for( size_t x = 0; x < w; x++ ) {
                if( dmapptr[ x ] > 0 ) {
                    Vector3f pt     = Kinv * Vector3f( x, y, 1.0f ) * dmapptr[ x ] * dscale;
                    Vector4f ptproj = KRT * Vector4f( pt.x, pt.y, pt.z, 1.0f );
                    ptproj.x        = ptproj.x / ptproj.z;
                    ptproj.y        = ptproj.y / ptproj.z;

                    int xi = Math::round( ptproj.x );
                    int yi = Math::round( ptproj.y );

                    if( xi > 0 && xi < dst.width() &&
                        yi > 0 && yi < dst.height() ) {
                        dstmap( xi, yi ) = dmapptr[ x ];
                    }
                }
            }
            dmap++;
        }

    }
}
