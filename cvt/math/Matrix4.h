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
    template<typename T> class Matrix3;
    template<typename T> class Matrix4;

    template<typename T>
    std::ostream& operator<<( std::ostream& out, const Matrix4<T>& m );

    /**
      \ingroup Math
    */
    template<typename T>
    class Matrix4 {
        public:
        typedef             T TYPE;
        enum                { DIMENSION = 4 };

                            Matrix4<T>( void );
                            Matrix4<T>( const Matrix4<T>& mat4 );
        explicit			Matrix4<T>( const Vector4<T>& x, const Vector4<T>& y, const Vector4<T>& z, const Vector4<T>& w );
        explicit			Matrix4<T>( const T a, const T b, const T c, const T d,
                                        const T e, const T f, const T g, const T h,
                                        const T i, const T j, const T k, const T l,
                                        const T m, const T n, const T o, const T p );
        explicit			Matrix4<T>( const T src[ 4 ][ 4 ] );
        explicit			Matrix4<T>( const Matrix3<T>& mat3 );

        const Vector4<T>&	operator[]( int index ) const;
        Vector4<T>&			operator[]( int index );
        const T&            operator()( int r, int c ) const;
        T&                  operator()( int r, int c );
        Matrix4<T>			operator-() const;
        Matrix4<T>			operator*( const T c ) const;
        Matrix4<T>			operator+( const T c ) const;
        Matrix4<T>			operator-( const T c ) const;
        Vector3<T>			operator*( const Vector3<T> &vec ) const;
        Vector4<T>			operator*( const Vector4<T> &vec ) const;
        Matrix4<T>			operator*( const Matrix4<T>& m ) const;
        Matrix4<T>			operator+( const Matrix4<T>& m ) const;
        Matrix4<T>			operator-( const Matrix4<T>& m ) const;
        Matrix4<T>&			operator*=( const T c );
        Matrix4<T>&			operator+=( const T c );
        Matrix4<T>&			operator-=( const T c );
        Matrix4<T>&			operator*=( const Matrix4<T>& m );
        Matrix4<T>&			operator+=( const Matrix4<T>& m );
        Matrix4<T>&			operator-=( const Matrix4<T>& m );

        template <typename T2>
        operator Matrix4<T2>() const;

        bool				operator==( const Matrix4<T> &m ) const;
        bool				operator!=( const Matrix4<T> &m ) const;

        Vector4<T>			row( size_t r ) const;
        Vector4<T>			col( size_t c ) const;

        void				setZero( void );
        void				setIdentity( void );
        bool				isIdentity( ) const;
        bool				isSymmetric( ) const;
        bool				isDiagonal( ) const;
        bool                isEqual( const Matrix4<T> & other, T epsilon ) const;

        void				setDiagonal( const Vector4<T>& diag );
        void				setRotationX( T rad );
        void				setRotationY( T rad );
        void				setRotationZ( T rad );
        void                setRotationXYZ( T angleX, T angleY, T angleZ );
        void				setRotation( const Vector3<T>& axis, T rad );
        void				setTranslation( T x, T y, T z );

        T					trace( void ) const;
        T					determinant( void ) const;
        Matrix4<T>			transpose( void ) const;
        Matrix4<T>&			transposeSelf( void );
        Matrix4<T>			inverse( void ) const;
        bool				inverseSelf( void );
        void				svd( Matrix4<T>& u, Matrix4<T>& d, Matrix4<T>& vt ) const;
        Matrix4<T>			pseudoInverse() const;

        Matrix3<T>			toMatrix3( void ) const;

        int					dimension( void ) const;
        const T*			ptr( void ) const;
        T*					ptr( void );

        String              toString( void ) const;
        static Matrix4<T>   fromString( const String & s );


        friend std::ostream& operator<< <>( std::ostream& out, const Matrix4<T>& m );

        private:
        Vector4<T>			mat[ 4 ];
    };

    template<typename T>
    inline Matrix4<T>::Matrix4()
    {
    }

    template<typename T>
    inline Matrix4<T>::Matrix4( const Matrix4<T>& m )
    {
        memcpy( ( uint8_t* ) this->ptr(), ( const uint8_t* ) m.ptr(), sizeof( T ) * 16 );
    }

    template<typename T>
    inline Matrix4<T>::Matrix4( const Matrix3<T>& m )
    {
        mat[ 0 ].x = m[ 0 ].x;
        mat[ 0 ].y = m[ 0 ].y;
        mat[ 0 ].z = m[ 0 ].z;
        mat[ 0 ].w = 0;

        mat[ 1 ].x = m[ 1 ].x;
        mat[ 1 ].y = m[ 1 ].y;
        mat[ 1 ].z = m[ 1 ].z;
        mat[ 1 ].w = 0;

        mat[ 2 ].x = m[ 2 ].x;
        mat[ 2 ].y = m[ 2 ].y;
        mat[ 2 ].z = m[ 2 ].z;
        mat[ 2 ].w = 0;

        mat[ 3 ].x = 0;
        mat[ 3 ].y = 0;
        mat[ 3 ].z = 0;
        mat[ 3 ].w = 1;

    }

    template<typename T>
    inline Matrix4<T>::Matrix4( const Vector4<T>& x, const Vector4<T>& y, const Vector4<T>& z, const Vector4<T>& w )
    {
        mat[ 0 ] = x;
        mat[ 1 ] = y;
        mat[ 2 ] = z;
        mat[ 3 ] = w;
    }

    template<typename T>
    inline	Matrix4<T>::Matrix4( const T a, const T b, const T c, const T d,
                                 const T e, const T f, const T g, const T h,
                                 const T i, const T j, const T k, const T l,
                                 const T m, const T n, const T o, const T p )
    {
        mat[ 0 ].x = a;
        mat[ 0 ].y = b;
        mat[ 0 ].z = c;
        mat[ 0 ].w = d;

        mat[ 1 ].x = e;
        mat[ 1 ].y = f;
        mat[ 1 ].z = g;
        mat[ 1 ].w = h;

        mat[ 2 ].x = i;
        mat[ 2 ].y = j;
        mat[ 2 ].z = k;
        mat[ 2 ].w = l;

        mat[ 3 ].x = m;
        mat[ 3 ].y = n;
        mat[ 3 ].z = o;
        mat[ 3 ].w = p;
    }

    template<typename T>
    inline Matrix4<T>::Matrix4( const T src[ 4 ][ 4 ] )
    {
        memcpy( this->ptr(), src, sizeof( T ) * 16 );
    }

    template<typename T>
    inline const Vector4<T>& Matrix4<T>::operator[]( int index ) const
    {
        return mat[ index ];
    }

    template<typename T>
    inline Vector4<T>& Matrix4<T>::operator[]( int index )
    {
        return mat[ index ];
    }

    template<typename T>
    inline const T& Matrix4<T>::operator()( int r, int c ) const
    {
        return mat[ r ][ c ];
    }

    template<typename T>
    inline T& Matrix4<T>::operator()( int r, int c )
    {
        return mat[ r ][ c ];
    }

    template<typename T>
    inline Matrix4<T> Matrix4<T>::operator-( ) const
    {
        return Matrix4<T>( -mat[ 0 ], -mat[ 1 ], -mat[ 2 ], -mat[ 3 ] );
    }

    template<typename T>
    inline Matrix4<T> Matrix4<T>::operator*( const T c  ) const
    {
        return Matrix4<T>( mat[ 0 ] * c, mat[ 1 ] * c, mat[ 2 ] * c, mat[ 3 ] * c );
    }

    template<typename T>
    inline Matrix4<T> Matrix4<T>::operator+( const T c  ) const
    {
        return Matrix4<T>( mat[ 0 ] + c, mat[ 1 ] + c, mat[ 2 ] + c, mat[ 3 ] + c );
    }

    template<typename T>
    inline Matrix4<T> Matrix4<T>::operator-( const T c  ) const
    {
        return Matrix4<T>( mat[ 0 ] - c, mat[ 1 ] - c, mat[ 2 ] - c, mat[ 3 ] - c );
    }


    template<typename T>
    inline Vector3<T>	Matrix4<T>::operator*( const Vector3<T> &vec ) const
    {
        Vector3<T> ret( mat[ 0 ].x * vec.x + mat[ 0 ].y * vec.y + mat[ 0 ].z * vec.z + mat[ 0 ].w,
                        mat[ 1 ].x * vec.x + mat[ 1 ].y * vec.y + mat[ 1 ].z * vec.z + mat[ 1 ].w,
                        mat[ 2 ].x * vec.x + mat[ 2 ].y * vec.y + mat[ 2 ].z * vec.z + mat[ 2 ].w );
        ret /= mat[ 3 ].x * vec.x + mat[ 3 ].y * vec.y + mat[ 3 ].z * vec.z + mat[ 3 ].w;
        return ret;
    }

    template<typename T>
    inline Vector4<T> Matrix4<T>::operator*( const Vector4<T>& vec  ) const
    {
        return Vector4<T>( mat[ 0 ].x * vec.x + mat[ 0 ].y * vec.y + mat[ 0 ].z * vec.z + mat[ 0 ].w * vec.w,
                           mat[ 1 ].x * vec.x + mat[ 1 ].y * vec.y + mat[ 1 ].z * vec.z + mat[ 1 ].w * vec.w,
                           mat[ 2 ].x * vec.x + mat[ 2 ].y * vec.y + mat[ 2 ].z * vec.z + mat[ 2 ].w * vec.w,
                           mat[ 3 ].x * vec.x + mat[ 3 ].y * vec.y + mat[ 3 ].z * vec.z + mat[ 3 ].w * vec.w );
    }

    template<typename T>
    inline Matrix4<T> Matrix4<T>::operator*( const Matrix4<T>& m  ) const
    {
        return Matrix4<T>( mat[ 0 ][ 0 ] * m[ 0 ][ 0 ] + mat[ 0 ][ 1 ] * m[ 1 ][ 0 ] + mat[ 0 ][ 2 ] * m[ 2 ][ 0 ] + mat[ 0 ][ 3 ] * m[ 3 ][ 0 ],
                           mat[ 0 ][ 0 ] * m[ 0 ][ 1 ] + mat[ 0 ][ 1 ] * m[ 1 ][ 1 ] + mat[ 0 ][ 2 ] * m[ 2 ][ 1 ] + mat[ 0 ][ 3 ] * m[ 3 ][ 1 ],
                           mat[ 0 ][ 0 ] * m[ 0 ][ 2 ] + mat[ 0 ][ 1 ] * m[ 1 ][ 2 ] + mat[ 0 ][ 2 ] * m[ 2 ][ 2 ] + mat[ 0 ][ 3 ] * m[ 3 ][ 2 ],
                           mat[ 0 ][ 0 ] * m[ 0 ][ 3 ] + mat[ 0 ][ 1 ] * m[ 1 ][ 3 ] + mat[ 0 ][ 2 ] * m[ 2 ][ 3 ] + mat[ 0 ][ 3 ] * m[ 3 ][ 3 ],

                           mat[ 1 ][ 0 ] * m[ 0 ][ 0 ] + mat[ 1 ][ 1 ] * m[ 1 ][ 0 ] + mat[ 1 ][ 2 ] * m[ 2 ][ 0 ] + mat[ 1 ][ 3 ] * m[ 3 ][ 0 ],
                           mat[ 1 ][ 0 ] * m[ 0 ][ 1 ] + mat[ 1 ][ 1 ] * m[ 1 ][ 1 ] + mat[ 1 ][ 2 ] * m[ 2 ][ 1 ] + mat[ 1 ][ 3 ] * m[ 3 ][ 1 ],
                           mat[ 1 ][ 0 ] * m[ 0 ][ 2 ] + mat[ 1 ][ 1 ] * m[ 1 ][ 2 ] + mat[ 1 ][ 2 ] * m[ 2 ][ 2 ] + mat[ 1 ][ 3 ] * m[ 3 ][ 2 ],
                           mat[ 1 ][ 0 ] * m[ 0 ][ 3 ] + mat[ 1 ][ 1 ] * m[ 1 ][ 3 ] + mat[ 1 ][ 2 ] * m[ 2 ][ 3 ] + mat[ 1 ][ 3 ] * m[ 3 ][ 3 ],

                           mat[ 2 ][ 0 ] * m[ 0 ][ 0 ] + mat[ 2 ][ 1 ] * m[ 1 ][ 0 ] + mat[ 2 ][ 2 ] * m[ 2 ][ 0 ] + mat[ 2 ][ 3 ] * m[ 3 ][ 0 ],
                           mat[ 2 ][ 0 ] * m[ 0 ][ 1 ] + mat[ 2 ][ 1 ] * m[ 1 ][ 1 ] + mat[ 2 ][ 2 ] * m[ 2 ][ 1 ] + mat[ 2 ][ 3 ] * m[ 3 ][ 1 ],
                           mat[ 2 ][ 0 ] * m[ 0 ][ 2 ] + mat[ 2 ][ 1 ] * m[ 1 ][ 2 ] + mat[ 2 ][ 2 ] * m[ 2 ][ 2 ] + mat[ 2 ][ 3 ] * m[ 3 ][ 2 ],
                           mat[ 2 ][ 0 ] * m[ 0 ][ 3 ] + mat[ 2 ][ 1 ] * m[ 1 ][ 3 ] + mat[ 2 ][ 2 ] * m[ 2 ][ 3 ] + mat[ 2 ][ 3 ] * m[ 3 ][ 3 ],

                           mat[ 3 ][ 0 ] * m[ 0 ][ 0 ] + mat[ 3 ][ 1 ] * m[ 1 ][ 0 ] + mat[ 3 ][ 2 ] * m[ 2 ][ 0 ] + mat[ 3 ][ 3 ] * m[ 3 ][ 0 ],
                           mat[ 3 ][ 0 ] * m[ 0 ][ 1 ] + mat[ 3 ][ 1 ] * m[ 1 ][ 1 ] + mat[ 3 ][ 2 ] * m[ 2 ][ 1 ] + mat[ 3 ][ 3 ] * m[ 3 ][ 1 ],
                           mat[ 3 ][ 0 ] * m[ 0 ][ 2 ] + mat[ 3 ][ 1 ] * m[ 1 ][ 2 ] + mat[ 3 ][ 2 ] * m[ 2 ][ 2 ] + mat[ 3 ][ 3 ] * m[ 3 ][ 2 ],
                           mat[ 3 ][ 0 ] * m[ 0 ][ 3 ] + mat[ 3 ][ 1 ] * m[ 1 ][ 3 ] + mat[ 3 ][ 2 ] * m[ 2 ][ 3 ] + mat[ 3 ][ 3 ] * m[ 3 ][ 3 ] );
    }

    template<typename T>
    inline Matrix4<T> Matrix4<T>::operator+( const Matrix4<T>& m  ) const
    {
        return Matrix4<T>( mat[ 0 ] + m[ 0 ], mat[ 1 ] + m[ 1 ], mat[ 2 ] + m[ 2 ], mat[ 3 ] + m[ 3 ] );
    }

    template<typename T>
    inline Matrix4<T> Matrix4<T>::operator-( const Matrix4<T>& m  ) const
    {
        return Matrix4<T>( mat[ 0 ] - m[ 0 ], mat[ 1 ] - m[ 1 ], mat[ 2 ] - m[ 2 ], mat[ 3 ] - m[ 3 ] );
    }

    template<typename T>
    inline Matrix4<T>& Matrix4<T>::operator*=( const T c  )
    {
        mat[ 0 ] *= c;
        mat[ 1 ] *= c;
        mat[ 2 ] *= c;
        mat[ 3 ] *= c;
        return *this;
    }

    template<typename T>
    inline Matrix4<T>& Matrix4<T>::operator+=( const T c  )
    {
        mat[ 0 ] += c;
        mat[ 1 ] += c;
        mat[ 2 ] += c;
        mat[ 3 ] += c;
        return *this;
    }

    template<typename T>
    inline Matrix4<T>& Matrix4<T>::operator-=( const T c  )
    {
        mat[ 0 ] -= c;
        mat[ 1 ] -= c;
        mat[ 2 ] -= c;
        mat[ 3 ] -= c;
        return *this;
    }

    template<typename T>
    inline Matrix4<T>& Matrix4<T>::operator+=( const Matrix4<T>& m  )
    {
        mat[ 0 ] += m[ 0 ];
        mat[ 1 ] += m[ 1 ];
        mat[ 2 ] += m[ 2 ];
        mat[ 3 ] += m[ 3 ];
        return *this;
    }

    template<typename T>
    inline Matrix4<T>& Matrix4<T>::operator-=( const Matrix4<T>& m  )
    {
        mat[ 0 ] -= m[ 0 ];
        mat[ 1 ] -= m[ 1 ];
        mat[ 2 ] -= m[ 2 ];
        mat[ 3 ] -= m[ 3 ];
        return *this;
    }

    template<typename T>
    inline Matrix4<T>&  Matrix4<T>::operator*=( const Matrix4<T>& m )
    {
        *this = (*this) * m;
        return *this;
    }


    template<typename T>
    inline bool Matrix4<T>::operator==( const Matrix4<T> &m ) const
    {
        return mat[ 0 ] == m[ 0 ] && mat[ 1 ] == m[ 1 ] && mat[ 2 ] == m[ 2 ] && mat[ 3 ] == m[ 3 ];
    }

    template<typename T>
    inline bool Matrix4<T>::operator!=( const Matrix4<T> &m ) const
    {
        return mat[ 0 ] != m[ 0 ] || mat[ 1 ] != m[ 1 ] || mat[ 2 ] != m[ 2 ] || mat[ 3 ] != m[ 3 ];
    }

    template <typename T> template <typename T2>
    inline Matrix4<T>::operator Matrix4<T2>() const
    {
        return Matrix4<T2>( ( T2 ) mat[ 0 ][ 0 ], ( T2 ) mat[ 0 ][ 1 ], ( T2 ) mat[ 0 ][ 2 ], ( T2 ) mat[ 0 ][ 3 ],
                            ( T2 ) mat[ 1 ][ 0 ], ( T2 ) mat[ 1 ][ 1 ], ( T2 ) mat[ 1 ][ 2 ], ( T2 ) mat[ 1 ][ 3 ],
                            ( T2 ) mat[ 2 ][ 0 ], ( T2 ) mat[ 2 ][ 1 ], ( T2 ) mat[ 2 ][ 2 ], ( T2 ) mat[ 2 ][ 3 ],
                            ( T2 ) mat[ 3 ][ 0 ], ( T2 ) mat[ 3 ][ 1 ], ( T2 ) mat[ 3 ][ 2 ], ( T2 ) mat[ 3 ][ 3 ] );
    }

    template<typename T>
    inline Vector4<T> Matrix4<T>::row( size_t r ) const
    {
        return Vector4<T>( mat[ r ][ 0 ], mat[ r ][ 1 ], mat[ r ][ 2 ], mat[ r ][ 3 ] );
    }

    template<typename T>
    inline Vector4<T> Matrix4<T>::col( size_t c ) const
    {
        return Vector4<T>( mat[ 0 ][ c ], mat[ 1 ][ c ], mat[ 2 ][ c ], mat[ 3 ][ c ] );
    }


    template<typename T>
    inline void Matrix4<T>::setZero()
    {
        mat[ 0 ].setZero();
        mat[ 1 ].setZero();
        mat[ 2 ].setZero();
        mat[ 3 ].setZero();
    }

    template<typename T>
    inline void Matrix4<T>::setIdentity()
    {
        mat[ 0 ].x = 1;
        mat[ 0 ].y = 0;
        mat[ 0 ].z = 0;
        mat[ 0 ].w = 0;

        mat[ 1 ].x = 0;
        mat[ 1 ].y = 1;
        mat[ 1 ].z = 0;
        mat[ 1 ].w = 0;

        mat[ 2 ].x = 0;
        mat[ 2 ].y = 0;
        mat[ 2 ].z = 1;
        mat[ 2 ].w = 0;

        mat[ 3 ].x = 0;
        mat[ 3 ].y = 0;
        mat[ 3 ].z = 0;
        mat[ 3 ].w = 1;
    }

    template<typename T>
    inline void Matrix4<T>::setDiagonal( const Vector4<T>& diag )
    {
        mat[ 0 ].x = diag.x;
        mat[ 0 ].y = 0;
        mat[ 0 ].z = 0;
        mat[ 0 ].w = 0;

        mat[ 1 ].x = 0;
        mat[ 1 ].y = diag.y;
        mat[ 1 ].z = 0;
        mat[ 1 ].w = 0;

        mat[ 2 ].x = 0;
        mat[ 2 ].y = 0;
        mat[ 2 ].z = diag.z;
        mat[ 2 ].w = 0;

        mat[ 3 ].x = 0;
        mat[ 3 ].y = 0;
        mat[ 3 ].z = 0;
        mat[ 3 ].w = diag.w;
    }


    template<typename T>
    inline void Matrix4<T>::setRotationX( T rad )
    {
        T s = Math::sin( rad );
        T c = Math::cos( rad );

        mat[ 0 ].x = 1;
        mat[ 0 ].y = 0;
        mat[ 0 ].z = 0;
        mat[ 0 ].w = 0;

        mat[ 1 ].x = 0;
        mat[ 1 ].y = c;
        mat[ 1 ].z = -s;
        mat[ 1 ].w = 0;

        mat[ 2 ].x = 0;
        mat[ 2 ].y = s;
        mat[ 2 ].z = c;
        mat[ 2 ].w = 0;

        mat[ 3 ].x = 0;
        mat[ 3 ].y = 0;
        mat[ 3 ].z = 0;
        mat[ 3 ].w = 1;
    }

    template<typename T>
    inline void Matrix4<T>::setRotationY( T rad )
    {
        T s = Math::sin( rad );
        T c = Math::cos( rad );

        mat[ 0 ].x = c;
        mat[ 0 ].y = 0;
        mat[ 0 ].z = s;
        mat[ 0 ].w = 0;

        mat[ 1 ].x = 0;
        mat[ 1 ].y = 1;
        mat[ 1 ].z = 0;
        mat[ 1 ].w = 0;

        mat[ 2 ].x = -s;
        mat[ 2 ].y = 0;
        mat[ 2 ].z = c;
        mat[ 1 ].w = 0;

        mat[ 3 ].x = 0;
        mat[ 3 ].y = 0;
        mat[ 3 ].z = 0;
        mat[ 3 ].w = 1;
    }

    template<typename T>
    inline void Matrix4<T>::setRotationZ( T rad )
    {
        T s = Math::sin( rad );
        T c = Math::cos( rad );

        mat[ 0 ].x = c;
        mat[ 0 ].y = -s;
        mat[ 0 ].z = 0;
        mat[ 0 ].w = 0;

        mat[ 1 ].x = s;
        mat[ 1 ].y = c;
        mat[ 1 ].z = 0;
        mat[ 1 ].w = 0;

        mat[ 2 ].x = 0;
        mat[ 2 ].y = 0;
        mat[ 2 ].z = 1;
        mat[ 2 ].w = 0;

        mat[ 3 ].x = 0;
        mat[ 3 ].y = 0;
        mat[ 3 ].z = 0;
        mat[ 3 ].w = 1;
    }

    template<typename T>
    inline void Matrix4<T>::setRotationXYZ( T angleX, T angleY, T angleZ )
    {
        T cx = Math::cos( angleX );
        T cy = Math::cos( angleY );
        T cz = Math::cos( angleZ );

        T sx = Math::sin( angleX );
        T sy = Math::sin( angleY );
        T sz = Math::sin( angleZ );

        mat[ 0 ][ 0 ] =  cy * cz;
        mat[ 0 ][ 1 ] = -cy * sz;
        mat[ 0 ][ 2 ] =       sy;
        mat[ 0 ][ 3 ] =        0;

        mat[ 1 ][ 0 ] = cx * sz + cz * sx * sy;
        mat[ 1 ][ 1 ] = cx * cz - sx * sy * sz;
        mat[ 1 ][ 2 ] =               -cy * sx;
        mat[ 1 ][ 3 ] =                      0;

        mat[ 2 ][ 0 ] =       sx * sz - cx * cz * sy;
        mat[ 2 ][ 1 ] =  cx * sy * sz +      cz * sx;
        mat[ 2 ][ 2 ] =                      cx * cy;
        mat[ 2 ][ 3 ] =                            0;

        mat[ 3 ][ 0 ] = 0;
        mat[ 3 ][ 1 ] = 0;
        mat[ 3 ][ 2 ] = 0;
        mat[ 3 ][ 3 ] = 1;
    }

    template<typename T>
    inline void	Matrix4<T>::setRotation( const Vector3<T>& _axis, T rad )
    {
        Vector3<T> axis( _axis );
        axis.normalize();
        T x, y, z, c, s;
        T wx, wy, wz;
        T xx, yy, yz;
        T xy, xz, zz;
        T x2, y2, z2;

        c = Math::cos( rad * ( T ) 0.5 );
        s = Math::sin( rad * ( T ) 0.5 );

        x = axis.x * s;
        y = axis.y * s;
        z = axis.z * s;

        x2 = x + x;
        y2 = y + y;
        z2 = z + z;

        xx = x * x2;
        xy = x * y2;
        xz = x * z2;

        yy = y * y2;
        yz = y * z2;
        zz = z * z2;

        wx = c * x2;
        wy = c * y2;
        wz = c * z2;

        mat[ 0 ][ 0 ] = ( T ) 1 - ( yy + zz );
        mat[ 0 ][ 1 ] = xy - wz;
        mat[ 0 ][ 2 ] = xz + wy;
        mat[ 0 ][ 3 ] = 0;

        mat[ 1 ][ 0 ] = xy + wz;
        mat[ 1 ][ 1 ] = ( T ) 1 - ( xx + zz );
        mat[ 1 ][ 2 ] = yz - wx;
        mat[ 1 ][ 3 ] = 0;

        mat[ 2 ][ 0 ] = xz - wy;
        mat[ 2 ][ 1 ] = yz + wx;
        mat[ 2 ][ 2 ] = ( T ) 1 - ( xx + yy );
        mat[ 3 ][ 3 ] = ( T ) 1;

        mat[ 3 ][ 0 ] = 0;
        mat[ 3 ][ 1 ] = 0;
        mat[ 3 ][ 2 ] = 0;
        mat[ 3 ][ 3 ] = ( T ) 1;
    }

    template<typename T>
    inline void Matrix4<T>::setTranslation( T x, T y, T z )
    {
        mat[ 0 ][ 3 ] = x;
        mat[ 1 ][ 3 ] = y;
        mat[ 2 ][ 3 ] = z;
    }


    template<>
    inline bool Matrix4<double>::isIdentity() const
    {
        return mat[ 0 ] == Vector4<double>( 1.0, 0.0, 0.0, 0.0 )
            && mat[ 1 ] == Vector4<double>( 0.0, 1.0, 0.0, 0.0 )
            && mat[ 2 ] == Vector4<double>( 0.0, 0.0, 1.0, 0.0 )
            && mat[ 3 ] == Vector4<double>( 0.0, 0.0, 0.0, 1.0 );
    }

    template<>
    inline bool Matrix4<float>::isIdentity() const
    {
        return mat[ 0 ] == Vector4<float>( 1.0f, 0.0f, 0.0f, 0.0f )
            && mat[ 1 ] == Vector4<float>( 0.0f, 1.0f, 0.0f, 0.0f )
            && mat[ 2 ] == Vector4<float>( 0.0f, 0.0f, 1.0f, 0.0f )
            && mat[ 3 ] == Vector4<float>( 0.0f, 0.0f, 0.0f, 1.0f );
    }

    template<>
    inline bool Matrix4<float>::isSymmetric() const
    {
        return Math::abs( mat[ 0 ][ 1 ] - mat[ 1 ][ 0 ] ) < Math::EPSILONF
            && Math::abs( mat[ 0 ][ 2 ] - mat[ 2 ][ 0 ] ) < Math::EPSILONF
            && Math::abs( mat[ 0 ][ 3 ] - mat[ 3 ][ 0 ] ) < Math::EPSILONF
            && Math::abs( mat[ 1 ][ 2 ] - mat[ 2 ][ 1 ] ) < Math::EPSILONF
            && Math::abs( mat[ 1 ][ 3 ] - mat[ 3 ][ 1 ] ) < Math::EPSILONF
            && Math::abs( mat[ 2 ][ 3 ] - mat[ 3 ][ 2 ] ) < Math::EPSILONF;

    }

    template<>
    inline bool Matrix4<float>::isDiagonal() const
    {
        return Math::abs( mat[ 0 ][ 1 ] ) < Math::EPSILONF
            && Math::abs( mat[ 0 ][ 2 ] ) < Math::EPSILONF
            && Math::abs( mat[ 0 ][ 3 ] ) < Math::EPSILONF
            && Math::abs( mat[ 1 ][ 0 ] ) < Math::EPSILONF
            && Math::abs( mat[ 1 ][ 2 ] ) < Math::EPSILONF
            && Math::abs( mat[ 1 ][ 3 ] ) < Math::EPSILONF
            && Math::abs( mat[ 2 ][ 0 ] ) < Math::EPSILONF
            && Math::abs( mat[ 2 ][ 1 ] ) < Math::EPSILONF
            && Math::abs( mat[ 2 ][ 3 ] ) < Math::EPSILONF
            && Math::abs( mat[ 3 ][ 0 ] ) < Math::EPSILONF
            && Math::abs( mat[ 3 ][ 1 ] ) < Math::EPSILONF
            && Math::abs( mat[ 3 ][ 2 ] ) < Math::EPSILONF;
    }

    template<>
    inline bool Matrix4<double>::isDiagonal() const
    {
        return Math::abs( mat[ 0 ][ 1 ] ) < Math::EPSILOND
            && Math::abs( mat[ 0 ][ 2 ] ) < Math::EPSILOND
            && Math::abs( mat[ 0 ][ 3 ] ) < Math::EPSILOND
            && Math::abs( mat[ 1 ][ 0 ] ) < Math::EPSILOND
            && Math::abs( mat[ 1 ][ 2 ] ) < Math::EPSILOND
            && Math::abs( mat[ 1 ][ 3 ] ) < Math::EPSILOND
            && Math::abs( mat[ 2 ][ 0 ] ) < Math::EPSILOND
            && Math::abs( mat[ 2 ][ 1 ] ) < Math::EPSILOND
            && Math::abs( mat[ 2 ][ 3 ] ) < Math::EPSILOND
            && Math::abs( mat[ 3 ][ 0 ] ) < Math::EPSILOND
            && Math::abs( mat[ 3 ][ 1 ] ) < Math::EPSILOND
            && Math::abs( mat[ 3 ][ 2 ] ) < Math::EPSILOND;
    }

    template<typename T>
    inline bool Matrix4<T>::isEqual(const Matrix4<T>& other, T epsilon) const
    {
        for( size_t i = 0; i < 4; i++ ){
            for( size_t k = 0; k < 4; k++ ){
                if( Math::abs( mat[ i ][ k ] - other[ i ][ k ] ) > epsilon )
                    return false;
            }
        }
        return true;
    }

    template<typename T>
    inline T Matrix4<T>::trace() const
    {
        return mat[ 0 ].x + mat[ 1 ].y + mat[ 2 ].z + mat[ 3 ].w;
    }

    template<typename T>
    inline T Matrix4<T>::determinant() const
    {
        T det2_23_01, det2_23_02, det2_23_03,
          det2_23_12, det2_23_13, det2_23_23;
        T det3_123_123, det3_123_023, det3_123_013, det3_123_012;

        det2_23_01 = mat[ 2 ][ 0 ] * mat[ 3 ][ 1 ] - mat[ 3 ][ 0 ] * mat[ 2 ][ 1 ];
        det2_23_02 = mat[ 2 ][ 0 ] * mat[ 3 ][ 2 ] - mat[ 3 ][ 0 ] * mat[ 2 ][ 2 ];
        det2_23_03 = mat[ 2 ][ 0 ] * mat[ 3 ][ 3 ] - mat[ 3 ][ 0 ] * mat[ 2 ][ 3 ];
        det2_23_12 = mat[ 2 ][ 1 ] * mat[ 3 ][ 2 ] - mat[ 3 ][ 1 ] * mat[ 2 ][ 2 ];
        det2_23_13 = mat[ 2 ][ 1 ] * mat[ 3 ][ 3 ] - mat[ 3 ][ 1 ] * mat[ 2 ][ 3 ];
        det2_23_23 = mat[ 2 ][ 2 ] * mat[ 3 ][ 3 ] - mat[ 3 ][ 2 ] * mat[ 2 ][ 3 ];

        det3_123_123  =  mat[ 1 ][ 1 ] * det2_23_23 - mat[ 1 ][ 2 ] * det2_23_13 + mat[ 1 ][ 3 ] * det2_23_12;
        det3_123_023  =  mat[ 1 ][ 0 ] * det2_23_23 - mat[ 1 ][ 2 ] * det2_23_03 + mat[ 1 ][ 3 ] * det2_23_02;
        det3_123_013  =  mat[ 1 ][ 0 ] * det2_23_13 - mat[ 1 ][ 1 ] * det2_23_03 + mat[ 1 ][ 3 ] * det2_23_01;
        det3_123_012  =  mat[ 1 ][ 0 ] * det2_23_12 - mat[ 1 ][ 1 ] * det2_23_02 + mat[ 1 ][ 2 ] * det2_23_01;

        return mat[ 0 ][ 0 ] * det3_123_123 - mat[ 0 ][ 1 ] * det3_123_023 + mat[ 0 ][ 2 ] * det3_123_013 - mat[ 0 ][ 3 ] * det3_123_012;
    }


    template<typename T>
    inline Matrix4<T> Matrix4<T>::transpose() const
    {
        return Matrix4<T>( mat[ 0 ][ 0 ], mat[ 1 ][ 0 ], mat[ 2 ][ 0 ], mat[ 3 ][ 0 ],
                           mat[ 0 ][ 1 ], mat[ 1 ][ 1 ], mat[ 2 ][ 1 ], mat[ 3 ][ 1 ],
                           mat[ 0 ][ 2 ], mat[ 1 ][ 2 ], mat[ 2 ][ 2 ], mat[ 3 ][ 2 ],
                           mat[ 0 ][ 3 ], mat[ 1 ][ 3 ], mat[ 2 ][ 3 ], mat[ 3 ][ 3 ] );
    }

    template<typename T>
    inline Matrix4<T>& Matrix4<T>::transposeSelf()
    {
        float tmp;
        tmp = mat[ 0 ][ 1 ];
        mat[ 0 ][ 1 ] = mat[ 1 ][ 0 ];
        mat[ 1 ][ 0 ] = tmp;

        tmp = mat[ 0 ][ 2 ];
        mat[ 0 ][ 2 ] = mat[ 2 ][ 0 ];
        mat[ 2 ][ 0 ] = tmp;

        tmp = mat[ 0 ][ 3 ];
        mat[ 0 ][ 3 ] = mat[ 3 ][ 0 ];
        mat[ 3 ][ 0 ] = tmp;

        tmp = mat[ 1 ][ 2 ];
        mat[ 1 ][ 2 ] = mat[ 2 ][ 1 ];
        mat[ 2 ][ 1 ] = tmp;

        tmp = mat[ 1 ][ 3 ];
        mat[ 1 ][ 3 ] = mat[ 3 ][ 1 ];
        mat[ 3 ][ 1 ] = tmp;

        tmp = mat[ 2 ][ 3 ];
        mat[ 2 ][ 3 ] = mat[ 3 ][ 2 ];
        mat[ 3 ][ 2 ] = tmp;

        return *this;
    }


    template<typename T>
    inline Matrix4<T> Matrix4<T>::inverse( void ) const
    {
        Matrix4<T> inv;

        inv = *this;
        inv.inverseSelf();
        return inv;
    }


    template<typename T>
    inline Matrix4<T> Matrix4<T>::pseudoInverse() const
    {
        Matrix4<T> U, E, V;

        svd( U, E, V );

        for( int i = 0; i < 4; i++ ) {
            if( Math::abs( E[ i ][ i ] ) > Math::epsilon<T>() )
                E[ i ][ i ] = ( T ) 1 / E[ i ][ i ];
        }

        return V * E * U.transpose();
    }

    template<typename T>
    inline Matrix3<T> Matrix4<T>::toMatrix3() const
    {
        return Matrix3<T>( mat[ 0 ][ 0 ], mat[ 0 ][ 1 ], mat[ 0 ][ 2 ],
                           mat[ 1 ][ 0 ], mat[ 1 ][ 1 ], mat[ 1 ][ 2 ],
                           mat[ 2 ][ 0 ], mat[ 2 ][ 1 ], mat[ 2 ][ 2 ] );
    }


    template<typename T>
    inline int	Matrix4<T>::dimension( void ) const
    {
        return 4;
    }

    template<typename T>
    inline const T* Matrix4<T>::ptr( void ) const
    {
        return mat[ 0 ].ptr();
    }

    template<typename T>
    inline T* Matrix4<T>::ptr( void )
    {
        return mat[ 0 ].ptr();
    }

    template<typename T>
    String Matrix4<T>::toString( void ) const
    {
        String s;
        s.sprintf( "%0.10f %0.10f %0.10f %0.10f\n"
                   "%0.10f %0.10f %0.10f %0.10f\n"
                   "%0.10f %0.10f %0.10f %0.10f\n"
                   "%0.10f %0.10f %0.10f %0.10f",
                   mat[ 0 ][ 0 ], mat[ 0 ][ 1 ], mat[ 0 ][ 2 ], mat[ 0 ][ 3 ],
                   mat[ 1 ][ 0 ], mat[ 1 ][ 1 ], mat[ 1 ][ 2 ], mat[ 1 ][ 3 ],
                   mat[ 2 ][ 0 ], mat[ 2 ][ 1 ], mat[ 2 ][ 2 ], mat[ 2 ][ 3 ],
                   mat[ 3 ][ 0 ], mat[ 3 ][ 1 ], mat[ 3 ][ 2 ], mat[ 3 ][ 3 ] );
        return s;
    }

    template<typename T>
    inline std::ostream& operator<<( std::ostream& out, const Matrix4<T>& m )
    {
        out << m[ 0 ] << std::endl;
        out << m[ 1 ] << std::endl;
        out << m[ 2 ] << std::endl;
        out << m[ 3 ];
        return out;
    }

}


#endif
