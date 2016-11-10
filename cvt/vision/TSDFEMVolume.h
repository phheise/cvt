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
#ifndef CVT_TSDFEMVOLUME_H
#define CVT_TSDFEMVOLUME_H

#include <cvt/cl/CLKernel.h>
#include <cvt/math/Matrix.h>
#include <cvt/util/String.h>
#include <cvt/geom/MarchingCubes.h>

namespace cvt
{
    /**@brief
     * A TSDF (truncated signed distance function) implementation with more robust distance function estimation
     * based on EM (expectation maximization).
     * The samples are accumulated and the underlying noise distribution is assumed to be a Gaussian plus Uniform mixture model.
     */
    class TSDFEMVolume
    {
        public:
            TSDFEMVolume( const Matrix4f& gridtoworld, size_t width, size_t height, size_t depth, float truncation = 0.1f );
            TSDFEMVolume( const Vector3f& boxpt1, const Vector3f& boxpt2, float resolution, size_t voxeltruncation = 10 );

            void clear( float value = 1.0f, float weight = 0.0f, float weight2 = 1e5f );

            void setWeight( float weight = 1.0f );
            void setVariance( float var = 1.0f );
            void setWeightVariance( float weight = 1.0f, float outweight = 2.0f, float variance = 1.0f );
            void scaleWeight( float scale );
            void scaleWeightVariance( float scale, float varscale, float minvar = 1e-5f );

            void calcWeight();

            void addDepthMap( const Matrix4f& proj, const Image& depthmap, float scale );
            void addDepthMap( const Matrix3f& intrinsics, const Matrix4f& extrinsics, const Image& depthmap, float scale = 1.0f );

            void addDepthMapWeighted( const Matrix4f& proj, const Image& depthmap, float scale, const Image& weight );
            void addDepthMapWeighted( const Matrix3f& intrinsics, const Matrix4f& extrinsics, const Image& depthmap, float scale, const Image& weight );

            void addDepthMapNormalWeighted( const Matrix4f& proj, const Image& depthmap, float scale, const Image& normal );
            void addDepthMapNormalWeighted( const Matrix3f& intrinsics, const Matrix4f& extrinsics, const Image& depthmap, float scale, const Image& normal );

            void addShilouette( const Matrix3f& intrinsics, const Matrix4f& extrinsics, const Image& shilouette );
            void addShilouette( const Matrix4f& proj, const Image& shilouette );

            void rayCastDepthMap( Image& depthmap, const Matrix3f& intrinsics, const Matrix4f& extrinsics, float scale = 1.0f );
            void rayCastDepthNormalMap( Image& depthmap, const Matrix3f& intrinsics, const Matrix4f& extrinsics, float scale = 1.0f );
            void rayCastDepthNormalConfidenceMap( Image& normaldepthmap, Image& confidence, const Matrix3f& intrinsics, const Matrix4f& extrinsics, float scale = 1.0f );

            void normalToWeight( Image& weight, const Image& normals );

            size_t width() const { return _width; }
            size_t height() const { return _height; }
            size_t depth() const { return _depth; }

            void toSceneMesh( SceneMesh& mesh, float minweight = 1.0f ) const;

            float truncation() const { return _trunc; }
            float& truncation() { return _trunc; }

            const Matrix4f& gridToWorld() const;

            /*
               o save or map-data
               o to SceneMesh / GLMesh using MC
               o addDepthNormalMap
               o getDepthMap for intrinsics/extrinsics ...
               o add loadRaw with width, height, depth
             */

            void save( const String& path ) const;
            void load( const String& path );

        private:
            CLKernel _clvolclear;
            CLKernel _clvolsetweight;
            CLKernel _clvolsetvar;
            CLKernel _clvolsetweightvar;
            CLKernel _clvolscaleweight;
            CLKernel _clvolscaleweightvar;
            CLKernel _clvoladd;
            CLKernel _clvoladdweighted;
            CLKernel _clraycastdepth;
            CLKernel _clraycastdepthnormal;
            CLKernel _clraycastdepthnormalconfid;
            CLKernel _clshilouette;
            CLKernel _clnormweight;
            CLBuffer _clproj;
            CLBuffer _clvolume;
            size_t   _width;
            size_t   _height;
            size_t   _depth;
            float    _trunc;
            Matrix4f _g2w;
    };

    inline const Matrix4f& TSDFEMVolume::gridToWorld() const
    {
        return _g2w;
    }
}
#endif
