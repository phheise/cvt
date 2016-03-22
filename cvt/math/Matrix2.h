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

namespace cvt {
    template<typename T> class Matrix2;
    template<typename T> class Matrix3;

    template<typename T>
    std::ostream& operator<<( std::ostream& out, const Matrix2<T>& m );

    /**
      \ingroup Math
    */
    template<typename T>
    class Matrix2 {
        public:
        typedef             T TYPE;
        enum                { DIMENSION = 2 };

                            Matrix2<T>( void );
        explicit			Matrix2<T>( const Vector2<T>& x, const Vector2<T>& y );
        explicit			Matrix2<T>( const T a, const T b, const T c, const T d );
        explicit			Matrix2<T>( const T src[ 2 ][ 2 ] );
        explicit			Matrix2<T>( const Matrix3<T>& mat3 );

        const Vector2<T>&	operator[]( int index ) const;
        Vector2<T>&			operator[]( int index );
        const T&            operator()( int r, int c ) const;
        T&                  operator()( int r, int c );
        Matrix2<T>			operator-() const;
        Matrix2<T>			operator*( const T c ) const;
        Matrix2<T>			operator+( const T c ) const;
        Matrix2<T>			operator-( const T c ) const;
        Vector2<T>			operator*( const Vector2<T> &vec ) const;
        Matrix2<T>			operator*( const Matrix2<T>& m ) const;
        Matrix2<T>			operator+( const Matrix2<T>& m ) const;
        Matrix2<T>			operator-( const Matrix2<T>& m ) const;
        Matrix2<T>&			operator*=( const T c );
        Matrix2<T>&			operator+=( const T c );
        Matrix2<T>&			operator-=( const T c );
        Matrix2<T>&			operator*=( const Matrix2<T>& m );
        Matrix2<T>&			operator+=( const Matrix2<T>& m );
        Matrix2<T>&			operator-=( const Matrix2<T>& m );

        bool				operator==( const Matrix2<T> &m ) const;
        bool				operator!=( const Matrix2<T> &m ) const;

        template <typename T2>
        operator Matrix2<T2>() const;

        Vector2<T>			row( size_t r ) const;
        Vector2<T>			col( size_t c ) const;

        void				setZero( void );
        void				setIdentity( void );
        bool				isIdentity( ) const;
        bool				isSymmetric( ) const;
        bool				isDiagonal( ) const;
        bool                isEqual( const Matrix2<T> & other, T epsilon ) const;

        void				setDiagonal( const Vector2<T>& diag );
        void				setRotationZ( T rad );

        T					trace( void ) const;
        T					determinant( void ) const;
        Matrix2<T>			transpose( void ) const;
        Matrix2<T>&			transposeSelf( void );
        Matrix2<T>			inverse( void ) const;
        bool				inverseSelf( void );
        void				svd( Matrix2<T>&, Matrix2<T>&, Matrix2<T>& ) const;
        Matrix2<T>			pseudoInverse() const;

        Matrix3<T>			toMatrix3( void ) const;

        int					dimension( void ) const;
        const T*			ptr( void ) const;
        T*					ptr( void );

        String              toString( void ) const;
        static Matrix2<T>   fromString( const String & s );

        friend std::ostream& operator<< <>( std::ostream& out, const Matrix2<T>& m );

        private:
        Vector2<T>			mat[ 2 ];
    };

    template<typename T>
    inline Matrix2<T>::Matrix2()
    {
    }

    template<typename T>
    inline Matrix2<T>::Matrix2( const Vector2<T>& x, const Vector2<T>& y )
    {
        mat[ 0 ] = x;
        mat[ 1 ] = y;
    }

    template<typename T>
    inline Matrix2<T>::Matrix2( const T a, const T b, const T c, const T d )
    {
        mat[ 0 ].x = a;
        mat[ 0 ].y = b;
        mat[ 1 ].x = c;
        mat[ 1 ].y = d;
    }

    template<typename T>
    inline Matrix2<T>::Matrix2( const T src[ 2 ][ 2 ] )
    {
        mat[ 0 ].x = src[ 0 ][ 0 ];
        mat[ 0 ].y = src[ 0 ][ 1 ];
        mat[ 1 ].x = src[ 1 ][ 0 ];
        mat[ 1 ].y = src[ 1 ][ 1 ];
    }

    template<typename T>
    inline Matrix2<T>::Matrix2( const Matrix3<T>& mat3 )
    {
        mat[ 0 ].x = mat3[ 0 ].x;
        mat[ 0 ].y = mat3[ 0 ].y;

        mat[ 1 ].x = mat3[ 1 ].x;
        mat[ 1 ].y = mat3[ 1 ].y;
    }

    template<typename T>
    inline const Vector2<T>& Matrix2<T>::operator[]( int index ) const
    {
        return mat[ index ];
    }

    template<typename T>
    inline Vector2<T>& Matrix2<T>::operator[]( int index )
    {
        return mat[ index ];
    }

    template<typename T>
    inline const T& Matrix2<T>::operator()( int r, int c ) const
    {
        return mat[ r ][ c ];
    }

    template<typename T>
    inline T& Matrix2<T>::operator()( int r, int c )
    {
        return mat[ r ][ c ];
    }

    template<typename T>
    inline Matrix2<T> Matrix2<T>::operator-( ) const
    {
        return Matrix2<T>( -mat[ 0 ][ 0 ], -mat[ 0 ][ 1 ],
                   -mat[ 1 ][ 0 ], -mat[ 1 ][ 1 ] );
    }

    template<typename T>
    inline Matrix2<T> Matrix2<T>::operator*( const T c  ) const
    {
        return Matrix2<T>( mat[ 0 ] * c, mat[ 1 ] * c );
    }

    template<typename T>
    inline Matrix2<T> Matrix2<T>::operator+( const T c  ) const
    {
        return Matrix2<T>( mat[ 0 ] + c, mat[ 1 ] + c );
    }

    template<typename T>
    inline Matrix2<T> Matrix2<T>::operator-( const T c  ) const
    {
        return Matrix2<T>( mat[ 0 ] - c, mat[ 1 ] - c );
    }

    template<typename T>
    inline Vector2<T> Matrix2<T>::operator*( const Vector2<T>& vec  ) const
    {
        return Vector2<T>( mat[ 0 ].x * vec.x + mat[ 0 ].y * vec.y,
                   mat[ 1 ].x * vec.x + mat[ 1 ].y * vec.y );
    }

    template<typename T>
    inline Matrix2<T> Matrix2<T>::operator*( const Matrix2<T>& m  ) const
    {
        return Matrix2<T>( mat[ 0 ].x * m[ 0 ].x + mat[ 0 ].y * m[ 1 ].x,
                   mat[ 0 ].x * m[ 0 ].y + mat[ 0 ].y * m[ 1 ].y,
                   mat[ 1 ].x * m[ 0 ].x + mat[ 1 ].y * m[ 1 ].x,
                   mat[ 1 ].x * m[ 0 ].y + mat[ 1 ].y * m[ 1 ].y );
    }

    template<typename T>
    inline Matrix2<T> Matrix2<T>::operator+( const Matrix2<T>& m  ) const
    {
        return Matrix2<T>( mat[ 0 ] + m[ 0 ], mat[ 1 ] + m[ 1 ] );
    }

    template<typename T>
    inline Matrix2<T> Matrix2<T>::operator-( const Matrix2<T>& m  ) const
    {
        return Matrix2<T>( mat[ 0 ] - m[ 0 ], mat[ 1 ] - m[ 1 ] );
    }

    template<typename T>
    inline Matrix2<T>& Matrix2<T>::operator*=( const T c  )
    {
        mat[ 0 ] *= c;
        mat[ 1 ] *= c;
        return *this;
    }

    template<typename T>
    inline Matrix2<T>& Matrix2<T>::operator+=( const T c  )
    {
        mat[ 0 ] += c;
        mat[ 1 ] += c;
        return *this;
    }

    template<typename T>
    inline Matrix2<T>& Matrix2<T>::operator-=( const T c  )
    {
        mat[ 0 ] -= c;
        mat[ 1 ] -= c;
        return *this;
    }

    template<typename T>
    inline Matrix2<T>& Matrix2<T>::operator*=( const Matrix2<T>& m  )
    {
        float x, y;
        x = mat[ 0 ].x;
        y = mat[ 0 ].y;
        mat[ 0 ].x = x * m[ 0 ].x + y * m[ 1 ].x;
        mat[ 0 ].y = x * m[ 0 ].y + y * m[ 1 ].y;
        x = mat[ 1 ].x;
        y = mat[ 1 ].y;
        mat[ 1 ].x = x * m[ 0 ].x + y * m[ 1 ].x;
        mat[ 1 ].y = x * m[ 0 ].y + y * m[ 1 ].y;
        return *this;
    }

    template<typename T>
    inline Matrix2<T>& Matrix2<T>::operator+=( const Matrix2<T>& m  )
    {
        mat[ 0 ] += m[ 0 ];
        mat[ 1 ] += m[ 1 ];
        return *this;
    }

    template<typename T>
    inline Matrix2<T>& Matrix2<T>::operator-=( const Matrix2<T>& m  )
    {
        mat[ 0 ] -= m[ 0 ];
        mat[ 1 ] -= m[ 1 ];
        return *this;
    }

    template<typename T>
    inline bool Matrix2<T>::operator==( const Matrix2<T> &m ) const
    {
        return mat[ 0 ] == m[ 0 ] && mat[ 1 ] == m[ 1 ];
    }

    template<typename T>
    inline bool Matrix2<T>::operator!=( const Matrix2<T> &m ) const
    {
        return mat[ 0 ] != m[ 0 ] || mat[ 1 ] != m[ 1 ];
    }

    template <typename T> template <typename T2>
    inline Matrix2<T>::operator Matrix2<T2>() const
    {
        return Matrix2<T2>( ( T2 ) mat[ 0 ][ 0 ], ( T2 ) mat[ 0 ][ 1 ],
                            ( T2 ) mat[ 1 ][ 0 ], ( T2 ) mat[ 1 ][ 1 ] );
    }

    template<typename T>
    inline Vector2<T> Matrix2<T>::row( size_t r ) const
    {
        return Vector2<T>( mat[ r ][ 0 ], mat[ r ][ 1 ] );
    }

    template<typename T>
    inline Vector2<T> Matrix2<T>::col( size_t c ) const
    {
        return Vector2<T>( mat[ 0 ][ c ], mat[ 1 ][ c ] );
    }

    template<typename T>
    inline void Matrix2<T>::setZero()
    {
        mat[ 0 ].setZero();
        mat[ 1 ].setZero();
    }

    template<typename T>
    inline void Matrix2<T>::setIdentity()
    {
        mat[ 0 ].x = 1;
        mat[ 0 ].y = 0;
        mat[ 1 ].x = 0;
        mat[ 1 ].y = 1;
    }

    template<>
    inline bool Matrix2<double>::isIdentity() const
    {
        return mat[ 0 ] == Vector2<double>( 1.0, 0.0 ) && mat[ 1 ] == Vector2<double>( 0.0, 1.0 );
    }

    template<>
    inline bool Matrix2<float>::isIdentity() const
    {
        return mat[ 0 ] == Vector2<float>( 1.0f, 0.0f ) && mat[ 1 ] == Vector2<float>( 0.0f, 1.0f );
    }

    template<>
    inline bool Matrix2<float>::isSymmetric() const
    {
        return Math::abs( mat[ 0 ][ 1 ] - mat[ 1 ][ 0 ] ) < Math::EPSILONF;
    }

    template<>
    inline bool Matrix2<double>::isSymmetric() const
    {
        return Math::abs( mat[ 0 ][ 1 ] - mat[ 1 ][ 0 ] ) < Math::EPSILOND;
    }

    template<>
    inline bool Matrix2<float>::isDiagonal() const
    {
        return Math::abs( mat[ 0 ].y ) < Math::EPSILONF && Math::abs( mat[ 1 ].x ) < Math::EPSILONF;
    }

    template<>
    inline bool Matrix2<double>::isDiagonal() const
    {
        return Math::abs( mat[ 0 ].y ) < Math::EPSILOND && Math::abs( mat[ 1 ].x ) < Math::EPSILOND;
    }

    template<typename T>
    inline bool Matrix2<T>::isEqual( const Matrix2<T>& other, T epsilon ) const
    {
        for( size_t i = 0; i < 2; i++ ){
            for( size_t k = 0; k < 2; k++ ){
                if( Math::abs( mat[ i ][ k ] - other[ i ][ k ] ) > epsilon )
                    return false;
            }
        }
        return true;
    }

    template<typename T>
    inline void Matrix2<T>::setRotationZ( T rad )
    {
        T s = Math::sin( rad );
        T c = Math::cos( rad );

        mat[ 0 ].x = c;
        mat[ 0 ].y = -s;

        mat[ 1 ].x = s;
        mat[ 1 ].y = c;
    }

    template<typename T>
    inline void Matrix2<T>::setDiagonal( const Vector2<T>& diag )
    {
        mat[ 0 ].x = diag.x;
        mat[ 0 ].y = 0;

        mat[ 1 ].x = 0;
        mat[ 1 ].y = diag.y;
    }

    template<typename T>
    inline T Matrix2<T>::trace() const
    {
        return mat[ 0 ].x + mat[ 1 ].y;
    }

    template<typename T>
    inline T Matrix2<T>::determinant() const
    {
        return mat[ 0 ].x * mat[ 1 ].y - mat[ 1 ].x * mat[ 0 ].y;
    }

    template<typename T>
    inline Matrix2<T> Matrix2<T>::transpose() const
    {
        return Matrix2<T>( mat[ 0 ].x, mat[ 1 ].x, mat[ 0 ].y, mat[ 1 ].y );
    }

    template<typename T>
    inline Matrix2<T>& Matrix2<T>::transposeSelf()
    {
        T tmp;
        tmp = mat[ 0 ].y;
        mat[ 0 ].y = mat[ 1 ].x;
        mat[ 1 ].x = tmp;
        return *this;
    }

    template<typename T>
    inline Matrix2<T> Matrix2<T>::pseudoInverse() const
    {
        Matrix2<T> U, E, V;

        svd( U, E, V );

        for( int i = 0; i < 2; i++ ) {
            if( Math::abs( E[ i ][ i ] ) > Math::epsilon<T>() )
                E[ i ][ i ] = ( T ) 1 / E[ i ][ i ];
        }

        return V * E * U.transpose();
    }

    template<typename T>
    inline Matrix3<T> Matrix2<T>::toMatrix3() const
    {
        return Matrix3<T>( *this );
    }

    template<typename T>
    inline Matrix2<T> Matrix2<T>::inverse( void ) const
    {
        Matrix2<T> inv;

        inv = *this;
        inv.inverseSelf();
        return inv;
    }

    template<typename T>
    int	Matrix2<T>::dimension( void ) const
    {
        return 2;
    }

    template<typename T>
    const T* Matrix2<T>::ptr( void ) const
    {
        return mat[ 0 ].ptr();
    }

    template<typename T>
    T* Matrix2<T>::ptr( void )
    {
        return mat[ 0 ].ptr();
    }

    template<typename T>
    String Matrix2<T>::toString( void ) const
    {
        String s;
        s.sprintf( "%0.10f %0.10f\n"
                   "%0.10f %0.10f",
                   mat[ 0 ][ 0 ], mat[ 0 ][ 1 ],
                   mat[ 1 ][ 0 ], mat[ 1 ][ 1 ]);
        return s;
    }

    template<typename T>
    inline std::ostream& operator<<( std::ostream& out, const Matrix2<T>& m )
    {
        out << m.mat[ 0 ] << std::endl;
        out << m.mat[ 1 ];
        return out;
    }

}


#endif
