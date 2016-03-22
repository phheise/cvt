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

#ifndef CVT_ELLIPSE_H
#define CVT_ELLIPSE_H

#include <cvt/math/Vector.h>
#include <cvt/util/String.h>

#include <cvt/util/DataIterator.h>

namespace cvt {
	template<typename T>
	class Ellipse {
		public:
			Ellipse();
			Ellipse( T x, T y, T semimajor, T semiminor, T orientation );
			Ellipse( const Vector2<T>& center, T semimajor, T semiminor, T orientation );
			Ellipse( const Ellipse<T>& ellipse );
			~Ellipse();

			const Ellipse<T>&	operator=( const Ellipse& elipse );

			const Vector2<T>&	center() const;
				  Vector2<T>&	center();

			T					semiMajor() const;
			T&					semiMajor();

			T					semiMinor() const;
			T&					semiMinor();

			T					orientation() const;
			T&					orientation();

			T					area() const;

			String				toString() const;

			static Ellipse<T>	fromString( const String& str );
		private:
			Vector2<T>	_center;
			T			_smajor;
			T			_sminor;
			T			_orientation;
	};

	typedef Ellipse<float> Ellipsef;
	typedef Ellipse<double> Ellipsed;

	template<typename T>
	inline Ellipse<T>::Ellipse() :
		_center( 0, 0 ),
		_smajor( 0 ),
		_sminor( 0 ),
		_orientation( 0 )
	{
	}

	template<typename T>
	inline Ellipse<T>::Ellipse( T x, T y, T semimajor, T semiminor, T orientation ) :
		_center( x, y ),
		_smajor( semimajor ),
		_sminor( semiminor ),
		_orientation( orientation )
	{
	}

	template<typename T>
	inline Ellipse<T>::Ellipse( const Vector2<T>& center, T semimajor, T semiminor, T orientation ) :
		_center( center ),
		_smajor( semimajor ),
		_sminor( semiminor ),
		_orientation( orientation )
	{
	}


	template<typename T>
	inline Ellipse<T>::Ellipse( const Ellipse<T>& ellipse )	:
		_center( ellipse._center ),
		_smajor( ellipse._smajor ),
		_sminor( ellipse._sminor ),
		_orientation( ellipse._orientation )
	{
	}

	template<typename T>
	inline Ellipse<T>::~Ellipse()
	{
	}


	template<typename T>
	const Ellipse<T>& Ellipse<T>::operator=( const Ellipse& other )
	{
		_center = other._center;
		_smajor = other._smajor;
		_sminor = other._sminor;
		_orientation = other._orientation;
		return *this;
	}

	template<typename T>
	inline const Vector2<T>& Ellipse<T>::center() const
	{
		return _center;
	}

	template<typename T>
	inline Vector2<T>&	Ellipse<T>::center()
	{
		return _center;
	}

	template<typename T>
	inline T Ellipse<T>::semiMajor() const
	{
		return _smajor;
	}

	template<typename T>
	inline T& Ellipse<T>::semiMajor()
	{
		return _smajor;
	}

	template<typename T>
	inline T Ellipse<T>::semiMinor() const
	{
		return _sminor;
	}

	template<typename T>
	inline T& Ellipse<T>::semiMinor()
	{
		return _sminor;
	}

	template<typename T>
	inline T Ellipse<T>::orientation() const
	{
		return _orientation;
	}

	template<typename T>
	inline T& Ellipse<T>::orientation()
	{
		return _orientation;
	}

	template<typename T>
	inline T Ellipse<T>::area() const
	{
		return Math::PI * _smajor * _sminor;
	}

    template<typename T>
    String Ellipse<T>::toString( void ) const
    {
        String s;
        s.sprintf( "%f %f %f %f %f", _center.x , _center.y , _sminor, _sminor, _orientation );
        return s;
    }

    template<typename T>
    Ellipse<T> Ellipse<T>::fromString( const String& s )
    {
        Ellipse<T> ret;

        DataIterator it( s );
        String token;
        String deliminators("\n\r\t ");
        for( size_t i = 0; i < 5; i++ ){
            if( !it.nextToken( token, deliminators ) )
                throw CVTException( "Could not create Matrix from String!" );
            T val = token.to<T>();
			switch( i ) {
				case 0: ret.center().x = val;
						break;
				case 1: ret.center().y = val;
						break;
				case 2: ret.semiMajor() = val;
						break;
				case 3: ret.semiMinor() = val;
						break;
				case 4: ret.orientation() = val;
						break;
			}
        }

        return ret;
    }

	template<typename T>
	inline std::ostream& operator<<( std::ostream& out, const Ellipse<T>& e )
	{
		out << e.center() << " " << e.semiMajor() << " , " << e.semiMinor() << " " << Math::rad2Deg( e.orientation() ) << " ( Area: " << e.area() << " )" << std::endl;
		return out;
	}
}

#endif
