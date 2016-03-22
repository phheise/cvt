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


#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <cvt/vision/rgbdvo/RGBDKeyframe.h>
#include <cvt/vision/rgbdvo/SystemBuilder.h>
#include <cvt/vision/rgbdvo/ApproxMedian.h>
#include <cvt/vision/rgbdvo/ErrorLogger.h>
#include <Eigen/LU>

namespace cvt {

    template <class Derived>
    class Optimizer
    {
        public:
            typedef CostFunction<Derived>                CostFuncType;
            typedef typename CostFuncType::DataType      T;
            typedef typename CostFuncType::JacobianType  JacobianType;
            typedef typename CostFuncType::HessianType   HessianType;
            typedef typename CostFuncType::ParameterType DeltaType;

            struct Result {
                Result() :
                    success( false ),
                    numPixels( 0 ),
                    costs( 0.0f )
                {
                }

                bool        success;
                size_t      iterations;
                size_t      numPixels;
                float       costs;
            };

            Optimizer( RobustEstimator<T>* robustEstimator );
            virtual ~Optimizer() {}

            void setMaxIterations( size_t iter )    { _maxIter = iter; }
            void setMinUpdate( float v )            { _minUpdate = v; }
            void setUseRegularization( bool v )     { _useRegularizer = v; }
            void setRegularizationMatrix( const HessianType& m ) { _regularizer = m; }
            void setRegularizationAlpha( float v )  { _regAlpha = v; }
            void setLogError( bool v )              { _logError = v; }

            /**
             * @brief setCostStopThreshold
             * @param v early stepout, if average costs are below v!
             */
            void setCostStopThreshold( float v )    { _costStopThreshold = v; }

            void optimize( Result& result,
                           CostFunction<Derived> &costFunc );

            void setErrorLoggerGTPose( const Matrix4f& mat ){ _logger.setGTPose( mat ); }

        protected:
            size_t          _maxIter;
            float           _minUpdate;
            float           _costStopThreshold;
            bool            _useRegularizer;
            float           _regAlpha;
            HessianType     _regularizer;
            DeltaType       _overallDelta;
            bool            _logError;
            ErrorLogger     _logger;

            RobustEstimator<float>* _robustEstimator;

            float computeMedian( const float* residuals, size_t n ) const;
            float computeMAD( const float* residuals, size_t n, float median ) const;
            bool checkResult( const Result& res ) const;

            float evaluateSystem( HessianType& hessian, JacobianType& deltaSum,
                                  const JacobianType* jacobians, const float* residuals, size_t n );

            void resetOverallDelta();

        protected:
            virtual void optimizeSingleScale( Result& /*result*/,
                                              CostFunction<Derived>& /*costFunc*/,
                                              size_t /*octave*/ )
            {
                throw CVTException( "new costfunction interface not yet implemented for this optimizer" );
            }

    };

    template <class Derived>
    inline Optimizer<Derived>::Optimizer( RobustEstimator<T>* estimator ) :
        _maxIter( 10 ),
        _minUpdate( 1e-6 ),
        _costStopThreshold( 0.002f ),
        _useRegularizer( false ),
        _regAlpha( 0.2f ),
        _regularizer( HessianType::Identity() ),
        _overallDelta( DeltaType::Zero() ),
        _robustEstimator( estimator )
    {
    }

    template <class Derived>
    inline void Optimizer<Derived>::optimize( Result& result,
                                              CostFunction<Derived> &costFunc )
    {
        result.costs = 0.0f;
        result.iterations = 0;
        result.numPixels = 0;

        Result saveResult = result;
        typename CostFunction<Derived>::ModelType savedModel( costFunc.model() );

        if( _useRegularizer ){
            resetOverallDelta();
        }

        for( size_t s = costFunc.scales() ; s > 0; s-- ){
            this->optimizeSingleScale( result, costFunc, s - 1 );

            /* check */
            if( checkResult( result ) ){
                saveResult = result;
                saveResult.success = true;
                savedModel = costFunc.model();
            } else {
                costFunc.setModel( savedModel );
            }
        }

        result = saveResult;
    }

    template <class Derived>
    inline void Optimizer<Derived>::resetOverallDelta()
    {
        _overallDelta.setZero();
    }

    template <class Derived>
    inline float Optimizer<Derived>::computeMedian( const float* residuals, size_t n ) const
    {
        if( n == 1 ){
            return residuals[ 0 ];
        }

        ApproxMedian medianSelector( 0.0f, 1.0f, 0.02f );

        for( size_t i = 0; i < n; ++i ){
            medianSelector.add( Math::abs( residuals[ i ] ) );
        }

        return medianSelector.approximateNth( n >> 1 );
    }

    template <class Derived>
    inline float Optimizer<Derived>::computeMAD( const float* residuals, size_t n, float median ) const
    {
        ApproxMedian medianSelector( 0.0f, 0.5f, 0.02f );

        for( size_t i = 0; i < n; ++i ){
            medianSelector.add( Math::abs( residuals[ i ] - median ) );
        }

        return medianSelector.approximateNth( n >> 1 );
    }

    template <class Derived>
    inline bool Optimizer<Derived>::checkResult( const Result& res ) const
    {
        // too few pixels projected into image
        if( res.numPixels == 0 ){
            return false;
        }
        return true;
    }


    template <class Derived>
    inline float Optimizer<Derived>::evaluateSystem( HessianType& hessian, JacobianType& deltaSum,
                                                     const JacobianType* jacobians, const float* residuals, size_t n  )
    {        
        float median = this->computeMedian( residuals, n );
        float mad = this->computeMAD( residuals, n, median );

        // this is an estimate for the standard deviation:
        _robustEstimator->setScale( 1.4826f * mad );
        float costs = SystemBuilder::build( *_robustEstimator,
                                            hessian,
                                            deltaSum,
                                            jacobians,
                                            residuals,
                                            n );
        if( _useRegularizer ){
            float norm = 1.0f / ( float )n;
            hessian *= norm;
            deltaSum *= norm;
            hessian.noalias()  += ( 2 * _regAlpha * _regularizer );
            deltaSum.noalias() += ( _regAlpha * _regularizer * _overallDelta ).transpose();
        }

        return costs;
    }

}

#endif // OPTIMIZER_H
