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

#ifndef CVT_CLNDRANGE_H
#define CVT_CLNDRANGE_H

#include <iostream>

namespace cvt {
	/**
	  \ingroup CL
	 */
	class CLNDRange
	{
		friend std::ostream& operator<<( std::ostream& out, const CLNDRange& ndrange );
		public:
			CLNDRange();
			CLNDRange( size_t x );
			CLNDRange( size_t x, size_t y );
			CLNDRange( size_t x, size_t y, size_t z );
			CLNDRange( const CLNDRange& x );

			/**
			  Return the value of the first dimension
			  */
			size_t x() const { return _ndrange[ 0 ]; };

			/**
			  Return the value of the second dimension
			  */
			size_t y() const { return _ndrange[ 1 ]; };

			/**
			  Return the value of the third dimension
			  */
			size_t z() const { return _ndrange[ 2 ]; };

			/**
			  Dimension of the CLNDRange object
			 */
			size_t dimension() const { return _dimension; }
			/**
			  Pointer to the range values
			  */
			const size_t* range() const { return _ndrange; }

			size_t operator[]( int index ) const { return _ndrange[ index ]; }

		private:
			cl_uint _dimension;
			size_t _ndrange[ 3 ];
	};

	inline CLNDRange::CLNDRange() : _dimension( 0 )
	{
		_ndrange[ 0 ] = 0;
		_ndrange[ 1 ] = 0;
		_ndrange[ 2 ] = 0;
	}

	/**
	  One dimensional CLNDRange
	  */
	inline CLNDRange::CLNDRange( size_t x  ) : _dimension( 1 )
	{
		_ndrange[ 0 ] = x;
		_ndrange[ 1 ] = 0;
		_ndrange[ 2 ] = 0;
	}

	/**
	  Two dimensional CLNDRange
	  */
	inline CLNDRange::CLNDRange( size_t x, size_t y ) : _dimension( 2 )
	{
		_ndrange[ 0 ] = x;
		_ndrange[ 1 ] = y;
		_ndrange[ 2 ] = 0;
	}

	/**
	  Three dimensional CLNDRange
	  */
	inline CLNDRange::CLNDRange( size_t x, size_t y, size_t z  ) : _dimension( 3 )
	{
		_ndrange[ 0 ] = x;
		_ndrange[ 1 ] = y;
		_ndrange[ 2 ] = z;
	}

	/**
	  Construct CLNDRange from other CLNDRange object
	*/
	inline CLNDRange::CLNDRange( const CLNDRange& x )
	{
		_dimension = x._dimension;
		_ndrange[ 0 ] = x._ndrange[ 0 ];
		_ndrange[ 1 ] = x._ndrange[ 1 ];
		_ndrange[ 2 ] = x._ndrange[ 2 ];
	}


	inline std::ostream& operator<<( std::ostream& out, const CLNDRange& r )
	{
		out << "CLNDRange: " << r._dimension << " ( " << r._ndrange[ 0 ] << " , " << r._ndrange[ 1 ] << " , " << r._ndrange[ 2 ] << " )\n";
		return out;
	}
}

#endif
