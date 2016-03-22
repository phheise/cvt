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

#include <cvt/gl/GFXEngineGL.h>
#include <cvt/io/Resources.h>
#include <cvt/geom/Polygon.h>


namespace cvt {
			GFXEngineGL::GFXEngineGL( GLContext* ctx ) : _ctx( ctx ), _fliph( false )
			{
				Resources res;
				String path = res.find("default.glfont");
				_glfont.load( path.c_str() );
			}

			GFXEngineGL::~GFXEngineGL()
			{
			}

			void GFXEngineGL::begin()
			{
				_ctx->makeCurrent();
				glEnable( GL_BLEND );
//				glEnable( GL_TEXTURE_2D );
#ifndef APPLE
				glEnable( GL_POINT_SPRITE );
#endif
				glEnable( GL_SCISSOR_TEST );
				glDisable( GL_DEPTH_TEST );
				glEnable( GL_LINE_SMOOTH );
				if( _ctx->format().samples() )
					glEnable( GL_MULTISAMPLE );
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glViewport( 0, 0, _viewport.width, _viewport.height );
				setClipRect( _viewport );
				_translation.set( 0, 0 );
				glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
			}

			void GFXEngineGL::end()
			{
				_ctx->swapBuffers();
			}

			void GFXEngineGL::setClipRect( const Recti& cliprect )
			{
				_cliprect = cliprect;
				Recti clip;
				clip = _viewport;
				clip.intersect( _cliprect );
				glScissor( clip.x, _viewport.height - ( clip.y + clip.height ), clip.width, clip.height );
			}

			void GFXEngineGL::drawLines( const Vector2f* pts, size_t n, float width, const Color& c )
			{
				Matrix4f proj;
				GL::orthoTranslation( proj, 0, ( float ) _viewport.width, 0, ( float ) _viewport.height, ( float ) _translation.x, ( float ) _translation.y, -1.0f, 1.0f, _fliph );
				basicp.bind();
				basicp.setProjection( proj );
				basicp.setColor( c );
				basicp.drawLines( pts, n, width );
				basicp.unbind();
			}

			void GFXEngineGL::fillRect( const Recti& rect, const Color& c )
			{
				Matrix4f proj;
				GL::orthoTranslation( proj, 0, ( float ) _viewport.width, 0, ( float ) _viewport.height, ( float ) _translation.x, ( float ) _translation.y, -1.0f, 1.0f, _fliph );
				basicp.bind();
				basicp.setProjection( proj );
				basicp.setColor( c );
				basicp.fillRect( rect.x, rect.y, rect.width, rect.height );
				basicp.unbind();
			}

			void GFXEngineGL::drawRect( const Recti& rect, float width, const Color& c )
			{}

			void GFXEngineGL::fillRoundRect( const Recti& rect, float radius, const Color& c )
			{
				Matrix4f proj;
				GL::orthoTranslation( proj, 0, ( float ) _viewport.width, 0, ( float ) _viewport.height, ( float ) _translation.x, ( float ) _translation.y, -1.0f, 1.0f, _fliph );
				fillrrectp.bind();
				fillrrectp.setProjection( proj );
				fillrrectp.setColor( c );
				fillrrectp.fillRoundRect( rect.x, rect.y, rect.width, rect.height, radius );
				fillrrectp.unbind();
			}

			void GFXEngineGL::drawRoundRect( const Recti& rect, float width, float r, const Color& c )
			{}

			void GFXEngineGL::drawEllipse( const Vector2f& center, float rx, float ry, float width, const Color& c )
			{}

			void GFXEngineGL::fillEllipse( const Vector2f& center, float rx, float ry, const Color& c )
			{}

			void GFXEngineGL::strokePath( const Pathf& path, float width, const Color& c, GFX::StrokeStyle style )
			{
				if( width <= 0.0f ) {
					Matrix4f proj;
					GL::orthoTranslation( proj, 0, ( float ) _viewport.width, 0, ( float ) _viewport.height, ( float ) _translation.x, ( float ) _translation.y, -1.0f, 1.0f, _fliph );
					basicp.bind();
					basicp.setProjection( proj );
					basicp.setColor( c );
					PolygonSetf polyset( path );
					// draw lines
					for( int i = 0, end = polyset.size(); i < end; i++ ) {
						const Polygonf& poly = polyset[ i ];
						basicp.drawLines( &poly[ 0 ], poly.size(), 1.0f, GL_LINE_STRIP );
					}
					basicp.unbind();
				}
			}

			void GFXEngineGL::fillPath( const Pathf& path, const Color& c, GFX::PolygonFillRule rule )
			{
				Matrix4f proj;
				GL::orthoTranslation( proj, 0, ( float ) _viewport.width, 0, ( float ) _viewport.height, ( float ) _translation.x, ( float ) _translation.y, -1.0f, 1.0f, _fliph );
				fillpathp.bind();
				fillpathp.setProjection( proj );
				fillpathp.setColor( c );
				fillpathp.fillPath( path, rule );
				fillpathp.unbind();
			}

			void GFXEngineGL::drawText( int x, int y, const char* text, const Color& c )
			{
				Matrix4f proj;
				GL::orthoTranslation( proj, 0, ( float ) _viewport.width, 0, ( float ) _viewport.height, ( float ) _translation.x, ( float ) _translation.y, -1.0f, 1.0f, _fliph );
				drawtextp.bind();
				drawtextp.setProjection( proj );
				drawtextp.setColor( c );
				drawtextp.drawText( x + _glfont.offsetX(), y + _glfont.offsetY(), text, _glfont );
				drawtextp.unbind();
			}

			void GFXEngineGL::drawImage( int x, int y, const Image& img, float alpha )
			{
				Image* tmp = NULL;

				if( img.memType() != IALLOCATOR_GL ) {
					tmp = new Image( img, IALLOCATOR_GL );
				}

				Matrix4f proj;
				GL::orthoTranslation( proj, 0, ( float ) _viewport.width, 0, ( float ) _viewport.height, ( float ) _translation.x, ( float ) _translation.y, -1.0f, 1.0f, _fliph );
				drawimgp.bind();
				drawimgp.setProjection( proj );
				drawimgp.setAlpha( alpha );
				drawimgp.drawImage( x, y, tmp?*tmp:img );
				drawimgp.unbind();

				if( tmp )
					delete tmp;
			}

			void GFXEngineGL::drawImage( const Recti& rect, const Image& img, float alpha )
			{
				Image* tmp = NULL;

				if( img.memType() != IALLOCATOR_GL ) {
					tmp = new Image( img, IALLOCATOR_GL );
				}

				Matrix4f proj;
				GL::orthoTranslation( proj, 0, ( float ) _viewport.width, 0, ( float ) _viewport.height, ( float ) _translation.x, ( float ) _translation.y, -1.0f, 1.0f, _fliph );
				drawimgp.bind();
				drawimgp.setProjection( proj );
				drawimgp.setAlpha( alpha );
				drawimgp.drawImage( rect.x, rect.y, rect.width, rect.height, tmp?*tmp:img );
				drawimgp.unbind();

				if( tmp )
					delete tmp;
			}

			void GFXEngineGL::drawIcon( int x, int y, GFX::Icon i, const Color& c )
			{
				Matrix4f proj;
				GL::orthoTranslation( proj, 0, ( float ) _viewport.width, 0, ( float ) _viewport.height, ( float ) _translation.x, ( float ) _translation.y, -1.0f, 1.0f, _fliph );
				drawiconp.bind();
				drawiconp.setProjection( proj );
				drawiconp.setColor( c );
				drawiconp.drawIcon( x, y, i );
				drawiconp.unbind();
			}

			void GFXEngineGL::drawIcons( const Vector2i* pts, size_t npts, GFX::Icon i, const Color& c )
			{
				Matrix4f proj;
				GL::orthoTranslation( proj, 0, ( float ) _viewport.width, 0, ( float ) _viewport.height, ( float ) _translation.x, ( float ) _translation.y, -1.0f, 1.0f, _fliph );
				drawiconp.bind();
				drawiconp.setProjection( proj );
				drawiconp.setColor( c );
				drawiconp.drawIcons( pts, npts, i );
				drawiconp.unbind();
			}

			void GFXEngineGL::drawIcons( const Vector2f* pts, size_t npts, GFX::Icon i, const Color& c )
			{
				Matrix4f proj;
				GL::orthoTranslation( proj, 0, ( float ) _viewport.width, 0, ( float ) _viewport.height, ( float ) _translation.x, ( float ) _translation.y, -1.0f, 1.0f, _fliph );
				drawiconp.bind();
				drawiconp.setProjection( proj );
				drawiconp.setColor( c );
				drawiconp.drawIcons( pts, npts, i );
				drawiconp.unbind();
			}

			void GFXEngineGL::beginGL()
			{
			}

			void GFXEngineGL::endGL()
			{
				Recti clip;

				glEnable( GL_BLEND );
//				glEnable( GL_TEXTURE_2D );
#ifndef APPLE
				glEnable( GL_POINT_SPRITE );
#endif
				glEnable( GL_SCISSOR_TEST );
				glDisable( GL_DEPTH_TEST );
				glEnable( GL_LINE_SMOOTH );
				if( _ctx->format().samples() )
					glEnable( GL_MULTISAMPLE );
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glViewport( 0, 0, _viewport.width, _viewport.height );

				clip = _viewport;
				clip.intersect( _cliprect );
				glScissor( clip.x, _viewport.height - ( clip.y + clip.height ), clip.width, clip.height );
			}

}
