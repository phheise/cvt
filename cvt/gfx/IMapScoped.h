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

#ifndef CVT_IMAPSCOPED_H
#define CVT_IMAPSCOPED_H

#include <cvt/gfx/Image.h>
#include <stdlib.h>

namespace cvt {

	template<typename T>
		class IMapScoped {
			private:
				template<typename T2>
					struct ITypeSelector;

				template<typename T2>
					struct ITypeSelector
					{
						typedef Image& IType;
						typedef uint8_t* IPtrType;
					};

				template<typename T2>
					struct ITypeSelector<const T2>
					{
						typedef const Image& IType;
						typedef const uint8_t* IPtrType;
					};

				typedef typename ITypeSelector<T>::IType IType;
				typedef typename ITypeSelector<T>::IPtrType IPtrType;

			public:
				IMapScoped( IType img );
				~IMapScoped();

				void		reset();
				T*			ptr();
				T*			ptr() const;
				T*			base();
				T*			line( size_t y );
				void		prevLine();
				void		nextLine();
				void		setLine( size_t l );
				void		operator++( int );
				void		operator--( int );
				size_t		stride() const;
				T&			operator()( size_t x, size_t y );
				const T&	operator()( size_t x, size_t y ) const;
				size_t		width() const;
				size_t		height() const;

			private:
				IType	    _img;
				IPtrType	_base;
				IPtrType	_line;
				size_t		_stride;
		};

	template<typename T>
	inline IMapScoped<T>::IMapScoped( IMapScoped<T>::IType img ) : _img( img )
	{
		_base = _img.map( &_stride );
		_line = _base;
	}

	template<typename T>
	inline IMapScoped<T>::~IMapScoped( )
	{
		_img.unmap( _base );
	}

	template<typename T>
	inline void IMapScoped<T>::reset()
	{
		_line = _base;
	}

	template<typename T>
	inline T* IMapScoped<T>::ptr()
	{
		return ( T* ) _line;
	}

	template<typename T>
	inline T* IMapScoped<T>::ptr() const
	{
		return ( T* ) _line;
	}

	template<typename T>
	inline T* IMapScoped<T>::base()
	{
		return ( T* ) _base;
	}


	template<typename T>
	inline T* IMapScoped<T>::line( size_t y )
	{
		return ( T* ) ( _base + _stride * y );
	}

	template <typename T>
	inline void IMapScoped<T>::setLine( size_t l )
	{
		if( l >= _img.height() )
			throw CVTException( "Trying to set line higher than actual image height!" );
		_line = _base + _stride * l;
	}

	template<typename T>
	inline void IMapScoped<T>::nextLine()
	{
		_line += _stride;
	}

	template<typename T>
	inline void IMapScoped<T>::prevLine()
	{
		_line -= _stride;
	}

	template<typename T>
	inline void IMapScoped<T>::operator++( int )
	{
		_line += _stride;
	}

	template<typename T>
	inline void IMapScoped<T>::operator--( int )
	{
		_line -= _stride;
	}

	template<typename T>
	inline size_t IMapScoped<T>::stride() const
	{
		return _stride;
	}

	template<typename T>
	inline T& IMapScoped<T>::operator()( size_t x, size_t y )
	{
		return *( ( T* ) ( _base + y * _stride + x * sizeof( T ) ) );
	}

	template<typename T>
	inline const T& IMapScoped<T>::operator()( size_t x, size_t y ) const
	{
		return *( ( const T* ) ( _base + y * _stride + x * sizeof( T ) ) );
	}

	template<typename T>
	inline size_t IMapScoped<T>::width() const
	{
		return _img.width();
	}

	template<typename T>
	inline size_t IMapScoped<T>::height() const
	{
		return _img.height();
	}

}

#endif
