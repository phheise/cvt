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


#ifndef CVT_RGBDVISUALODOMETRY_H
#define CVT_RGBDVISUALODOMETRY_H

#include <cvt/gfx/Image.h>
#include <cvt/util/EigenBridge.h>
#include <cvt/util/Signal.h>
#include <cvt/util/CVTAssert.h>
#include <cvt/util/ConfigFile.h>

#include <cvt/vision/rgbdvo/RGBDKeyframe.h>
#include <cvt/vision/rgbdvo/Optimizer.h>
#include <cvt/vision/rgbdvo/DVOCostFunction.h>

namespace cvt {

    template <class Derived>
    class RGBDVisualOdometry
    {
        public:
            typedef Optimizer<Derived>	OptimizerType;
            typedef typename OptimizerType::Result			Result;

            struct Params {
                Params() :
                    pyrOctaves( 3 ),
                    pyrScale( 0.5f ),
                    maxTranslationDistance( 0.4f ),
                    maxRotationDistance( Math::deg2Rad( 5.0f ) ),
                    maxSSDSqr( Math::sqr( 0.2f ) ),
                    minPixelPercentage( 0.3f ),
                    autoReferenceUpdate( true ),
                    depthScale( 1000.0f ),
                    minDepth( 0.5f ),
                    maxDepth( 10.0f ),
                    gradientThreshold( 0.02f ),
                    useInformationSelection( false ),
                    selectionPixelPercentage( 0.3f ),
                    maxIters( 10 ),
                    minParameterUpdate( 1e-6 ),
                    maxNumKeyframes( 1 )
                {}

                Params( ConfigFile& cfg ) :
                    pyrOctaves( cfg.valueForName<int>( "pyrOctaves", 3 ) ),
                    pyrScale( cfg.valueForName<float>( "pyrScale", 0.5f ) ),
                    maxTranslationDistance( cfg.valueForName<float>( "maxTranslationDistance", 0.4f ) ),
                    maxRotationDistance( Math::deg2Rad( cfg.valueForName<float>( "maxRotationDistance", 5.0f ) ) ),
                    maxSSDSqr( Math::sqr( cfg.valueForName<float>( "maxSSD", 0.2f ) ) ),
                    minPixelPercentage( cfg.valueForName<float>( "minPixelPercentage", 0.3f ) ),
                    autoReferenceUpdate( cfg.valueForName<bool>( "autoReferenceUpdate", true ) ),
                    depthScale( cfg.valueForName<float>( "depthFactor", 1000.0f ) *
                                cfg.valueForName<float>( "depthScale", 1.0f ) ),
                    minDepth( cfg.valueForName<float>( "minDepth", 0.5f ) ),
                    maxDepth( cfg.valueForName<float>( "maxDepth", 10.0f ) ),
                    gradientThreshold( cfg.valueForName<float>( "gradientThreshold", 0.02f ) ),
                    useInformationSelection( cfg.valueForName<bool>( "useInformationSelection", false ) ),
                    selectionPixelPercentage( cfg.valueForName<float>( "selectionPixelPercentage", 0.3f ) ),
                    maxIters( cfg.valueForName<int>( "maxIters", 10 ) ),
                    minParameterUpdate( cfg.valueForName<float>( "minParameterUpdate", 1e-6f ) ),
                    maxNumKeyframes( cfg.valueForName<int>( "maxNumKeyframes", 1 ) )
                {
                    // TODO: Params should become a parameterset
                    // conversion between paramset and configfile!
                }

               // pyramid
               size_t pyrOctaves;
               float  pyrScale;

               // keyframe recreation thresholds
               float maxTranslationDistance;
               float maxRotationDistance;
               float maxSSDSqr;
               float minPixelPercentage;

               // automatic update of reference when needed
               bool autoReferenceUpdate;

			   // depthScale = #pixels / m
               float depthScale;
               float minDepth;
               float maxDepth;
               float gradientThreshold;

               bool  useInformationSelection;
               float selectionPixelPercentage;

               // optimizer:
               size_t   maxIters;
               float    minParameterUpdate;

               int      maxNumKeyframes;
            };

            RGBDVisualOdometry( OptimizerType* optimizer,
                                DVOCostFunction<Derived>* costFunc,
                                const Matrix3f& K,
                                const Params& params );
            ~RGBDVisualOdometry();

            /**
             *  \brief  update the pose by using the given pose as starting point
             *  \param  pose will be the initial value for the optimization and contains the computed result
             *          it has to be the pose from world to camera!
             */
            void updatePose( Matrix4f& pose, const Image& gray, const Image& depth );

            /**
             *  \param kfPose   pose of the keyframe: T_wk
             */
            void addNewKeyframe( const Image& gray, const Image& depth, const Matrix4f& pose );

            void addNewKeyframe();

            /**
             *  \brief  get the absolute (world) pose of the last aligned image
             */
            const Matrix4f& pose() const;
            const Matrix3f& intrinsics()    const                   { return _intrinsics; }
            void            setPose( const Matrix4f& pose )         { _currentPose = pose; }
            size_t          numOverallKeyframes() const             { return _numCreated; }
            float           lastSSD()             const             { return _lastResult.costs; }
            size_t          lastNumPixels()       const             { return _lastResult.numPixels; }
            void            setParameters( const Params& p )        { _params = p; }// TODO: some updates might not get reflected this way!
            const Params&   parameters() const                      { return _params; }
            OptimizerType*  optimizer()                             { return _optimizer; }

            /******** SIGNALS ************/
            /**
             *  \brief  Signal that will be emitted when a new keyframe was added
             *  \param  Matrix4f will be the pose of the new keyframe in the map
             */
            Signal<const Matrix4f&>    keyframeAdded;

            EIGEN_MAKE_ALIGNED_OPERATOR_NEW
        private:
            Params                      _params;

            Optimizer<Derived>*         _optimizer;
            DVOCostFunction<Derived>*   _costFunc;
            Matrix3f                    _intrinsics;

            // current active keyframe
            size_t                      _numCreated;

            ImagePyramid                _pyramid;
            Matrix4<float>              _currentPose;

            Result                      _lastResult;

            bool needNewKeyframe() const;
            void setKeyframeParams();
    };

    template <class Derived>
    inline RGBDVisualOdometry<Derived>::RGBDVisualOdometry( OptimizerType* optimizer,
                                                            DVOCostFunction<Derived>* costFunc,
                                                            const Matrix3f& K,
                                                            const Params& p ) :
        _params( p ),
        _optimizer( optimizer ),
        _costFunc( costFunc ),
        _intrinsics( K ),
        _numCreated( 0 ),
        _pyramid( p.pyrOctaves, p.pyrScale )
    {
        _currentPose.setIdentity();
    }

    template <class Derived>
    inline RGBDVisualOdometry<Derived>::~RGBDVisualOdometry()
    {
    }

    template <class Derived>
    inline void RGBDVisualOdometry<Derived>::updatePose( Matrix4f& pose,
                                                         const Image& gray,
                                                         const Image& depth )
    {
        CVT_ASSERT( ( gray.format()  == IFormat::GRAY_FLOAT ), "Gray image format has to be GRAY_FLOAT" );
        CVT_ASSERT( ( depth.format() == IFormat::GRAY_FLOAT ), "Depth image format has to be GRAY_FLOAT" );
        _costFunc->setInput( gray, depth );

        //_optimizer->optimizeMultiframe( _lastResult, pose, &_keyframes[ 0 ], _keyframes.size(), _pyramid, depth );
        _costFunc->setPose( pose );
        _optimizer->optimize( _lastResult, *_costFunc );

        _currentPose = _costFunc->pose();

        // check if we need a new keyframe
        if( _params.autoReferenceUpdate && needNewKeyframe() ){
            addNewKeyframe();
        }

        pose = _currentPose;
    }

    template <class Derived>
    inline void RGBDVisualOdometry<Derived>::addNewKeyframe()
    {
        _costFunc->updateOfflineData( );
        _numCreated++;

        // notify observers
        keyframeAdded.notify( _currentPose );
    }

    template <class Derived>
    inline void RGBDVisualOdometry<Derived>::addNewKeyframe( const Image& gray, const Image& depth, const Matrix4f& pose )
    {
        _costFunc->setPose( pose );
        _costFunc->setInput( gray, depth );
        _currentPose = pose;
        addNewKeyframe();
    }

    template <class Derived>
    inline bool RGBDVisualOdometry<Derived>::needNewKeyframe() const
    {
        float pixPercentage = _lastResult.numPixels / ( float )_costFunc->modelSize();
        if( pixPercentage < _params.minPixelPercentage ){
            return true;
        }

        if( _lastResult.costs > _params.maxSSDSqr ){
            return true;
        }

        Matrix4f relPose = _currentPose.inverse() * _costFunc->referencePose();

        Vector4f t = relPose.col( 3 );
        t[ 3 ] = 0;
        float tmp = t.length();
        if( tmp > _params.maxTranslationDistance ){
            return true;
        }

        Matrix3f R = relPose.toMatrix3();
        Quaternionf q( R );
        Vector3f euler = q.toEuler();
        tmp = euler.length();
        if( tmp > _params.maxRotationDistance ){
            return true;
        }
        return false;
    }

    template <class Derived>
    inline const Matrix4f& RGBDVisualOdometry<Derived>::pose() const
    {
        return _currentPose;
    }

}

#endif // RGBDVISUALODOMETRY_H
