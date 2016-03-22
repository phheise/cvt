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

#ifndef CVT_RDCLASSHISTOGRAM_H
#define CVT_RDCLASSHISTOGRAM_H

#include <iostream>
#include <string.h>

#include <cvt/math/Math.h>


namespace cvt {
	template<size_t N>
	class RDFClassHistogram
	{
		public:
			RDFClassHistogram();
			RDFClassHistogram( const RDFClassHistogram<N>& other );
			~RDFClassHistogram();

			RDFClassHistogram<N>& operator=( const RDFClassHistogram<N>& other );
			RDFClassHistogram<N>& operator+=( const RDFClassHistogram<N>& other );

			float				  probability( size_t ) const;
			float				  entropy() const;

			void				  addSample( size_t classLabel );
			size_t				  sampleCount() const;
			void				  clear();

		private:
			size_t		 _numSamples;
			size_t		 _bin[ N ];
	};


	template<size_t N>
	inline RDFClassHistogram<N>::RDFClassHistogram() : _numSamples( 0 )
	{
		clear();
	}

	template<size_t N>
	inline RDFClassHistogram<N>::RDFClassHistogram( const RDFClassHistogram<N>& other ) : _numSamples( other._numSamples )
	{
		::memcpy( _bin, other._bin, sizeof( size_t ) * N );
	}

	template<size_t N>
	inline RDFClassHistogram<N>::~RDFClassHistogram()
	{
	}

	template<size_t N>
	inline float RDFClassHistogram<N>::probability( size_t classN ) const
	{
		return ( float ) _bin[ classN ] / ( float ) _numSamples;
	}

	template<size_t N>
	inline RDFClassHistogram<N>& RDFClassHistogram<N>::operator=( const RDFClassHistogram<N>& other )
	{
		if( this != &other ) {
			_numSamples = other._numSamples;
			::memcpy( _bin, other._bin, sizeof( size_t ) * N );
		}
		return *this;
	}

	template<size_t N>
	inline RDFClassHistogram<N>& RDFClassHistogram<N>::operator+=( const RDFClassHistogram<N>& other )
	{
		_numSamples += other._numSamples;
		for( size_t i = 0; i < N; i++ )
			_bin [ i ] += other._bin[ i ];
		return *this;
	}

	template<size_t N>
	inline size_t RDFClassHistogram<N>::sampleCount() const
	{
		return _numSamples;
	}

	template<size_t N>
	inline float RDFClassHistogram<N>::entropy() const
	{
		float ret = 0.0;;
		if( !_numSamples ) return 0.0f;

		for( size_t i = 0; i < N; i++ )
		{
			if( _bin[ i ] ) {
				float p = probability( i );
				ret -= p * Math::log2( p );
			}
		}
		return ret;
	}

	template<size_t N>
	inline void RDFClassHistogram<N>::addSample( size_t classLabel )
	{
		_bin[ classLabel ]++;
		_numSamples++;
	}

	template<size_t N>
	inline void RDFClassHistogram<N>::clear()
	{
		_numSamples = 0;
		::memset( _bin, 0, sizeof( size_t  ) * N );
	}

    template<size_t N>
    std::ostream& operator<<( std::ostream& out, const RDFClassHistogram<N>& hist )
    {
		for( size_t i = 0; i < N; i++ )
			out << " | " <<  i << " : " << hist.probability( i );
		out << " |";
        return out;
    }

}

#endif
