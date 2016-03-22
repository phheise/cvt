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

#ifndef CVT_LMOPTIMIZER_H
#define CVT_LMOPTIMIZER_H

#include <cvt/vision/rgbdvo/Optimizer.h>

namespace cvt {

    template <class Derived>
    class LMOptimizer : public Optimizer<Derived>
    {
            typedef Optimizer<Derived>              Base;
            typedef typename Base::T                T;
            typedef typename Base::JacobianType     JacobianType;
            typedef typename Base::HessianType      HessianType;
            typedef typename Base::DeltaType        DeltaType;
            typedef typename Base::Result           ResultType;

        public:
            LMOptimizer( RobustEstimator<T>* estimator );
            ~LMOptimizer(){}

        private:
            typedef typename CostFunction<Derived>::ModelType ModelType;

            void optimizeSingleScale( ResultType& result,
                                      CostFunction<Derived> &costFunc,
                                      size_t octave );
    };

    template <class Derived>
    inline LMOptimizer<Derived>::LMOptimizer( RobustEstimator<T>* estimator ) :
        Base( estimator )
    {
    }

    template <class Derived>
    inline void LMOptimizer<Derived>::optimizeSingleScale( ResultType& result,
                                                           CostFunction<Derived>& costFunc,
                                                           size_t octave )
    {
        JacobianType deltaSum;
        HessianType  hessian;
        float lambda = 0.00001f;

        result.iterations = 0;
        result.numPixels = 0;

        SIMD* simd = SIMD::instance();

        std::vector<float> residuals;
        typename CostFunction<Derived>::JacobianVectorType jacobians;


        // initial costs
        costFunc.evaluate( residuals, jacobians, octave );
        result.costs = Base::evaluateSystem( hessian, deltaSum, &jacobians[ 0 ], &residuals[ 0 ], residuals.size() );
        result.numPixels = residuals.size();

        ModelType saved( costFunc.model() );

        HessianType hTmp;
        while( result.iterations < this->_maxIter ){
            hTmp = hessian;

            // multiplicative damping
            for( int i = 0; i < hTmp.rows(); ++i )
                hTmp( i, i ) += ( lambda * hTmp( i, i ) );
            DeltaType deltaP = -hTmp.inverse() * deltaSum.transpose();

            if( deltaP.norm() < this->_minUpdate )
                break;
            costFunc.update( deltaP );

            // TODO: here we would only need to evaluate the current costs
            // not the jacobians etc.
            costFunc.evaluate( residuals, jacobians, octave );
            float currentCosts = simd->sumSqr( &residuals[ 0 ], residuals.size() ) / residuals.size();

            if( residuals.size() && currentCosts < result.costs ){
                // step accept - update the system:
                this->_overallDelta.noalias() += deltaP;
                result.costs = Base::evaluateSystem( hessian, deltaSum, &jacobians[ 0 ], &residuals[ 0 ], residuals.size() );
                saved = costFunc.model();
                lambda *= 0.1f;
                result.numPixels = residuals.size();

                if( result.costs < Base::_costStopThreshold ){
                    // stop optimization, costs have reached sufficient minimum
                    break;
                }
            } else {
                // step reject
                // undo the step
                costFunc.setModel( saved );

                // update the damping
                lambda *= 2.0f;
            }

            result.iterations++;

            residuals.clear();
            jacobians.clear();
        }
    }

}


#endif // LMOPTIMIZER_H
