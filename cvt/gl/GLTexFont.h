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

#ifndef CVT_GLTEXTFONT_H
#define CVT_GLTEXTFONT_H

#include <cvt/gfx/Font.h>
#include <cvt/gl/GLTexture.h>

namespace cvt {

	/**
	  \ingroup GL
	*/
	class GLTexFont : public Font {
		public:
			GLTexFont();
			void load( const char* path );
			int spriteSize() const;
			const GLTexture& texture() const;
			int size() const;
			int ascent() const;
			int descent() const;
			int offsetX() const;
			int offsetY() const;
			Recti stringBounds( const char* str, size_t n = 0 ) const;
			int advance( size_t glyph ) const;

		private:
			GLTexFont( const GLTexFont& );
			const GLTexFont& operator=( const GLTexFont& );

			int32_t _width, _height;
			int32_t _fontsize;
			int32_t _ascent, _descent;
			int32_t _spritesize;
			int32_t _offx, _offy;
			struct GlyphMetric {
				int32_t width, height;
				int32_t bearingX, bearingY;
				int32_t advance;
			};
			GlyphMetric _metric[ 256 ];
			GLTexture _tex;
	};

	inline int GLTexFont::spriteSize() const
	{
		return _spritesize;
	}

	inline const GLTexture& GLTexFont::texture() const
	{
		return _tex;
	}

	inline int GLTexFont::size() const
	{
		return _fontsize;
	}

	inline int GLTexFont::ascent() const
	{
		return _ascent;
	}

	inline int GLTexFont::descent() const
	{
		return _descent;
	}

	inline int GLTexFont::offsetX() const
	{
		return _offx;
	}

	inline int GLTexFont::offsetY() const
	{
		return _offy;
	}



	inline Recti GLTexFont::stringBounds( const char* _str, size_t n ) const
	{
		uint8_t* str = ( uint8_t* ) _str;
		Recti rect;
		int x, h;

		h = ascent() - descent();

		if( str ) {
			if( n && *str ) {
				rect.set( _metric[ *str ].bearingX, 0, _metric[ *str ].width, h );
				x = _metric[ *str ].bearingX + _metric[ *str ].advance;
				str++;
				n--;
				while( *str && n-- ) {
					rect.join( x + _metric[ *str ].bearingX, 0, _metric[ *str ].width, h );
					x += _metric[ *str ].advance;
					str++;
				}
			} else {
				if( *str ) {
					rect.set( _metric[ *str ].bearingX, 0, _metric[ *str ].width, h );
					x = _metric[ *str ].bearingX + _metric[ *str ].advance;
					str++;
					while( *str ) {
						rect.join( x + _metric[ *str ].bearingX, 0, _metric[ *str ].width, h );
						x += _metric[ *str ].advance;
						str++;
					}
				}
			}
		}
		return rect;
	}


	inline int GLTexFont::advance( size_t glyph ) const
	{
		if( glyph > 0xff )
			return 0;
		return _metric[ glyph ].advance;
	}
}

#endif
