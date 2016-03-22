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

#include <cvt/gl/OpenGL.h>


#include <cstdlib>

#ifdef APPLE
	//#include <mach-o/dyld.h>
	#include <dlfcn.h>
#else // this needs to be included here, not in the header because of Eigen! 
	#include <GL/glext.h>
	#include <GL/glx.h>
#endif

namespace cvt {
	unsigned int GL::_glmajor = 0;
	unsigned int GL::_glminor = 0;
	unsigned int GL::_glslmajor = 0;
	unsigned int GL::_glslminor = 0;
	std::vector<String> GL::_extensions;

	// VertexArray
	void ( *GL::glBindVertexArray )( GLuint array ) = NULL;
	void ( *GL::glDeleteVertexArrays )( GLsizei n, const GLuint *arrays ) = NULL;
	void ( *GL::glGenVertexArrays )( GLsizei n, GLuint *arrays) = NULL;
	GLboolean ( *GL::glIsVertexArray )( GLuint array) = NULL;

	// FrameBuffer
	void ( *GL::glBindRenderbuffer )( GLenum target, GLuint renderbuffer) = NULL;
	void ( *GL::glDeleteRenderbuffers )( GLsizei n, const GLuint *renderbuffers) = NULL;
	void ( *GL::glGenRenderbuffers )( GLsizei n, GLuint *renderbuffers) = NULL;
	void ( *GL::glRenderbufferStorage )( GLenum target, GLenum internalformat, GLsizei width, GLsizei height) = NULL;
	
	void ( *GL::glBindFramebuffer )( GLenum target, GLuint framebuffer ) = NULL;
	void ( *GL::glDeleteFramebuffers )( GLsizei n, const GLuint *framebuffers ) = NULL;
	void ( *GL::glGenFramebuffers )( GLsizei n, GLuint *framebuffers ) = NULL;
	
	void ( *GL::glFramebufferTexture1D )( GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level ) = NULL;
	void ( *GL::glFramebufferTexture2D )( GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level ) = NULL;
	void ( *GL::glFramebufferTexture3D )( GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset ) = NULL;
	
	void ( *GL::glFramebufferRenderbuffer )( GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) = NULL;


	void GL::parseVersion( const char* str, unsigned int* major, unsigned int* minor )
	{
		const char* sptr = str;
		char* eptr;

		if( !str ){
			*major = 0;
			*minor = 0;
			return;
		}

		*major = ( unsigned int ) strtol( sptr, &eptr, 10 );
		if( sptr == eptr )
			return;
		while( *eptr && ( *eptr == '.' || *eptr == ' ' ) )
			eptr++;
		sptr = eptr;
		*minor = ( unsigned int ) strtol( sptr, &eptr, 10 );
	}

	void GL::init()
	{
		static bool init = false;
		const GLubyte* str;
		if( init )
			return;
		init = true;

		/* version check */
		str = glGetString( GL_VERSION );
		parseVersion( ( const char* ) str, &_glmajor, &_glminor );
		str = glGetString( GL_SHADING_LANGUAGE_VERSION );
		parseVersion( ( const char* ) str, &_glslmajor, &_glslminor );

		if( _glmajor < 2  || ( _glmajor == 2 && _glminor == 0 ) ) {
			std::cerr << "GL Version too old, at least GL 2.1 is needed" << std::endl;
			std::exit( EXIT_FAILURE );
		}

		if( _glslmajor == 1 && _glslminor < 20 ) {
			std::cerr << "GLSL Version too old, at least GLSL 1.20 is needed" << std::endl;
			std::exit( EXIT_FAILURE );
		}

		/* extensions check */
		if( _glmajor > 2 ) {
			int numext;
			glGetIntegerv( GL_NUM_EXTENSIONS, &numext );
			for( int i = 0; i < numext; i++ ) {
				_extensions.push_back( ( const char* ) glGetStringi( GL_EXTENSIONS, i ) );
			}
		} else {
			String cvtstr( ( const char* ) glGetString( GL_EXTENSIONS ) );
			cvtstr.tokenize( _extensions, ' ' );
		}

		if( existsExtension( "GL_ARB_vertex_array_object" ) || ( _glmajor > 2 || ( _glmajor == 2 && _glminor >= 1 ) ) ) {
			glGenVertexArrays = ( void (*)( GLsizei, GLuint* ) ) getProcAddress( "glGenVertexArrays" );
			glDeleteVertexArrays = ( void (*)( GLsizei, const GLuint* ) ) getProcAddress( "glDeleteVertexArrays" );
			glBindVertexArray = ( void (*)( GLuint ) ) getProcAddress( "glBindVertexArray" );
			glIsVertexArray = ( GLboolean (*)( GLuint ) ) getProcAddress( "glIsVertexArray" );
		} else if( existsExtension( "GL_APPLE_vertex_array_object" ) ) {
			glGenVertexArrays = ( void (*)( GLsizei, GLuint* ) ) getProcAddress( "glGenVertexArraysAPPLE" );
			glDeleteVertexArrays = ( void (*)( GLsizei, const GLuint* ) ) getProcAddress( "glDeleteVertexArraysAPPLE" );
			glBindVertexArray = ( void (*)( GLuint ) ) getProcAddress( "glBindVertexArrayAPPLE" );
			glIsVertexArray = ( GLboolean (*)( GLuint ) ) getProcAddress( "glIsVertexArrayAPPLE" );
		} else {
			std::cerr << "GL vertex array object extension missing" << std::endl;
			std::exit( EXIT_FAILURE );
		}

		if( !existsExtension( "GL_ARB_texture_non_power_of_two" ) && _glmajor <= 2 ) {
			std::cerr << "GL texture non power of two extension missing" << std::endl;
			std::exit( EXIT_FAILURE );
		}

		if( existsExtension( "GL_EXT_framebuffer_object" ) || existsExtension( "GL_ARB_framebuffer_object" ) || _glmajor > 2 ) {
			glBindRenderbuffer = ( void (*)( GLenum target, GLuint renderbuffer) ) getProcAddress( "glBindRenderbuffer" );
			glDeleteRenderbuffers = ( void (*)( GLsizei n, const GLuint *renderbuffers) ) getProcAddress( "glDeleteRenderbuffers" );
			glGenRenderbuffers = ( void (*)( GLsizei n, GLuint *renderbuffers) ) getProcAddress( "glGenRenderbuffers" );
			glRenderbufferStorage = ( void (*)( GLenum target, GLenum internalformat, GLsizei width, GLsizei height) ) getProcAddress( "glRenderbufferStorage" );

			glBindFramebuffer = ( void (*)( GLenum target, GLuint framebuffer ) ) getProcAddress( "glBindFramebuffer" );
			glDeleteFramebuffers = ( void (*)( GLsizei n, const GLuint *framebuffers ) ) getProcAddress( "glDeleteFramebuffers" );
			glGenFramebuffers = ( void (*)( GLsizei n, GLuint *framebuffers ) ) getProcAddress( "glGenFramebuffers" );

			glFramebufferTexture1D = ( void (*)( GLenum target,  GLenum attachment,  GLenum textarget, GLuint texture, GLint level ) ) getProcAddress( "glFramebufferTexture1D" );
			glFramebufferTexture2D = ( void (*)( GLenum target,  GLenum attachment,  GLenum textarget, GLuint texture, GLint level ) ) getProcAddress( "glFramebufferTexture2D" );
			glFramebufferTexture3D = ( void (*)( GLenum target,  GLenum attachment,  GLenum textarget, GLuint texture, GLint level, GLint zoffset ) ) getProcAddress( "glFramebufferTexture3D" );

			glFramebufferRenderbuffer = ( void (*)( GLenum target,  GLenum attachment,  GLenum renderbuffertarget, GLuint renderbuffer) ) getProcAddress( "glFramebufferRenderbuffer" );
		} else {
			std::cerr << "GL framebuffer extension missing" << std::endl;
			std::exit( EXIT_FAILURE );
		}

	}

	bool GL::existsExtension( const String& extname )
	{
		for( std::vector<String>::const_iterator it = _extensions.begin(); it != _extensions.end(); ++it ) {
			if( extname == *it ) {
				return true;
			}
		}
		return false;
	}

	void (* GL::getProcAddress(const char* name ))()
	{
#ifdef APPLE
		return ( void (*)() ) dlsym( RTLD_DEFAULT, name );
#else
		return glXGetProcAddressARB( ( const GLubyte * ) name );
#endif
	}

	void GL::info( std::ostream& out )
	{
		out << "GL-Version: " << _glmajor << "." << _glminor << std::endl;
		out << "GLSL-Version: " << _glslmajor << "." << _glslminor << std::endl;
		out << "GL Extensions: " << std::endl;
		for( std::vector<String>::const_iterator it = _extensions.begin(); it != _extensions.end(); ++it )
			out << "\t" << *it << std::endl;
	}

	void GL::ortho( Matrix4f& mat, float left, float right, float top, float bottom, float near, float far, bool fliph )
	{
		if( fliph ) {
			float tmp = top;
			top = bottom;
			bottom = tmp;
		}
		mat.setZero();

		mat[ 0 ][ 0 ] = 2.0f / ( right - left );
		mat[ 0 ][ 3 ] = - ( right + left ) / ( right - left );

		mat[ 1 ][ 1 ] = 2.0f / ( top - bottom );
		mat[ 1 ][ 3 ] = - ( top + bottom ) / ( top - bottom );

		mat[ 2 ][ 2 ] = -2.0f / ( far - near );
		mat[ 2 ][ 3 ] = - ( far + near ) / ( far - near );

		mat[ 3 ][ 3 ] = 1.0f;
	}

	void GL::orthoTranslation( Matrix4f& mat, float left, float right, float top, float bottom, float transx, float transy, float near, float far, bool fliph )
	{
		if( fliph ) {
			float tmp = top;
			top = bottom;
			bottom = tmp;
		}
		mat.setZero();
		mat[ 0 ][ 0 ]  = 2.0f / ( right - left );
		mat[ 0 ][ 3 ]  = - ( right + left ) / ( right - left );
		mat[ 0 ][ 3 ] += transx * mat[ 0 ][ 0 ];

		mat[ 1 ][ 1 ]  = 2.0f / ( top - bottom );
		mat[ 1 ][ 3 ]  = - ( top + bottom ) / ( top - bottom );
		mat[ 1 ][ 3 ] += transy * mat[ 1 ][ 1 ];

		mat[ 2 ][ 2 ]  = -2.0f / ( far - near );
		mat[ 2 ][ 3 ]  = - ( far + near ) / ( far - near );

		mat[ 3 ][ 3 ]  = 1.0f;
	}

	void GL::frustum( Matrix4f& mat, float left, float right, float top, float bottom, float near, float far, bool fliph )
	{
		if( fliph ) {
			float tmp = top;
			top = bottom;
			bottom = tmp;
		}
		mat.setZero();
		mat[ 0 ][ 0 ] = ( 2.0f * near ) / ( right - left );
		mat[ 0 ][ 2 ] = ( right + left ) / ( right - left );
		mat[ 1 ][ 1 ] = 2.0f * near / ( top - bottom );
		mat[ 1 ][ 2 ] = ( top + bottom ) / ( top - bottom );
		mat[ 2 ][ 2 ] = - ( far + near )  / ( far - near );
		mat[ 2 ][ 3 ] = - ( 2.0f * far * near ) / ( far - near );
		mat[ 3 ][ 2 ] = - 1.0f;
	}

	void GL::perspective( Matrix4f& mat, float fovy, float aspect, float near, float far, bool fliph )
	{
		float range = Math::tan( Math::deg2Rad( fovy / 2.0f ) ) * near;
		float left = -range * aspect;
		float right = range * aspect;
		float bottom = -range;
		float top = range;

		if( fliph ) {
			float tmp = top;
			top = bottom;
			bottom = tmp;
		}

		mat.setZero();
		mat[0][0] = ( 2.0f * near ) / ( right - left );
		mat[1][1] = ( 2.0f * near ) / ( top - bottom );
		mat[2][2] = - ( far + near ) / ( far - near );
		mat[2][3] = - ( 2.0f * far * near) / ( far - near );
		mat[3][2] = - 1.0f;
	}

	void GL::subviewport( Matrix4f& mat, int x, int y, int w, int h, int viewportwidth, int viewportheight )
	{
		mat.setIdentity();
		mat[ 0 ][ 0 ] = ( float ) w / ( float ) viewportwidth;
		mat[ 0 ][ 2 ] = ( 2.0f * ( float ) x + ( float ) w - ( float ) viewportwidth ) / ( float ) viewportwidth;
		mat[ 1 ][ 1 ] = ( float ) h / ( float ) viewportheight;
		mat[ 1 ][ 2 ] = ( -2.0f * ( float ) y - ( float ) h + ( float ) viewportheight ) / ( float ) viewportheight;
	}
}
