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

#ifndef CVT_LEVENBERG_MARQUARD_H
#define CVT_LEVENBERG_MARQUARD_H

#include <Eigen/Core>
#include <Eigen/Cholesky>

#include <cvt/math/TerminationCriteria.h>
#include <cvt/math/CostFunction.h>
#include <cvt/vision/MeasurementModel.h>

namespace cvt {
	
	template<typename T>
	class LevenbergMarquard
	{
		public:
			LevenbergMarquard()
			{}

			~LevenbergMarquard()
			{}

			template < template <typename Type> class Derived, class AType, class bType, class ParamType, class MeasType >
			void	optimize( MeasurementModel<T, Derived, AType, bType, ParamType, MeasType> & model, 
							  const CostFunction<T, MeasType> & costFunc, 
							  const TerminationCriteria<T> & termCrit );
			T		costs() const { return _lastCosts; }
			size_t	iterations() const { return _iterations; }
							
		private:
			size_t						_iterations;
			T							_lastCosts;
	};
	
	template<typename T>
	template < template <typename Type> class Derived, class AType, class bType, class ParamType, class MeasType >
	inline void LevenbergMarquard<T>::optimize( MeasurementModel<T, Derived, AType, bType, ParamType, MeasType> & model, 
											    const CostFunction<T, MeasType> & costFunc, 
											    const TerminationCriteria<T> & termCrit )
	{
		T currentCosts = model.evaluateCosts( costFunc );
		
		_lastCosts = currentCosts;
		_iterations = 0;
		
		AType A;
		bType b;
		ParamType delta;
		AType lamda = AType::Identity();
		
		bool reEvaluate = true;
		
		while( !termCrit.finished( _lastCosts, _iterations ) ){
			if( reEvaluate ){
				currentCosts = model.buildLSSystem( A, b, costFunc );
			}
			
			// solve the system
			delta.setZero();
			delta = ( A + lamda ).llt().solve( b );
			
			// apply delta parameters:			
			model.apply( delta );
			
			// calculate residuals & current costs
			currentCosts = model.evaluateCosts( costFunc );
			
			if( currentCosts < _lastCosts ){
				lamda *= 0.1;
				_lastCosts = currentCosts;
				reEvaluate = true;
			} else {
				lamda *= 10.0;
				model.apply( -delta );
				reEvaluate = false;
			}
			_iterations++;
		}		
	}	
}

#endif
