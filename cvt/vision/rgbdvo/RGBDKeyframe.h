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


#ifndef RGBDKEYFRAME_H
#define RGBDKEYFRAME_H

#include <cvt/math/Matrix.h>
#include <cvt/vision/ImagePyramid.h>
#include <cvt/gfx/IMapScoped.h>
#include <cvt/util/EigenBridge.h>

#include <cvt/vision/rgbdvo/RGBDWarp.h>
#include <cvt/vision/rgbdvo/InformationSelection.h>
#include <cvt/vision/rgbdvo/CostFunction.h>
#include <cvt/vision/rgbdvo/KeyframeData.h>
#include <cvt/vision/rgbdvo/ReferenceFactory.h>

#include <Eigen/StdVector>

namespace cvt
{
    template <class Warp>
    class RGBDKeyframe {
        public:
            EIGEN_MAKE_ALIGNED_OPERATOR_NEW
            typedef typename Warp::JacobianType     JacobianType;
            typedef typename Warp::ScreenJacType    ScreenJacobianType;
            typedef typename Warp::HessianType      HessianType;
            typedef Eigen::Matrix<float, 1, 2>      GradientType;

            typedef std::vector<ScreenJacobianType, Eigen::aligned_allocator<ScreenJacobianType> > ScreenJacVec;
            typedef std::vector<JacobianType, Eigen::aligned_allocator<JacobianType> > JacobianVec;           

            RGBDKeyframe( const ReferenceFactory& factory,
                          const Matrix3f& K,
                          size_t octaves, float scale );

            virtual ~RGBDKeyframe();

			const Matrix4f&   pose()					const { return _pose; }

            void setGradientThreshold( float thresh )       { _gradientThreshold = thresh; }
            void setSelectionPixelPercentage( float n )     { _pixelPercentageToSelect = n; }
            void setUseInformationSelection( bool v )       { _useInformationSelection = v; }

//            virtual void updateOfflineData( const Matrix4f& pose, const ImagePyramid& pyramid, const Image& depth ) = 0;
//            virtual void updateOnlineData( const Matrix4f& cam2World, const ImagePyramid& pyrf, const Image& depth ) = 0;

            const ReferencePoints*  dataForScale( size_t octave ) const { return _referenceData[ octave ]; }

        protected:
            std::vector<ReferencePoints*> _referenceData;
            Matrix4f            _pose;

            float               _gradientThreshold;
            float               _pixelPercentageToSelect;
            bool                _useInformationSelection;


    };

    template <class Warp>
    inline RGBDKeyframe<Warp>::RGBDKeyframe( const ReferenceFactory& factory,
                                             const Matrix3f& K,
                                             size_t octaves,
                                             float scale ) :
        _gradientThreshold( 0.0f ),
        _pixelPercentageToSelect( 0.3f ),
        _useInformationSelection( false )
    {
        Matrix3f Ks( K );
        for( size_t i = 0; i < octaves; ++i ){
            _referenceData.push_back( factory.create() );
            _referenceData.back()->setIntrinsics( Ks );
            Ks *= scale;
            Ks[ 2 ][ 2 ] = 1.0f;
        }
    }

    template <class Warp>
    inline RGBDKeyframe<Warp>::~RGBDKeyframe()
    {
        for( size_t i = 0; i < _referenceData.size(); ++i ){
            delete _referenceData[ i ];
        }
        _referenceData.clear();
    }


}

#endif // RGBDKEYFRAME_H
