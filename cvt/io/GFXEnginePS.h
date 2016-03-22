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

#ifndef CVT_GFXENGINEPS_H
#define CVT_GFXENGINEPS_H

#include <cvt/gfx/GFXEngine.h>
#include <cvt/gfx/DummyFont.h>
#include <string>

namespace cvt {
	class GFXEnginePS : public GFXEngine
	{
		public:
			GFXEnginePS( const std::string& path, size_t width = 1024, size_t height = 786 );
			void begin();
			void end();
			void drawLines( const Vector2f* pts, size_t n, float width, const Color& c );
			void fillRect( const Recti& rect, const Color& c );
			void drawRect( const Recti& rect, float width, const Color& c );
			void fillRoundRect( const Recti& rect, float r, const Color& c ) {};
			void drawRoundRect( const Recti& rect, float width, float r, const Color& c ) {};
			void drawEllipse( const Vector2f& center, float rx, float ry, float width, const Color& c ) {};
			void fillEllipse( const Vector2f& center, float rx, float ry, const Color& c ) {};
			void strokePath( const Pathf& path, float width, const Color& c, GFX::StrokeStyle style ) {};
			void fillPath( const Pathf& path, const Color& c, GFX::PolygonFillRule rule ) {};
			void drawText( int x, int y, const char* text, const Color& c ) {};
			void drawImage( int x, int y, const Image& img, float alpha ) {};
			void drawImage( const Recti& rect, const Image& img, float alpha ) {};
			void drawIcon( int x, int y, GFX::Icon i, const Color& c ) {};
			void drawIcons( const Vector2i* pts, size_t npts, GFX::Icon i, const Color& c ) {};
			void drawIcons( const Vector2f* pts, size_t npts, GFX::Icon i, const Color& c ) {};

			const Font& font() const { return _dfont; };

			void setClipRect( const Recti& clip ) { _crect = clip; };
			const Recti& clipRect() const { return _crect;};

			void setTranslation( const Vector2i& t ) { _translation = t; };
                        const Vector2i& translation() const { return _translation; }

		private:
			void psColor( const Color& c );

			std::string		_filename;
                        size_t			_width;
			size_t			_height;
			FILE*			_fd;
			Recti			_crect;
			Vector2i		_translation;
			DummyFont		_dfont;
	};

	inline GFXEnginePS::GFXEnginePS( const std::string& path, size_t width, size_t height ) : _filename( path ), _width( width ), _height( height ), _fd( 0 ), _translation( 0, 0 )
	{
	}

	inline void GFXEnginePS::begin()
	{
		_fd = fopen( _filename.c_str(), "w+" );
		if( !_fd )
			throw CVTException( "Unable to open file!" );
		fputs("%!\n", _fd );
		fprintf(_fd, "/PageSize [%zd %zd]\n0 %zd translate\n1 -1 scale\n", _width, _height, _height );
	}

	inline void GFXEnginePS::end()
	{
		fputs("showpage\n", _fd );
		fclose( _fd );
	}


	inline void GFXEnginePS::drawLines( const Vector2f* pts, size_t n, float width, const Color& c )
	{
		fputs("newpath\n", _fd );
		for( size_t i = 0; i < n; i++, pts += 2 )
			fprintf( _fd, "%f %f moveto\n%f %f lineto\n", ( *pts ).x, ( *pts ).y, ( *( pts + 1 ) ).x, ( *( pts + 1 ) ).y );
		fprintf( _fd, "gsave\n%f %f %f setrgbcolor\n%f setlinewidth\nstroke\ngrestore\n", c.red(), c.green(), c.blue(), width );
	}


	inline void GFXEnginePS::fillRect( const Recti& rect, const Color& c )
	{
		fprintf( _fd, "newpath\n" \
					  "%d %d moveto\n"\
					  "%d %d lineto\n"\
					  "%d %d lineto\n"\
					  "%d %d lineto\n"\
					  "closepath\n", rect.x, rect.y, rect.x + rect.width, rect.y, rect.x + rect.width, rect.y + rect.height, rect.x, rect.y + rect.height );
		fprintf( _fd, "gsave\n%f %f %f setrgbcolor\nfill\ngrestore\n", c.red(), c.green(), c.blue() );
	}

	inline void GFXEnginePS::drawRect( const Recti& rect, float width, const Color& c )
	{
		fprintf( _fd, "newpath\n" \
					  "%d %d moveto\n"\
					  "%d %d lineto\n"\
					  "%d %d lineto\n"\
					  "%d %d lineto\n"\
					  "closepath\n", rect.x, rect.y, rect.x + rect.width, rect.y, rect.x + rect.width, rect.y + rect.height, rect.x, rect.y + rect.height );
		fprintf( _fd, "gsave\n%f %f %f setrgbcolor\n%f setlinewidth\nstroke\ngrestore\n", width, c.red(), c.green(), c.blue() );
	}

}

#endif
