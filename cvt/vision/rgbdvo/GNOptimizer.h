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


#ifndef CVT_GNOPTIMIZER_H
#define CVT_GNOPTIMIZER_H

#include <cvt/vision/rgbdvo/Optimizer.h>

#include <Eigen/LU>

namespace cvt {

    template <class Derived>
    class GNOptimizer : public Optimizer<Derived>
    {
            typedef Optimizer<Derived>              Base;
            typedef typename Base::T                T;
            typedef typename Base::JacobianType     JacobianType;
            typedef typename Base::HessianType      HessianType;
            typedef typename Base::DeltaType        DeltaType;
            typedef typename Base::Result           ResultType;

        public:
            GNOptimizer( RobustEstimator<T>* estimator );
            virtual ~GNOptimizer(){}

        protected:

            virtual void optimizeSingleScale( ResultType& result,
                                              CostFunction<Derived> &costFunc,
                                              size_t octave );
    };

    template <class Derived>
    inline GNOptimizer<Derived>::GNOptimizer( RobustEstimator<T>* estimator ) :
        Optimizer<Derived>( estimator )
    {
    }

    template <class Derived>
    inline void GNOptimizer<Derived>::optimizeSingleScale( ResultType& result,
                                                           CostFunction<Derived>& costFunc,
                                                           size_t octave )
    {
        JacobianType deltaSum;
        HessianType  hessian;

        result.iterations = 0;
        result.numPixels = 0;

        std::vector<float> residuals;
        typename CostFunction<Derived>::JacobianVectorType jacobians;

        while( result.iterations < this->_maxIter ){
            residuals.clear();
            jacobians.clear();

            // re-evaluate the cost function
            costFunc.evaluate( residuals, jacobians, octave );

            result.numPixels = residuals.size();
            result.costs = this->evaluateSystem( hessian,
                                                 deltaSum,
                                                 &jacobians[ 0 ],
                                                 &residuals[ 0 ],
                                                 residuals.size() );

            if( !result.numPixels /* no pixels projected */ ||
                result.costs < this->_costStopThreshold ){
                break;
            }

            DeltaType deltaP = -hessian.inverse() * deltaSum.transpose();
            this->_overallDelta.noalias() += deltaP;

            if( deltaP.norm() < this->_minUpdate )
                break;

            costFunc.update( deltaP );


            result.iterations++;
        }
    }
}

#endif // CVT_GNOPTIMIZER_H
