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

/*
 *  MeasurementModel.h
 *  CVTools
 *
 *  Created by Sebastian Klose on 03.02.11.
 *  Copyright 2011. All rights reserved.
 *
 */

#ifndef CVT_MEASUREMENTMODEL_H
#define CVT_MEASUREMENTMODEL_H

#include <cvt/math/CostFunction.h>

namespace cvt
{	
	/**
	 MeasurmentModel Interface
	 CRTP is needed, because the interface is dependant on template variables 
	 (AType, bType often have known sizes at compile time) so the functions cannot be virtual
	 */
	
	template < typename T, template <typename Type> class Derived, class AType, class bType, class ParamType, class MeasType >
	class MeasurementModel
	{
		public:			
			virtual ~MeasurementModel() {}
            
			inline T buildLSSystem( AType & A, bType & b, const CostFunction<T, MeasType> & costFunc )
			{ 
				return static_cast< Derived<T>* >( this )->buildLSSystem( A, b, costFunc ); 
			}
					
			inline T evaluateCosts( const CostFunction<T, MeasType> & costFunc )
			{
				return static_cast< Derived<T>* >( this )->evaluateCosts( costFunc ); 
			}
		
			inline void	apply( const ParamType & delta )
			{
				static_cast< Derived<T>* >( this )->apply( delta );
			}	
	};	
}

#endif
