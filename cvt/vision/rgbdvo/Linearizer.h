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

#ifndef CVT_LINEARIZER_H
#define CVT_LINEARIZER_H

#include <vector>
#include <cvt/vision/ImagePyramid.h>

namespace cvt {

    // inverse compositional linearizer
    template <class AlignData>
    class InvCompLinearizer
    {
        public:
            typedef AlignData                       AlignDataType;
            typedef typename AlignData::WarpType    WarpType;
            typedef typename AlignData::JacobianVec JacobianVecType;

            InvCompLinearizer( const IKernel&, const IKernel&, size_t /*octaves*/, float /*scale*/ ){}

            void recomputeJacobians( JacobianVecType& jacobians,
                                     std::vector<float>& residuals,
                                     const std::vector<Vector2f>& /*warpedPts*/,
                                     const std::vector<float>& interpolated,
                                     const AlignData& data,
                                     size_t /*octave*/ ) const
            {
                // sort out bad pixels (out of image)
                const JacobianVecType& refJacs = data.jacobians();
                size_t savePos = 0;
                for( size_t i = 0; i < data.size(); ++i ){
                    if( interpolated[ i ] >= 0.0f ){
                        jacobians[ savePos ] = refJacs[ i ];
                        residuals[ savePos ] = residuals[ i ];
                        ++savePos;
                    }
                }
                residuals.erase( residuals.begin() + savePos, residuals.end() );
                jacobians.erase( jacobians.begin() + savePos, jacobians.end() );
            }

            void updateOnlineData( const Matrix4f&, const ImagePyramid&, const Image& )
            {
                // nothing to be done for inverse
            }

            void relinearize( AlignData&, const Matrix4f& ){ /* stays the same for inverse */ }
    };

    template <class AlignData>
    class FwdCompLinearizer {
        public:
            typedef AlignData                           AlignDataType;
            typedef typename AlignData::WarpType        WarpType;
            typedef typename AlignData::JacobianVec     JacobianVecType;
            typedef typename AlignData::ScreenJacVec    ScreenJacVecType;
            typedef typename AlignData::GradientType    GradientType;

            FwdCompLinearizer( const IKernel& kdx, const IKernel& kdy, size_t octaves, float scale ) :
                _onlineGradientsX( octaves, scale ),
                _onlineGradientsY( octaves, scale ),
                _kx( kdx ), _ky( kdy )
            {}

            void recomputeJacobians( JacobianVecType& jacobians,
                                     std::vector<float>& residuals,
                                     const std::vector<Vector2f>& warpedPts,
                                     const std::vector<float>& interpolated,
                                     const AlignData& data,
                                     size_t octave ) const
            {
                std::vector<float> intGradX;
                std::vector<float> intGradY;
                size_t n = data.size();
                intGradX.resize( n );
                intGradY.resize( n );
                // evaluate the gradients at the warped positions
                SIMD* simd = SIMD::instance();
                AlignData::interpolateGradients( intGradX, _onlineGradientsX[ octave ], warpedPts, simd );
                AlignData::interpolateGradients( intGradY, _onlineGradientsY[ octave ], warpedPts, simd );

                // sort out bad pixels (out of image)
                const ScreenJacVecType& sj = data.screenJacobians();
                GradientType grad;
                size_t savePos = 0;

                for( size_t i = 0; i < n; ++i ){
                    if( interpolated[ i ] >= 0.0f ){
                        grad.coeffRef( 0, 0 ) = intGradX[ i ];
                        grad.coeffRef( 0, 1 ) = intGradY[ i ];
                        // compute the Fwd jacobians
                        WarpType::computeJacobian( jacobians[ savePos ], sj[ i ], grad, interpolated[ i ] );
                        residuals[ savePos ] = residuals[ i ];
                        ++savePos;
                    }
                }
                residuals.erase( residuals.begin() + savePos, residuals.end() );
                jacobians.erase( jacobians.begin() + savePos, jacobians.end() );
            }

            void updateOnlineData( const Matrix4f& cam2World,
                                   const ImagePyramid& pyrf,
                                   const Image& depth )
            {
                // compute the gradients of the input
                pyrf.convolve( _onlineGradientsX, this->_kx );
                pyrf.convolve( _onlineGradientsY, this->_ky );
            }

            // needed for Fwd and ESM linearizer only
            void relinearize( AlignData& data, const Matrix4f& cam2World )
            {
                data.relinearize( cam2World );
            }

        protected:
            ImagePyramid    _onlineGradientsX;
            ImagePyramid    _onlineGradientsY;
            IKernel         _kx;
            IKernel         _ky;
    };

    template <class AlignData>
    class ESMLinearizer : public FwdCompLinearizer<AlignData>
    {
        public:
            typedef AlignData                           AlignDataType;
            typedef typename AlignData::WarpType        WarpType;
            typedef typename AlignData::JacobianType    JacobianType;
            typedef typename AlignData::JacobianVec     JacobianVecType;
            typedef typename AlignData::ScreenJacVec    ScreenJacVecType;
            typedef typename AlignData::GradientType    GradientType;

            ESMLinearizer( const IKernel& kdx, const IKernel& kdy, size_t octaves, float scale ) :
                FwdCompLinearizer<AlignData>( kdx, kdy, octaves, scale )
            {}

            void recomputeJacobians( JacobianVecType& jacobians,
                                     std::vector<float>& residuals,
                                     const std::vector<Vector2f>& warpedPts,
                                     const std::vector<float>& interpolated,
                                     const AlignData& data,
                                     size_t octave ) const
            {
                std::vector<float> intGradX, intGradY;
                size_t n = data.size();
                intGradX.resize( n );
                intGradY.resize( n );

                // evaluate the gradients at the warped positions
                SIMD* simd = SIMD::instance();
                AlignData::interpolateGradients( intGradX, this->_onlineGradientsX[ octave ], warpedPts, simd );
                AlignData::interpolateGradients( intGradY, this->_onlineGradientsY[ octave ], warpedPts, simd );

                // sort out bad pixels (out of image)
                const JacobianVecType& refJacs = data.jacobians();
                const ScreenJacVecType& sj = data.screenJacobians();
                GradientType grad;
                size_t savePos = 0;

                JacobianType jCur;
                for( size_t i = 0; i < n; ++i ){
                    if( interpolated[ i ] >= 0.0f ){
                        grad.coeffRef( 0, 0 ) = intGradX[ i ];
                        grad.coeffRef( 0, 1 ) = intGradY[ i ];
                        // compute the ESM jacobians
                        WarpType::computeJacobian( jCur, sj[ i ], grad, interpolated[ i ] );
                        jacobians[ savePos ] = 0.5f * ( refJacs[ i ] + jCur );
                        residuals[ savePos ] = residuals[ i ];
                        ++savePos;
                    }
                }
                residuals.erase( residuals.begin() + savePos, residuals.end() );
                jacobians.erase( jacobians.begin() + savePos, jacobians.end() );
            }
    };


}

#endif // LINEARIZER_H
