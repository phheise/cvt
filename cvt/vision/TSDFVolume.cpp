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

#include <cvt/vision/TSDFVolume.h>
#include <cvt/cl/kernel/TSDFVolume/TSDFVolume.h>

namespace cvt
{

	TSDFVolume::TSDFVolume( const Matrix4f& gridtoworld, size_t width, size_t height, size_t depth, float truncation ) :
		_width( width ),
		_height( height ),
		_depth( depth ),
		_trunc( truncation ),
		_g2w( gridtoworld ),
		_clvolume( sizeof( cl_float2 ) * width * height * depth ),
		_clproj( sizeof( float ) * 12 ),
		_clvolclear( _TSDFVolume_source, "TSDFVolume_clear" ),
		_clvoladd( _TSDFVolume_source, "TSDFVolume_add" ),
		_clsliceX( _TSDFVolume_source, "TSDFVolume_sliceX"),
		_clsliceY( _TSDFVolume_source, "TSDFVolume_sliceY"),
		_clsliceZ( _TSDFVolume_source, "TSDFVolume_sliceZ"),
		_clraycastdepth( _TSDFVolume_source, "TSDFVolume_rayCastDepthmap" )
	{
	}

	void TSDFVolume::clear( float weight )
	{
		/* clear the volume */
		_clvolclear.setArg( 0, _clvolume );
		_clvolclear.setArg( 1, ( int ) _width);
		_clvolclear.setArg( 2, ( int ) _height );
		_clvolclear.setArg( 3, ( int ) _depth);
		_clvolclear.setArg( 4, weight );
		// FIXME: maybe 8 x 8 x ? for the local range is better
		_clvolclear.run( CLNDRange( Math::pad16( _width ), Math::pad16( _height ), _depth ), CLNDRange( 16, 16, 1 ) );
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

	void TSDFVolume::addDepthMap( const Matrix3f& intrinsics, const Matrix4f& extrinsics, const Image& depthmap, float scale )
	{
		Matrix4f proj = intrinsics.toMatrix4();
		proj *= extrinsics;
		addDepthMap( proj, depthmap, scale );
	}


	void TSDFVolume::rayCastDepthMap( Image& depthmap, const Matrix3f& intrinsics, const Matrix4f& extrinsics, float scale )
	{
		Matrix4f proj = intrinsics.toMatrix4();
		proj *= extrinsics;
		rayCastDepthMap( depthmap, proj, scale );
	}

	void TSDFVolume::rayCastDepthMap( Image& depthmap, const Matrix4f& proj, float scale )
	{
		Matrix4f projall = proj * _g2w;

		depthmap.reallocate( depthmap.width(), depthmap.height(), IFormat::GRAY_FLOAT, IALLOCATOR_CL );

		_clraycastdepth.setArg( 0, depthmap );
		_clraycastdepth.setArg( 1, _clvolume );
		_clraycastdepth.setArg( 2, ( int ) _width);
		_clraycastdepth.setArg( 3, ( int ) _height );
		_clraycastdepth.setArg( 4, ( int ) _depth);
		_clraycastdepth.setArg( 5, sizeof( float ) * 16, projall.inverse().ptr() );
		_clraycastdepth.setArg( 6, sizeof( float ) * 16, projall.ptr() );
		_clraycastdepth.setArg( 7, scale );
		_clraycastdepth.run( CLNDRange( Math::pad16( depthmap.width() ), Math::pad16( depthmap.height() ) ), CLNDRange( 16, 16 ) );
	}


	void TSDFVolume::toSceneMesh( SceneMesh& mesh ) const
	{
		float* ptr = ( float* ) _clvolume.map();
		MarchingCubes mc( ptr, _width, _height, _depth, true );
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
