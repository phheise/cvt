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

#ifdef CVT_VECTOR_H

#include <cvt/util/DataIterator.h>

namespace cvt {

    template<typename T> class Vector4;

    /**
      \ingroup Math
    */
    template<typename T>
    class Vector3 {
        public:
        typedef     T TYPE;
        enum        { DIMENSION = 3 };


                    Vector3();
                    Vector3( T x, T y, T z );
                    Vector3( const Vector3<T>& vec );
                    Vector3( const Vector4<T>& vec );

        void		set( T x, T y, T z );
        void		setZero( void );

        T			operator[]( int index ) const;
        T&			operator[]( int index );
        Vector3<T>	operator-() const;
        T			operator*( const Vector3<T> &v ) const;
        Vector3<T>	operator*( const T c ) const;
        T			dot( const Vector3<T>& v ) const;
        Vector3<T>	cmul( const Vector3<T>& v ) const;
        Vector3<T>	cross( const Vector3<T> &v ) const;
        Vector3<T>& cross( const Vector3<T>& a, const Vector3<T>& b );
        Vector3<T>	operator/( const T c ) const;
        Vector3<T>	operator+( const Vector3<T> &v ) const;
        Vector3<T>	operator-( const Vector3<T> &v ) const;
        Vector3<T>&	operator+=( const Vector3<T> &v );
        Vector3<T>&	operator-=( const Vector3<T> &v );
        Vector3<T>&	operator/=( const Vector3<T> &v );
        Vector3<T>&	operator+=( const T c );
        Vector3<T>&	operator-=( const T c );
        Vector3<T>&	operator/=( const T c );
        Vector3<T>&	operator*=( const T c );

        template <typename T2>
        operator Vector3<T2>() const;

        bool		operator==( const Vector3<T> &v ) const;
        bool		operator!=( const Vector3<T> &v ) const;

        T			length( void ) const;
        T			lengthSqr( void ) const;
        T			normalize( void );		// returns length
        void		clamp( const Vector3<T>& min, const Vector3<T>& max );

        size_t		dimension( void ) const;

        bool        isEqual( const Vector3<T> & other, T epsilon ) const;

        const T*	ptr( void ) const;
        T*			ptr( void );

        void		mix( const Vector3<T> &v1, const Vector3<T> &v2, float alpha );

        String      toString( void ) const;
        static Vector3<T> fromString( const String & s );

        T x, y, z;
    };

    template<typename T>
    inline Vector3<T>::Vector3()
    {
    }

    template<typename T>
    inline Vector3<T>::Vector3( T x, T y, T z )
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    template<typename T>
    inline Vector3<T>::Vector3( const Vector3<T>& v )
    {
        x = v.x;
        y = v.y;
        z = v.z;
    }


    template<typename T>
    inline Vector3<T>::Vector3( const Vector4<T>& v )
    {
        T invw = 1.0 / v.w;
        x = invw * v.x;
        y = invw * v.y;
        z = invw * v.z;
    }

    template<typename T>
    inline void Vector3<T>::set( T x, T y, T z )
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    template<typename T>
    inline void Vector3<T>::setZero( )
    {
        x = 0;
        y = 0;
        z = 0;
    }

    template<typename T>
    inline T Vector3<T>::operator[]( int index ) const
    {
        return ( &x )[ index ];
    }

    template<typename T>
    inline T& Vector3<T>::operator[]( int index )
    {
        return ( &x )[ index ];
    }

    template<typename T>
    inline Vector3<T> Vector3<T>::operator-() const
    {
        return Vector3<T>( -x, -y, -z );
    }

    template<typename T>
    inline Vector3<T> operator*( float c, const Vector3<T> &v )
    {
        return Vector3<T>( c * v.x, c * v.y, c * v.z );
    }

    template<typename T>
    inline Vector3<T> Vector3<T>::operator*( const T c ) const
    {
        return Vector3<T>( x * c, y * c, z * c );
    }


    template<typename T>
    inline T Vector3<T>::dot( const Vector3<T>& v ) const
    {
        return v.x * x + v.y * y + v.z * z;
    }

    template<typename T>
    inline Vector3<T> Vector3<T>::cmul( const Vector3<T>& v ) const
    {
        return Vector3<T>( v.x * x, v.y * y, v.z * z );
    }

    template<typename T>
    inline Vector3<T> Vector3<T>::cross( const Vector3<T> &v ) const
    {
        return Vector3<T>( y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x );
    }

    template<typename T>
    inline Vector3<T>& Vector3<T>::cross( const Vector3<T>& a, const Vector3<T>& b )
    {
        x = a.y * b.z - a.z * b.y;
        y = a.z * b.x - a.x * b.z;
        z = a.x * b.y - a.y * b.x;
        return *this;
    }


    template<typename T>
    inline T Vector3<T>::operator*( const Vector3<T>& v ) const
    {
        return v.x * x + v.y * y + v.z * z;
    }

    template<typename T>
    inline Vector3<T> Vector3<T>::operator/( const T c ) const
    {
        T inv = ( ( T ) 1 ) / c;
        return Vector3<T>( x * inv, y * inv, z * inv );
    }

    template<typename T>
    inline Vector3<T> Vector3<T>::operator+( const Vector3<T> &v ) const
    {
        return Vector3<T>( x + v.x, y + v.y, z + v.z );
    }

    template<typename T>
    inline Vector3<T> Vector3<T>::operator-( const Vector3<T> &v ) const
    {
        return Vector3<T>( x - v.x, y - v.y, z - v.z );
    }

    template<typename T>
    inline Vector3<T>& Vector3<T>::operator+=( const Vector3<T> &v )
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    template<typename T>
    inline Vector3<T>& Vector3<T>::operator-=( const Vector3<T> &v )
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    template<typename T>
    inline Vector3<T>& Vector3<T>::operator/=( const Vector3<T> &v )
    {
        x /= v.x;
        y /= v.y;
        z /= v.z;
        return *this;
    }

    template<typename T>
    inline Vector3<T>& Vector3<T>::operator+=( const T c )
    {
        x += c;
        y += c;
        z += c;
        return *this;
    }

    template<typename T>
    inline Vector3<T>& Vector3<T>::operator-=( const T c )
    {
        x -= c;
        y -= c;
        z -= c;
        return *this;
    }


    template<typename T>
    inline Vector3<T>& Vector3<T>::operator/=( const T c )
    {
        x /= c;
        y /= c;
        z /= c;
        return *this;
    }

    template<typename T>
    inline Vector3<T>& Vector3<T>::operator*=( const T c )
    {
        x *= c;
        y *= c;
        z *= c;
        return *this;
    }

    template <typename T> template <typename T2>
    inline Vector3<T>::operator Vector3<T2>() const
    {
        return Vector3<T2>( ( T2 ) x, ( T2 ) y, ( T2 )z );
    }

    template<>
    inline bool Vector3<float>::operator==( const Vector3<float> &v ) const
    {
        return Math::abs( v.x - x ) < Math::EPSILONF
            && Math::abs( v.y - y ) < Math::EPSILONF
            && Math::abs( v.z - z ) < Math::EPSILONF;
    }

    template<>
    inline bool Vector3<float>::operator!=( const Vector3<float> &v ) const
    {
        return Math::abs( v.x - x ) > Math::EPSILONF
            || Math::abs( v.y - y ) > Math::EPSILONF
            || Math::abs( v.z - z ) > Math::EPSILONF;
    }

    template<>
    inline bool Vector3<double>::operator==( const Vector3<double> &v ) const
    {
        return Math::abs( v.x - x ) < Math::EPSILOND
            && Math::abs( v.y - y ) < Math::EPSILOND
            && Math::abs( v.z - z ) < Math::EPSILOND;
    }

    template<>
    inline bool Vector3<double>::operator!=( const Vector3<double> &v ) const
    {
        return Math::abs( v.x - x ) > Math::EPSILOND
            || Math::abs( v.y - y ) > Math::EPSILOND
            || Math::abs( v.z - z ) > Math::EPSILOND;
    }


    template<typename T>
    inline T Vector3<T>::length( void ) const
    {
        return Math::sqrt( x * x + y * y + z * z );
    }

    template<typename T>
    inline T Vector3<T>::lengthSqr( void ) const
    {
        return x * x + y * y + z * z;
    }

    template<typename T>
    T Vector3<T>::normalize( void )
    {
        T lenSqr, lenInv;

        lenSqr = x * x + y * y + z * z;
        lenInv = Math::invSqrt( lenSqr );
        x *= lenInv;
        y *= lenInv;
        z *= lenInv;

        //FIXME: return nan on for zero vector

        return lenSqr;// * lenInv;
    }

    template<typename T>
    void Vector3<T>::clamp( const Vector3<T> &min, const Vector3<T> &max )
    {
        x = Math::clamp( x, min.x, max.x );
        y = Math::clamp( y, min.y, max.y );
        z = Math::clamp( z, min.z, max.z );
    }

    template<typename T>
    size_t Vector3<T>::dimension( void ) const
    {
        return 3;
    }

    template <typename T>
    bool Vector3<T>::isEqual( const Vector3<T>& other, T epsilon ) const
    {
        return ( Math::abs( x - other.x ) <= epsilon ) &&
               ( Math::abs( y - other.y ) <= epsilon ) &&
               ( Math::abs( z - other.z ) <= epsilon );
    }

    template<typename T>
    const T* Vector3<T>::ptr( void ) const
    {
        return &x;
    }

    template<typename T>
    T* Vector3<T>::ptr( void )
    {
        return &x;
    }

    template<typename T>
    void Vector3<T>::mix( const Vector3<T> &v1, const Vector3<T> &v2, float alpha )
    {
        x = Math::mix( v1.x, v2.x, alpha );
        y = Math::mix( v1.y, v2.y, alpha );
        z = Math::mix( v1.z, v2.z, alpha );
    }

    template<typename T>
    String Vector3<T>::toString( void ) const
    {
        String s;
        s.sprintf( "%0.10f %0.10f %0.10f", x, y, z );

        return s;
    }

    template<typename T>
    Vector3<T> Vector3<T>::fromString( const String& s )
    {
        Vector3<T> m;

        DataIterator it( s );
        String token;
        String deliminators("\n\r\t ");
        for( size_t i = 0; i < 3; i++ ){
            if( !it.nextToken( token, deliminators ) )
                throw CVTException( "Could not create Matrix from String!" );
            m[ i ] = token.to<T>();
        }

        return m;
    }

    template<typename T>
    static inline std::ostream& operator<<( std::ostream& out, const Vector3<T> &v )
    {
        out << " | " << std::setprecision( 5 ) << std::setw( 12 ) << v.x << " " << std::setw( 12 ) << v.y << " " << std::setw( 12 ) << v.z << " | ";
        return out;
    }


}

#endif
