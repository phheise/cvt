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

#ifndef CVT_COMPLEX_H
#define CVT_COMPLEX_H

#include <cvt/math/Math.h>
#include <iostream>

namespace cvt {

	/**
	  \ingroup Math
	*/
	template<typename T>
		class Complex {
			public:
				Complex();
				Complex( T re, T im = 0 );
				Complex( const Complex<T>& cx );

				void set( T re, T im = 0 );
				void setZero();

				Complex<T>& operator=( const Complex<T>& cx );

				Complex<T> operator-() const;

				Complex<T>  operator*( const Complex<T>& cx ) const;
				Complex<T>  operator/( const Complex<T>& cx ) const;
				Complex<T>  operator+( const Complex<T>& cx ) const;
				Complex<T>  operator-( const Complex<T>& cx ) const;

				Complex<T>& operator*=( const Complex<T>& cx );
				Complex<T>& operator/=( const Complex<T>& cx );
				Complex<T>& operator+=( const Complex<T>& cx );
				Complex<T>& operator-=( const Complex<T>& cx );

				Complex<T>  operator*( T s ) const;
				Complex<T>  operator/( T s ) const;
				Complex<T>  operator+( T s ) const;
				Complex<T>  operator-( T s ) const;

				Complex<T>& operator*=( T s );
				Complex<T>& operator/=( T s );
				Complex<T>& operator+=( T s );
				Complex<T>& operator-=( T s );

				bool operator==( const Complex<T>& cx ) const;
				bool operator!=( const Complex<T>& cx ) const;

				T		   abs( void ) const;
				Complex<T> sqrt( void ) const;
				Complex<T> conj( void ) const;

				T re;
				T im;
		};

		template<typename T>
		inline Complex<T>::Complex()
		{
		}

		template<typename T>
		inline Complex<T>::Complex( T r, T i ) : re( r ), im( i )
		{
		}

		template<typename T>
		inline Complex<T>::Complex(const Complex<T>& cx ) : re( cx.re ), im( cx.im )
		{
		}

		template<typename T>
		inline void Complex<T>::set( T r, T i )
		{
			re = r;
			im = i;
		}

		template<typename T>
		inline void Complex<T>::setZero()
		{
			re = 0;
			im = 0;
		}

		template<typename T>
		inline Complex<T>& Complex<T>::operator=( const Complex<T>& cx )
		{
			re = cx.re;
			im = cx.im;
			return *this;
		}

		template<typename T>
		inline Complex<T> Complex<T>::operator-() const
		{
			return Complex<T>( -re, -im );
		}

		template<typename T>
		inline Complex<T> Complex<T>::operator*( const Complex<T>& cx ) const
		{
			return Complex<T>( re * cx.re - im * cx.im, im * cx.re + re * cx.im );
		}

		template<typename T>
		inline Complex<T> Complex<T>::operator/( const Complex<T>& cx ) const
		{
			T s, t;
			if ( Math::abs( cx.re ) >= Math::abs( cx.im ) ) {
				s = cx.im / cx.re;
				t = ( T ) 1 / ( cx.re + s * cx.im );
				return Complex<T>( ( re + s * im ) * t, ( im - s * re ) * t );
			} else {
				s = cx.re / cx.im;
				t = ( T ) 1 / ( s * cx.re + cx.im );
				return Complex<T>( ( re * s + im ) * t, ( im * s - re ) * t );
			}
		}


		template<typename T>
		inline Complex<T> Complex<T>::operator+( const Complex<T>& cx ) const
		{
			return Complex<T>( re + cx.re, im + cx.im );
		}

		template<typename T>
		inline Complex<T> Complex<T>::operator-( const Complex<T>& cx ) const
		{
			return Complex<T>( re - cx.re, im - cx.im );
		}

		template<typename T>
		inline Complex<T>& Complex<T>::operator*=( const Complex<T>& cx )
		{
			*this = Complex<T>( re * cx.re - im * cx.im, im * cx.re + re * cx.im );
			return *this;
		}

		template<typename T>
		inline Complex<T>& Complex<T>::operator/=( const Complex<T>& cx )
		{
			T s, t;
			if ( Math::abs( cx.re ) >= Math::abs( cx.im ) ) {
				s = cx.im / cx.re;
				t = ( T ) 1 / ( cx.re + s * cx.im );
				*this = Complex<T>( ( re + s * im ) * t, ( im - s * re ) * t );
			} else {
				s = cx.re / cx.im;
				t = ( T ) 1 / ( s * cx.re + cx.im );
				*this = Complex<T>( ( re * s + im ) * t, ( im * s - re ) * t );
			}
			return *this;
		}

		template<typename T>
		inline Complex<T>& Complex<T>::operator+=( const Complex<T>& cx )
		{
			re += cx.re;
			im += cx.im;
			return *this;
		}

		template<typename T>
		inline Complex<T>& Complex<T>::operator-=( const Complex<T>& cx )
		{
			re -= cx.re;
			im -= cx.im;
			return *this;
		}

		template<typename T>
		inline Complex<T> Complex<T>::operator*( T s ) const
		{
			return Complex<T>( re * s , im * s );
		}

		template<typename T>
		inline Complex<T> Complex<T>::operator/( T s ) const
		{
			T inv = ( T ) 1 / s;
			return Complex<T>( re * inv , im * inv );
		}

		template<typename T>
		inline Complex<T> Complex<T>::operator+( T s ) const
		{
			return Complex<T>( re + s , im );
		}

		template<typename T>
		inline Complex<T> Complex<T>::operator-( T s ) const
		{
			return Complex<T>( re - s , im );
		}

		template<typename T>
		inline Complex<T>& Complex<T>::operator*=( T s )
		{
			re *= s;
			im *= s;
			return *this;
		}

		template<typename T>
		inline Complex<T>& Complex<T>::operator/=( T s )
		{
			T inv = ( T ) 1 / s;
			re *= inv;
			im *= inv;
			return *this;
		}

		template<typename T>
		inline Complex<T>& Complex<T>::operator+=( T s )
		{
			re += s;
			return *this;
		}

		template<typename T>
		inline Complex<T>& Complex<T>::operator-=( T s )
		{
			re -= s;
			return *this;
		}

		template<typename T>
		inline Complex<T> operator*( T s, const Complex<T>& cx )
		{
			return Complex<T>( cx.re * s , cx.im * s );
		}

		template<typename T>
		inline Complex<T> operator/( T a, const Complex<T>& cx )
		{
			T s, t;
			if ( Math::abs( cx.re ) >= Math::abs( cx.im ) ) {
				s = cx.im / cx.re;
				t = a / ( cx.re + s * cx.im );
				return Complex<T>( t, - s * t );
			} else {
				s = cx.re / cx.im;
				t = a / ( s * cx.re + cx.im );
				return Complex<T>( s * t, - t );
			}
		}

		template<typename T>
		inline Complex<T> operator+( T s, const Complex<T>& cx )
		{
			return Complex<T>( cx.re + s , cx.im );
		}

		template<typename T>
		inline Complex<T> operator-( T s, const Complex<T>& cx )
		{
			return Complex<T>( s - cx.re, - cx.im );
		}


		template<typename T>
		inline Complex<T> Complex<T>::conj() const
		{
			return Complex<T>( re, -im );
		}

		template<typename T>
		inline T Complex<T>::abs( void ) const
		{
			T r = Math::abs( re );
			T i = Math::abs( im );
			if( r == 0 )
				return i;
			else if( i == 0 )
				return r;
			else if( r > i ) {
				T t = i / r;
				return r * Math::sqrt( ( T ) 1 + Math::sqr( t ) );
			} else {
				T t = r / i;
				return i * Math::sqrt( ( T ) 1 + Math::sqr( t ) );
			}
		}

		template<typename T>
		inline Complex<T> Complex<T>::sqrt( void ) const
		{
			if( re == 0 && im == 0 )
				return Complex<T>( 0, 0 );

			T r = Math::abs( re );
			T i = Math::abs( im );
			T w;
			if( r >= i ) {
				w = i / r;
				w = Math::sqrt( r ) * Math::sqrt( ( ( T )0.5 ) * ( ( T ) 1 + Math::sqrt( ( T ) 1 + Math::sqr( w ) ) ) );
			} else {
				w = r / i;
				w = Math::sqrt( i ) * Math::sqrt( ( ( T )0.5 ) * ( ( T ) w + Math::sqrt( ( T ) 1 + Math::sqr( w ) ) ) );
			}
			if( w == 0 )
				return Complex<T>( 0, 0 );
			else if( re >= 0)
				return Complex<T>( w, ( ( T )0.5 ) * im / w );
			else
				return Complex<T>( ( ( T ) 0.5 ) * i / w, ( im >= 0 ) ? w : -w );
		}

		template<typename T>
		bool Complex<T>::operator==( const Complex<T>& cx ) const
		{
			return ( re == cx.re && im == cx.im );
		}

		template<typename T>
		bool Complex<T>::operator!=( const Complex<T>& cx ) const
        {
			return ( re != cx.re || im != cx.im );
		}

		template<typename T>
		static inline std::ostream& operator<<( std::ostream& out, const Complex<T>& cx )
		{
			out << cx.re << ", " << cx.im;
			return out;
		}

		typedef Complex<float> Complexf;
		typedef Complex<double> Complexd;
}

#endif
