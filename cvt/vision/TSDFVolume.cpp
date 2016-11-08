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
#include <cvt/vision/TSDFVolume.h>
#include <cvt/cl/kernel/TSDFVolume/TSDFVolume.h>

namespace cvt
{

    TSDFVolume::TSDFVolume( const Matrix4f& gridtoworld, size_t width, size_t height, size_t depth, float truncation ) :
        _clvolclear( _TSDFVolume_source, "TSDFVolume_clear" ),
        _clvolsetweight( _TSDFVolume_source, "TSDFVolume_setWeight" ),
        _clvolscaleweight( _TSDFVolume_source, "TSDFVolume_scaleWeight" ),
        _clvoladd( _TSDFVolume_source, "TSDFVolume_add" ),
        _clvoladdweighted( _TSDFVolume_source, "TSDFVolume_addWeighted" ),
        _clsliceX( _TSDFVolume_source, "TSDFVolume_sliceX"),
        _clsliceY( _TSDFVolume_source, "TSDFVolume_sliceY"),
        _clsliceZ( _TSDFVolume_source, "TSDFVolume_sliceZ"),
        _clraycastdepth( _TSDFVolume_source, "TSDFVolume_rayCastDepthmap" ),
        _clraycastdepthnormal( _TSDFVolume_source, "TSDFVolume_rayCastDepthNormalMap" ),
        _clraycastdepthnormalslope( _TSDFVolume_source, "TSDFVolume_rayCastDepthNormalMapSlope" ),
        _clsilhouette( _TSDFVolume_source, "TSDFVolume_addSilhouette" ),
        _clnormweight( _TSDFVolume_source, "TSDFVolume_normalToWeight" ),
        _clproj( sizeof( float ) * 12 ),
        _clvolume( sizeof( cl_float2 ) * width * height * depth ),
        _width( width ),
        _height( height ),
        _depth( depth ),
        _trunc( truncation ),
        _g2w( gridtoworld )
    {

    }

    TSDFVolume::TSDFVolume( const Vector3f& boxpt1, const Vector3f& boxpt2, float resolution, size_t voxeltruncation ) :
        _clvolclear( _TSDFVolume_source, "TSDFVolume_clear" ),
        _clvolsetweight( _TSDFVolume_source, "TSDFVolume_setWeight" ),
        _clvolscaleweight( _TSDFVolume_source, "TSDFVolume_scaleWeight" ),
        _clvoladd( _TSDFVolume_source, "TSDFVolume_add" ),
        _clvoladdweighted( _TSDFVolume_source, "TSDFVolume_addWeighted" ),
        _clsliceX( _TSDFVolume_source, "TSDFVolume_sliceX"),
        _clsliceY( _TSDFVolume_source, "TSDFVolume_sliceY"),
        _clsliceZ( _TSDFVolume_source, "TSDFVolume_sliceZ"),
        _clraycastdepth( _TSDFVolume_source, "TSDFVolume_rayCastDepthmap" ),
        _clraycastdepthnormal( _TSDFVolume_source, "TSDFVolume_rayCastDepthNormalMap" ),
        _clraycastdepthnormalslope( _TSDFVolume_source, "TSDFVolume_rayCastDepthNormalMapSlope" ),
        _clsilhouette( _TSDFVolume_source, "TSDFVolume_addSilhouette" ),
        _clnormweight( _TSDFVolume_source, "TSDFVolume_normalToWeight" ),
        _clproj( sizeof( float ) * 12 ),
        _clvolume( sizeof( float ) )
    {
        float x1 = Math::min( boxpt1.x, boxpt2.x );
        float x2 = Math::max( boxpt1.x, boxpt2.x );
        float y1 = Math::min( boxpt1.y, boxpt2.y );
        float y2 = Math::max( boxpt1.y, boxpt2.y );
        float z1 = Math::min( boxpt1.z, boxpt2.z );
        float z2 = Math::max( boxpt1.z, boxpt2.z );

        float width  = ( x2 - x1 ) / resolution;
        float height = ( y2 - y1 ) / resolution;
        float depth  = ( z2 - z1 ) / resolution;

        _width  = 2 + ( size_t ) Math::ceil( width );
        _height = 2 + ( size_t ) Math::ceil( height );
        _depth  = 2 + ( size_t ) Math::ceil( depth );

        float xoff = resolution * ( float ) _width  - ( x2 - x1 );
        float yoff = resolution * ( float ) _height - ( y2 - y1 );
        float zoff = resolution * ( float ) _depth  - ( z2 - z1 );

        _g2w = Matrix4f( resolution,       0.0f,       0.0f, x1 - 0.5f * xoff,
                               0.0f, resolution,       0.0f, y1 - 0.5f * yoff,
                               0.0f,       0.0f, resolution, z1 - 0.5f * zoff,
                               0.0f,       0.0f,       0.0f,             1.0f );

        _trunc = voxeltruncation * resolution;

        _clvolume = CLBuffer( sizeof( cl_float2 ) * _width * _height * _depth );
        //std::cout << _clvolume.size() / sizeof( cl_float2 ) << " " << _width * _height * _depth << std::endl;
    }

    void TSDFVolume::clear( float value, float weight )
    {
        /* clear the volume */
        _clvolclear.setArg( 0, _clvolume );
        _clvolclear.setArg( 1, ( int ) _width);
        _clvolclear.setArg( 2, ( int ) _height );
        _clvolclear.setArg( 3, ( int ) _depth);
        _clvolclear.setArg( 4, value );
        _clvolclear.setArg( 5, weight );
        // FIXME: maybe 8 x 8 x ? for the local range is better
        _clvolclear.run( CLNDRange( Math::pad16( _width ), Math::pad16( _height ), _depth ), CLNDRange( 16, 16, 1 ) );
    }

    void TSDFVolume::setWeight( float weight )
    {
        /* clear the volume */
        _clvolsetweight.setArg( 0, _clvolume );
        _clvolsetweight.setArg( 1, ( int ) _width);
        _clvolsetweight.setArg( 2, ( int ) _height );
        _clvolsetweight.setArg( 3, ( int ) _depth);
        _clvolsetweight.setArg( 4, weight );
        // FIXME: maybe 8 x 8 x ? for the local range is better
        _clvolsetweight.run( CLNDRange( Math::pad16( _width ), Math::pad16( _height ), _depth ), CLNDRange( 16, 16, 1 ) );
    }

    void TSDFVolume::scaleWeight( float scale )
    {
        /* clear the volume */
        _clvolscaleweight.setArg( 0, _clvolume );
        _clvolscaleweight.setArg( 1, ( int ) _width);
        _clvolscaleweight.setArg( 2, ( int ) _height );
        _clvolscaleweight.setArg( 3, ( int ) _depth);
        _clvolscaleweight.setArg( 4, scale );
        // FIXME: maybe 8 x 8 x ? for the local range is better
        _clvolscaleweight.run( CLNDRange( Math::pad16( _width ), Math::pad16( _height ), _depth ), CLNDRange( 16, 16, 1 ) );
    }

    void TSDFVolume::addDepthMap( const Matrix4f& proj, const Image& depthmap, float scale )
    {
        // update projection matrix
        Matrix4f projall = proj * _g2w;

        // add depthmap
        _clvoladd.setArg( 0, _clvolume );
        _clvoladd.setArg( 1, ( int ) _width );
        _clvoladd.setArg( 2, ( int ) _height );
        _clvoladd.setArg( 3, ( int ) _depth );
        _clvoladd.setArg( 4, depthmap );
        _clvoladd.setArg( 5, scale );
        _clvoladd.setArg( 6, sizeof( float ) * 16, projall.ptr() );
        _clvoladd.setArg( 7, _trunc );
        _clvoladd.run( CLNDRange( Math::pad16( _width ), Math::pad16( _height ), _depth ), CLNDRange( 16, 16, 1 ) );
    }

    void TSDFVolume::addDepthMapWeighted( const Matrix4f& proj, const Image& depthmap, float scale, const Image& weight )
    {
        // update projection matrix
        Matrix4f projall = proj * _g2w;

        // add depthmap
        _clvoladdweighted.setArg( 0, _clvolume );
        _clvoladdweighted.setArg( 1, ( int ) _width );
        _clvoladdweighted.setArg( 2, ( int ) _height );
        _clvoladdweighted.setArg( 3, ( int ) _depth );
        _clvoladdweighted.setArg( 4, depthmap );
        _clvoladdweighted.setArg( 5, scale );
        _clvoladdweighted.setArg( 6, sizeof( float ) * 16, projall.ptr() );
        _clvoladdweighted.setArg( 7, weight );
        _clvoladdweighted.setArg( 8, _trunc );
        _clvoladdweighted.run( CLNDRange( Math::pad16( _width ), Math::pad16( _height ), _depth ), CLNDRange( 16, 16, 1 ) );
    }

    void TSDFVolume::addDepthMap( const Matrix3f& intrinsics, const Matrix4f& extrinsics, const Image& depthmap, float scale )
    {
        Matrix4f proj = intrinsics.toMatrix4();
        proj *= extrinsics;
        addDepthMap( proj, depthmap, scale );
    }

    void TSDFVolume::addDepthMapWeighted( const Matrix3f& intrinsics, const Matrix4f& extrinsics, const Image& depthmap, float scale, const Image& weight )
    {
        Matrix4f proj = intrinsics.toMatrix4();
        proj *= extrinsics;
        addDepthMapWeighted( proj, depthmap, scale, weight );
    }

    void TSDFVolume::addSilhouette( const Matrix3f& intrinsics, const Matrix4f& extrinsics, const Image& silhouette )
    {
        Matrix4f proj = intrinsics.toMatrix4();
        proj *= extrinsics;
        addSilhouette( proj, silhouette );
    }

    void TSDFVolume::addDepthMapNormalWeighted( const Matrix4f& proj, const Image& depthmap, float scale, const Image& normal )
    {
        Image weight;
        normalToWeight( weight, normal );

        addDepthMapWeighted( proj, depthmap, scale, weight );
    }

    void TSDFVolume::addDepthMapNormalWeighted( const Matrix3f& intrinsics, const Matrix4f& extrinsics, const Image& depthmap, float scale, const Image& normal )
    {
        Matrix4f proj = intrinsics.toMatrix4();
        proj *= extrinsics;
        addDepthMapNormalWeighted( proj, depthmap, scale, normal );
    }

    void TSDFVolume::addSilhouette( const Matrix4f& proj, const Image& silhouette )
    {
        // update projection matrix
        Matrix4f projall = proj * _g2w;

        // add depthmap
        _clsilhouette.setArg( 0, _clvolume );
        _clsilhouette.setArg( 1, ( int ) _width );
        _clsilhouette.setArg( 2, ( int ) _height );
        _clsilhouette.setArg( 3, ( int ) _depth );
        _clsilhouette.setArg( 4, silhouette );
        _clsilhouette.setArg( 5, sizeof( float ) * 16, projall.ptr() );
        _clsilhouette.run( CLNDRange( Math::pad16( _width ), Math::pad16( _height ), _depth ), CLNDRange( 16, 16, 1 ) );
    }

    void TSDFVolume::rayCastDepthMap( Image& depthmap, const Matrix3f& intrinsics, const Matrix4f& extrinsics, float scale )
    {
        Matrix4f proj = intrinsics.toMatrix4();
        Matrix4f MVP = proj * ( extrinsics * _g2w );
        Matrix4f MV  = extrinsics * _g2w;

        depthmap.reallocate( depthmap.width(), depthmap.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );

        _clraycastdepth.setArg( 0, depthmap );
        _clraycastdepth.setArg( 1, _clvolume );
        _clraycastdepth.setArg( 2, ( int ) _width);
        _clraycastdepth.setArg( 3, ( int ) _height );
        _clraycastdepth.setArg( 4, ( int ) _depth);
        _clraycastdepth.setArg( 5, sizeof( float ) * 16, MVP.inverse().ptr() );
        _clraycastdepth.setArg( 6, sizeof( float ) * 16, MV.ptr() );
        _clraycastdepth.setArg( 7, scale );
        _clraycastdepth.run( CLNDRange( Math::pad16( depthmap.width() ), Math::pad16( depthmap.height() ) ), CLNDRange( 16, 16 ) );
    }

    void TSDFVolume::rayCastDepthNormalMap( Image& depthmap, const Matrix3f& intrinsics, const Matrix4f& extrinsics, float scale )
    {
        Matrix4f proj = intrinsics.toMatrix4();
        Matrix4f MVP = proj * ( extrinsics * _g2w );
        Matrix4f MV  = extrinsics * _g2w;

        depthmap.reallocate( depthmap.width(), depthmap.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );

        _clraycastdepthnormal.setArg( 0, depthmap );
        _clraycastdepthnormal.setArg( 1, _clvolume );
        _clraycastdepthnormal.setArg( 2, ( int ) _width);
        _clraycastdepthnormal.setArg( 3, ( int ) _height );
        _clraycastdepthnormal.setArg( 4, ( int ) _depth);
        _clraycastdepthnormal.setArg( 5, sizeof( float ) * 16, MVP.inverse().ptr() );
        _clraycastdepthnormal.setArg( 6, sizeof( float ) * 16, MV.ptr() );
        _clraycastdepthnormal.setArg( 7, scale );
        _clraycastdepthnormal.run( CLNDRange( Math::pad16( depthmap.width() ), Math::pad16( depthmap.height() ) ), CLNDRange( 16, 16 ) );
    }

    void TSDFVolume::rayCastDepthNormalMapSlope( Image& depthmap, Image& slope, const Matrix3f& intrinsics, const Matrix4f& extrinsics, float scale )
    {
        Matrix4f proj = intrinsics.toMatrix4();
        Matrix4f MVP = proj * ( extrinsics * _g2w );
        Matrix4f MV  = extrinsics * _g2w;

        depthmap.reallocate( depthmap.width(), depthmap.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        slope.reallocate( depthmap.width(), depthmap.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );

        _clraycastdepthnormalslope.setArg( 0, depthmap );
        _clraycastdepthnormalslope.setArg( 1, slope );
        _clraycastdepthnormalslope.setArg( 2, _clvolume );
        _clraycastdepthnormalslope.setArg( 3, ( int ) _width);
        _clraycastdepthnormalslope.setArg( 4, ( int ) _height );
        _clraycastdepthnormalslope.setArg( 5, ( int ) _depth);
        _clraycastdepthnormalslope.setArg( 6, sizeof( float ) * 16, MVP.inverse().ptr() );
        _clraycastdepthnormalslope.setArg( 7, sizeof( float ) * 16, MV.ptr() );
        _clraycastdepthnormalslope.setArg( 8, scale );
        _clraycastdepthnormalslope.run( CLNDRange( Math::pad16( depthmap.width() ), Math::pad16( depthmap.height() ) ), CLNDRange( 16, 16 ) );
    }

    void TSDFVolume::normalToWeight( Image& weight, const Image& normals )
    {
        weight.reallocate( normals.width(), normals.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );

        _clnormweight.setArg( 0, weight );
        _clnormweight.setArg( 1, normals );
        _clnormweight.run( CLNDRange( Math::pad16( normals.width() ), Math::pad16( normals.height() ) ), CLNDRange( 16, 16 ) );
    }

    void TSDFVolume::toSceneMesh( SceneMesh& mesh, float minweight ) const
    {
        float* ptr = ( float* ) _clvolume.map();
        MarchingCubes mc( ptr, _width, _height, _depth, true, minweight );
        mc.triangulateWithNormals( mesh, 0.0f );
        _clvolume.unmap( ptr );
    }

    void TSDFVolume::sliceX( Image& img ) const
    {
    }

    void TSDFVolume::sliceY( Image& img ) const
    {
    }

    void TSDFVolume::sliceZ( Image& img ) const
    {
    }


    void TSDFVolume::saveRaw( const String& path, bool weighted ) const
    {
        float* ptr = ( float* ) _clvolume.map();
        float* origptr = ptr;
        size_t n = _width * _height * _depth;

        FILE* f;
        f = fopen( path.c_str(),"wb");

        if( weighted ) {
            fwrite( ptr, sizeof( float ), 2 * n, f );
        } else {
            while( n-- ) {
                fwrite( ptr, sizeof( float ), 1, f );
                ptr += 2;
            }
        }
        fclose( f );

        _clvolume.unmap( origptr );
    }
}
