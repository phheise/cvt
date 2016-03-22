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


#ifndef CVT_PHOTOMETRICERROR_H
#define CVT_PHOTOMETRICERROR_H

#include <cvt/math/Matrix.h>
#include <cvt/vision/ImagePyramid.h>
#include <cvt/vision/rgbdvo/DVOCostFunction.h>
#include <cvt/vision/rgbdvo/IntensityKeyframe.h>

#include <cvt/vision/rgbdvo/ReferenceFactory.h>

#include <cvt/vision/rgbdvo/KeyframeData.h>
#include <cvt/util/ConfigFile.h>

namespace cvt {
    template <class Warp> class PhotometricError;

    // Partial Specialization
    template <class Warp>
    struct CostFuncTrait<PhotometricError<Warp> >
    {
        typedef float                                       DataType;
        typedef Eigen::Matrix<DataType, Warp::NParams, 1>   ParameterType;
        typedef Eigen::Matrix<DataType, 1, Warp::NParams>   JacobianType;
        typedef Eigen::aligned_allocator<JacobianType>      AlocType;
        typedef std::vector<JacobianType, AlocType>         JacobianVectorType;
        typedef Eigen::Matrix<DataType, Warp::NParams, Warp::NParams>  HessianType;

        typedef DataType                                    ResidualType;
        typedef std::vector<ResidualType>                   ResidualVectorType;
        typedef Warp                                        ModelType;
    };

    template <class Warp>
    class PhotometricError : public DVOCostFunction<PhotometricError<Warp> >
    {
            typedef CostFunction<PhotometricError<Warp> >   BaseType;
            typedef typename BaseType::ResidualVectorType   ResidualVectorType;
            typedef typename BaseType::JacobianVectorType   JacobianVectorType;
            typedef typename BaseType::ParameterType        ParameterType;
            typedef typename BaseType::ModelType            ModelType;

        public:
            typedef IntensityKeyframe<Warp>     KFType;

            enum LinType
            {
                InvComp,
                FwdComp,
                ESM
            };

            static const IntensityDataFactory<Warp>& Factory( LinType type )
            {
                static IntensityDataInvCompFactory<Warp> InvFac;
                static IntensityDataFwdCompFactory<Warp> FwdFac;
                static IntensityDataESMFactory<Warp> ESMFac;
                switch( type ){
                    case InvComp: return InvFac;
                    case FwdComp: return FwdFac;
                    case ESM:     return ESMFac;
                }
                return InvFac; // should not happen!
            }

            struct Params {
                    Params():
                        octaves( 3 ),
                        scale( 0.5f ),
                        depthScale( 1000.0f ),
                        minDepth( 0.5f ),
                        maxDepth( 10.0f ),
                        gradientThreshold( 0.02f ),
                        useInformationSelection( false ),
                        selectionPixelPercentage( 0.3f ),
                        linearizer( InvComp )
                    {}

                    Params( ConfigFile& cfg ) :
                        octaves( cfg.valueForName<int>( "pyrOctaves", 3 ) ),
                        scale( cfg.valueForName<float>( "pyrScale", 0.5f ) ),
                        depthScale( cfg.valueForName<float>( "depthFactor", 1000.0f ) *
                                    cfg.valueForName<float>( "depthScale", 1.0f ) ),
                        minDepth( cfg.valueForName<float>( "minDepth", 0.5f ) ),
                        maxDepth( cfg.valueForName<float>( "maxDepth", 10.0f ) ),
                        gradientThreshold( cfg.valueForName<float>( "gradientThreshold", 0.02f ) ),
                        useInformationSelection( cfg.valueForName<bool>( "useInformationSelection", false ) ),
                        selectionPixelPercentage( cfg.valueForName<float>( "selectionPixelPercentage", 0.3f ) ),
                        linearizer( InvComp )
                    {
                        String linName = cfg.valueForName<String>( "linearizer", "IC" ).toUpper();
                        if( linName == "IC" ) linearizer = InvComp;
                        else if( linName == "FC" ) linearizer = FwdComp;
                        else if( linName == "ESM" ) linearizer = ESM;
                        else { linearizer = InvComp; }
                    }

                    size_t octaves;
                    float  scale;
                    float  depthScale;
                    float  minDepth;
                    float  maxDepth;
                    float  gradientThreshold;
                    bool   useInformationSelection;
                    float  selectionPixelPercentage;

                    LinType linearizer;

            };

            PhotometricError( const Matrix3f& K,
                              const Params& p = Params() ) :
                _grayPyr( p.octaves, p.scale ),
                _reference( Factory( p.linearizer ), K, p.octaves, p.scale )
            {
                // TODO: move this to DVOCostFunction
                RGBDPreprocessor::instance().setDepthScale( p.depthScale );
                RGBDPreprocessor::instance().setMaxDepth( p.maxDepth );
                RGBDPreprocessor::instance().setMinDepth( p.minDepth );
                _reference.setGradientThreshold( p.gradientThreshold );
                _reference.setUseInformationSelection( p.useInformationSelection );
                _reference.setSelectionPixelPercentage( p.selectionPixelPercentage );

            }

            virtual ~PhotometricError(){}

            void setInput( const Image& gray,
                           const Image& depth )
            {
                // TODO: check if the types are all GRAY_FLOAT!
                _grayPyr.update( gray );
                _depthMap = depth;

                _reference.updateOnlineData( _warp.pose(), _grayPyr, depth );
            }

            KFType& reference() { return _reference; }

            /** set the current estimate/prediction
             *  pose is meant to be from world to camera (T_wc)
             */
            void setPose( const Matrix4f& pose )
            {
                _warp.setPose( pose.inverse() );
            }

            Matrix4f pose() const { return _warp.pose().inverse(); }

            const Matrix4f& referencePose() const
            {
                return _reference.pose();
            }

            size_t scales() const { return _grayPyr.octaves(); }

            void evaluate( ResidualVectorType& residuals,
                           JacobianVectorType& jacobians,
                           size_t scale );

            void update( const ParameterType& delta )
            {
                _warp.updateParameters( delta );
            }

            void setModel( const ModelType& warp )
            {
                _warp = warp;
            }

            const ModelType& model() const { return _warp; }

            void updateOfflineData();

            size_t modelSize( size_t octave = 0 ) const { return _reference.dataForScale( octave )->size(); }

        private:
            // current warp: cam -> world
            Warp                _warp;

            // current grayscale pyramid:
            ImagePyramid        _grayPyr;

            // current depthmap
            Image               _depthMap;

            // IntensityKeyframe holds the reference data
            KFType              _reference;

    };

    template <class Warp>
    inline void PhotometricError<Warp>::evaluate( ResidualVectorType& residuals,
                                                  JacobianVectorType& jacobians,
                                                  size_t scale )
    {
        SIMD* simd = SIMD::instance();

        // retrieve corresponding scale space data
        IMapScoped<const float> gray( _grayPyr[ scale ] );

        const IntensityData<Warp>* referenceData = ( const IntensityData<Warp>* )_reference.dataForScale( scale );

        const size_t width = gray.width();
        const size_t height = gray.height();

        size_t n = referenceData->size();
        std::vector<Vector2f> warpedPts( n );
        std::vector<float> interpolatedPixels( n );
        // resize the data storage
        residuals.resize( n );
        jacobians.resize( n );

        // construct the projection matrix
        Matrix4f projMat( referenceData->intrinsics() );

        // pose is already cam2World
        projMat *= _warp.pose();

        // project the points:
        simd->projectPoints( &warpedPts[ 0 ], projMat, referenceData->points(), n );

        // interpolate the pixel values
        simd->warpBilinear1f( &interpolatedPixels[ 0 ], &warpedPts[ 0 ].x, gray.ptr(), gray.stride(), width, height, -10.0f, n );

        // compute the residuals
        _warp.computeResiduals( &residuals[ 0 ], referenceData->pixels(), &interpolatedPixels[ 0 ], n );

        referenceData->recomputeJacobians( jacobians, residuals, warpedPts, interpolatedPixels );
    }

    template <class Warp>
    inline void PhotometricError<Warp>::updateOfflineData()
    {
        _reference.updateOfflineData( pose(), _grayPyr, _depthMap );
    }

}

#endif // PHOTOMETRICERROR_H
