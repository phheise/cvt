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

#ifndef CVT_GFXENGINEGLFBO_H
#define CVT_GFXENGINEGLFBO_H

#include <cvt/gl/GFXEngineGL.h>

namespace cvt {
	class GFXEngineGLFBO : public GFXEngineGL
	{
		public:
			GFXEngineGLFBO( GLFBO& fbo, GLContext* ctx = NULL );
			~GFXEngineGLFBO();

			void begin();
			void end();

		private:
			GLFBO& _fbo;
	};

	inline GFXEngineGLFBO::GFXEngineGLFBO( GLFBO& fbo, GLContext* ctx ) : GFXEngineGL( ctx?ctx:GLContext::currentContext() ), _fbo( fbo )
	{
		_fliph = true;
	}

	inline GFXEngineGLFBO::~GFXEngineGLFBO()
	{
	}

	void GFXEngineGLFBO::begin()
	{
		_ctx->makeCurrent();
		glEnable( GL_BLEND );
//		glEnable( GL_TEXTURE_2D );
#ifndef APPLE
		glEnable( GL_POINT_SPRITE );
#endif
		glEnable( GL_SCISSOR_TEST );
		glDisable( GL_DEPTH_TEST );
		glEnable( GL_LINE_SMOOTH );
//		if( _ctx->format().samples() )
//			glEnable( GL_MULTISAMPLE );
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		_fbo.bind();
		setViewport( Recti( 0, 0, _fbo.width(), _fbo.height() ) );
		setClipRect( Recti( 0, 0, _fbo.width(), _fbo.height() ) );
		_translation.set( 0, 0 );
	}

	void GFXEngineGLFBO::end()
	{
		_fbo.unbind();
	}
}

#endif
