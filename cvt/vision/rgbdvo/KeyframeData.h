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

#ifndef CVT_KEYFRAMEDATA_H
#define CVT_KEYFRAMEDATA_H

#include <Eigen/Core>
#include <cvt/gfx/Image.h>
#include <cvt/vision/ImagePyramid.h>
#include <cvt/gfx/IMapScoped.h>
#include <cvt/vision/rgbdvo/RGBDPreprocessor.h>
#include <cvt/vision/rgbdvo/GradientThresholdSelection.h>

namespace cvt {

    class ReferencePoints
    {
        public:
            ReferencePoints(){}
            virtual ~ReferencePoints(){}

            virtual void reserve( size_t size ){ _points3d.reserve( size ); }
            virtual void clear(){ _points3d.clear(); }

            void addPoint( const Vector3f& point ){ _points3d.push_back( point ); }

            size_t size() const { return _points3d.size(); }

            const Matrix3f& intrinsics() const { return _intrinsics; }
            void setIntrinsics( const Matrix3f& intr ){ _intrinsics = intr; }

            const Vector3f* points() const { return &_points3d[ 0 ]; }


        protected:
            Matrix3f                _intrinsics;
            std::vector<Vector3f>   _points3d;

            void  initializePointLookUps( float* vals, size_t n, float foc, float c ) const
            {
                float invF = 1.0f / foc;
                for( size_t i = 0; i < n; i++ ){
                    vals[ i ] = ( i - c ) * invF;
                }
            }
    };

    /**
     * \class AlignmentData for reference (template) information
     */
    template <class Warp>
    class IntensityData : public ReferencePoints {
        public:
            //EIGEN_MAKE_ALIGNED_OPERATOR_NEW
            typedef Warp                            WarpType;
            typedef typename Warp::JacobianType     JacobianType;
            typedef typename Warp::ScreenJacType    ScreenJacobianType;
            typedef typename Warp::HessianType      HessianType;
            typedef Eigen::Matrix<float, 1, 2>      GradientType;
            typedef std::vector<ScreenJacobianType, Eigen::aligned_allocator<ScreenJacobianType> > ScreenJacVec;
            typedef std::vector<JacobianType, Eigen::aligned_allocator<JacobianType> > JacobianVec;

            IntensityData(){}
            virtual ~IntensityData(){}

            virtual void clear()
            {
                ReferencePoints::clear();
                _pixelValues.clear();
                _jacobians.clear();
            }

            virtual void reserve( size_t size )
            {
                ReferencePoints::reserve( size );
                _pixelValues.reserve( size );
                _jacobians.reserve( size );
            }

            virtual void recomputeJacobians( JacobianVec& jacobians,
                                             std::vector<float>& residuals,
                                             const std::vector<Vector2f>& warpedPts,
                                             const std::vector<float>& interpolated ) const = 0;

            const float* pixels()    const { return &_pixelValues[ 0 ]; }

            virtual void erase( size_t n )
            {
                _jacobians.erase( _jacobians.begin() + n, _jacobians.end() );
                _points3d.erase( _points3d.begin() + n, _points3d.end() );
                _pixelValues.erase( _pixelValues.begin() + n, _pixelValues.end() );
            }


            static void interpolateGradients( std::vector<float>& result, const Image& gradImg, const std::vector<Vector2f>& positions, const SIMD* simd, float outOfBoundsVal = -20.0f )
            {
                IMapScoped<const float> map( gradImg );
                simd->warpBilinear1f( &result[ 0 ], &positions[ 0 ].x, map.ptr(), map.stride(), gradImg.width(), gradImg.height(), outOfBoundsVal, positions.size() );
            }

            const JacobianVec& jacobians() const { return _jacobians; }

            virtual void updateOfflineData( const Matrix4f& world2cam,
                                            const Image& gray,
                                            const Image& depth,
                                            float scaleFac,
                                            float gradientThresh ) = 0;

            virtual void updateOnlineData( const Matrix4f&,
                                           const Image&,
                                           const Image& ){}

        protected:
            std::vector<float>          _pixelValues;
            JacobianVec                 _jacobians;            


    };

    template <class Warp>
    class IntensityDataInvComp : public IntensityData<Warp>
    {
        public:
            typedef IntensityData<Warp>                 Base;
            typedef typename Base::JacobianType         JacobianType;
            typedef typename Base::JacobianVec          JacobianVecType;
            typedef typename Base::ScreenJacobianType   ScreenJacobianType;
            typedef typename Base::GradientType         GradientType;
            typedef typename Base::ScreenJacVec         ScreenJacVec;

            IntensityDataInvComp(){}


            void recomputeJacobians( JacobianVecType& jacobians,
                                     std::vector<float>& residuals,
                                     const std::vector<Vector2f>& /*warpedPts*/,
                                     const std::vector<float>& interpolated ) const
            {

                const JacobianVecType& refJacs = this->jacobians();
                size_t savePos = 0;
                // sort out data which is out of image bounds:
                for( size_t i = 0; i < this->size(); ++i ){
                    if( interpolated[ i ] >= 0.0f ){
                        jacobians[ savePos ] = refJacs[ i ];
                        residuals[ savePos ] = residuals[ i ];
                        ++savePos;
                    }
                }
                residuals.erase( residuals.begin() + savePos, residuals.end() );
                jacobians.erase( jacobians.begin() + savePos, jacobians.end() );
            }

            void updateOfflineData( const Matrix4f& world2Cam,
                                    const Image& gray,
                                    const Image& depth,
                                    float scaleFactor,
                                    float gradientThresh )
            {
                IMapScoped<const float> depthMap( depth );
                const float* d = depthMap.ptr();
                size_t depthStride = depthMap.stride() / sizeof( float );

                Vector2f currP;
                Vector3f p3d, p3dw;
                GradientType g;
                JacobianType j;
                ScreenJacobianType sj;

                // compute image gradient
                Image gradX, gradY;
                RGBDPreprocessor::instance().gradient( gradX, gradY, gray );


                GradientThresholdSelection threshSelector( gradX, gradY, depth );
                //gradientThresh = threshSelector.threshold( gradientThresh * scaleFactor );
                size_t selectionSize = gradX.width() * gradX.height() * gradientThresh * scaleFactor;
                gradientThresh = threshSelector.selectBiggestN( selectionSize );

                // remove old data
                this->clear();

                size_t pixelsOnOctave = ( gray.width() * gray.height() );
                this->reserve( 0.4f * pixelsOnOctave );

                // TODO: replace this by a simd function!
                // temp vals
                std::vector<float> tmpx( gray.width() );
                std::vector<float> tmpy( gray.height() );

                const Matrix3f& intr = this->intrinsics();

                this->initializePointLookUps( &tmpx[ 0 ], tmpx.size(), intr[ 0 ][ 0 ], intr[ 0 ][ 2 ] );
                this->initializePointLookUps( &tmpy[ 0 ], tmpy.size(), intr[ 1 ][ 1 ], intr[ 1 ][ 2 ] );

                IMapScoped<const float> gxMap( gradX );
                IMapScoped<const float> gyMap( gradY );
                IMapScoped<const float> grayMap( gray );

                for( size_t y = 0; y < gray.height() - 1; y++ ){
                    const float* gx = gxMap.ptr();
                    const float* gy = gyMap.ptr();
                    const float* value = grayMap.ptr();

                    // scale the point
                    currP.y = scaleFactor * y;

                    for( size_t x = 0; x < gray.width() - 1; x++ ){
                        g( 0, 0 ) = gx[ x ];
                        g( 0, 1 ) = gy[ x ];
                        float salience = Math::abs( g.coeff( 0, 0 ) ) + Math::abs( g.coeff( 0, 1 ) );
                        if( salience < gradientThresh )
                            continue;

                        currP.x = scaleFactor * x;
                        float z = RGBDPreprocessor::instance().interpolateDepth( currP, d, depthStride );
                        if( z > 0.0f ){
                            p3d[ 0 ] = tmpx[ x ] * z;
                            p3d[ 1 ] = tmpy[ y ] * z;
                            p3d[ 2 ] = z;

                            // point in world coord frame
                            // TODO: change back to KF relative storage of points?
                            p3dw = world2Cam * p3d;

                            Warp::screenJacobian( sj, p3d, intr );

                            // add jacobian for the point
                            this->_jacobians.push_back( JacobianType() );
                            JacobianType& j = this->_jacobians.back();

                            // precompute it using the gradient and screen jacobian value
                            Warp::computeJacobian( j, sj, g, value[ x ] );

                            // this is to get the inverse incremental pose update
                            j.template head<6>() *= -1.0f;

                            // add point
                            this->_points3d.push_back( p3dw );

                            // add pixel value
                            this->_pixelValues.push_back( value[ x ] );

                        }
                    }
                    gxMap++;
                    gyMap++;
                    grayMap++;
                }
            }

    };

    template <class Warp>
    class IntensityDataFwdComp : public IntensityData<Warp>
    {
        public:
            typedef IntensityData<Warp>                 Base;
            typedef typename Base::JacobianType         JacobianType;
            typedef typename Base::JacobianVec          JacobianVecType;
            typedef typename Base::ScreenJacobianType   ScreenJacobianType;
            typedef typename Base::GradientType         GradientType;
            typedef typename Base::ScreenJacVec         ScreenJacVec;

            IntensityDataFwdComp(){}

            virtual void reserve( size_t size )
            {
                IntensityData<Warp>::reserve( size );
                _screenJacobians.reserve( size );
            }

            virtual void clear()
            {
                IntensityData<Warp>::clear();
                _screenJacobians.clear();
            }

            const ScreenJacVec& screenJacobians() const { return _screenJacobians; }

            virtual void recomputeJacobians( JacobianVecType& jacobians,
                                             std::vector<float>& residuals,
                                             const std::vector<Vector2f>& warpedPts,
                                             const std::vector<float>& interpolated ) const
            {
                size_t n = this->size();
                std::vector<float> intGradX( n );
                std::vector<float> intGradY( n );

                // evaluate the gradients at the warped positions
                SIMD* simd = SIMD::instance();
                this->interpolateGradients( intGradX, _gradX, warpedPts, simd );
                this->interpolateGradients( intGradY, _gradY, warpedPts, simd );

                // sort out bad pixels (out of image)
                const ScreenJacVec& sj = _screenJacobians;
                GradientType grad;
                size_t savePos = 0;

                for( size_t i = 0; i < n; ++i ){
                    if( interpolated[ i ] >= 0.0f ){
                        grad.coeffRef( 0, 0 ) = intGradX[ i ];
                        grad.coeffRef( 0, 1 ) = intGradY[ i ];

                        // compute the Fwd jacobians
                        Warp::computeJacobian( jacobians[ savePos ], sj[ i ], grad, interpolated[ i ] );
                        jacobians[ savePos ] *= -1.0f;
                        residuals[ savePos ] = residuals[ i ];
                        ++savePos;
                    }
                }
                residuals.erase( residuals.begin() + savePos, residuals.end() );
                jacobians.erase( jacobians.begin() + savePos, jacobians.end() );
            }


            virtual void erase( size_t n )
            {
                IntensityData<Warp>::erase( n );
                _screenJacobians.erase( _screenJacobians.begin() + n, _screenJacobians.end() );
            }

            virtual void updateOnlineData( const Matrix4f& pose,
                                           const Image& gray,
                                           const Image& /*depth*/ )
            {
                RGBDPreprocessor::instance().gradient( _gradX, _gradY, gray );
                relinearize( pose );
            }

            /* update the screen jacobians */
            void relinearize( const Matrix4f& cam2World )
            {
                // transform points into camera coordinate frame
                size_t n = this->size();
                std::vector<Vector3f> pCam( n );
                SIMD::instance()->transformPoints( pCam.data(), cam2World, this->points(), n );

                // re-evaluate the screen jacobians
                _screenJacobians.resize( n );
                for( size_t i = 0; i < n; ++i ){
                    Warp::screenJacobian( _screenJacobians[ i ], pCam[ i ], this->_intrinsics );
                }
            }

            void updateOfflineData( const Matrix4f& pose,
                                    const Image& gray,
                                    const Image& depth,
                                    float scaleFactor,
                                    float gradientThresh )
            {
                IMapScoped<const float> depthMap( depth );
                const float* d = depthMap.ptr();
                size_t depthStride = depthMap.stride() / sizeof( float );

                Vector2f currP;
                Vector3f p3d, p3dw;
                GradientType g;

                // compute image gradient
                Image gradX, gradY;
                RGBDPreprocessor::instance().gradient( gradX, gradY, gray );

                // remove old data
                this->clear();

                size_t pixelsOnOctave = ( gray.width() * gray.height() );
                this->reserve( 0.4f * pixelsOnOctave );

                // TODO: replace this by a simd function!
                // temp vals
                std::vector<float> tmpx( gray.width() );
                std::vector<float> tmpy( gray.height() );

                const Matrix3f& intr = this->intrinsics();
                this->initializePointLookUps( &tmpx[ 0 ], tmpx.size(), intr[ 0 ][ 0 ], intr[ 0 ][ 2 ] );
                this->initializePointLookUps( &tmpy[ 0 ], tmpy.size(), intr[ 1 ][ 1 ], intr[ 1 ][ 2 ] );

                IMapScoped<const float> gxMap( gradX );
                IMapScoped<const float> gyMap( gradY );
                IMapScoped<const float> grayMap( gray );

                for( size_t y = 0; y < gray.height() - 1; y++ ){
                    const float* gx = gxMap.ptr();
                    const float* gy = gyMap.ptr();
                    const float* value = grayMap.ptr();

                    // scale the point
                    currP.y = scaleFactor * y;

                    for( size_t x = 0; x < gray.width() - 1; x++ ){
                        g( 0, 0 ) = gx[ x ];
                        g( 0, 1 ) = gy[ x ];

                        float salience = Math::abs( g.coeff( 0, 0 ) ) + Math::abs( g.coeff( 0, 1 ) );
                        if( salience < gradientThresh )
                            continue;

                        currP.x = scaleFactor * x;
                        float z = RGBDPreprocessor::instance().interpolateDepth( currP, d, depthStride );
                        if( z > 0.0f ){
                            p3d[ 0 ] = tmpx[ x ] * z;
                            p3d[ 1 ] = tmpy[ y ] * z;
                            p3d[ 2 ] = z;

                            // point in world coord frame
                            // TODO: change back to KF relative storage of points?
                            p3dw = pose * p3d;

                            // add point
                            this->_points3d.push_back( p3dw );

                            // add pixel value
                            this->_pixelValues.push_back( value[ x ] );
                        }
                    }
                    gxMap++;
                    gyMap++;
                    grayMap++;
                }
            }

        protected:
            ScreenJacVec    _screenJacobians;
            Image           _gradX;
            Image           _gradY;
    };

    template <class Warp>
    class IntensityDataESM : public IntensityDataFwdComp<Warp>
    {
        public:
            typedef IntensityData<Warp>                 Base;
            typedef typename Base::JacobianType         JacobianType;
            typedef typename Base::JacobianVec          JacobianVecType;
            typedef typename Base::ScreenJacobianType   ScreenJacobianType;
            typedef typename Base::ScreenJacVec         ScreenJacVec;
            typedef typename Base::GradientType         GradientType;

            IntensityDataESM(){}

            virtual void reserve( size_t size )
            {
                IntensityDataFwdComp<Warp>::reserve( size );
                _referenceGradients.reserve( size );
            }

            virtual void clear()
            {
                IntensityDataFwdComp<Warp>::clear();
                _referenceGradients.clear();
            }

            virtual void erase( size_t n )
            {
                IntensityDataFwdComp<Warp>::erase( n );
                _referenceGradients.erase( _referenceGradients.begin() + n, _referenceGradients.end() );
            }

            void recomputeJacobians( JacobianVecType& jacobians,
                                     std::vector<float>& residuals,
                                     const std::vector<Vector2f>& warpedPts,
                                     const std::vector<float>& interpolated ) const
            {
                size_t n = this->size();
                std::vector<float> intGradX( n );
                std::vector<float> intGradY( n );

                // evaluate the gradients at the warped positions
                SIMD* simd = SIMD::instance();
                this->interpolateGradients( intGradX, this->_gradX, warpedPts, simd );
                this->interpolateGradients( intGradY, this->_gradY, warpedPts, simd );

                // sort out bad pixels (out of image)
                const ScreenJacVec& sj = this->screenJacobians();
                GradientType grad;
                size_t savePos = 0;

                for( size_t i = 0; i < n; ++i ){
                    if( interpolated[ i ] >= 0.0f ){
                        grad.coeffRef( 0, 0 ) = 0.5 * ( intGradX[ i ] + _referenceGradients[ i ].coeffRef( 0, 0 ) );
                        grad.coeffRef( 0, 1 ) = 0.5 * ( intGradY[ i ] + _referenceGradients[ i ].coeffRef( 0, 1 ) );

                        // compute the ESM jacobians
                        Warp::computeJacobian( jacobians[ savePos ], sj[ i ], grad, interpolated[ i ] );
                        jacobians[ savePos ] *= -1.0f;
                        residuals[ savePos ] = residuals[ i ];
                        ++savePos;
                    }
                }
                residuals.erase( residuals.begin() + savePos, residuals.end() );
                jacobians.erase( jacobians.begin() + savePos, jacobians.end() );
            }

            void updateOfflineData( const Matrix4f& pose,
                                    const Image& gray,
                                    const Image& depth,
                                    float scaleFactor,
                                    float gradientThresh )
            {
                IMapScoped<const float> depthMap( depth );
                const float* d = depthMap.ptr();
                size_t depthStride = depthMap.stride() / sizeof( float );

                Vector2f currP;
                Vector3f p3d, p3dw;
                GradientType g;

                // compute image gradient
                Image gradX, gradY;
                RGBDPreprocessor::instance().gradient( gradX, gradY, gray );

                // remove old data
                this->clear();

                size_t pixelsOnOctave = ( gray.width() * gray.height() );
                this->reserve( 0.4f * pixelsOnOctave );

                // TODO: replace this by a simd function!
                // temp vals
                std::vector<float> tmpx( gray.width() );
                std::vector<float> tmpy( gray.height() );

                const Matrix3f& intr = this->intrinsics();
                this->initializePointLookUps( &tmpx[ 0 ], tmpx.size(), intr[ 0 ][ 0 ], intr[ 0 ][ 2 ] );
                this->initializePointLookUps( &tmpy[ 0 ], tmpy.size(), intr[ 1 ][ 1 ], intr[ 1 ][ 2 ] );

                IMapScoped<const float> gxMap( gradX );
                IMapScoped<const float> gyMap( gradY );
                IMapScoped<const float> grayMap( gray );

                for( size_t y = 0; y < gray.height() - 1; y++ ){
                    const float* gx = gxMap.ptr();
                    const float* gy = gyMap.ptr();
                    const float* value = grayMap.ptr();

                    // scale the point
                    currP.y = scaleFactor * y;

                    for( size_t x = 0; x < gray.width() - 1; x++ ){
                        g( 0, 0 ) = gx[ x ];
                        g( 0, 1 ) = gy[ x ];

                        float salience = Math::abs( g.coeff( 0, 0 ) ) + Math::abs( g.coeff( 0, 1 ) );
                        if( salience < gradientThresh )
                            continue;

                        currP.x = scaleFactor * x;
                        float z = RGBDPreprocessor::instance().interpolateDepth( currP, d, depthStride );
                        if( z > 0.0f ){
                            p3d[ 0 ] = tmpx[ x ] * z;
                            p3d[ 1 ] = tmpy[ y ] * z;
                            p3d[ 2 ] = z;

                            // point in world coord frame
                            // TODO: change back to KF relative storage of points?
                            p3dw = pose * p3d;

                            // add point
                            this->_points3d.push_back( p3dw );

                            // add pixel value
                            this->_pixelValues.push_back( value[ x ] );
                            _referenceGradients.push_back( g );
                        }
                    }
                    gxMap++;
                    gyMap++;
                    grayMap++;
                }
            }

        protected:
            std::vector<GradientType>  _referenceGradients;
    };

}

#endif // CVT_KEYFRAMEDATA_H
