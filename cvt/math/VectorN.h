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

#ifndef CVT_VECTORN_H
#define CVT_VECTORN_H

#include <string.h>

namespace cvt {

	template<size_t N, typename T>
		class Vector {
			public:
				Vector();
				Vector( const Vector<N,T>& other );

				T			dot( const Vector<N,T>& vec ) const;

				T			operator[]( size_t i ) const;
				T&			operator[]( size_t i );

				bool		operator==( const Vector<N,T> &v ) const;
				bool		operator!=( const Vector<N,T> &v ) const;

				T			length( void ) const;
				T			lengthSqr( void ) const;
				T			normalize( void );		// returns length
				void		clamp( const Vector<N,T>& min, const Vector<N,T>& max );

				size_t		dimension( void ) const;
				size_t		rows() const;
				size_t		cols() const;

				bool        isEqual( const Vector<N,T> & other, T epsilon ) const;

				const T*	ptr( void ) const;
				T*			ptr( void );

				void		mix( const Vector<N,T> &v1, const Vector<N,T> &v2, float alpha );

			private:
				T _vec[ N ];
		};

}

#endif
