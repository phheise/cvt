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

#ifndef CVT_BOX_H
#define CVT_BOX_H

#include <cvt/math/Vector.h>

namespace cvt {

	template<typename T>
		class Box
		{
			public:
				Box( T x = 0, T y = 0, T z = 0, T x2 = 0, T y2 = 0, T z2 = 0 );
				Box( const Vector3<T>& p1, const Vector3<T>& p2 );

				const Box<T>& operator=( const Box<T>& b2 );
				void		  copy( const Box<T>& b2 );
				void		  set( const Vector3<T>& p1, const Vector3<T>& p2 );
				void		  set( T x = 0, T y = 0, T z = 0, T x2 = 0, T y2 = 0, T z2 = 0 );
				void		  setPosition( T x, T y, T z );
				void		  setSize( T w, T h, T d );
				void		  translate( T x, T y, T z );
				void		  translate( const Vector3<T>& t );
				T			  size() const;
				void		  getPosition( T& x, T& y, T& z ) const;
				void		  getPosition( Vector3<T>& pos ) const;
				void		  getSize( T& w, T& h, T& d ) const;
				void		  getSize( Vector3<T>& size ) const;
				bool		  contains( const Vector3<T>& pt ) const;
				bool		  contains( const Box<T>& b2 ) const;
				bool		  intersects( const Box<T>& t2 ) const;
				void		  intersect( const Box<T>& t2 );
				bool		  isEmpty() const;
				bool		  operator==( const Box<T>& t2 ) const;

			private:
				Vector3<T> p1, p2;
		};

	template<typename T>
		inline Box<T>::Box( T x, T y, T z, T x2, T y2, T z2 )
		{
			p1.set( x, y, z );
			p2.set( x2, y2, z2 );
		}


	template<typename T>
		inline Box<T>::Box( const Vector3<T>& p1, const Vector3<T>& p2 )
		{
			this->p1 = p1;
			this->p2 = p2;
		}


	template<typename T>
		inline const Box<T>& Box<T>::operator=( const Box<T>& b2 )
		{
			p1 = b2.p1;
			p2 = b2.p2;
		}

	template<typename T>
		inline void Box<T>::copy( const Box<T>& b2 )
		{
			p1 = b2.p1;
			p2 = b2.p2;
		}

	template<typename T>
		inline void Box<T>::set( const Vector3<T>& p1, const Vector3<T>& p2 )
		{
			this->p1 = p1;
			this->p2 = p2;
		}

	template<typename T>
		inline void Box<T>::set( T x, T y, T z, T x2, T y2, T z2 )
		{
			p1.set( x, y, z );
			p2.set( x2, y2, z2 );
		}

	template<typename T>
		inline void Box<T>::setPosition( T x, T y, T z )
		{
			p1.set( x, y, z );
		}

	template<typename T>
		inline void Box<T>::setSize( T w, T h, T d )
		{
			p2 = p1 + Vector3<T>( w, h, d );
		}

	template<typename T>
		inline void Box<T>::translate( T x, T y, T z )
		{
			p1.x += x;
			p1.y += y;
			p1.z += z;
		}

	template<typename T>
		inline void Box<T>::translate( const Vector3<T>& t )
		{
			p1 += t;
		}

	template<typename T>
		inline T Box<T>::size() const
		{
			return p2.x * p2.y * p2.z;
		}

	template<typename T>
		inline void Box<T>::getPosition( T& x, T& y, T& z ) const
		{
			x = p1.x;
			y = p1.y;
			z = p1.z;
		}

	template<typename T>
		inline void Box<T>::getPosition( Vector3<T>& pos ) const
		{
			pos = p1;
		}

	template<typename T>
		inline void Box<T>::getSize( T& w, T& h, T& d ) const
		{
			w = p2.x - p1.x;
			h = p2.y - p1.y;
			d = p2.z - p1.z;
		}

	template<typename T>
		inline void Box<T>::getSize( Vector3<T>& size ) const
		{
			size = p2 - p1;
		}

	template<typename T>
		inline bool Box<T>::contains( const Vector3<T>& pt ) const
		{
			if( pt.x >= p1.x && pt.x <= p2.x &&
			   pt.y >= p1.y && pt.y <= p2.y &&
			   pt.z >= p1.z && pt.z <= p2.z )
				return true;
			return false;
		}

	template<typename T>
		inline bool Box<T>::contains( const Box<T>& b2 ) const
		{
			if( !contains( b2.p1 ) )
				return false;
			if( !contains( b2.p2 ) )
				return false;
			return true;
		}

	template<typename T>
		inline bool Box<T>::intersects( const Box<T>& b2 ) const
		{
			if( b2.p2.x < p1.x || b2.p1.x > p2.x )
				return false;
			if( b2.p2.y < p2.y || b2.p1.y > p2.y )
				return false;
			if( b2.p2.z < p2.z || b2.p1.z > p2.z )
				return false;
			return true;
		}

	template<typename T>
		inline void Box<T>::intersect( const Box<T>& b2 )
		{
			if( !intersects( b2 ) ) {
				p1.setZero();
				p2.setZero();
				return;
			}
			p1.x = Math::max( p1.x, b2.p1.x );
			p1.y = Math::max( p1.y, b2.p1.y );
			p1.z = Math::max( p1.z, b2.p1.z );
			p2.x = Math::min( p2.x, b2.p2.x );
			p2.y = Math::min( p2.y, b2.p2.y );
			p2.z = Math::min( p2.z, b2.p2.z );
		}

	template<typename T>
		inline bool Box<T>::isEmpty() const
		{
			return p1 == p2;
		}

	template<typename T>
		inline bool Box<T>::operator==( const Box<T>& b2 ) const
		{
			return ( p1 == b2.p1 && p2 == b2.p2 );
		}

	template<typename T>
	inline std::ostream& operator<<( std::ostream& out, const Box<T>& box )
	{
		T x, y, z, w, h, d;

		box.getPosition( x, y, z );
		box.getSize( w, h, d );
		out << "Box: ( " << x << " , " << y << " , " << z << " ) ( "<< w << " x " << h << " x " << h << " ) "  << "\n";
		return out;
	}

	typedef Box<float> Boxf;
	typedef Box<double> Boxd;
}

#endif
