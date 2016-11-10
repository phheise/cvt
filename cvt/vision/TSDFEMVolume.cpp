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
#include <cvt/vision/TSDFEMVolume.h>
#include <cvt/cl/kernel/TSDFVolume/TSDFEMVolume.h>

namespace cvt
{

    TSDFEMVolume::TSDFEMVolume( const Matrix4f& gridtoworld, size_t width, size_t height, size_t depth, float truncation ) :
        _clvolclear( _TSDFEMVolume_source, "TSDFEMVolume_clear" ),
        _clvolsetweight( _TSDFEMVolume_source, "TSDFEMVolume_setWeight" ),
        _clvolsetvar( _TSDFEMVolume_source, "TSDFEMVolume_setVariance" ),
        _clvolsetweightvar( _TSDFEMVolume_source, "TSDFEMVolume_setWeightVariance" ),
        _clvolscaleweight( _TSDFEMVolume_source, "TSDFEMVolume_scaleWeight" ),
        _clvolscaleweightvar( _TSDFEMVolume_source, "TSDFEMVolume_scaleWeightVariance" ),
        _clvoladd( _TSDFEMVolume_source, "TSDFEMVolume_add" ),
        _clvoladdweighted( _TSDFEMVolume_source, "TSDFEMVolume_addWeighted" ),
        _clraycastdepth( _TSDFEMVolume_source, "TSDFEMVolume_rayCastDepthmap" ),
        _clraycastdepthnormal( _TSDFEMVolume_source, "TSDFEMVolume_rayCastDepthNormalMap" ),
        _clraycastdepthnormalconfid( _TSDFEMVolume_source, "TSDFEMVolume_rayCastDepthNormalMapConfidence" ),
        _clshilouette( _TSDFEMVolume_source, "TSDFEMVolume_addSilhouette" ),
        _clnormweight( _TSDFEMVolume_source, "TSDFEMVolume_normalToWeight" ),
        _clproj( sizeof( float ) * 12 ),
        _clvolume( sizeof( cl_float4 ) * width * height * depth ),
        _width( width ),
        _height( height ),
        _depth( depth ),
        _trunc( truncation ),
        _g2w( gridtoworld )
    {

    }


    TSDFEMVolume::TSDFEMVolume( const Vector3f& boxpt1, const Vector3f& boxpt2, float resolution, size_t voxeltruncation ) :
        _clvolclear( _TSDFEMVolume_source, "TSDFEMVolume_clear" ),
        _clvolsetweight( _TSDFEMVolume_source, "TSDFEMVolume_setWeight" ),
        _clvolsetvar( _TSDFEMVolume_source, "TSDFEMVolume_setVariance" ),
        _clvolsetweightvar( _TSDFEMVolume_source, "TSDFEMVolume_setWeightVariance" ),
        _clvolscaleweight( _TSDFEMVolume_source, "TSDFEMVolume_scaleWeight" ),
        _clvolscaleweightvar( _TSDFEMVolume_source, "TSDFEMVolume_scaleWeightVariance" ),
        _clvoladd( _TSDFEMVolume_source, "TSDFEMVolume_add" ),
        _clvoladdweighted( _TSDFEMVolume_source, "TSDFEMVolume_addWeighted" ),
        _clraycastdepth( _TSDFEMVolume_source, "TSDFEMVolume_rayCastDepthmap" ),
        _clraycastdepthnormal( _TSDFEMVolume_source, "TSDFEMVolume_rayCastDepthNormalMap" ),
        _clraycastdepthnormalconfid( _TSDFEMVolume_source, "TSDFEMVolume_rayCastDepthNormalMapConfidence" ),
        _clshilouette( _TSDFEMVolume_source, "TSDFEMVolume_addSilhouette" ),
        _clnormweight( _TSDFEMVolume_source, "TSDFEMVolume_normalToWeight" ),
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

        _clvolume = CLBuffer( sizeof( cl_float4 ) * _width * _height * _depth );
        //std::cout << _clvolume.size() / sizeof( cl_float2 ) << " " << _width * _height * _depth << std::endl;
    }

    void TSDFEMVolume::clear( float value, float weight, float value2 )
    {
        /* clear the volume */
        _clvolclear.setArg( 0, _clvolume );
        _clvolclear.setArg( 1, ( int ) _width);
        _clvolclear.setArg( 2, ( int ) _height );
        _clvolclear.setArg( 3, ( int ) _depth);
        _clvolclear.setArg( 4, value );
        _clvolclear.setArg( 5, value2 );
        _clvolclear.setArg( 6, weight );
        // FIXME: maybe 8 x 8 x ? for the local range is better
        _clvolclear.run( CLNDRange( Math::pad16( _width ), Math::pad16( _height ), _depth ), CLNDRange( 16, 16, 1 ) );
    }

    void TSDFEMVolume::setWeight( float weight )
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

    void TSDFEMVolume::setVariance( float weight )
    {
        /* clear the volume */
        _clvolsetvar.setArg( 0, _clvolume );
        _clvolsetvar.setArg( 1, ( int ) _width);
        _clvolsetvar.setArg( 2, ( int ) _height );
        _clvolsetvar.setArg( 3, ( int ) _depth);
        _clvolsetvar.setArg( 4, weight );
        // FIXME: maybe 8 x 8 x ? for the local range is better
        _clvolsetvar.run( CLNDRange( Math::pad16( _width ), Math::pad16( _height ), _depth ), CLNDRange( 16, 16, 1 ) );
    }


    void TSDFEMVolume::setWeightVariance( float weight, float variance, float outweight )
    {
        /* clear the volume */
        _clvolsetweightvar.setArg( 0, _clvolume );
        _clvolsetweightvar.setArg( 1, ( int ) _width);
        _clvolsetweightvar.setArg( 2, ( int ) _height );
        _clvolsetweightvar.setArg( 3, ( int ) _depth);
        _clvolsetweightvar.setArg( 4, weight );
        _clvolsetweightvar.setArg( 5, outweight );
        _clvolsetweightvar.setArg( 6, variance );
        // FIXME: maybe 8 x 8 x ? for the local range is better
        _clvolsetweightvar.run( CLNDRange( Math::pad16( _width ), Math::pad16( _height ), _depth ), CLNDRange( 16, 16, 1 ) );
    }



    void TSDFEMVolume::scaleWeight( float scale )
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

    void TSDFEMVolume::scaleWeightVariance( float scale, float varscale, float minvar )
    {
        /* clear the volume */
        _clvolscaleweightvar.setArg( 0, _clvolume );
        _clvolscaleweightvar.setArg( 1, ( int ) _width);
        _clvolscaleweightvar.setArg( 2, ( int ) _height );
        _clvolscaleweightvar.setArg( 3, ( int ) _depth);
        _clvolscaleweightvar.setArg( 4, scale );
        _clvolscaleweightvar.setArg( 5, varscale );
        _clvolscaleweightvar.setArg( 6, minvar );
        // FIXME: maybe 8 x 8 x ? for the local range is better
        _clvolscaleweightvar.run( CLNDRange( Math::pad16( _width ), Math::pad16( _height ), _depth ), CLNDRange( 16, 16, 1 ) );
    }

    void TSDFEMVolume::addDepthMap( const Matrix4f& proj, const Image& depthmap, float scale )
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


    void TSDFEMVolume::addDepthMapWeighted( const Matrix4f& proj, const Image& depthmap, float scale, const Image& weight )
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

    void TSDFEMVolume::addDepthMap( const Matrix3f& intrinsics, const Matrix4f& extrinsics, const Image& depthmap, float scale )
    {
        Matrix4f proj = intrinsics.toMatrix4();
        proj *= extrinsics;
        addDepthMap( proj, depthmap, scale );
    }

    void TSDFEMVolume::addShilouette( const Matrix3f& intrinsics, const Matrix4f& extrinsics, const Image& shilouette )
    {
        Matrix4f proj = intrinsics.toMatrix4();
        proj *= extrinsics;
        addShilouette( proj, shilouette );
    }

    void TSDFEMVolume::addDepthMapWeighted( const Matrix3f& intrinsics, const Matrix4f& extrinsics, const Image& depthmap, float scale, const Image& weight )
    {
        Matrix4f proj = intrinsics.toMatrix4();
        proj *= extrinsics;
        addDepthMapWeighted( proj, depthmap, scale, weight );
    }

    void TSDFEMVolume::addDepthMapNormalWeighted( const Matrix4f& proj, const Image& depthmap, float scale, const Image& normal )
    {
        Image weight;
        normalToWeight( weight, normal );

        addDepthMapWeighted( proj, depthmap, scale, weight );
    }

    void TSDFEMVolume::addDepthMapNormalWeighted( const Matrix3f& intrinsics, const Matrix4f& extrinsics, const Image& depthmap, float scale, const Image& normal )
    {
        Matrix4f proj = intrinsics.toMatrix4();
        proj *= extrinsics;
        addDepthMapNormalWeighted( proj, depthmap, scale, normal );
    }

    void TSDFEMVolume::addShilouette( const Matrix4f& proj, const Image& shilouette )
    {
       // update projection matrix
        Matrix4f projall = proj * _g2w;

        // add depthmap
        _clshilouette.setArg( 0, _clvolume );
        _clshilouette.setArg( 1, ( int ) _width );
        _clshilouette.setArg( 2, ( int ) _height );
        _clshilouette.setArg( 3, ( int ) _depth );
        _clshilouette.setArg( 4, shilouette );
        _clshilouette.setArg( 5, sizeof( float ) * 16, projall.ptr() );
        _clshilouette.run( CLNDRange( Math::pad16( _width ), Math::pad16( _height ), _depth ), CLNDRange( 16, 16, 1 ) );
    }

    void TSDFEMVolume::rayCastDepthMap( Image& depthmap, const Matrix3f& intrinsics, const Matrix4f& extrinsics, float scale )
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

    void TSDFEMVolume::rayCastDepthNormalMap( Image& depthmap, const Matrix3f& intrinsics, const Matrix4f& extrinsics, float scale )
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

    void TSDFEMVolume::rayCastDepthNormalConfidenceMap( Image& depthmap, Image& confidence, const Matrix3f& intrinsics, const Matrix4f& extrinsics, float scale )
    {
        Matrix4f proj = intrinsics.toMatrix4();
        Matrix4f MVP = proj * ( extrinsics * _g2w );
        Matrix4f MV  = extrinsics * _g2w;

        depthmap.reallocate( depthmap.width(), depthmap.height(), IFormat::RGBA_FLOAT, IALLOCATOR_CL );
        confidence.reallocate( depthmap.width(), depthmap.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );

        _clraycastdepthnormalconfid.setArg( 0, depthmap );
        _clraycastdepthnormalconfid.setArg( 1, confidence );
        _clraycastdepthnormalconfid.setArg( 2, _clvolume );
        _clraycastdepthnormalconfid.setArg( 3, ( int ) _width);
        _clraycastdepthnormalconfid.setArg( 4, ( int ) _height );
        _clraycastdepthnormalconfid.setArg( 5, ( int ) _depth);
        _clraycastdepthnormalconfid.setArg( 6, sizeof( float ) * 16, MVP.inverse().ptr() );
        _clraycastdepthnormalconfid.setArg( 7, sizeof( float ) * 16, MV.ptr() );
        _clraycastdepthnormalconfid.setArg( 8, scale );
        _clraycastdepthnormalconfid.run( CLNDRange( Math::pad16( depthmap.width() ), Math::pad16( depthmap.height() ) ), CLNDRange( 16, 16 ) );
    }

    void TSDFEMVolume::toSceneMesh( SceneMesh& mesh, float minweight ) const
    {
        float* ptr = ( float* ) _clvolume.map();
        MarchingCubes mc( ptr, _width, _height, _depth, true, minweight, sizeof( cl_float4 ) / sizeof( float ) );
        mc.triangulateWithNormals( mesh, 0.0f, false );
        _clvolume.unmap( ptr );
    }

    void TSDFEMVolume::save( const String& path ) const
    {
        float* ptr = ( float* ) _clvolume.map();
        float* origptr = ptr;
        size_t n = _width * _height * _depth;

        FILE* f;
        f = fopen( path.c_str(),"wb");
        fwrite( &_width, sizeof( size_t ), 1, f );
        fwrite( &_height, sizeof( size_t ), 1, f );
        fwrite( &_depth, sizeof( size_t ), 1, f );
        fwrite( &_trunc, sizeof( float ), 1, f );
        fwrite( _g2w.ptr(), sizeof( float ), 16, f );
        fwrite( ptr, sizeof( cl_float4 ), n, f );
        fclose( f );

        _clvolume.unmap( origptr );
    }

    void TSDFEMVolume::load( const String& path )
    {
        FILE* f;
        f = fopen( path.c_str(),"rb");
        size_t rn;
        //TODO: check rn
        rn = fread( &_width, sizeof( size_t ), 1, f );
        rn = fread( &_height, sizeof( size_t ), 1, f );
        rn = fread( &_depth, sizeof( size_t ), 1, f );
        rn = fread( &_trunc, sizeof( float ), 1, f );
        rn = fread( _g2w.ptr(), sizeof( float ), 16, f );


        _clvolume = CLBuffer( sizeof( cl_float4 ) * _width * _height * _depth );

        float* ptr = ( float* ) _clvolume.map();
        size_t n = _width * _height * _depth;
        //TODO: check rn
        rn = fread( ptr, sizeof( cl_float4 ), n, f );
        _clvolume.unmap( ptr );

        fclose( f );
    }

    void TSDFEMVolume::normalToWeight( Image& weight, const Image& normals )
    {
        weight.reallocate( normals.width(), normals.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );

        _clnormweight.setArg( 0, weight );
        _clnormweight.setArg( 1, normals );
        _clnormweight.run( CLNDRange( Math::pad16( normals.width() ), Math::pad16( normals.height() ) ), CLNDRange( 16, 16 ) );
    }
}
