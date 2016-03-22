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

#include <cvt/math/Quaternion.h>

namespace cvt {
    template<typename T> class Matrix2;
    template<typename T> class Matrix3;
    template<typename T> class Matrix4;
    template<typename T> class Quaternion;

    template<typename T>
    std::ostream& operator<<( std::ostream& out, const Matrix3<T>& m );

    /**
      \ingroup Math
    */
    template<typename T>
    class Matrix3 {
        public:
        typedef             T TYPE;
        enum                { DIMENSION = 3 };

                            Matrix3<T>( void );
                            Matrix3<T>( const Matrix3<T>& mat3 );
        explicit			Matrix3<T>( const Vector3<T>& x, const Vector3<T>& y, const Vector3<T>& z );
        explicit			Matrix3<T>( const T a, const T b, const T c,
                                        const T d, const T e, const T f,
                                        const T g, const T h, const T i );
        explicit			Matrix3<T>( const T src[ 3 ][ 3 ] );
        explicit			Matrix3<T>( const Matrix2<T>& mat2 );
        explicit			Matrix3<T>( const Matrix4<T>& mat4 );

        const Vector3<T>&	operator[]( int index ) const;
        Vector3<T>&			operator[]( int index );
        const T&            operator()( int r, int c ) const;
        T&                  operator()( int r, int c );
        Matrix3<T>			operator-() const;
        Matrix3<T>			operator*( const T c ) const;
        Matrix3<T>			operator+( const T c ) const;
        Matrix3<T>			operator-( const T c ) const;
        Vector2<T>			operator*( const Vector2<T>& vec  ) const;
        Vector3<T>			operator*( const Vector3<T>& vec ) const;
        Matrix3<T>			operator*( const Matrix3<T>& m ) const;
        Matrix3<T>			operator+( const Matrix3<T>& m ) const;
        Matrix3<T>			operator-( const Matrix3<T>& m ) const;
        Matrix3<T>&			operator*=( const T c );
        Matrix3<T>&			operator+=( const T c );
        Matrix3<T>&			operator-=( const T c );
        Matrix3<T>&			operator*=( const Matrix3<T>& m );
        Matrix3<T>&			operator+=( const Matrix3<T>& m );
        Matrix3<T>&			operator-=( const Matrix3<T>& m );

        template <typename T2>
        operator Matrix3<T2>() const;

        bool				operator==( const Matrix3<T> &m ) const;
        bool				operator!=( const Matrix3<T> &m ) const;

        Vector3<T>			row( size_t r ) const;
        Vector3<T>			col( size_t c ) const;

        void				setZero( void );
        void				setIdentity( void );
        bool				isIdentity( ) const;
        bool				isSymmetric( ) const;
        bool				isDiagonal( ) const;
        bool                isEqual( const Matrix3<T> & other, T epsilon ) const;

        void				setDiagonal( const Vector3<T>& diag );
        void				setRotationX( T rad );
        void				setRotationY( T rad );
        void				setRotationZ( T rad );
        void                setRotationXYZ( T angleX, T angleY, T angleZ );
        void				setRotation( const Vector3<T>& axis, T rad );

        void				setAffine( T theta, T phi, T sx, T sy, T tx, T ty );
        void				setHomography( T theta, T phi, T sx, T sy, T tx, T ty, T v0, T v1 );
        void				setProjectedRotation( T radx, T rady, T radz, T fx = 1024, T fy = 1024, T d = 1024 );
        void				setSkewSymmetric( const Vector3<T>& t );


        T					trace( void ) const;
        T					determinant( void ) const;
        Matrix3<T>			transpose( void ) const;
        Matrix3<T>&			transposeSelf( void );
        Matrix3<T>			inverse( void ) const;
        bool				inverseSelf( void );
        void				svd( Matrix3<T>& u, Matrix3<T>& d, Matrix3<T>& vt ) const;
        Matrix3<T>			pseudoInverse() const;

        void				toAxisAngle( Vector3<T>& axis, T& angle ) const;

        Matrix4<T>			toMatrix4( void ) const;
        Matrix2<T>			toMatrix2( void ) const;

        int					dimension( void ) const;
        const T*			ptr( void ) const;
        T*					ptr( void );

        String              toString( void ) const;
        static Matrix3<T>   fromString( const String & s );

        friend std::ostream& operator<< <>( std::ostream& out, const Matrix3<T>& m );

        private:
        Vector3<T>			mat[ 3 ];
    };

    template<typename T>
    inline Matrix3<T>::Matrix3()
    {
    }

    template<typename T>
    inline Matrix3<T>::Matrix3( const Vector3<T>& x, const Vector3<T>& y, const Vector3<T>& z )
    {
        mat[ 0 ] = x;
        mat[ 1 ] = y;
        mat[ 2 ] = z;
    }

    template<typename T>
    inline Matrix3<T>::Matrix3( const T a, const T b, const T c,
                                const T d, const T e, const T f,
                                const T g, const T h, const T i )
    {
        mat[ 0 ].x = a;
        mat[ 0 ].y = b;
        mat[ 0 ].z = c;

        mat[ 1 ].x = d;
        mat[ 1 ].y = e;
        mat[ 1 ].z = f;

        mat[ 2 ].x = g;
        mat[ 2 ].y = h;
        mat[ 2 ].z = i;
    }

    template<typename T>
    inline Matrix3<T>::Matrix3( const Matrix2<T>& mat2 )
    {
        mat[ 0 ].x = mat2[ 0 ].x;
        mat[ 0 ].y = mat2[ 0 ].y;
        mat[ 0 ].z = 0;

        mat[ 1 ].x = mat2[ 1 ].x;
        mat[ 1 ].y = mat2[ 1 ].y;
        mat[ 1 ].z = 0;

        mat[ 2 ].x = 0;
        mat[ 2 ].y = 0;
        mat[ 2 ].z = 1;
    }

    template<typename T>
    inline Matrix3<T>::Matrix3( const Matrix3<T>& mat3 )
    {
        memcpy( ( uint8_t* ) this->ptr(), ( const uint8_t* ) mat3.ptr(), sizeof( T ) * 9 );
    }

    template<typename T>
    inline Matrix3<T>::Matrix3( const Matrix4<T>& mat4 )
    {
        mat[ 0 ].x = mat4[ 0 ].x;
        mat[ 0 ].y = mat4[ 0 ].y;
        mat[ 0 ].z = mat4[ 0 ].z;

        mat[ 1 ].x = mat4[ 1 ].x;
        mat[ 1 ].y = mat4[ 1 ].y;
        mat[ 1 ].z = mat4[ 1 ].z;

        mat[ 2 ].x = mat4[ 2 ].x;
        mat[ 2 ].y = mat4[ 2 ].y;
        mat[ 2 ].z = mat4[ 2 ].z;
    }

    template<typename T>
    inline Matrix3<T>::Matrix3( const T src[ 3 ][ 3 ] )
    {
        memcpy( this->ptr(), src, sizeof( T ) * 9 );
    }

    template<typename T>
    inline const Vector3<T>& Matrix3<T>::operator[]( int index ) const
    {
        return mat[ index ];
    }

    template<typename T>
    inline Vector3<T>& Matrix3<T>::operator[]( int index )
    {
        return mat[ index ];
    }

    template<typename T>
    inline const T& Matrix3<T>::operator()( int r, int c ) const
    {
        return mat[ r ][ c ];
    }

    template<typename T>
    inline T& Matrix3<T>::operator()( int r, int c )
    {
        return mat[ r ][ c ];
    }

    template<typename T>
    inline Matrix3<T> Matrix3<T>::operator-( ) const
    {
        return Matrix3<T>( -mat[ 0 ], -mat[ 1 ], -mat[ 2 ] );
    }

    template<typename T>
    inline Matrix3<T> Matrix3<T>::operator*( const T c  ) const
    {
        return Matrix3<T>( mat[ 0 ] * c, mat[ 1 ] * c, mat[ 2 ] * c );
    }

    template<typename T>
    inline Matrix3<T> Matrix3<T>::operator+( const T c  ) const
    {
        return Matrix3<T>( mat[ 0 ] + c, mat[ 1 ] + c, mat[ 2 ] + c );
    }

    template<typename T>
    inline Matrix3<T> Matrix3<T>::operator-( const T c  ) const
    {
        return Matrix3<T>( mat[ 0 ] - c, mat[ 1 ] - c, mat[ 2 ] - c );
    }

    template<typename T>
    inline Vector3<T> Matrix3<T>::operator*( const Vector3<T>& vec  ) const
    {
        return Vector3<T>( mat[ 0 ].x * vec.x + mat[ 0 ].y * vec.y + mat[ 0 ].z * vec.z,
                           mat[ 1 ].x * vec.x + mat[ 1 ].y * vec.y + mat[ 1 ].z * vec.z,
                           mat[ 2 ].x * vec.x + mat[ 2 ].y * vec.y + mat[ 2 ].z * vec.z );
    }

    template<typename T>
    inline Vector2<T> Matrix3<T>::operator*( const Vector2<T>& vec  ) const
    {
        Vector2<T> ret( mat[ 0 ].x * vec.x + mat[ 0 ].y * vec.y + mat[ 0 ].z,
                        mat[ 1 ].x * vec.x + mat[ 1 ].y * vec.y + mat[ 1 ].z );
        ret /= mat[ 2 ].x * vec.x + mat[ 2 ].y * vec.y + mat[ 2 ].z;
        return ret;
    }


    template<typename T>
    inline Matrix3<T> Matrix3<T>::operator*( const Matrix3<T>& m  ) const
    {
        return Matrix3<T>( mat[ 0 ][ 0 ] * m[ 0 ][ 0 ] + mat[ 0 ][ 1 ] * m[ 1 ][ 0 ] + mat[ 0 ][ 2 ] * m[ 2 ][ 0 ],
                           mat[ 0 ][ 0 ] * m[ 0 ][ 1 ] + mat[ 0 ][ 1 ] * m[ 1 ][ 1 ] + mat[ 0 ][ 2 ] * m[ 2 ][ 1 ],
                           mat[ 0 ][ 0 ] * m[ 0 ][ 2 ] + mat[ 0 ][ 1 ] * m[ 1 ][ 2 ] + mat[ 0 ][ 2 ] * m[ 2 ][ 2 ],
                           mat[ 1 ][ 0 ] * m[ 0 ][ 0 ] + mat[ 1 ][ 1 ] * m[ 1 ][ 0 ] + mat[ 1 ][ 2 ] * m[ 2 ][ 0 ],
                           mat[ 1 ][ 0 ] * m[ 0 ][ 1 ] + mat[ 1 ][ 1 ] * m[ 1 ][ 1 ] + mat[ 1 ][ 2 ] * m[ 2 ][ 1 ],
                           mat[ 1 ][ 0 ] * m[ 0 ][ 2 ] + mat[ 1 ][ 1 ] * m[ 1 ][ 2 ] + mat[ 1 ][ 2 ] * m[ 2 ][ 2 ],
                           mat[ 2 ][ 0 ] * m[ 0 ][ 0 ] + mat[ 2 ][ 1 ] * m[ 1 ][ 0 ] + mat[ 2 ][ 2 ] * m[ 2 ][ 0 ],
                           mat[ 2 ][ 0 ] * m[ 0 ][ 1 ] + mat[ 2 ][ 1 ] * m[ 1 ][ 1 ] + mat[ 2 ][ 2 ] * m[ 2 ][ 1 ],
                           mat[ 2 ][ 0 ] * m[ 0 ][ 2 ] + mat[ 2 ][ 1 ] * m[ 1 ][ 2 ] + mat[ 2 ][ 2 ] * m[ 2 ][ 2 ] );
    }


    template<typename T>
    inline Matrix3<T> Matrix3<T>::operator+( const Matrix3<T>& m  ) const
    {
        return Matrix3<T>( mat[ 0 ] + m[ 0 ], mat[ 1 ] + m[ 1 ], mat[ 2 ] + m[ 2 ] );
    }

    template<typename T>
    inline Matrix3<T> Matrix3<T>::operator-( const Matrix3<T>& m  ) const
    {
        return Matrix3<T>( mat[ 0 ] - m[ 0 ], mat[ 1 ] - m[ 1 ], mat[ 2 ] - m[ 2 ] );
    }

    template<typename T>
    inline Matrix3<T>& Matrix3<T>::operator*=( const T c  )
    {
        mat[ 0 ] *= c;
        mat[ 1 ] *= c;
        mat[ 2 ] *= c;
        return *this;
    }

    template<typename T>
    inline Matrix3<T>& Matrix3<T>::operator+=( const T c  )
    {
        mat[ 0 ] += c;
        mat[ 1 ] += c;
        mat[ 2 ] += c;
        return *this;
    }

    template<typename T>
    inline Matrix3<T>& Matrix3<T>::operator-=( const T c  )
    {
        mat[ 0 ] -= c;
        mat[ 1 ] -= c;
        mat[ 2 ] -= c;
        return *this;
    }

    template<typename T>
    inline Matrix3<T>& Matrix3<T>::operator+=( const Matrix3<T>& m  )
    {
        mat[ 0 ] += m[ 0 ];
        mat[ 1 ] += m[ 1 ];
        mat[ 2 ] += m[ 2 ];
        return *this;
    }

    template<typename T>
    inline Matrix3<T>& Matrix3<T>::operator-=( const Matrix3<T>& m  )
    {
        mat[ 0 ] -= m[ 0 ];
        mat[ 1 ] -= m[ 1 ];
        mat[ 2 ] -= m[ 2 ];
        return *this;
    }

    template<typename T>
    inline Matrix3<T>&  Matrix3<T>::operator*=( const Matrix3<T>& m )
    {
        *this = (*this) * m;
        return *this;
    }

    template<typename T>
    inline bool Matrix3<T>::operator==( const Matrix3<T> &m ) const
    {
        return mat[ 0 ] == m[ 0 ] && mat[ 1 ] == m[ 1 ] && mat[ 2 ] == m[ 2 ];
    }

    template<typename T>
    inline bool Matrix3<T>::operator!=( const Matrix3<T> &m ) const
    {
        return mat[ 0 ] != m[ 0 ] || mat[ 1 ] != m[ 1 ] || mat[ 2 ] != m[ 2 ];
    }

    template <typename T> template <typename T2>
    inline Matrix3<T>::operator Matrix3<T2>() const
    {
        return Matrix3<T2>( ( T2 ) mat[ 0 ][ 0 ], ( T2 ) mat[ 0 ][ 1 ], ( T2 ) mat[ 0 ][ 2 ],
                            ( T2 ) mat[ 1 ][ 0 ], ( T2 ) mat[ 1 ][ 1 ], ( T2 ) mat[ 1 ][ 2 ],
                            ( T2 ) mat[ 2 ][ 0 ], ( T2 ) mat[ 2 ][ 1 ], ( T2 ) mat[ 2 ][ 2 ] );
    }

    template<typename T>
    inline Vector3<T> Matrix3<T>::row( size_t r ) const
    {
        return mat[ r ];
    }

    template<typename T>
    inline Vector3<T> Matrix3<T>::col( size_t c ) const
    {
        return Vector3<T>( mat[ 0 ][ c ], mat[ 1 ][ c ], mat[ 2 ][ c ] );
    }

    template<typename T>
    inline void Matrix3<T>::setZero()
    {
        mat[ 0 ].setZero();
        mat[ 1 ].setZero();
        mat[ 2 ].setZero();
    }

    template<typename T>
    inline void Matrix3<T>::setIdentity()
    {
        mat[ 0 ].x = 1;
        mat[ 0 ].y = 0;
        mat[ 0 ].z = 0;

        mat[ 1 ].x = 0;
        mat[ 1 ].y = 1;
        mat[ 1 ].z = 0;

        mat[ 2 ].x = 0;
        mat[ 2 ].y = 0;
        mat[ 2 ].z = 1;
    }

    template<typename T>
    inline void Matrix3<T>::setDiagonal( const Vector3<T>& diag )
    {
        mat[ 0 ].x = diag.x;
        mat[ 0 ].y = 0;
        mat[ 0 ].z = 0;

        mat[ 1 ].x = 0;
        mat[ 1 ].y = diag.y;
        mat[ 1 ].z = 0;

        mat[ 2 ].x = 0;
        mat[ 2 ].y = 0;
        mat[ 2 ].z = diag.z;
    }

    template<typename T>
    inline void Matrix3<T>::setRotationX( T rad )
    {
        T s = Math::sin( rad );
        T c = Math::cos( rad );

        mat[ 0 ].x = 1;
        mat[ 0 ].y = 0;
        mat[ 0 ].z = 0;

        mat[ 1 ].x = 0;
        mat[ 1 ].y = c;
        mat[ 1 ].z = -s;

        mat[ 2 ].x = 0;
        mat[ 2 ].y = s;
        mat[ 2 ].z = c;
    }

    template<typename T>
    inline void Matrix3<T>::setRotationY( T rad )
    {
        T s = Math::sin( rad );
        T c = Math::cos( rad );

        mat[ 0 ].x = c;
        mat[ 0 ].y = 0;
        mat[ 0 ].z = s;

        mat[ 1 ].x = 0;
        mat[ 1 ].y = 1;
        mat[ 1 ].z = 0;

        mat[ 2 ].x = -s;
        mat[ 2 ].y = 0;
        mat[ 2 ].z = c;
    }

    template<typename T>
    inline void Matrix3<T>::setRotationZ( T rad )
    {
        T s = Math::sin( rad );
        T c = Math::cos( rad );

        mat[ 0 ].x = c;
        mat[ 0 ].y = -s;
        mat[ 0 ].z = 0;

        mat[ 1 ].x = s;
        mat[ 1 ].y = c;
        mat[ 1 ].z = 0;

        mat[ 2 ].x = 0;
        mat[ 2 ].y = 0;
        mat[ 2 ].z = 1;
    }

    template<typename T>
    inline void Matrix3<T>::setRotationXYZ( T angleX, T angleY, T angleZ )
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

        mat[ 1 ][ 0 ] = cx * sz + cz * sx * sy;
        mat[ 1 ][ 1 ] = cx * cz - sx * sy * sz;
        mat[ 1 ][ 2 ] =               -cy * sx;

        mat[ 2 ][ 0 ] =       sx * sz - cx * cz * sy;
        mat[ 2 ][ 1 ] =  cx * sy * sz +      cz * sx;
        mat[ 2 ][ 2 ] =                      cx * cy;
    }


    template<typename T>
    inline void	Matrix3<T>::setRotation( const Vector3<T>& _axis, T rad )
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

        mat[ 1 ][ 0 ] = xy + wz;
        mat[ 1 ][ 1 ] = ( T ) 1 - ( xx + zz );
        mat[ 1 ][ 2 ] = yz - wx;

        mat[ 2 ][ 0 ] = xz - wy;
        mat[ 2 ][ 1 ] = yz + wx;
        mat[ 2 ][ 2 ] = ( T ) 1 - ( xx + yy );
    }

    template<typename T>
    inline void Matrix3<T>::setAffine( T theta, T phi, T sx, T sy, T tx, T ty )
    {
        Matrix3<T> tmp, tmpInv, s;
        tmp.setRotationZ( phi );

        tmpInv = tmp.transpose();

        // scale
        s.setZero();
        s[ 0 ][ 0 ] = sx;
        s[ 1 ][ 1 ] = sy;
        s[ 2 ][ 2 ] =  1;

        tmp = s * tmp;

        // rotate back
        tmp = tmpInv * tmp;

        // rotate by theta
        setRotationZ( theta );

        *this *= tmp;

        mat[ 0 ][ 2 ] = tx;
        mat[ 1 ][ 2 ] = ty;
    }

    template<typename T>
    inline void Matrix3<T>::setHomography( T theta, T phi, T sx, T sy, T tx, T ty, T v0, T v1 )
    {
        setAffine( theta, phi, sx, sy, tx, ty );
        mat[ 2 ].x = v0;
        mat[ 2 ].y = v1;
    }

    template<typename T>
    inline void	Matrix3<T>::setProjectedRotation( T radx, T rady, T radz, T fx, T fy, T d )
    {
        T sx, cx, sy, cy, sz, cz;

        sx = Math::sin( radx );
        cx = Math::cos( radx );
        sy = Math::sin( rady );
        cy = Math::cos( rady );
        sz = Math::sin( radz );
        cz = Math::cos( radz );

        mat[ 0 ][ 0 ] =  cy * cz * fx;
        mat[ 0 ][ 1 ] = -cy * fx * sz;
        mat[ 0 ][ 2 ] =  0;

        mat[ 1 ][ 0 ] =  fx * ( cx * sz + cz * sx * sy );
        mat[ 1 ][ 1 ] =  fy * ( cx * cz - sx * sy * sz );
        mat[ 1 ][ 2 ] =  0;

        mat[ 2 ][ 0 ] =  sx * sz - cx * cz * sy;
        mat[ 2 ][ 1 ] =  cx * sy * sz + cz * sx;
        mat[ 2 ][ 2 ] =  d;
    }

    template<typename T>
    inline void Matrix3<T>::setSkewSymmetric( const Vector3<T>& t )
    {
        mat[ 0 ][ 0 ] =    0;	mat[ 0 ][ 1 ] = -t.z;	mat[ 0 ][ 2 ] =  t.y;
        mat[ 1 ][ 0 ] =  t.z;	mat[ 1 ][ 1 ] =    0;   mat[ 1 ][ 2 ] = -t.x;
        mat[ 2 ][ 0 ] = -t.y;	mat[ 2 ][ 1 ] =  t.x;	mat[ 2 ][ 2 ] =    0;
    }

    template<>
    inline bool Matrix3<double>::isIdentity() const
    {
        return mat[ 0 ] == Vector3<double>( 1.0, 0.0, 0.0 )
            && mat[ 1 ] == Vector3<double>( 0.0, 1.0, 0.0 )
            && mat[ 2 ] == Vector3<double>( 0.0, 0.0, 1.0 );
    }

    template<>
    inline bool Matrix3<float>::isIdentity() const
    {
        return mat[ 0 ] == Vector3<float>( 1.0f, 0.0f, 0.0f )
            && mat[ 1 ] == Vector3<float>( 0.0f, 1.0f, 0.0f )
            && mat[ 2 ] == Vector3<float>( 0.0f, 0.0f, 1.0f );
    }

    template<>
    inline bool Matrix3<float>::isSymmetric() const
    {
        return Math::abs( mat[ 0 ][ 1 ] - mat[ 1 ][ 0 ] ) < Math::EPSILONF
            && Math::abs( mat[ 0 ][ 2 ] - mat[ 2 ][ 0 ] ) < Math::EPSILONF
            && Math::abs( mat[ 1 ][ 2 ] - mat[ 2 ][ 1 ] ) < Math::EPSILONF;
    }

    template<>
    inline bool Matrix3<double>::isSymmetric() const
    {
        return Math::abs( mat[ 0 ][ 1 ] - mat[ 1 ][ 0 ] ) < Math::EPSILOND
            && Math::abs( mat[ 0 ][ 2 ] - mat[ 2 ][ 0 ] ) < Math::EPSILOND
            && Math::abs( mat[ 1 ][ 2 ] - mat[ 2 ][ 1 ] ) < Math::EPSILOND;
    }


    template<>
    inline bool Matrix3<float>::isDiagonal() const
    {
        return Math::abs( mat[ 0 ][ 1 ] ) < Math::EPSILONF
            && Math::abs( mat[ 0 ][ 2 ] ) < Math::EPSILONF
            && Math::abs( mat[ 1 ][ 0 ] ) < Math::EPSILONF
            && Math::abs( mat[ 1 ][ 2 ] ) < Math::EPSILONF
            && Math::abs( mat[ 2 ][ 0 ] ) < Math::EPSILONF
            && Math::abs( mat[ 2 ][ 1 ] ) < Math::EPSILONF;
    }

    template<>
    inline bool Matrix3<double>::isDiagonal() const
    {
        return Math::abs( mat[ 0 ][ 1 ] ) < Math::EPSILOND
            && Math::abs( mat[ 0 ][ 2 ] ) < Math::EPSILOND
            && Math::abs( mat[ 1 ][ 0 ] ) < Math::EPSILOND
            && Math::abs( mat[ 1 ][ 2 ] ) < Math::EPSILOND
            && Math::abs( mat[ 2 ][ 0 ] ) < Math::EPSILOND
            && Math::abs( mat[ 2 ][ 1 ] ) < Math::EPSILOND;
    }

    template<typename T>
    inline bool Matrix3<T>::isEqual( const Matrix3<T>& other, T epsilon ) const
    {
        for( size_t i = 0; i < 3; i++ ){
            for( size_t k = 0; k < 3; k++ ){
                if( Math::abs( mat[ i ][ k ] - other[ i ][ k ] ) > epsilon )
                    return false;
            }
        }
        return true;
    }

    template<typename T>
    inline T Matrix3<T>::trace() const
    {
        return mat[ 0 ].x + mat[ 1 ].y + mat[ 2 ].z;
    }

    template<typename T>
    inline T Matrix3<T>::determinant() const
    {
        T cofactor[ 3 ];

        cofactor[ 0 ] = mat[ 1 ][ 1 ] * mat[ 2 ][ 2 ] - mat[ 2 ][ 1 ] * mat[ 1 ][ 2 ];
        cofactor[ 1 ] = mat[ 2 ][ 0 ] * mat[ 1 ][ 2 ] - mat[ 1 ][ 0 ] * mat[ 2 ][ 2 ];
        cofactor[ 2 ] = mat[ 1 ][ 0 ] * mat[ 2 ][ 1 ] - mat[ 2 ][ 0 ] * mat[ 1 ][ 1 ];

        return mat[ 0 ][ 0 ] * cofactor[ 0 ] + mat[ 0 ][ 1 ] * cofactor[ 1 ] + mat[ 0 ][ 2 ] * cofactor[ 2 ];
    }


    template<typename T>
    inline Matrix3<T> Matrix3<T>::transpose() const
    {
        return Matrix3<T>( mat[ 0 ][ 0 ], mat[ 1 ][ 0 ], mat[ 2 ][ 0 ],
                           mat[ 0 ][ 1 ], mat[ 1 ][ 1 ], mat[ 2 ][ 1 ],
                           mat[ 0 ][ 2 ], mat[ 1 ][ 2 ], mat[ 2 ][ 2 ] );
    }

    template<typename T>
    inline Matrix3<T>& Matrix3<T>::transposeSelf()
    {
        float tmp;
        tmp = mat[ 0 ][ 1 ];
        mat[ 0 ][ 1 ] = mat[ 1 ][ 0 ];
        mat[ 1 ][ 0 ] = tmp;

        tmp = mat[ 0 ][ 2 ];
        mat[ 0 ][ 2 ] = mat[ 2 ][ 0 ];
        mat[ 2 ][ 0 ] = tmp;

        tmp = mat[ 1 ][ 2 ];
        mat[ 1 ][ 2 ] = mat[ 2 ][ 1 ];
        mat[ 2 ][ 1 ] = tmp;

        return *this;
    }

    template<typename T>
    inline Matrix3<T> Matrix3<T>::pseudoInverse() const
    {
        Matrix3<T> U, E, V;

        svd( U, E, V );

        for( int i = 0; i < 3; i++ ) {
            if( Math::abs( E[ i ][ i ] ) > Math::epsilon<T>() )
                E[ i ][ i ] = ( T ) 1 / E[ i ][ i ];
        }

        return V * E * U.transpose();
    }

    template<typename T>
    inline Matrix2<T> Matrix3<T>::toMatrix2() const
    {
        return Matrix2<T>( mat[ 0 ][ 0 ], mat[ 0 ][ 1 ],
                           mat[ 1 ][ 0 ], mat[ 1 ][ 1 ] );
    }

    template<typename T>
    inline void Matrix3<T>::toAxisAngle( Vector3<T>& axis, T& angle ) const
    {
        Quaternion<T> q( *this );
        q.toAxisAngle( axis, angle );
        /*angle = Math::acos( ( trace() -  ( T ) 1 ) / ( T ) 2 );
        axis.x = mat[ 2 ][ 1 ] - mat[ 1 ][ 2 ];
        axis.y = mat[ 0 ][ 2 ] - mat[ 2 ][ 0 ];
        axis.z = mat[ 1 ][ 0 ] - mat[ 0 ][ 1 ];
        axis /= Math::sin( angle ) * ( T ) 2;
        axis.normalize();*/
    }

    template<typename T>
    inline Matrix4<T> Matrix3<T>::toMatrix4() const
    {
        return Matrix4<T>( *this );
    }

    template<typename T>
    inline Matrix3<T> Matrix3<T>::inverse( void ) const
    {
        Matrix3<T> inv;

        inv = *this;
        inv.inverseSelf();
        return inv;
    }

    template<typename T>
    int	Matrix3<T>::dimension( void ) const
    {
        return 3;
    }

    template<typename T>
    const T* Matrix3<T>::ptr( void ) const
    {
        return mat[ 0 ].ptr();
    }

    template<typename T>
    T* Matrix3<T>::ptr( void )
    {
        return mat[ 0 ].ptr();
    }

    template<typename T>
    String Matrix3<T>::toString( void ) const
    {
        String s;
        s.sprintf( "%0.10f %0.10f %0.10f\n"
                   "%0.10f %0.10f %0.10f\n"
                   "%0.10f %0.10f %0.10f",
                   mat[ 0 ][ 0 ], mat[ 0 ][ 1 ], mat[ 0 ][ 2 ],
                   mat[ 1 ][ 0 ], mat[ 1 ][ 1 ], mat[ 1 ][ 2 ],
                   mat[ 2 ][ 0 ], mat[ 2 ][ 1 ], mat[ 2 ][ 2 ] );
        return s;
    }

    template<typename T>
    inline std::ostream& operator<<( std::ostream& out, const Matrix3<T>& m )
    {
        out << m[ 0 ] << std::endl;
        out << m[ 1 ] << std::endl;
        out << m[ 2 ];
        return out;
    }

}


#endif
