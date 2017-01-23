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

#ifndef CVT_SSDVOCL_H
#define CVT_SSDVOCL_H

#include <cvt/gfx/Image.h>
#include <cvt/geom/scene/ScenePoints.h>

#include <cvt/vision/StereoCameraCalibration.h>
#include <cvt/cl/CLKernel.h>
#include <cvt/cl/CLBuffer.h>
#include <cvt/cl/CLPyramid.h>
#include <cvt/cl/CLRGBDWarpReduce.h>
#include <cvt/vision/FeatureCL.h>
#include <cvt/vision/FASTCL.h>
#include <cvt/vision/BRIEFCL.h>

#include <Eigen/Core>

namespace cvt {

    class SSDVOCL {
        public:
            SSDVOCL( const StereoCameraCalibration& stereocalib, int levels = 5, float scale = 0.5f );
            ~SSDVOCL();

            void align( Matrix4f& RT, const Image& left, const Image& right, bool initialguess = false, size_t* inlier = NULL, float* cost = NULL );
            void updateReference( const Image& left, const Image& right, const Matrix4f* RT = NULL );

            void debug( bool warp, bool stereo );

            const Image* debugWarp() const
            {
                return _debugWarp;
            }

            const Image* debugStereo() const
            {
                return _debugStereo;
            }

            void toScenePoints( ScenePoints& pts) const;
        private:
            SSDVOCL( const SSDVOCL& other );

            void initEntries( CLBuffer& buf, const Matrix3f& Kinv, float fBfactor,
                              const CLBuffer& stereomatches, int numstereomatches );

            void mergeInit( const Matrix4f& RT, CLBuffer& stereomatches, int numstereomatches );

            void warp( CLBuffer& output, int lvl, const Matrix4f& RT,
                       const Image& frame0, const Image& frame1,
                       const Matrix3f& K, float threshold, int remove );

            void warp2( CLBuffer& output, int lvl, const Matrix4f& RT,
                        const Image& frame0, const Image& frame1,
                        const Matrix3f& K, float threshold, int remove );

            float solveDelta( Eigen::Matrix<float, 6, 1>& delta, size_t& numinlier,
                              const CLBuffer& output, size_t size, float L2reg );

            float solveDelta2( Eigen::Matrix<float, 6, 1>& delta, size_t& numinlier,
                               const CLBuffer& rgbdentry, size_t size, const Matrix3f& K, int lvl, float threshold, float L2reg );

            void detectFeatures( int& featuresize0, int& featuresize1, const Image& left, const Image& right );

            void stereoMatch( const Image& left, const Image& right,
                              const CLBuffer& features0, int size_features0,
                              const CLBuffer& features1, int size_features1,
                              const CLBuffer& sclidx1, float threshold );

            void stereoMatchBRIEF( const Image& left, const Image& right,
                                   const CLBuffer& features0, int size_features0,
                                   const CLBuffer& features1, int size_features1,
                                   const CLBuffer& sclidx1, int threshold );

            void refineStereoMatches( const Image& left, const Image& right,
                                      CLBuffer& stereomatches, int numstereomatches );

            void refineDepth( const Image& left, const Image& righti, const Matrix4f& RT );

            void debugStereoMatches( const Image& left, const Image& right,
                                     const CLBuffer& stereomatches, int numstereomatches ) const;
            void debugWarp( const Image& left, const Matrix4f& RT, const Matrix3f& K ) const;

            void clbufferAllocMinSize( CLBuffer** buf, size_t minSize, cl_mem_flags flags = CL_MEM_READ_WRITE )
            {
                // avoid unnecessary re-allocations which might be very costly on some platforms
                if( !( *buf ) || ( *buf )->size() < minSize ) {
                    if( *buf ) delete *buf;
                    *buf = new CLBuffer( minSize, flags );
                }
            }

            typedef struct {
                cl_int    valid;
                cl_float  cost;
                cl_float  weight;
                cl_float  jac[ 6 ];
                cl_float  hess[ 6 + 5 + 4 + 3 + 2 + 1 ];
            } RGBDJacobianHessCL;

            typedef struct {
                cl_int    valid;
                cl_float2 pt;
                cl_float  depth;
                cl_float  weight;
            } RGBDEntryCL;

            class CmpRGBDEntryPos
            {
                public:
                    bool operator()( const RGBDEntryCL& e1, const RGBDEntryCL& e2 )
                    {
                        if( ( int ) Math::round( e1.pt.y ) == ( int ) Math::round( e2.pt.y ) )
                            return ( int ) Math::round( e1.pt.x ) < ( int ) Math::round( e2.pt.x );
                        return ( int ) Math::round( e1.pt.y ) < ( int ) Math::round( e2.pt.y );
                    }
            };

            class CmpMatchPos
            {
                public:
                    bool operator()( const cl_float3& m1, const cl_float3& m2 )
                    {
                        if( ( int ) Math::round( m1.y ) == ( int ) Math::round( m2.y ) )
                            return ( int ) Math::round( m1.x ) < ( int ) Math::round( m2.x );
                        return ( int ) Math::round( m1.y ) < ( int ) Math::round( m2.y );
                    }
            };

            StereoCameraCalibration _stereocalib;
            Matrix3f      _K;
            float         _fBfactor;
            CLPyramid     _pyrKeyLeft;
            CLPyramid     _pyrLeft;
            CLBuffer*     _clentries;
            unsigned int  _size;
            CLBuffer      _features0;
            CLBuffer      _features1;
            CLBuffer*     _sclIdx;
            CLBuffer*     _rgbdBuffer;
            CLBuffer*     _mergeBuffer;
            CLBuffer*     _briefDesc0;
            CLBuffer*     _briefDesc1;
            CLBuffer*     _stereoMatches;
            int           _stereoMatchesSize;

            Image*   _debugWarp;
            Image*   _debugStereo;

            CLKernel _clssdvowarp;
            CLKernel _clssdvowarp2;
            CLKernel _clssdvoinit;
            CLKernel _clssdvotransformentries;
            CLKernel _clssdvostereomatch;
            CLKernel _clssdvostereorefine;
            CLKernel _clssdvorefinedepth;
            CLRGBDWarpReduce _rgbdReduce;
            FASTCL   _fastcl;
            BRIEFCL  _briefcl;
    };

}

#endif
