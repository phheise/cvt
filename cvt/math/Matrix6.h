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

#ifdef CVT_MATRIX_H

namespace cvt
{
	template<typename T>
	class Matrix6;

	template<typename T>
	std::ostream& operator<<( std::ostream& out, const Matrix4<T>& m );

	/**
	   \ingroup Math
	 */
	template<typename T>
	class Matrix6
	{
		public:
            typedef             T TYPE;
            enum                { DIMENSION = 6 };

			                    Matrix6<T>( void );
			                    Matrix6<T>( const Matrix6<T>& mat6 );
			explicit            Matrix6<T>( const Vector6<T>& a, const Vector6<T>& b, const Vector6<T>& c,
										    const Vector6<T>& d, const Vector6<T>& e, const Vector6<T>& f );
			explicit            Matrix6<T>( const T src[ 6 ][ 6 ] );

			const Vector6<T>&   operator[]( int index ) const;
			Vector6<T>&         operator[]( int index );
            const T&            operator()( int r, int c ) const;
            T&                  operator()( int r, int c );
			Matrix6<T>          operator-( ) const;
			Matrix6<T>          operator*( const T c ) const;
			Matrix6<T>          operator+( const T c ) const;
			Matrix6<T>          operator-( const T c ) const;
			Vector6<T>          operator*( const Vector6<T> &vec ) const;
			Matrix6<T>          operator+( const Matrix6<T>& m ) const;

			Matrix6<T>          inverse( void ) const;
			bool                inverseSelf( void );

			Vector6<T>          row( size_t r ) const;
			Vector6<T>          col( size_t c ) const;

			void                setZero( void );
			void				setIdentity( void );

			Matrix6<T>          transpose( void ) const;
			const T*            ptr( void ) const;
			T*                  ptr( void );

		private:
			Vector6<T>          mat[ 6 ];
	};


	template<typename T>
	inline Matrix6<T>::Matrix6( )
	{}

	template<typename T>
	inline Matrix6<T>::Matrix6( const Matrix6<T>& m )
	{
		memcpy( ( uint8_t* ) this->ptr( ), ( const uint8_t* ) m.ptr( ), sizeof( T ) * 36 );
	}

	template<typename T>
	inline Matrix6<T>::Matrix6(  const Vector6<T>& a, const Vector6<T>& b, const Vector6<T>& c, const Vector6<T>& d, const Vector6<T>& e, const Vector6<T>& f )
	{
		mat[ 0 ] = a;
		mat[ 1 ] = b;
		mat[ 2 ] = c;
		mat[ 3 ] = d;
		mat[ 4 ] = e;
		mat[ 5 ] = f;
	}

	template<typename T>
	inline Matrix6<T>::Matrix6( const T src[ 6 ][ 6 ] )
	{
		memcpy( this->ptr( ), src, sizeof( T ) * 36 );
	}

	template<typename T>
	inline const Vector6<T>& Matrix6<T>::operator[]( int index ) const
	{
		return mat[ index ];
	}

	template<typename T>
	inline Vector6<T>& Matrix6<T>::operator[]( int index )
	{
		return mat[ index ];
	}

    template<typename T>
    inline const T& Matrix6<T>::operator()( int r, int c ) const
    {
        return mat[ r ][ c ];
    }

    template<typename T>
    inline T& Matrix6<T>::operator()( int r, int c )
    {
        return mat[ r ][ c ];
    }

	template<typename T>
	inline Matrix6<T> Matrix6<T>::operator-( ) const
	{
		return Matrix6<T>( -mat[ 0 ], -mat[ 1 ], -mat[ 2 ], -mat[ 3 ], -mat[ 4 ], -mat[ 5 ] );
	}

	template<typename T>
	inline Matrix6<T> Matrix6<T>::operator*( const T c  ) const
	{
		return Matrix6<T>( mat[ 0 ] * c, mat[ 1 ] * c, mat[ 2 ] * c, mat[ 3 ] * c, mat[ 4 ] * c, mat[ 5 ] * c );
	}

	template<typename T>
	inline Matrix6<T> Matrix6<T>::operator+( const T c  ) const
	{
		return Matrix6<T>( mat[ 0 ] + c, mat[ 1 ] + c, mat[ 2 ] + c, mat[ 3 ] + c, mat[ 4 ] + c, mat[ 5 ] + c );
	}

	template<typename T>
	inline Matrix6<T> Matrix6<T>::operator-( const T c  ) const
	{
		return Matrix6<T>( mat[ 0 ] - c, mat[ 1 ] - c, mat[ 2 ] - c, mat[ 3 ] - c, mat[ 4 ] - c, mat[ 5 ] - c );
	}

	template<typename T>
	inline Vector6<T> Matrix6<T>::operator*( const Vector6<T>& vec ) const
	{
		return Vector6<T>( mat[ 0 ].x * vec.x + mat[ 0 ].y * vec.y + mat[ 0 ].z * vec.z + mat[ 0 ].u * vec.u + mat[ 0 ].v * vec.v + mat[ 0 ].w * vec.w,
						   mat[ 1 ].x * vec.x + mat[ 1 ].y * vec.y + mat[ 1 ].z * vec.z + mat[ 1 ].u * vec.u + mat[ 1 ].v * vec.v + mat[ 1 ].w * vec.w,
						   mat[ 2 ].x * vec.x + mat[ 2 ].y * vec.y + mat[ 2 ].z * vec.z + mat[ 2 ].u * vec.u + mat[ 2 ].v * vec.v + mat[ 2 ].w * vec.w,
						   mat[ 3 ].x * vec.x + mat[ 3 ].y * vec.y + mat[ 3 ].z * vec.z + mat[ 3 ].u * vec.u + mat[ 3 ].v * vec.v + mat[ 3 ].w * vec.w,
						   mat[ 4 ].x * vec.x + mat[ 4 ].y * vec.y + mat[ 4 ].z * vec.z + mat[ 4 ].u * vec.u + mat[ 4 ].v * vec.v + mat[ 4 ].w * vec.w,
						   mat[ 5 ].x * vec.x + mat[ 5 ].y * vec.y + mat[ 5 ].z * vec.z + mat[ 5 ].u * vec.u + mat[ 5 ].v * vec.v + mat[ 5 ].w * vec.w );
	}

	template<typename T>
	inline Matrix6<T> Matrix6<T>::operator+( const Matrix6<T>& m  ) const
	{
		return Matrix6<T>( mat[ 0 ] + m[ 0 ], mat[ 1 ] + m[ 1 ], mat[ 2 ] + m[ 2 ],
						   mat[ 3 ] + m[ 3 ], mat[ 4 ] + m[ 4 ], mat[ 5 ] + m[ 5 ]  );
	}

	template<typename T>
	inline Vector6<T> Matrix6<T>::row( size_t r ) const
	{
		return Vector6<T>( mat[ r ][ 0 ], mat[ r ][ 1 ], mat[ r ][ 2 ], mat[ r ][ 3 ], mat[ r ][ 4 ], mat[ r ][ 5 ] );
	}

	template<typename T>
	inline Vector6<T> Matrix6<T>::col( size_t c ) const
	{
		return Vector6<T>( mat[ 0 ][ c ], mat[ 1 ][ c ], mat[ 2 ][ c ], mat[ 3 ][ c ], mat[ 4 ][ c ], mat[ 5 ][ c ] );
	}

	template<typename T>
	inline void Matrix6<T>::setZero( )
	{
		mat[ 0 ].setZero( );
		mat[ 1 ].setZero( );
		mat[ 2 ].setZero( );
		mat[ 3 ].setZero( );
		mat[ 4 ].setZero( );
		mat[ 5 ].setZero( );
	}

    template<typename T>
    inline void Matrix6<T>::setIdentity()
    {
		for( int r = 0; r < 6; r++ ) {
			for( int c = 0; c < 6; c++ ) {
					mat[ r ][ c ] = ( r == c ) ? 1 : 0;
			}
		}
    }


	template<typename T>
	inline Matrix6<T> Matrix6<T>::transpose( ) const
	{
		return Matrix6<T>( Vector6<T>( mat[ 0 ][ 0 ], mat[ 1 ][ 0 ], mat[ 2 ][ 0 ], mat[ 3 ][ 0 ], mat[ 4 ][ 0 ], mat[ 5 ][ 0 ] ),
						   Vector6<T>( mat[ 0 ][ 1 ], mat[ 1 ][ 1 ], mat[ 2 ][ 1 ], mat[ 3 ][ 1 ], mat[ 4 ][ 1 ], mat[ 5 ][ 1 ] ),
						   Vector6<T>( mat[ 0 ][ 2 ], mat[ 1 ][ 2 ], mat[ 2 ][ 2 ], mat[ 3 ][ 2 ], mat[ 4 ][ 2 ], mat[ 5 ][ 2 ] ),
						   Vector6<T>( mat[ 0 ][ 3 ], mat[ 1 ][ 3 ], mat[ 2 ][ 3 ], mat[ 3 ][ 3 ], mat[ 4 ][ 3 ], mat[ 5 ][ 3 ] ),
						   Vector6<T>( mat[ 0 ][ 4 ], mat[ 1 ][ 4 ], mat[ 2 ][ 4 ], mat[ 3 ][ 4 ], mat[ 4 ][ 4 ], mat[ 5 ][ 4 ] ),
						   Vector6<T>( mat[ 0 ][ 5 ], mat[ 1 ][ 5 ], mat[ 2 ][ 5 ], mat[ 3 ][ 5 ], mat[ 4 ][ 5 ], mat[ 5 ][ 5 ] ) );
	}

	template<typename T>
	inline const T* Matrix6<T>::ptr( void ) const
	{
		return mat[ 0 ].ptr( );
	}

	template<typename T>
	inline T* Matrix6<T>::ptr( void )
	{
		return mat[ 0 ].ptr( );
	}

	template<typename T>
	inline Matrix6<T> Matrix6<T>::inverse( void ) const
	{
		Matrix6<T> inv;

		inv = *this;
		inv.inverseSelf( );
		return inv;
	}

	template<typename T>
	inline std::ostream& operator<<( std::ostream& out, const Matrix6<T>& m )
	{
		out << m[ 0 ] << std::endl;
		out << m[ 1 ] << std::endl;
		out << m[ 2 ] << std::endl;
		out << m[ 3 ] << std::endl;
		out << m[ 4 ] << std::endl;
		out << m[ 5 ];
		return out;
	}

}

#endif // MATRIX6_H
