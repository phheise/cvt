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

#ifndef CVTCOSTFUNCTION_H
#define CVTCOSTFUNCTION_H

#include <cvt/math/Math.h>

namespace cvt {
	
	template <typename T, class ComputeType>
	class CostFunction
	{
		public:
			CostFunction( bool robust ) : isRobust( robust ) {}
			virtual		~CostFunction(){}
			virtual		T cost( const ComputeType & r, T  & weight ) const = 0;
			const bool isRobust;
	};
	
	template<typename T, class ComputeType>
	class SquaredDistance : public CostFunction<T, ComputeType>
	{
		public:			
			SquaredDistance() : CostFunction<T, ComputeType>( false ) {}
			~SquaredDistance(){}
			inline T cost( const ComputeType & r, T& ) const { return 0.5 * r.squaredNorm(); }
	};
	
	template <typename T>
	class SquaredDistance<T, T> : public CostFunction<T, T>
	{
		public:
			SquaredDistance() : CostFunction<T, T>( false ) {}
			~SquaredDistance(){}
			inline T cost( const T & r, T& ) const { return r*r; }
	};
	
	template<typename T, class ComputeType>
	class RobustHuber : public CostFunction<T, ComputeType>
	{
		public:		
			RobustHuber( T k ) : CostFunction<T, ComputeType>( true ), _k( k ) {}
			~RobustHuber(){}
		
			inline T cost( const ComputeType & r, T & weight ) const 
			{
				T norm = r.norm();
				
				if( norm <= _k ){
					weight = 1; 
					return norm * norm;
				} else {
					weight = _k / norm;
					return _k * ( norm - 0.5 * _k );
				}				
			}
		
		private:
			T _k;
	};
	
	template< typename T >
	class RobustHuber<T, T> : public CostFunction<T, T>
	{
	public:		
		RobustHuber( T k ) : CostFunction<T, T>( true ), _k( k ) {}
		~RobustHuber(){}
		
		inline T cost( const T & r, T & weight ) const 
		{
			T norm = Math::abs( r );
			
			if( norm <= _k ){
				weight = 1; 
				return norm * norm;
			} else {
				weight = _k / norm;
				return _k * ( norm - 0.5 * _k );
			}
		}
		
	private:
		T _k;
	};
	

	template< typename T >
	class L1 : public CostFunction<T, T>
	{
	public:		
		L1() : CostFunction<T, T>( true ) {}
		~L1(){}
		
		inline T cost( const T & r, T & weight ) const 
		{
			weight = ( T ) 1 / ( Math::abs( r ) + 1e-5 );
			return Math::abs( r );
		}
		
	};
}

#endif
