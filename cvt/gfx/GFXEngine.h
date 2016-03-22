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

#ifndef CVT_GFXENGINE_H
#define CVT_GFXENGINE_H

#include <cvt/geom/Rect.h>
#include <cvt/math/Vector.h>
#include <cvt/gfx/Color.h>
#include <cvt/gfx/Path.h>
#include <cvt/gfx/GFX.h>
#include <cvt/gfx/Font.h>

namespace cvt {

	class GFXEngine
	{
		public:

            virtual ~GFXEngine() {}
			virtual void begin() = 0;
			virtual void end() = 0;
			virtual void drawLines( const Vector2f* pts, size_t n, float width, const Color& c ) = 0;
			virtual void fillRect( const Recti& rect, const Color& c ) = 0;
			virtual void drawRect( const Recti& rect, float width, const Color& c ) = 0;
			virtual void fillRoundRect( const Recti& rect, float r, const Color& c ) = 0;
			virtual void drawRoundRect( const Recti& rect, float width, float r, const Color& c ) = 0;
			virtual void drawEllipse( const Vector2f& center, float rx, float ry, float width, const Color& c ) = 0;
			virtual void fillEllipse( const Vector2f& center, float rx, float ry, const Color& c ) = 0;
			virtual void strokePath( const Pathf& path, float width, const Color& c, GFX::StrokeStyle style ) = 0;
			virtual void fillPath( const Pathf& path, const Color& c, GFX::PolygonFillRule rule ) = 0;
			virtual void drawText( int x, int y, const char* text, const Color& c ) = 0;
			virtual void drawImage( int x, int y, const Image& img, float alpha ) = 0;
			virtual void drawImage( const Recti& rect, const Image& img, float alpha ) = 0;
			virtual void drawIcon( int x, int y, GFX::Icon i, const Color& c ) = 0;
			virtual void drawIcons( const Vector2i* pts, size_t npts, GFX::Icon i, const Color& c ) = 0;
			virtual void drawIcons( const Vector2f* pts, size_t npts, GFX::Icon i, const Color& c ) = 0;
			virtual const Font& font() const = 0;

			virtual bool supportsGL() const { return false; }
			virtual void beginGL() {}
			virtual void endGL() {}

			virtual void setClipRect( const Recti& childrect ) = 0;
			virtual const Recti& clipRect() const = 0;

			virtual void setTranslation( const Vector2i& trans ) = 0;
			virtual const Vector2i& translation() const = 0;

	};
}

#endif
