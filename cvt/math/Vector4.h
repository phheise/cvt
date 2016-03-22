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

    /**
      \ingroup Math
    */
    template<typename T>
    class Vector4 {
        public:
        typedef     T TYPE;
        enum        { DIMENSION = 4 };

                    Vector4();
                    Vector4( T x, T y, T z, T w );
                    Vector4( const Vector4<T>& vec );

        void		set( T x, T y, T z, T w );
        void		setZero( void );

        T			operator[]( int index ) const;
        T&			operator[]( int index );
        Vector4<T>	operator-() const;
        T			operator*( const Vector4<T> &v ) const;
        Vector4<T>	operator*( const T c ) const;
        Vector4<T>	cmul( const Vector4<T>& v ) const;
        Vector4<T>	operator/( const T c ) const;
        Vector4<T>	operator+( const Vector4<T> &v ) const;
        Vector4<T>	operator-( const Vector4<T> &v ) const;
        Vector4<T>&	operator+=( const Vector4<T> &v );
        Vector4<T>&	operator-=( const Vector4<T> &v );
        Vector4<T>&	operator/=( const Vector4<T> &v );
        Vector4<T>&	operator+=( const T c );
        Vector4<T>&	operator-=( const T c );
        Vector4<T>&	operator/=( const T c );
        Vector4<T>&	operator*=( const T c );

        template <typename T2>
        operator Vector4<T2>() const;

        bool		operator==( const Vector4<T> &v ) const;
        bool		operator!=( const Vector4<T> &v ) const;

        T			length( void ) const;
        T			lengthSqr( void ) const;
        T			normalize( void );		// returns length
        void		clamp( const Vector4<T> &min, const Vector4<T> &max );

        size_t		dimension( void ) const;

        bool        isEqual( const Vector4<T> & other, T epsilon ) const;

        const T*	ptr( void ) const;
        T*			ptr( void );

        String      toString( void ) const;

        static Vector4<T> fromString( const String & s );

        void		mix( const Vector4<T> &v1, const Vector4<T> &v2, float alpha );

        T x, y, z, w;
    };

    template<typename T>
    inline Vector4<T>::Vector4()
    {
    }

    template<typename T>
    inline Vector4<T>::Vector4( T x, T y, T z, T w )
    {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }

    template<typename T>
    inline Vector4<T>::Vector4( const Vector4<T>& v )
    {
        x = v.x;
        y = v.y;
        z = v.z;
        w = v.w;
    }

    template<typename T>
    inline void Vector4<T>::set( T x, T y, T z, T w )
    {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }

    template<typename T>
    inline void Vector4<T>::setZero( )
    {
        x = 0;
        y = 0;
        z = 0;
        w = 0;
    }

    template<typename T>
    inline T Vector4<T>::operator[]( int index ) const
    {
        return ( &x )[ index ];
    }

    template<typename T>
    inline T& Vector4<T>::operator[]( int index )
    {
        return ( &x )[ index ];
    }

    template<typename T>
    inline Vector4<T> Vector4<T>::operator-() const
    {
        return Vector4<T>( -x, -y, -z, -w );
    }

    template<typename T>
    inline Vector4<T> operator*( float c, const Vector4<T> &v )
    {
        return Vector4<T>( c * v.x, c * v.y, c * v.z, c * v.w );
    }

    template<typename T>
    inline Vector4<T> Vector4<T>::operator*( const T c ) const
    {
        return Vector4<T>( x * c, y * c, z * c, w * c );
    }

    template<typename T>
    inline Vector4<T> Vector4<T>::cmul( const Vector4<T>& v ) const
    {
        return Vector4<T>( v.x * x, v.y * y, v.z * z, v.w * w );
    }

    template<typename T>
    inline T Vector4<T>::operator*( const Vector4<T>& v ) const
    {
        return v.x * x + v.y * y + v.z * z + v.w * w;
    }

    template<typename T>
    inline Vector4<T> Vector4<T>::operator/( const T c ) const
    {
        T inv = ( ( T ) 1.0 ) / c;
        return Vector4<T>( x * inv, y * inv, z * inv, w * inv );
    }

    template<>
    inline Vector4<float> Vector4<float>::operator/( const float c ) const
    {
        float inv = 1.0f / c;
        return Vector4<float>( x * inv, y * inv, z * inv, w * inv );
    }

    template<>
    inline Vector4<double> Vector4<double>::operator/( const double c ) const
    {
        double inv = 1.0 / c;
        return Vector4<double>( x * inv, y * inv, z * inv, w * inv );
    }

    template<typename T>
    inline Vector4<T> Vector4<T>::operator+( const Vector4<T> &v ) const
    {
        return Vector4<T>( x + v.x, y + v.y, z + v.z, w + v.w );
    }

    template<typename T>
    inline Vector4<T> Vector4<T>::operator-( const Vector4<T> &v ) const
    {
        return Vector4<T>( x - v.x, y - v.y, z - v.z, w - v.w );
    }

    template<typename T>
    inline Vector4<T>& Vector4<T>::operator+=( const Vector4<T> &v )
    {
        x += v.x;
        y += v.y;
        z += v.z;
        w += v.w;
        return *this;
    }

    template<typename T>
    inline Vector4<T>& Vector4<T>::operator-=( const Vector4<T> &v )
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        w -= v.w;
        return *this;
    }

    template<typename T>
    inline Vector4<T>& Vector4<T>::operator/=( const Vector4<T> &v )
    {
        x /= v.x;
        y /= v.y;
        z /= v.z;
        w /= v.w;
        return *this;
    }

    template<typename T>
    inline Vector4<T>& Vector4<T>::operator+=( const T c )
    {
        x += c;
        y += c;
        z += c;
        w += c;
        return *this;
    }

    template<typename T>
    inline Vector4<T>& Vector4<T>::operator-=( const T c )
    {
        x -= c;
        y -= c;
        z -= c;
        w -= c;
        return *this;
    }


    template<typename T>
    inline Vector4<T>& Vector4<T>::operator/=( const T c )
    {
        x /= c;
        y /= c;
        z /= c;
        w /= c;
        return *this;
    }

    template<typename T>
    inline Vector4<T>& Vector4<T>::operator*=( const T c )
    {
        x *= c;
        y *= c;
        z *= c;
        w *= c;
        return *this;
    }

    template <typename T> template <typename T2>
    inline Vector4<T>::operator Vector4<T2>() const
    {
        return Vector4<T2>( ( T2 ) x, ( T2 ) y, ( T2 ) z, ( T2 ) w );
    }

    template<>
    inline bool Vector4<float>::operator==( const Vector4<float> &v ) const
    {
        return Math::abs( v.x - x ) < Math::EPSILONF
            && Math::abs( v.y - y ) < Math::EPSILONF
            && Math::abs( v.z - z ) < Math::EPSILONF
            && Math::abs( v.w - w ) < Math::EPSILONF;
    }

    template<>
    inline bool Vector4<float>::operator!=( const Vector4<float> &v ) const
    {
        return Math::abs( v.x - x ) > Math::EPSILONF
            || Math::abs( v.y - y ) > Math::EPSILONF
            || Math::abs( v.z - z ) > Math::EPSILONF
            || Math::abs( v.w - w ) > Math::EPSILONF;
    }

    template<>
    inline bool Vector4<double>::operator==( const Vector4<double> &v ) const
    {
        return Math::abs( v.x - x ) < Math::EPSILOND
            && Math::abs( v.y - y ) < Math::EPSILOND
            && Math::abs( v.z - z ) < Math::EPSILOND
            && Math::abs( v.w - w ) < Math::EPSILOND;
    }

    template<>
    inline bool Vector4<double>::operator!=( const Vector4<double> &v ) const
    {
        return Math::abs( v.x - x ) > Math::EPSILOND
            || Math::abs( v.y - y ) > Math::EPSILOND
            || Math::abs( v.z - z ) > Math::EPSILOND
            || Math::abs( v.w - w ) > Math::EPSILOND;
    }


    template<typename T>
    inline T Vector4<T>::length( void ) const
    {
        return Math::sqrt( x * x + y * y + z * z  + w * w );
    }

    template<typename T>
    inline T Vector4<T>::lengthSqr( void ) const
    {
        return x * x + y * y + z * z + w * w;
    }

    template<typename T>
    T Vector4<T>::normalize( void )
    {
        T lenSqr, lenInv;

        lenSqr = x * x + y * y + z * z + w * w;
        lenInv = Math::invSqrt( lenSqr );
        x *= lenInv;
        y *= lenInv;
        z *= lenInv;
        w *= lenInv;
        return lenSqr * lenInv;
    }

    template<typename T>
    void Vector4<T>::clamp( const Vector4<T> &min, const Vector4<T> &max )
    {
        x = Math::clamp( x, min.x, max.x );
        y = Math::clamp( y, min.y, max.y );
        z = Math::clamp( z, min.z, max.z );
        w = Math::clamp( w, min.w, max.w );
    }

    template<typename T>
    size_t Vector4<T>::dimension( void ) const
    {
        return 4;
    }

    template <typename T>
    bool Vector4<T>::isEqual( const Vector4<T>& other, T epsilon ) const
    {
        return ( Math::abs( x - other.x ) <= epsilon ) &&
               ( Math::abs( y - other.y ) <= epsilon ) &&
               ( Math::abs( z - other.z ) <= epsilon ) &&
               ( Math::abs( w - other.w ) <= epsilon );
    }

    template<typename T>
    const T* Vector4<T>::ptr( void ) const
    {
        return &x;
    }

    template<typename T>
    T* Vector4<T>::ptr( void )
    {
        return &x;
    }

    template<typename T>
    void Vector4<T>::mix( const Vector4<T> &v1, const Vector4<T> &v2, float alpha )
    {
        x = Math::mix( v1.x, v2.x, alpha );
        y = Math::mix( v1.y, v2.y, alpha );
        z = Math::mix( v1.z, v2.z, alpha );
        w = Math::mix( v1.w, v2.w, alpha );
    }

    template<typename T>
    String Vector4<T>::toString( void ) const
    {
        String s;
        s.sprintf( "%0.10f %0.10f %0.10f %0.10f", x, y, z, w );

        return s;
    }

    template<typename T>
    Vector4<T> Vector4<T>::fromString( const String& s )
    {
        Vector4<T> m;

        DataIterator it( s );
        String token;
        String deliminators("\n\r\t ");
        for( size_t i = 0; i < 4; i++ ){
            if( !it.nextToken( token, deliminators ) )
                throw CVTException( "Could not create Matrix from String!" );
            m[ i ] = token.to<T>();
        }

        return m;
    }

    template<typename T>
    std::ostream& operator<<( std::ostream& out, const Vector4<T> &v )
    {
        out << " | " << std::setprecision( 5 ) << std::setw( 12 ) << v.x << " " << std::setw( 12 ) << v.y << " " << std::setw( 12 ) << v.z << " " << std::setw( 12 ) << v.w << " | ";
        return out;
    }

}

#endif
