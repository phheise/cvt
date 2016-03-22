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

#ifndef CVT_OPTIMIZATION_H
#define CVT_OPTIMIZATION_H

#include <cvt/util/Flags.h>
#include <limits>

namespace cvt
{
	enum TerminationTypeFlags {
		TERM_MAX_ITER = ( 1 << 0 ),
		TERM_COSTS_THRESH = ( 1 << 1 )
	};

	CVT_ENUM_TO_FLAGS( TerminationTypeFlags, TerminationType )

	template<typename T>
	class TerminationCriteria {
		public:
			TerminationCriteria( TerminationType termType = ( TERM_COSTS_THRESH | TERM_MAX_ITER ) ) :
				_costThreshold( std::numeric_limits<T>::min() ),
				_maxIterations( 40 ),
				_termType( termType )
			{}

			TerminationCriteria( const TerminationCriteria & other ) :
				_costThreshold( other._costThreshold ), _maxIterations( other._maxIterations )
			{}

			void setCostThreshold( T c ){ _costThreshold = c; }
			void setMaxIterations( size_t c ){ _maxIterations = c; }

			bool finished( T costs, size_t iters ) const
			{
				bool ret = false;
				if( _termType & TERM_MAX_ITER ){
					ret |= checkIters( iters );
				}
				if( _termType & TERM_COSTS_THRESH ){
					ret |= checkCosts( costs );
				}
				return ret;
			}
		
			bool checkCosts( T c ) const
			{
				return c < _costThreshold;
			}
		
			bool checkIters( size_t i ) const
			{
				return i > _maxIterations;
			}

			private:
				T					_costThreshold;
				size_t				_maxIterations;
				TerminationType		_termType;
	};
}

#endif
