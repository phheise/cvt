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


#ifndef CVT_TSDFVOLUME_H
#define CVT_TSDFVOLUME_H

#include <cvt/cl/CLKernel.h>
#include <cvt/math/Matrix.h>
#include <cvt/util/String.h>
#include <cvt/geom/MarchingCubes.h>

namespace cvt
{
	class TSDFVolume
	{
		public:
			TSDFVolume( const Matrix4f& gridtoworld, size_t width, size_t height, size_t depth, float truncation = 0.1f );

			void clear( float weight = 0.0f );
			void addDepthMap( const Matrix4f& proj, const Image& depthmap, float scale );
			void addDepthMap( const Matrix3f& intrinsics, const Matrix4f& extrinsics, const Image& depthmap, float scale = 1.0f );

			void rayCastDepthMap( Image& depthmap, const Matrix4f& proj, float scale );
			void rayCastDepthMap( Image& depthmap, const Matrix3f& intrinsics, const Matrix4f& extrinsics, float scale = 1.0f );

			size_t width() const { return _width; }
			size_t height() const { return _height; }
			size_t depth() const { return _depth; }

			void toSceneMesh( SceneMesh& mesh ) const;

			void sliceX( Image& img ) const;
			void sliceY( Image& img ) const;
			void sliceZ( Image& img ) const;

			/*
			   o save or map-data
			   o to SceneMesh / GLMesh using MC
			   o addDepthNormalMap
			   o getDepthMap for intrinsics/extrinsics ...
			   o add loadRaw with width, height, depth
			 */

			void saveRaw( const String& path, bool weighted ) const;

		private:
			size_t	 _width;
			size_t	 _height;
			size_t	 _depth;
			float	 _trunc;
			Matrix4f _g2w;
			CLBuffer _clvolume;
			CLBuffer _clproj;
			CLKernel _clvolclear;
			CLKernel _clvoladd;
			CLKernel _clsliceX, _clsliceY, _clsliceZ;
			CLKernel _clraycastdepth;
	};


}

#endif
