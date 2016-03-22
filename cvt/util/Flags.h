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

#ifndef CVT_FLAGS_H
#define CVT_FLAGS_H

namespace cvt
{	
	template<typename ENUM>
	class Flags
	{
		public:
			inline explicit Flags( size_t flags ) : _flags( flags ) {}
			inline Flags( ) : _flags( 0 ) {}
			inline Flags( const Flags<ENUM> &f ) : _flags( f._flags ) {}
			inline Flags( ENUM f ) : _flags( f ) {}

			inline Flags<ENUM>& operator=( const Flags<ENUM> &f ) { _flags = f._flags; return *this; }
			inline Flags<ENUM>& operator|=( Flags<ENUM> f ) { _flags |= f._flags; return *this; }
			inline Flags<ENUM>& operator^=( Flags<ENUM> f ) { _flags ^= f._flags; return *this; }
			inline Flags<ENUM> operator|( Flags<ENUM> f ) const { Flags<ENUM> g; g._flags = _flags | f._flags; return g; }
			inline Flags<ENUM> operator^( Flags<ENUM> f ) const { Flags<ENUM> g; g._flags = _flags ^ f._flags; return g; }

			inline Flags<ENUM>& operator&=( size_t mask ) { _flags &= mask; return *this; }
			inline Flags<ENUM>& operator|=( ENUM f ) { _flags |= f; return *this; }			
			inline Flags<ENUM> operator&( size_t mask ) const { Flags<ENUM> g; g._flags = _flags & mask; return g; }
			inline Flags<ENUM> operator&( ENUM f ) const { Flags<ENUM> g; g._flags = _flags & f; return g; }
			inline Flags<ENUM> operator|( ENUM f ) const { Flags<ENUM> g; g._flags = _flags | f; return g; }
			inline Flags<ENUM> operator^( ENUM f ) const { Flags<ENUM> g; g._flags = _flags ^ f; return g; }
			inline Flags<ENUM>& operator^=( ENUM f ) { _flags ^= f; return *this; }

			inline operator size_t( ) const { return _flags; }

		private:
			size_t _flags;
	};
	
#define CVT_ENUM_TO_FLAGS( ENUM, NAME ) \
	typedef Flags<ENUM> NAME; \
	inline Flags<ENUM> operator|( ENUM flag1, ENUM flag2 ) \
	{ \
		return Flags<ENUM>( flag1 ) | flag2; \
	}

}

#endif
