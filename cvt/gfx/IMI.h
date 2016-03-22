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

#ifndef CVT_IMI_H
#define CVT_IMI_H

#include <cvt/gfx/Image.h>
#include <cvt/gfx/IHistogram.h>

namespace cvt {
	template<typename T>
	class IMI {
		public:
			IMI( size_t bins );
			~IMI();

			size_t size() const;
			void update( const Image& one, const Image& two, IHistogramType type = IHISTOGRAM_BSPLINE );
			T operator()( size_t x, size_t y ) const;
			T operator() const;

		private:
			const size_t _size;
			T _joinhist[ ( _size + 1 ) * ( _size + 1 ) ];
			IHistogram<T> _histone;
			IHistogram<T> _histtwo;
	};

	template<typename T>
	inline IMI<T>::IMI( size_t bins ) : _size( bins ), _histone( _size ), _histtwo( _size )
	{
	}

	template<typename T>
	inline IMI<T>::~IMI()
	{
	}

	template<typename T>
	inline size_t IMI<T>::size() const
	{
		return _size;
	}

	template<typename T>
	inline T IMI<T>::operator()( size_t x, size_t y ) const
	{
		return _hist[ y * ( _size + 1 ) + y ];
	}

	template<typename T>
	inline T IMI<T>::operator()() const
	{
		T ret = 0;
		for( size_t y = 0; y < _size; y++ ) {
			for( size_t x = 0; x < _size; x++ ) {
				ret += _hist[ y * ( _size + 1 ) + x ] * Math::log( _hist[ y * ( _size + 1 ) + x ] / ( _histone( y ) * _histtwo( x ) + 1e-10f ) );
			}
		}
		return ret;
	}

	template<typename T>
	inline void IMI<T>::update( const Image& ione, const Image& itwo, IHistogramType type )
	{
		clear();
		_histone.update( ione, type );
		_histtwo.update( itwo, type );

		switch( type ) {
			case IHISTOGRAM_BSPLINE:
				{
					switch( i.format().formatID ) {
						//case IFORMAT_GRAY_UINT8:	updateBSpline1u8( i ); break;
						case IFORMAT_GRAY_FLOAT:	updateBSpline1f( ione, itwo ); break;
						//case IFORMAT_RGBA_UINT8:
						//case IFORMAT_BGRA_UINT8:
						//							updateBSpline4u8( i ); break;
						//case IFORMAT_RGBA_FLOAT:
						//case IFORMAT_BGRA_FLOAT:
						//							updateBSpline4f( i ); break;
						default:
							throw CVTException( "Unimplemented" );
					}
					break;
				}
			case IHISTOGRAM_NOINTERP:
				{
					switch( i.format().formatID ) {
						//case IFORMAT_GRAY_UINT8:	update1u8( i ); break;
						//case IFORMAT_GRAY_FLOAT:	update1f( i ); break;
						//case IFORMAT_RGBA_UINT8:
						//case IFORMAT_BGRA_UINT8:
						//							update4u8( i ); break;
						//case IFORMAT_RGBA_FLOAT:
						//case IFORMAT_BGRA_FLOAT:
						//							update4f( i ); break;
						default:
							throw CVTException( "Unimplemented" );
					}
					break;
				}
		}
		normalize();
	}
	template<typename T>
	inline void IMI<T>::clear()
	{
		size_t size = ( _size + 1 ) * ( _size + 1 );
			for( size_t i = 0; i < size; i++ )
				_hist[ i ] = 0;
	}

	template<typename T>
	inline void IMI<T>::normalize()
	{
		size_t size = ( _size + 1 ) * ( _size + 1 );
			T sum = 0;
			for( size_t i = 0; i < size; i++ )
				sum += _hist[ i ];
			sum = 1 / sum;
			for( size_t i = 0; i < size; i++ )
				_hist[ i ] *= sum;
	}

	template<typename T>
	inline void IMI<T>::updateBSpline1f( const Image& ione, const Image& itwo )
	{
		size_t stride;
		size_t tstride;
		const float* ptr = ione.map<float>( &stride );
		const float* tptr = itwo.map<float>( &tstride );
		size_t w, h;
		float sum = 0;

		w = itwo.width();
		h = itwo.height();

		const float* pi = ptr;
		const float* pit = tptr;

		while( h-- ) {
			size_t n = w;
			const float* pval = pi;
			const float* ptval = pit;
			while( n-- ) {
				float r, t;
				t = *pval * ( _size - 3 ) + 1.0f;
				r = *ptval * ( _size - 3 ) + 1.0f;
				int tidx = ( int ) t;
				int ridx = ( int ) r;
				for( int m = -1; m <= 2; m++ ) {
					for( int o = -1; o <= 2; o++ ) {
						float val = BSplinef::eval( -t + ( float ) ( tidx + o ) )
							* BSplinef::eval( -r + ( float ) ( ridx + m ) );
						_hist[ ( ridx + m ) *  ( _size + 1 ) + ( tidx + o ) ] += val;
						sum += val;
					}
				}
			}
			pi += stride;
			pit += tstride;
		}

		ione.unmap( ptr );
		itwo.unmap( tptr );
	}

	typedef IMI<float> IMIf;
	typedef IMI<double> IMId;
}
}

#endif
