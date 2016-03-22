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

#include <cvt/math/Fixed.h>

#include <cvt/util/CVTTest.h>

namespace cvt
{
	/* CORDIC table for zfixed_sin/cos -> atan(2^(-[0:16]) in ZFixed format*/
	static const uint32_t _table_fixed_atan_cordic[ 16 ] = 
	{
	   2949120, 1740963, 919876, 466945,
	   234378, 117303, 58666, 29334,
	   14667, 7333, 3666, 1833,
	   458, 229, 114, 57
	};

	void  Fixed::sin_cos( Fixed* sinOutput, Fixed* cosOutput ) const
	{
		int32_t x = 39796;
		int32_t y = 0;    /*sin value = 0*/
		int32_t curdeg = 0;

	    int32_t newx;
	    int step;
	    int csign = 0;

		int32_t deg = _val;

	    /* well this sucks, but is necessary for reasonable output*/
	    while( deg > 11796480 )
			deg -= 23592960;
		while( deg < -11796480 )
			deg += 23592960;

		if( deg > 5898240 ) {
			deg = 11796480 - deg;
			csign = 1;
		} else if( deg < -5898240 ) {
			deg = -11796480 - deg;
			csign = 1;
		}

		for( step = 0; step < 16; step++ ) {
			if( deg > curdeg ) {
				newx = x - ( y >> step );
				y = ( x >> step ) + y;
				x = newx;
				curdeg += _table_fixed_atan_cordic[ step ];
			} else {
				newx = x + ( y >> step );
				y = -( x >> step ) + y;
				x = newx;
				curdeg -= _table_fixed_atan_cordic[ step ];
			}
		}

		if( csign )
			x = -x;

		sinOutput->_val = y;
		cosOutput->_val = x;
	}

	Fixed Fixed::sin() const
	{
		int32_t x = 39796;/* cos value in fixed-point */
		int32_t y = 0;    /*sin value*/
		int32_t curdeg = 0;
		int32_t newx;
		int step;

		int32_t deg = _val;
		/* well this sucks, but is necessary for reasonable output*/
		while( deg > 11796480 )
			deg -= 23592960;
		while( deg < -11796480 )
			deg += 23592960;

		if( deg > 5898240 ) {
			deg = 11796480 - deg;
		} else if( deg < -5898240 ) {
			deg = -11796480 - deg;
		}

		for( step = 0; step < 16; step++ ){
			if( deg > curdeg ) {
				newx = x - ( y >> step );
				y = ( x >> step ) + y;
				x = newx;
				curdeg += _table_fixed_atan_cordic[ step ];
			} else {
				newx = x + ( y >> step );
				y = -( x >> step ) + y;
				x = newx;
				curdeg -= _table_fixed_atan_cordic[ step ];
			}
		}

		Fixed ret;
		ret._val = y;
		return ret;
	}

	Fixed Fixed::cos() const
	{
		int32_t x = 39796; /*cos value in fixed-point*/
		int32_t y = 0;    /*sin value*/
		int32_t curdeg = 0;
		int32_t newx;
		int step;
		int csign = 0;

		int32_t deg = _val;

		/* well this sucks, but is necessary for reasonable output*/
		while( deg > 11796480 )
			deg -= 23592960;
		while( deg < -11796480 )
			deg += 23592960;

		if( deg > 5898240 ) {
			deg = 11796480 - deg;
			csign = 1;
		} else if( deg < -5898240 ) {
			deg = -11796480 - deg;
			csign = 1;
		}

		for( step = 0; step < 16; step++ ){
			if( deg > curdeg ) {
				newx = x - ( y >> step );
				y = ( x >> step ) + y;
				x = newx;
				curdeg += _table_fixed_atan_cordic[ step ];
			} else {
				newx = x + ( y >> step );
			    y = -( x >> step ) + y;
			    x = newx;
			    curdeg -= _table_fixed_atan_cordic[ step ];
			}
		}

		if( csign )
			x = -x;
		Fixed ret;
		ret._val = x;
	    return ret;
	}


	BEGIN_CVTTEST( Fixed )
		bool result = true;
		bool b;

		float v = 0.5f;
		Fixed f( v );
		Fixed copy( f );

		b = ( f == copy );
		CVTTEST_PRINT( "Copy Constructor", b );
		result &=b;

		b = ( f.toFloat() == v );
		CVTTEST_PRINT( "toFloat()", b );
		result &=b;

		Fixed roundIt( 1.55f );
		b = ( roundIt.round() == 2 );
		roundIt = 1.45f;
		b &= ( roundIt.round() == 1 );
		CVTTEST_PRINT( "round()", b );
		result &=b;

		float fVal = 34.5643f;
		Fixed fixedFrac( fVal );
		int frac = fixedFrac.frac();
		float fFrac = fVal - floor( fVal );
		b = ( ( frac / 0xFFFF ) - fFrac ) < 0.0001;
		CVTTEST_PRINT( "frac()", b );
		result &=b;

		b = ( roundIt.floor() == 1 );
		roundIt = 1.85f;
		b &= ( roundIt.floor() == 1 );
		roundIt = -2.2f;
		b &= ( roundIt.floor() == -3 );
		CVTTEST_PRINT( "floor()", b );
		result &=b;

		roundIt = 1.85f;
		b = ( roundIt.ceil() == 2 );
		roundIt = 1.85f;
		b &= ( roundIt.ceil() == 2 );
		roundIt = -2.2f;
		b &= ( roundIt.ceil() == -2 );
		CVTTEST_PRINT( "ceil()", b );
		result &=b;

		roundIt = 2.5f;
		b &= ( roundIt.abs().toFloat() == 2.5f );
		roundIt = -2.5f;
		b &= ( roundIt.abs().toFloat() == 2.5f );
		CVTTEST_PRINT( "abs()", b );
		result &=b;

		roundIt = 4.0f;
		b &= ( roundIt.sqrt().toFloat() == 2.0f );
		float fl = 45.87;
		roundIt = fl;
		b &= ( ( roundIt.sqrt().toFloat() - Math::sqrt( fl ) ) < 0.000001 );
		CVTTEST_PRINT( "sqrt()", b );
		result &=b;

		b &= ( ( roundIt.recip().toFloat() - ( 1.0 / fl ) ) < 0.00001 );
		CVTTEST_PRINT( "recip()", b );
		result &=b;


		f+=1.5f;
		v+=1.5f;
		b = ( f.toFloat() == v );
		CVTTEST_PRINT( "operator+=()", b );
		result &=b;

		Fixed f2( 2.0f );
		Fixed f3 = f + f2;
		b = ( f3.toFloat() == ( v + 2.0f ) );
		CVTTEST_PRINT( "operator+()", b );
		result &=b;

		Fixed fm = f - (Fixed)1.5f;
		b = ( fm.toFloat() == ( v - 1.5f ) );
		CVTTEST_PRINT( "operator-()", b );
		result &=b;

		Fixed f4 = f3 * f;
		b = ( f4.toFloat() == ( ( v + 2.0f ) * v ) );
		CVTTEST_PRINT( "operator*", b );
		result &=b;

		f = f4 / f2;
		v = ( ( v + 2.0f ) * v ) / 2.0f;
		b = ( f.toFloat() == v );
		CVTTEST_PRINT( "operator/", b );
		result &=b;

		{
			Fixed f0( 1234.5f );
			Fixed f1( -13.5f );
			Fixed f = f0 / f1;
			float v = 1234.5f / -13.5f;
//			std::cout << (float)f << " <-> " << v << std::endl;
			b = ( Math::abs( f.toFloat() == v ) < 0.00001f );
			CVTTEST_PRINT( "operator/", b );
			result &=b;
		}

		b = ( f == ( Fixed )v );
		CVTTEST_PRINT( "operator==", b );
		result &=b;

		b = !( f != ( Fixed )v );
		CVTTEST_PRINT( "operator!=", b );
		result &=b;

		Fixed i( ( int16_t )5 );
		b = ( i.toFloat() == 5.0f );
		CVTTEST_PRINT( "Fixed( int16_t )", b );
		result &=b;

		float angle = Math::PI / 6.0f;
		float sf = Math::sin( angle );
		Fixed degree( angle * 180.0f / Math::PI );
		b = ( Math::abs( degree.sin().toFloat() - sf ) < 0.001 );
		CVTTEST_PRINT( "sin()", b );
		result &=b;

		float cf = Math::cos( angle );
		b = ( Math::abs( degree.cos().toFloat() - cf ) < 0.001 );
		CVTTEST_PRINT( "cos()", b );
		result &=b;

		Fixed cFixed, sFixed;
		degree.sin_cos( &sFixed, &cFixed );
		b = ( Math::abs( cFixed.toFloat() - cf ) < 0.001 ) && ( Math::abs( sFixed.toFloat() - sf ) < 0.001 );
		CVTTEST_PRINT( "sin_cos()", b );
		result &=b;

		float scale = -0.5f;
		Fixed scaled;
		Fixed sum;
		b = true;
		uint8_t u8 = 0;
		while ( u8 != 255 ) {
			f = scale;
			scaled = f * u8;
			b &= ( scaled.toFloat() == ( scale * (float)( u8 ) ) );
			f = -scale;
			scaled = f * u8;
			b &= ( scaled.toFloat() == ( -scale * (float)( u8 ) ) );
			u8++;
		}
		CVTTEST_PRINT( "operator*( uint8_t )", b );
		result &=b;

		u8 = 0;
		b = true;
		scale = 40.0f;
		while( u8 != 255 ){
			f = scale;
			sum = f + u8;
			b &= ( sum.toFloat() == ( scale + ( uint32_t )u8 ) );
			u8++;
		}
		CVTTEST_PRINT( "operator+( uint8_t )", b );
		result &=b;

		return result;
	END_CVTTEST

}
