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

#ifndef CVT_FIXED_H
#define CVT_FIXED_H

#include <cvt/math/Math.h>

namespace cvt
{
	/**
	  \ingroup Math
	*/
	/* fixed point format class (16x16) */
	class Fixed {
		public:
			Fixed();
			Fixed( int16_t i );
			Fixed( float f );
			Fixed( const Fixed & other );

			float	toFloat() const;
			int		round()	const;
			int		frac() const;
			int		floor() const;
			int		ceil() const;
			Fixed	abs() const;
			Fixed	sqrt() const;
			Fixed	recip() const;

			Fixed& operator=( const Fixed & other );

			bool operator==( const Fixed & other ) const;
			bool operator!=( const Fixed & other ) const;

			/* to allow cast float f = (float)fixed; */
			//operator float() const;

			Fixed operator+( Fixed other ) const;
			Fixed operator-( Fixed other ) const;
			Fixed operator*( Fixed other ) const;
			Fixed operator*( uint8_t other ) const;
			Fixed operator+( uint8_t other ) const;
			Fixed operator/( Fixed other ) const;
			Fixed operator<<( uint8_t other ) const;
			Fixed operator>>( uint8_t other ) const;
			int32_t native() const;
			int32_t& native();

			Fixed operator+=( Fixed other );
			Fixed operator-=( Fixed other );
			Fixed operator*=( Fixed other );
			Fixed operator/=( Fixed other );

			void  sin_cos( Fixed* sinOutput, Fixed* cosOutput ) const;
			Fixed sin() const;
			Fixed cos() const;

		private:
			static const uint8_t	_fixFracShift = 16;
			static const uint32_t	_fixFracMask = ( 1 << ( _fixFracShift + 1 ) ) - 1;
			static const uint32_t	_fixedMax = ( 1 << 31 );
			int32_t		_val;
	};

	inline Fixed::Fixed()
	{
	}

	inline Fixed::Fixed( int16_t i )
	{
		_val = (int32_t) ( i << _fixFracShift );
	}

	inline Fixed::Fixed( float f )
	{
		_val = (int32_t) Math::round( ( f * (float) ( 1 << _fixFracShift ) ) );
	}

	inline Fixed::Fixed( const Fixed & other ) : _val( other._val )
	{
	}

	inline float Fixed::toFloat() const
	{
		return ( (float) _val ) / (float) ( 1 << _fixFracShift );
	}

	inline int Fixed::round() const
	{
		return ( ( _val + ( 1 << ( _fixFracShift - 1 ) ) ) >> _fixFracShift );
	}

	inline int Fixed::frac() const
	{
		return _val & _fixFracMask;
	}

	inline int Fixed::floor() const
	{
		return _val >= 0 ? ( ( _val ) >> _fixFracShift ) : ( -( ( -_val - 1 ) >> _fixFracShift ) - 1 );
	}

	inline int Fixed::ceil() const
	{
		return _val >= 0 ? ( ( _val - 1 ) >> _fixFracShift ) + 1 : ( -( ( -_val ) >> _fixFracShift ) );
	}

	inline Fixed Fixed::abs() const
	{
		Fixed ret = *this;

		if( _val < 0 )
			ret._val = -ret._val;

		return ret;
	}

/*	inline Fixed::operator float() const
	{
		return toFloat();
	}*/

	inline Fixed & Fixed::operator=( const Fixed & other )
	{
		_val = other._val;
		return *this;
	}

	inline Fixed Fixed::operator+( Fixed other ) const
	{
		other._val += _val;
		return other;
	}

	inline Fixed Fixed::operator-( Fixed other ) const
	{
		Fixed r = *this;
		r._val -= other._val;
		return r;
	}

	inline Fixed Fixed::operator*( Fixed other ) const
	{
//		int s = 1;

//		int32_t a = _val;

//	    if( a < 0 ) { a = -a; s = -1; }
//	    if( other._val < 0 ) { other._val = -other._val; s = -s; }

		int32_t c;
	    c = ( ( (int64_t) _val * other._val + 0x8000 ) >> 16 );

		Fixed ret;
		ret._val = c;
//		ret._val = ( s > 0 ) ? c : -c;

	    return ret;
	}

	inline Fixed operator*( uint8_t u, const Fixed & f ) {
		return f.operator*( u );
	}

	inline Fixed Fixed::operator*( uint8_t other ) const
	{
		Fixed ret( *this );
		ret._val *= other;
	    return ret;
	}

	inline Fixed Fixed::operator+( uint8_t other ) const
	{
		Fixed ret( *this );
		ret._val += ( ( (uint32_t)other ) << 16 );
		return ret;
	}

	inline Fixed operator+( uint8_t u, const Fixed & f ) {
		return f.operator+( u );
	}


	inline Fixed Fixed::operator/( Fixed other ) const
	{
//		int s = 1;

//		int32_t a = _val;

//	    if( a < 0 ) { a = -a; s = -1; }
//	    if( other._val < 0 ) { other._val = -other._val; s = -s; }

	    int32_t q = ( ( ( (int64_t) _val << 16 ) + ( other._val >> 1 ) ) / other._val );
		Fixed ret;
		ret._val = q;//( s < 0 ? -q : q );
	    return ret;
	}

	inline Fixed Fixed::operator<<( uint8_t other ) const
	{
		Fixed ret( *this );
		ret._val <<= other;
		return ret;
	}

	inline Fixed Fixed::operator>>( uint8_t other ) const
	{
		Fixed ret( *this );
		ret._val >>= other;
		return ret;
	}

	inline int32_t Fixed::native() const
	{
		return _val;
	}

	inline int32_t& Fixed::native()
	{
		return _val;
	}

	inline Fixed Fixed::operator+=( Fixed other )
	{
		_val += other._val;
		return *this;
	}

	inline Fixed Fixed::operator-=( Fixed other )
	{
		_val -= other._val;
		return *this;
	}

	inline Fixed Fixed::operator*=( Fixed other )
	{
		return *this * other;
	}

	inline Fixed Fixed::operator/=( Fixed other )
	{
		return *this / other;
	}

	inline Fixed Fixed::recip() const
	{
		Fixed ret( (int16_t) 1 );
		return	ret / *this;
	}

	inline Fixed Fixed::sqrt() const
	{
		uint32_t m, y, b;
	    int32_t a, t;

		a = _val;
	    if( a < 0 ) a = -a;

	    m = 0x40000000;
	    y = 0;
	    while( m != 0 ) {
			b = y | m;
			y = y >> 1;
			t = (int32_t) ( a | ~( a - b ) ) >> 31;
			a = a - ( b & t );
			y = y | ( m & t );
			m = m >> 2;
	    }
		Fixed ret;
		ret._val = ( y << 8 );
	    return ret;
	}

	inline bool Fixed::operator==( const Fixed & other ) const
	{
		return _val == other._val;
	}

	inline bool Fixed::operator!=( const Fixed & other ) const
	{
		return _val != other._val;
	}

	inline std::ostream & operator<<( std::ostream & out, const Fixed& f )
	{
		out << f.toFloat() << "_fxd";
		return out;
	}

}

#endif
