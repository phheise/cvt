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

#ifndef CVT_IHISTOGRAM_H
#define CVT_IHISTOGRAM_H

#include <cvt/gfx/Image.h>
#include <cvt/math/BSpline.h>
#include <cvt/util/Exception.h>

namespace cvt {
	enum IHistogramType {
		IHISTOGRAM_BSPLINE,
		IHISTOGRAM_NOINTERP
	};

	template<typename T>
	class IHistogram {
		public:
			IHistogram( size_t bins );
			~IHistogram();

			void update( const Image& img, IHistogramType = IHISTOGRAM_BSPLINE );
			size_t size() const;
			size_t channels() const;
			T operator()( size_t index ) const;
			T operator()( size_t channel, size_t index ) const;

		private:
			void alloc( const Image& img );
			void normalize( T sum );
			void updateBSpline1f( const Image& i );
			void updateBSpline1u8( const Image& i );
			void updateBSpline4f( const Image& i );
			void updateBSpline4u8( const Image& i );
			void update1f( const Image& i );
			void update1u8( const Image& i );
			void update4f( const Image& i );
			void update4u8( const Image& i );

			size_t _size;
			size_t _channels;
			T* _hist;
	};

	template<typename T>
	inline IHistogram<T>::IHistogram( size_t bins ) : _size( bins ), _channels( 0 ), _hist( NULL )
	{
	}

	template<typename T>
	inline IHistogram<T>::~IHistogram()
	{
		if( _hist )
			delete[] _hist;
	}

	template<typename T>
	inline size_t IHistogram<T>::size() const
	{
		return _size;
	}

	template<typename T>
	inline size_t IHistogram<T>::channels() const
	{
		return _channels;
	}

	template<typename T>
	inline T IHistogram<T>::operator()( size_t index) const
	{
		return _hist[ index ];
	}

	template<typename T>
	inline T IHistogram<T>::operator()( size_t channel, size_t index ) const
	{
		return _hist[ channel * ( _size + 1 ) + index ];
	}


	template<typename T>
	inline void IHistogram<T>::alloc( const Image& img )
	{
		if( _channels != img.format().channels || !_hist ) {
			_channels = img.format().channels;
			if( _hist )
				delete[] _hist;
			_hist = new T[ ( _size + 1 ) * _channels ];
		}
		for( size_t i = 0; i < ( _size + 1 ) * _channels; i++ )
			_hist[ i ] = 0;
	}

	template<typename T>
	inline void IHistogram<T>::update( const Image& i, IHistogramType type )
	{
		alloc( i );
		switch( type ) {
			case IHISTOGRAM_BSPLINE:
				{
					switch( i.format().formatID ) {
						//case IFORMAT_GRAY_UINT8:	updateBSpline1u8( i ); break;
						case IFORMAT_GRAY_FLOAT:	updateBSpline1f( i ); break;
						//case IFORMAT_RGBA_UINT8:
						//case IFORMAT_BGRA_UINT8:
						//							updateBSpline4u8( i ); break;
						case IFORMAT_RGBA_FLOAT:
						case IFORMAT_BGRA_FLOAT:
													updateBSpline4f( i ); break;
						default:
							throw CVTException( "Unimplemented" );
					}
					break;
				}
			case IHISTOGRAM_NOINTERP:
				{
					switch( i.format().formatID ) {
						//case IFORMAT_GRAY_UINT8:	update1u8( i ); break;
						case IFORMAT_GRAY_FLOAT:	update1f( i ); break;
						//case IFORMAT_RGBA_UINT8:
						//case IFORMAT_BGRA_UINT8:
						//							update4u8( i ); break;
						case IFORMAT_RGBA_FLOAT:
						case IFORMAT_BGRA_FLOAT:
													update4f( i ); break;
						default:
							throw CVTException( "Unimplemented" );
					}
					break;
				}
		}
		normalize( i.width() * i.height() );
	}

	template<typename T>
	inline void IHistogram<T>::normalize( T sum )
	{
		sum = ( ( T ) 1 ) / sum;
		for( size_t c = 0; c < _channels; c++ ) {
			for( size_t i = 0; i < _size; i++ )
				_hist[ c * ( _size + 1 ) + i ] *= sum;
		}
	}

	template<typename T>
	inline void IHistogram<T>::updateBSpline1f( const Image& img )
	{
		size_t stride;
		const uint8_t* ptr;
		const uint8_t* map;
		size_t w, h;

		w = img.width();
		h = img.height();
		map = img.map( &stride );
		ptr = map;
		while( h-- ) {
			const float* p = ( const float * ) ptr;
			size_t n = w;
			while( n-- ) {
				float fidx =  *p++ * ( float ) ( _size - 3 ) + 1.0f;
				int idx = ( int ) fidx;
				_hist[ idx ] += BSpline<T>::eval( -fidx + ( float ) idx );
				_hist[ idx + 1 ] += BSpline<T>::eval( - fidx + ( float ) ( idx + 1 ) );
				_hist[ idx - 1 ] += BSpline<T>::eval( - fidx + ( float ) ( idx - 1 ) );
				_hist[ idx + 2 ] += BSpline<T>::eval( - fidx + ( float ) ( idx + 2 ) );
			}
			ptr += stride;
		}
		img.unmap( map );
	}

	template<typename T>
	inline void IHistogram<T>::updateBSpline4f( const Image& img )
	{
		size_t stride;
		const uint8_t* ptr;
		const uint8_t* map;
		size_t w, h;

		w = img.width();
		h = img.height();
		map = img.map( &stride );
		ptr = map;
		while( h-- ) {
			const float* p = ( const float * ) ptr;
			size_t n = w;
			while( n-- ) {
				for( size_t c = 0; c < 4; c++ ) {
					float fidx =  *p++ * ( float ) ( _size - 3 ) + 1.0f;
					int idx = ( int ) fidx;
					_hist[ ( _size + 1 ) * c + idx ] += BSpline<T>::eval( - fidx - ( float ) idx );
					_hist[ ( _size + 1 ) * c + idx + 1 ] += BSpline<T>::eval( - fidx + ( float ) ( idx + 1 ) );
					_hist[ ( _size + 1 ) * c + idx - 1 ] += BSpline<T>::eval( - fidx + ( float ) ( idx - 1 ) );
					_hist[ ( _size + 1 ) * c + idx + 2 ] += BSpline<T>::eval( - fidx + ( float ) ( idx + 2 ) );
				}
			}
			ptr += stride;
		}
		img.unmap( map );
	}

	template<typename T>
	inline void IHistogram<T>::update1f( const Image& img )
	{
		size_t stride;
		const uint8_t* ptr;
		const uint8_t* map;
		size_t w, h;

		w = img.width();
		h = img.height();
		map = img.map( &stride );
		ptr = map;
		while( h-- ) {
			const float* p = ( const float * ) ptr;
			size_t n = w;
			while( n-- ) {
				float fidx =  *p++ * ( float ) ( _size - 1 ) + 0.5f;
				int idx = ( int ) fidx;
				_hist[ idx ] += 1;
			}
			ptr += stride;
		}
		img.unmap( map );
	}

	template<typename T>
	inline void IHistogram<T>::update4f( const Image& img )
	{
		size_t stride;
		const uint8_t* ptr;
		const uint8_t* map;
		size_t w, h;

		w = img.width();
		h = img.height();
		map = img.map( &stride );
		ptr = map;
		while( h-- ) {
			const float* p = ( const float * ) ptr;
			size_t n = w;
			while( n-- ) {
				for( size_t c = 0; c < 4; c++ ) {
					float fidx =  *p++ * ( float ) ( _size - 1 ) + 0.5f;
					int idx = ( int ) fidx;
					_hist[ ( _size + 1 ) * c + idx ] += 1;
				}
			}
			ptr += stride;
		}
		img.unmap( map );
	}

	typedef IHistogram<float> IHistogramf;
	typedef IHistogram<double> IHistogramd;
}

#endif
