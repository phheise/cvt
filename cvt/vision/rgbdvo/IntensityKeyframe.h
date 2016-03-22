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

#ifndef CVT_INTENSITYKEYFRAME_H
#define CVT_INTENSITYKEYFRAME_H

#include <cvt/vision/rgbdvo/RGBDKeyframe.h>

namespace cvt {

    template <class Warp>
    class IntensityKeyframe : public RGBDKeyframe<Warp> {
        public:
            EIGEN_MAKE_ALIGNED_OPERATOR_NEW
            typedef RGBDKeyframe<Warp>                  Base;
            typedef float                               T;
            typedef typename Base::JacobianType         JacobianType;
            typedef typename Base::ScreenJacobianType   ScreenJacobianType;
            typedef typename Base::JacobianVec          JacobianVec;
            typedef typename Base::ScreenJacVec         ScreenJacVec;            
            typedef typename Base::GradientType         GradientType;

            IntensityKeyframe( const IntensityDataFactory<Warp>& factory,
                               const Matrix3f &K,
                               size_t octaves,
                               float scale );
            ~IntensityKeyframe();


            void updateOnlineData( const Matrix4f& cam2World,
                                   const ImagePyramid& pyrf,
                                   const Image& depth );

            void updateOfflineData( const Matrix4f& cam2World,
                                    const ImagePyramid& pyrf,
                                    const Image& depth );

            void recompute( std::vector<float>& residuals,
                            JacobianVec& jacobians,
                            const Warp& warp,
                            const IMapScoped<const float>& gray,
                            const IMapScoped<const float>& depth,
                            size_t octave );

    };

    template <class Warp>
    inline IntensityKeyframe<Warp>::IntensityKeyframe( const IntensityDataFactory<Warp>& factory,
                                                       const Matrix3f &K,
                                                       size_t octaves, float scale ) :
        RGBDKeyframe<Warp>( factory, K, octaves, scale )
    {
    }

    template <class AlignData>
    inline IntensityKeyframe<AlignData>::~IntensityKeyframe()
    {
    }

    template <class Warp>
    inline void IntensityKeyframe<Warp>::updateOfflineData( const Matrix4f& world2Cam,
                                                            const ImagePyramid& grayPyr,
                                                            const Image& depth )
    {
        float scale = 1.0f;

        // update the reference pose
        this->_pose = world2Cam;
        for( size_t i = 0; i < grayPyr.octaves(); i++ ){
            IntensityData<Warp>* data = ( IntensityData<Warp>* )this->_referenceData[ i ];
            data->updateOfflineData( world2Cam, grayPyr[ i ], depth, scale, this->_gradientThreshold );
            scale /= grayPyr.scaleFactor();
        }

    }

    template <class Warp>
    inline void IntensityKeyframe<Warp>::recompute( std::vector<float>& residuals,
                                                    JacobianVec& jacobians,
                                                    const Warp &warp,
                                                    const IMapScoped<const float>& gray,
                                                    const IMapScoped<const float>& /*depth*/,
                                                    size_t octave )
    {
        SIMD* simd = SIMD::instance();
        const size_t width = gray.width();
        const size_t height = gray.height();
        std::vector<Vector2f> warpedPts;
        std::vector<float> interpolatedPixels;

        const IntensityData<Warp>* data = ( const IntensityData<Warp>* )this->dataForScale( octave );
        size_t n = data->size();

        // construct the projection matrix
        Matrix4f projMat( data->intrinsics() );
        projMat *= warp.pose();

        // resize the data storage
        warpedPts.resize( n );
        interpolatedPixels.resize( n );
        residuals.resize( n );
        jacobians.resize( n );

        // project the points:
        simd->projectPoints( &warpedPts[ 0 ], projMat, data->points(), n );

        // interpolate the pixel values
        simd->warpBilinear1f( &interpolatedPixels[ 0 ], &warpedPts[ 0 ].x, gray.ptr(), gray.stride(), width, height, -10.0f, n );

        // compute the residuals
        warp.computeResiduals( &residuals[ 0 ], data->pixels(), &interpolatedPixels[ 0 ], n );

        data->recomputeJacobians( jacobians, residuals, warpedPts, interpolatedPixels );
    }

    template <class Warp>
    inline void IntensityKeyframe<Warp>::updateOnlineData( const Matrix4f& cam2World,
                                                           const ImagePyramid& pyrf,
                                                           const Image& depth )
	{
		for( size_t i = 0; i < pyrf.octaves(); ++i ){
            IntensityData<Warp>* data = ( IntensityData<Warp>* )this->_referenceData[ i ];
            data->updateOnlineData( cam2World, pyrf[ i ], depth );
		}
	}

}

#endif
