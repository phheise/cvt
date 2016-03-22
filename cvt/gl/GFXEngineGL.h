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

#include <cvt/gfx/GFXEngine.h>
#include <cvt/gl/GLContext.h>

#include <cvt/gl/OpenGL.h>
#include <cvt/gl/GLProgram.h>
#include <cvt/gl/GLVertexArray.h>

#include <cvt/gl/progs/GLBasicProg.h>
#include <cvt/gl/progs/GLFillRoundRectProg.h>
#include <cvt/gl/progs/GLDrawTextProg.h>
#include <cvt/gl/progs/GLDrawImageProg.h>
#include <cvt/gl/progs/GLDrawIconProg.h>
#include <cvt/gl/progs/GLDrawModelProg.h>
#include <cvt/gl/progs/GLFillPathProg.h>

namespace cvt {

	class GFXEngineGL : public GFXEngine
	{
		public:
			GFXEngineGL( GLContext* ctx );
			~GFXEngineGL();

			void begin();
			void end();
			void drawLines( const Vector2f* pts, size_t n, float width, const Color& c );
			void fillRect( const Recti& rect, const Color& c );
			void drawRect( const Recti& rect, float width, const Color& c );
			void fillRoundRect( const Recti& rect, float r, const Color& c );
			void drawRoundRect( const Recti& rect, float width, float r, const Color& c );
			void drawEllipse( const Vector2f& center, float rx, float ry, float width, const Color& c );
			void fillEllipse( const Vector2f& center, float rx, float ry, const Color& c );
			void strokePath( const Pathf& path, float width, const Color& c, GFX::StrokeStyle style );
			void fillPath( const Pathf& path, const Color& c, GFX::PolygonFillRule rule );
			void drawText( int x, int y, const char* text, const Color& c );
			void drawImage( int x, int y, const Image& img, float alpha );
			void drawImage( const Recti& rect, const Image& img, float alpha );
			void drawIcon( int x, int y, GFX::Icon i, const Color& c );
			void drawIcons( const Vector2i* pts, size_t npts, GFX::Icon i, const Color& c );
			void drawIcons( const Vector2f* pts, size_t npts, GFX::Icon i, const Color& c );
			const Font& font() const;

			//void drawModel( GLModel& mdl, const Matrix4f& modelview, float near, float far, const Color& c );
			bool supportsGL() const { return true; }
			void beginGL();
			void endGL();

			void setViewport( const Recti& viewport );
			const Recti& viewport() const;

			void setClipRect( const Recti& childrect );
			const Recti& clipRect() const;

			void setTranslation( const Vector2i& trans );
			const Vector2i& translation() const;

		protected:
			GFXEngineGL( const GFXEngineGL& );

			GLContext*		_ctx;
			Recti			_viewport;
			Recti			_cliprect;
			Vector2i		_translation;
			bool			_fliph;

			GLBasicProg			basicp;
			GLFillRoundRectProg fillrrectp;
			GLDrawTextProg		drawtextp;
			GLDrawImageProg		drawimgp;
			GLDrawIconProg		drawiconp;
			GLDrawModelProg		modelp;
			GLFillPathProg		fillpathp;
			GLTexFont			_glfont;
	};

	inline void GFXEngineGL::setViewport( const Recti& viewport )
	{
		_viewport = viewport;
	}

	inline const Recti& GFXEngineGL::viewport() const
	{
		return _viewport;
	}

	inline const Recti& GFXEngineGL::clipRect() const
	{
		return _cliprect;
	}

	inline const Font& GFXEngineGL::font() const
	{
		return _glfont;
	}

	inline void GFXEngineGL::setTranslation( const Vector2i& trans )
	{
		_translation = trans;
	}

	inline const Vector2i& GFXEngineGL::translation() const
	{
		return _translation;
	}



}
