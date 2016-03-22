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

#ifndef CVT_GLXCONTEXT_H
#define CVT_GLXCONTEXT_H

#include <cvt/gui/internal/X11/X.h>
#include <cvt/gl/GLContext.h>

namespace cvt {
	class ApplicationX11;

	class GLXContext : public GLContext
	{
		friend class ApplicationX11;
		public:
			GLXContext( ::Display* dpy, const GLFormat& format, const GLXContext* share = NULL );
			~GLXContext();
			void setDrawable( ::Window win );

			virtual void makeCurrent();
			virtual void swapBuffers();
			virtual void resetCurrent();

			virtual void shareCL( cl_context_properties* props, int size, int* retsize ) const;

			virtual GLPlatform platform() const { return GL_PLATFORM_X11; }

			::Display*	glXDisplay() const { return _dpy; }
			::GLXContext glXContext() const { return _glxctx; }

			const ::XVisualInfo* XVisualInfo() const;

		private:
			::Display* _dpy;
			::Window _win;
			::GLXContext _glxctx;
			::XVisualInfo* _visual;
	};

	inline GLXContext::GLXContext( ::Display* dpy, const GLFormat& format, const GLXContext* share ) : GLContext( format ), _dpy( dpy ), _win( 0 )
	{
		::GLXFBConfig* fbconfig = NULL;
		int nconfigs, val;
		int attribs[ 20 ] = {
			GLX_RED_SIZE,	  _format.redSize(),
			GLX_GREEN_SIZE,	  _format.greenSize(),
			GLX_BLUE_SIZE,	  _format.blueSize(),
			GLX_ALPHA_SIZE,	  _format.alphaSize(),
			GLX_DEPTH_SIZE,	  _format.depthSize(),
			GLX_STENCIL_SIZE, _format.stencilSize(),
			GLX_DOUBLEBUFFER, _format.doubleBuffer(),
			None
		};

		if( _format.samples() != 0 ) {
			attribs[ 14 ] = GLX_SAMPLE_BUFFERS;
			attribs[ 15 ] = GL_TRUE;
			attribs[ 16 ] = GLX_SAMPLES;
			attribs[ 17 ] = _format.samples();
			attribs[ 18 ] = None;
		}

		fbconfig = glXChooseFBConfig( _dpy,  DefaultScreen( _dpy ), attribs, &nconfigs );
		if( !nconfigs )
			throw CVTException( "Error: GLXContext creation failed\n" );

		_visual = glXGetVisualFromFBConfig( _dpy, fbconfig[ 0 ] );
		_glxctx  = glXCreateNewContext( _dpy, fbconfig[ 0 ], GLX_RGBA_TYPE, share?share->_glxctx:NULL, format.direct() );

		if( _glxctx == NULL )
			throw CVTException( "Error: GLXContext creation failed\n" );

		glXGetFBConfigAttrib( _dpy, fbconfig[ 0 ], GLX_RED_SIZE , &val );
		_format.setRedSize( val );
		glXGetFBConfigAttrib( _dpy, fbconfig[ 0 ], GLX_GREEN_SIZE , &val );
		_format.setGreenSize( val );
		glXGetFBConfigAttrib( _dpy, fbconfig[ 0 ], GLX_BLUE_SIZE , &val );
		_format.setBlueSize( val );
		glXGetFBConfigAttrib( _dpy, fbconfig[ 0 ], GLX_ALPHA_SIZE , &val );
		_format.setAlphaSize( val );
		glXGetFBConfigAttrib( _dpy, fbconfig[ 0 ], GLX_DEPTH_SIZE , &val );
		_format.setDepthSize( val );
		glXGetFBConfigAttrib( _dpy, fbconfig[ 0 ], GLX_STENCIL_SIZE , &val );
		_format.setStencilSize( val );
		glXGetFBConfigAttrib( _dpy, fbconfig[ 0 ], GLX_DOUBLEBUFFER , &val );
		_format.setDoubleBuffer( val );
		_format._direct = glXIsDirect(_dpy, _glxctx);

		::XFree( fbconfig );
	}

	inline GLXContext::~GLXContext()
	{
		resetCurrent();
		::XFree( _visual );
	}

	inline void GLXContext::setDrawable( ::Window win )
	{
		_win = win;
	}

	inline const ::XVisualInfo* GLXContext::XVisualInfo() const
	{
		return _visual;
	}

	inline void GLXContext::makeCurrent()
	{
		if( GLContext::currentContext() == this )
			return;
		glXMakeCurrent( _dpy, _win, _glxctx );
		GLContext::setCurrent( this );
	}

	inline void GLXContext::swapBuffers()
	{
		glXSwapBuffers( _dpy, _win );
	}

	inline void GLXContext::resetCurrent()
	{
		GLContext::setCurrent( NULL );
		glXMakeCurrent( _dpy, None, NULL );
	}

	inline void GLXContext::shareCL( cl_context_properties* props, int size, int* retsize ) const
	{
		if( retsize ) {
			*retsize = 2;
			if( !props )
				return;
		}

		if( !props || size < 2 )
			throw CVTException( "Not enough memory for cl_context_properties!" );

		*props++ = CL_GL_CONTEXT_KHR;
	    *props++ = (cl_context_properties) glXContext();
		*props++ = CL_GLX_DISPLAY_KHR;
	    *props++ = (cl_context_properties) glXDisplay();
	}
}

#endif
