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

#ifndef CVTCOLOR_H
#define CVTCOLOR_H

#include <cvt/math/Math.h>

namespace cvt {
	class Image;

	class Color
	{
		friend class Image;

		public:
			Color();
			Color( float r, float g, float b, float a );
			Color( float gray );
			Color( float gray, float alpha );
			Color( int r, int g, int b, int a );
			Color( int gray );
			Color( int gray, int alpha );
			Color( const Color& c );

			void set( float r, float g, float b, float a = 1.0f );
			void set( float g, float a );
			void set( int r, int g, int b, int a );

			void get( float& r, float& g, float& b, float& a ) const;
			void get( int& r, int& g, int& b, int& a ) const;

			float red() const { return _r; }
			float green() const { return _g; }
			float blue() const { return _b; }
			float alpha() const { return _a; }
			float gray() const;

			void setRed( float r ) { _r = r; }
			void setGreen( float g ) { _g = g; }
			void setBlue( float b ) { _b = b; }
			void setAlpha( float a ) { _a = a; }

			Color& operator=( const Color & c );
			Color operator+( const Color & c ) const;
			Color operator-( const Color & c ) const;
			Color operator*( float s ) const;

			bool operator==( const Color& c ) const;
			bool operator!=( const Color& c ) const;

			void mix( const Color& c1, const Color& c2, float alpha );

			const float* data() const { return &_r; }
			const float* ptr() const { return &_r; }

			static const Color BLACK;
			static const Color WHITE;
			static const Color RED;
			static const Color GREEN;
			static const Color BLUE;
            static const Color YELLOW;
			static const Color PINK;
			static const Color GRAY;

		private:
			/*union {
				struct {
					float _r, _g, _b, _a;
				};
				float v[ 4 ];
			}  __attribute__((aligned(16)));*/
					float _r, _g, _b, _a;
	};

	inline Color::Color()
	{
		set( 0.0f, 0.0f, 0.0f, 0.0f );
	}

	inline Color::Color( float r, float g, float b, float a )
	{
		set( r, g, b, a );
	}

	inline Color::Color( float g )
	{
		set( g, g, g, 1.0f );
	}

	inline Color::Color( float g, float alpha )
	{
		set( g, g, g, alpha );
	}

	inline Color::Color( int r, int g, int b, int a )
	{
		_r = Math::clamp( ( float ) r / 255.0f, 0.0f, 1.0f );
		_g = Math::clamp( ( float ) g / 255.0f, 0.0f, 1.0f );
		_b = Math::clamp( ( float ) b / 255.0f, 0.0f, 1.0f );
		_a = Math::clamp( ( float ) a / 255.0f, 0.0f, 1.0f );
	}

	inline Color::Color( int gray, int alpha )
	{
		_r = Math::clamp( ( float ) gray / 255.0f, 0.0f, 1.0f );
		_g = _r;
		_b = _r;
		_a = Math::clamp( ( float ) alpha / 255.0f, 0.0f, 1.0f );
	}

	inline Color::Color( int gray )
	{
		_r = Math::clamp( ( float ) gray / 255.0f, 0.0f, 1.0f );
		_g = _r;
		_b = _r;
		_a = 1.0f;
	}

	inline Color::Color( const Color& c )
	{
		_r = c._r;
		_g = c._g;
		_b = c._b;
		_a = c._a;
	}

	inline void Color::set( float r, float g, float b, float a )
	{
		_r = r;
		_g = g;
		_b = b;
		_a = a;
	}

	inline void Color::set( float g, float a )
	{
		_r = g;
		_g = g;
		_b = g;
		_a = a;
	}

	inline void Color::set( int r, int g, int b, int a )
	{
		_r = Math::clamp( ( float ) r / 255.0f, 0.0f, 1.0f );
		_g = Math::clamp( ( float ) g / 255.0f, 0.0f, 1.0f );
		_b = Math::clamp( ( float ) b / 255.0f, 0.0f, 1.0f );
		_a = Math::clamp( ( float ) a / 255.0f, 0.0f, 1.0f );
	}

	inline void Color::get( float& r, float& g, float& b, float& a ) const
	{
		r = _r;
		g = _g;
		b = _b;
		a = _a;
	}

	inline void Color::get( int& r, int& g, int& b, int& a ) const
	{
		r = ( int ) ( _r * 255.0f );
		g = ( int ) ( _g * 255.0f );
		b = ( int ) ( _b * 255.0f );
		a = ( int ) ( _a * 255.0f );
	}

	inline float Color::gray() const
	{
		return 0.2126f * _r + 0.7152f * _g + 0.0722f * _b;
	}

	inline Color& Color::operator=( const Color & c )
	{
		_r = c._r;
		_g = c._g;
		_b = c._b;
		_a = c._a;
		return *this;
	}

	inline Color Color::operator+( const Color & c ) const
	{
		Color r;
		r._r = _r + c._r;
		r._g = _g + c._g;
		r._b = _b + c._b;
		r._a = _a + c._a;

		return r;
	}

	inline Color Color::operator-( const Color & c ) const
	{
		Color r;
		r._r = _r - c._r;
		r._g = _g - c._g;
		r._b = _b - c._b;
		r._a = _a - c._a;

		return r;
	}

	inline Color Color::operator*( float s ) const
	{
		Color r;
		r._r = _r * s;
		r._g = _g * s;
		r._b = _b * s;
		r._a = _a * s;
		return r;
	}


	inline bool Color::operator==( const Color& c ) const
	{
		return _r == c._r && _g == c._g &&_b == c._b && _a == c._a;
	}

	inline bool Color::operator!=( const Color& c ) const
	{
		return !( _r == c._r && _g == c._g &&_b == c._b && _a == c._a );
	}

    inline void Color::mix( const Color& c1, const Color& c2, float alpha )
    {
        _r = Math::mix( c1._r, c2._r, alpha );
        _g = Math::mix( c1._g, c2._g, alpha );
        _b = Math::mix( c1._b, c2._b, alpha );
        _a = Math::mix( c1._a, c2._a, alpha );
    }

	inline std::ostream& operator<<( std::ostream& out, const Color& c )
	{
		out << "R: " << c.red() << " G: " << c.green() << " B: " << c.blue() << " A: " << c.alpha();
		return out;
	}
}

#endif
