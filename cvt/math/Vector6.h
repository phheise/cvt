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

namespace cvt {

    /**
      \ingroup Math
    */
    template<typename T>
    class Vector6 {
        public:
        typedef     T TYPE;
        enum        { DIMENSION = 6 };


                    Vector6();
                    Vector6( T x, T y, T z, T u, T v, T w );
                    Vector6( const Vector6<T>& vec );

		void		set( T x, T y, T z, T u, T v, T w );
        void		setZero( void );

        T			operator[]( int index ) const;
        T&			operator[]( int index );
        Vector6<T>	operator-() const;
        T			operator*( const Vector6<T> &v ) const;
        Vector6<T>	operator*( const T c ) const;
        Vector6<T>	cmul( const Vector6<T>& v ) const;
        Vector6<T>	operator/( const T c ) const;
        Vector6<T>	operator+( const Vector6<T> &v ) const;
        Vector6<T>	operator-( const Vector6<T> &v ) const;
        Vector6<T>&	operator+=( const Vector6<T> &v );
        Vector6<T>&	operator-=( const Vector6<T> &v );
        Vector6<T>&	operator/=( const Vector6<T> &v );
        Vector6<T>&	operator+=( const T c );
        Vector6<T>&	operator-=( const T c );
        Vector6<T>&	operator/=( const T c );
        Vector6<T>&	operator*=( const T c );

        template <typename T2>
        operator Vector6<T2>() const;

        bool		operator==( const Vector6<T> &v ) const;
        bool		operator!=( const Vector6<T> &v ) const;

        T			length( void ) const;
        T			lengthSqr( void ) const;
        T			normalize( void );		// returns length
        void		clamp( const Vector6<T> &min, const Vector6<T> &max );

        size_t		dimension( void ) const;

        bool        isEqual( const Vector6<T> & other, T epsilon ) const;

        const T*	ptr( void ) const;
        T*			ptr( void );

        String      toString( void ) const;

        static Vector6<T> fromString( const String & s );

        void		mix( const Vector6<T> &v1, const Vector6<T> &v2, float alpha );

        T x, y, z, u, v, w;
    };

    template<typename T>
    inline Vector6<T>::Vector6()
    {
    }

    template<typename T>
    inline Vector6<T>::Vector6( T x, T y, T z, T u, T v, T w )
    {
        this->x = x;
        this->y = y;
        this->z = z;
        this->u = u;
        this->v = v;
        this->w = w;
    }

    template<typename T>
    inline Vector6<T>::Vector6( const Vector6<T>& vec )
    {
        x = vec.x;
        y = vec.y;
        z = vec.z;
        u = vec.u;
        v = vec.v;
        w = vec.w;
    }

    template<typename T>
    inline void Vector6<T>::set( T x, T y, T z, T u, T v, T w )
    {
        this->x = x;
        this->y = y;
        this->z = z;
        this->u = u;
        this->v = v;
        this->w = w;
    }

    template<typename T>
    inline void Vector6<T>::setZero( )
    {
        x = 0;
        y = 0;
        z = 0;
        u = 0;
        v = 0;
        w = 0;
    }

    template<typename T>
    inline T Vector6<T>::operator[]( int index ) const
    {
        return ( &x )[ index ];
    }

    template<typename T>
    inline T& Vector6<T>::operator[]( int index )
    {
        return ( &x )[ index ];
    }

    template<typename T>
    inline Vector6<T> Vector6<T>::operator-() const
    {
        return Vector6<T>( -x, -y, -z, -u, -v, -w );
    }

    template<typename T>
    inline Vector6<T> operator*( float c, const Vector6<T> &v )
    {
        return Vector6<T>( c * v.x, c * v.y, c * v.z, c * v.u, c * v.v, c * v.w );
    }

    template<typename T>
    inline Vector6<T> Vector6<T>::operator*( const T c ) const
    {
        return Vector6<T>( x * c, y * c, z * c, u * c, v* c, w * c );
    }

    template<typename T>
    inline Vector6<T> Vector6<T>::cmul( const Vector6<T>& vec ) const
    {
        return Vector6<T>( vec.x * x, vec.y * y, vec.z * z, vec.u * u, vec.v * v, vec.w * w );
    }

    template<typename T>
    inline T Vector6<T>::operator*( const Vector6<T>& vec ) const
    {
        return vec.x * x + vec.y * y + vec.z * z + vec.u * u + vec.v * v + vec.w * w;
    }

    template<typename T>
    inline Vector6<T> Vector6<T>::operator/( const T c ) const
    {
        T inv = ( ( T ) 1.0 ) / c;
        return Vector6<T>( x * inv, y * inv, z * inv, u * inv, v * inv, w * inv );
    }

    template<typename T>
    inline Vector6<T> Vector6<T>::operator+( const Vector6<T> &vec ) const
    {
        return Vector6<T>( x + vec.x, y + vec.y, z + vec.z, u + vec.u, v + vec.v, w + vec.w );
    }

    template<typename T>
    inline Vector6<T> Vector6<T>::operator-( const Vector6<T> &vec ) const
    {
        return Vector6<T>( x - vec.x, y - vec.y, z - vec.z, u - vec.u, v - vec.v, w - vec.w );
    }

    template<typename T>
    inline Vector6<T>& Vector6<T>::operator+=( const Vector6<T> &vec )
    {
        x += vec.x;
        y += vec.y;
        z += vec.z;
        u += vec.u;
        v += vec.v;
        w += vec.w;
        return *this;
    }

    template<typename T>
    inline Vector6<T>& Vector6<T>::operator-=( const Vector6<T> &vec )
    {
        x -= vec.x;
        y -= vec.y;
        z -= vec.z;
        u -= vec.u;
        v -= vec.v;
        w -= vec.w;
        return *this;
    }

    template<typename T>
    inline Vector6<T>& Vector6<T>::operator/=( const Vector6<T> &vec )
    {
        x /= vec.x;
        y /= vec.y;
        z /= vec.z;
        u /= vec.u;
        v /= vec.v;
        w /= vec.w;
        return *this;
    }

    template<typename T>
    inline Vector6<T>& Vector6<T>::operator+=( const T c )
    {
        x += c;
        y += c;
        z += c;
        u += c;
        v += c;
        w += c;
        return *this;
    }

    template<typename T>
    inline Vector6<T>& Vector6<T>::operator-=( const T c )
    {
        x -= c;
        y -= c;
        z -= c;
        u -= c;
        v -= c;
        w -= c;
        return *this;
    }


    template<typename T>
    inline Vector6<T>& Vector6<T>::operator/=( const T c )
    {
        x /= c;
        y /= c;
        z /= c;
        u /= c;
        v /= c;
        w /= c;
        return *this;
    }

    template<typename T>
    inline Vector6<T>& Vector6<T>::operator*=( const T c )
    {
        x *= c;
        y *= c;
        z *= c;
        u *= c;
        v *= c;
        w *= c;
        return *this;
    }

    template <typename T> template <typename T2>
    inline Vector6<T>::operator Vector6<T2>() const
    {
		return Vector6<T2>( ( T2 )x, ( T2 )y, ( T2 )z, ( T2 )u, ( T2 )v, ( T2 )w );
    }

    template<>
	inline bool Vector6<float>::operator==( const Vector6<float> &vec ) const
    {
		return Math::abs( vec.x - x ) < Math::EPSILONF
			&& Math::abs( vec.y - y ) < Math::EPSILONF
			&& Math::abs( vec.z - z ) < Math::EPSILONF
			&& Math::abs( vec.u - u ) < Math::EPSILONF
			&& Math::abs( vec.v - v ) < Math::EPSILONF
			&& Math::abs( vec.w - w ) < Math::EPSILONF;
    }

    template<>
	inline bool Vector6<float>::operator!=( const Vector6<float> &vec ) const
    {
		return Math::abs( vec.x - x ) > Math::EPSILONF
			|| Math::abs( vec.y - y ) > Math::EPSILONF
			|| Math::abs( vec.z - z ) > Math::EPSILONF
			|| Math::abs( vec.u - u ) > Math::EPSILONF
			|| Math::abs( vec.v - v ) > Math::EPSILONF
			|| Math::abs( vec.w - w ) > Math::EPSILONF;
    }

    template<>
	inline bool Vector6<double>::operator==( const Vector6<double> &vec ) const
    {
		return Math::abs( vec.x - x ) < Math::EPSILOND
			&& Math::abs( vec.y - y ) < Math::EPSILOND
			&& Math::abs( vec.z - z ) < Math::EPSILOND
			&& Math::abs( vec.u - u ) < Math::EPSILOND
			&& Math::abs( vec.v - v ) < Math::EPSILOND
			&& Math::abs( vec.w - w ) < Math::EPSILOND;
    }

    template<>
	inline bool Vector6<double>::operator!=( const Vector6<double> &vec ) const
    {
		return Math::abs( vec.x - x ) > Math::EPSILOND
			|| Math::abs( vec.y - y ) > Math::EPSILOND
			|| Math::abs( vec.z - z ) > Math::EPSILOND
			|| Math::abs( vec.u - u ) > Math::EPSILOND
			|| Math::abs( vec.v - v ) > Math::EPSILOND
			|| Math::abs( vec.w - w ) > Math::EPSILOND;
    }


    template<typename T>
    inline T Vector6<T>::length( void ) const
    {
        return Math::sqrt( x * x + y * y + z * z + u * u + v * v + w * w );
    }

    template<typename T>
    inline T Vector6<T>::lengthSqr( void ) const
    {
        return x * x + y * y + z * z + u * u + v * v + w * w;
    }

    template<typename T>
    T Vector6<T>::normalize( void )
    {
        T lenSqr, lenInv;

        lenSqr = lengthSqr();
        lenInv = Math::invSqrt( lenSqr );
		this->operator *=( lenInv );
        return lenSqr * lenInv;
    }

    template<typename T>
    void Vector6<T>::clamp( const Vector6<T> &min, const Vector6<T> &max )
    {
        x = Math::clamp( x, min.x, max.x );
        y = Math::clamp( y, min.y, max.y );
        z = Math::clamp( z, min.z, max.z );
        u = Math::clamp( u, min.u, max.u );
        v = Math::clamp( v, min.v, max.v );
        w = Math::clamp( w, min.w, max.w );
    }

    template<typename T>
    size_t Vector6<T>::dimension( void ) const
    {
        return 6;
    }

    template <typename T>
    bool Vector6<T>::isEqual( const Vector6<T>& other, T epsilon ) const
    {
        return ( Math::abs( x - other.x ) <= epsilon ) &&
               ( Math::abs( y - other.y ) <= epsilon ) &&
               ( Math::abs( z - other.z ) <= epsilon ) &&
               ( Math::abs( u - other.u ) <= epsilon ) &&
               ( Math::abs( v - other.v ) <= epsilon ) &&
               ( Math::abs( w - other.w ) <= epsilon );
    }

    template<typename T>
    const T* Vector6<T>::ptr( void ) const
    {
        return &x;
    }

    template<typename T>
    T* Vector6<T>::ptr( void )
    {
        return &x;
    }

    template<typename T>
    void Vector6<T>::mix( const Vector6<T> &v1, const Vector6<T> &v2, float alpha )
    {
        x = Math::mix( v1.x, v2.x, alpha );
        y = Math::mix( v1.y, v2.y, alpha );
        z = Math::mix( v1.z, v2.z, alpha );
        u = Math::mix( v1.u, v2.u, alpha );
        v = Math::mix( v1.v, v2.v, alpha );
        w = Math::mix( v1.w, v2.w, alpha );
    }

    template<typename T>
    String Vector6<T>::toString( void ) const
    {
        String s;
        s.sprintf( "%0.10f %0.10f %0.10f %0.10f %0.10f %0.10f", x, y, z, u, v, w );

        return s;
    }

    template<typename T>
    std::ostream& operator<<( std::ostream& out, const Vector6<T> &v )
    {
        out << " | " 
			<< std::setprecision( 5 ) 
			<< std::setw( 12 ) << v.x << " " << std::setw( 12 ) << v.y << " " << std::setw( 12 ) << v.z << " " 
			<< std::setw( 12 ) << v.u << " " << std::setw( 12 ) << v.v << " " << std::setw( 12 ) << v.w 
			<< " | ";
        return out;
    }

}

#endif
