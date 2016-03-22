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

#ifndef CVTGFX_H
#define CVTGFX_H

#include <cvt/geom/Rect.h>
#include <cvt/gfx/Color.h>
#include <cvt/gfx/Image.h>
#include <cvt/math/Vector.h>
#include <cvt/gfx/Drawable.h>
#include <cvt/gfx/Font.h>
#include <cvt/gfx/Alignment.h>
#include <cvt/gfx/Path.h>
#include <cvt/util/Flags.h>

namespace cvt {
	class GFXEngine;

    enum GFXStatusEnum {
        GFX_ACTIVE        = 1,
        GFX_DELETE_ENGINE = 2
    };

    CVT_ENUM_TO_FLAGS( GFXStatusEnum, GFXStatus )

	class GFX {
		friend class Widget;
		friend class WidgetImplWinGLX11;
        friend class GLView;

		public:
		enum PolygonFillRule { WINDING_EVEN_ODD, WINDING_NONZERO };
		enum StrokeStyle { STROKE_MITER, STROKE_ROUND, STROKE_BEVEL };
		enum Icon { ICON_CIRCLECROSS = 0,
					ICON_RECTCROSS,
					ICON_CIRCLERIGHT,
					ICON_CIRCLELEFT,
					ICON_RECT,
					ICON_CIRCLE,
					ICON_CIRCLEUP,
					ICON_CIRCLEDOWN,
					ICON_CORNER,
					ICON_CROSS,
					ICON_CIRCLEEMPTY,
					ICON_TINY_CROSS	};

			GFX();
			GFX( const GFX& g );
			GFX( GFXEngine* engine );
			GFX( Drawable* drawable );
			GFX( Drawable& drawable );
			~GFX();

			void begin();
			void begin( GFXEngine* engine );
			void begin( Drawable* engine );
			void end();

			void setDefault();

			void setColor( const Color& c );
			Color& color();
			void setLineWidth( float w );
			float lineWidth();

			void drawLine( int x1, int y1, int x2, int y2 );
			void drawLine( const Vector2f& pt1, const Vector2f& pt2 );
			void drawLines( const Vector2f* pts, size_t n );

			void fillRect( const Recti& rect );
			void fillRect( int x, int y, int width, int height );

			void drawRect( const Recti& rect );

			void fillRoundRect( const Recti& rect, float radius );
			void fillRoundRect( int x, int y, int width, int height, float radius );

			void fillPath( const Pathf& path, GFX::PolygonFillRule rule = GFX::WINDING_NONZERO );
			void strokePath( const Pathf& path );

			const Font& font( ) const;
			void drawText( int x, int y, const char* text );
			void drawText( const Recti& rect, Alignment alignment, const char* text );
			void drawText( int x, int y, int width, int height, Alignment alignment, const char* text );

			void drawImage( int x, int y, const Image& img );
			void drawImage( int x, int y, int width, int height, const Image& img );

			void drawIcon( int x, int y, Icon i );
			void drawIcons( const Vector2i* pts, size_t npts, Icon i );
			void drawIcons( const Vector2f* pts, size_t npts, Icon i );

			bool supportsGL() const;
			void beginGL();
			void endGL();

			void setTranslation( const Vector2i& translation );
			void setTranslation( int tx, int ty );

			const Vector2i& translation() const;

		private:
			void setClipRect( const Recti& r );
			const Recti& clipRect() const;

			/* setDefault needs to be called after changing the global translation */
			void setTranslationGlobal( const Vector2i& translation );
			void getTranslationGlobal( Vector2i& translation ) const;

			GFXEngine* _engine;
			GFXStatus  _status;
			Color	   _color;
			float	   _linewidth;
			Vector2i   _translation;
			Vector2i   _translationGlobal;
	};

	inline void GFX::setColor( const Color& c )
	{
		_color = c;
	}

	inline Color& GFX::color()
	{
		return _color;
	}

	inline void GFX::setLineWidth( float w )
	{
		_linewidth = w;
	}

	inline float GFX::lineWidth()
	{
		return _linewidth;
	}

	inline void GFX::drawText( int x, int y, int width, int height, Alignment alignment, const char* text )
	{
		drawText( Recti( x, y, width, height ), alignment, text );
	}
}

#endif
