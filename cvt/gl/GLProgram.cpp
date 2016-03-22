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

#include <cvt/gl/GLProgram.h>
#include <cvt/gfx/Color.h>

namespace cvt {
	GLProgram::GLProgram()
	{
		program = glCreateProgram();
		bindAttribLocation( "in_Vertex", GLSL_VERTEX_IDX );
		bindAttribLocation( "in_Color", GLSL_COLOR_IDX );
		bindAttribLocation( "in_Normal", GLSL_NORMAL_IDX );
		bindAttribLocation( "in_TexCoord", GLSL_TEXCOORD_IDX );
	}

	GLProgram::~GLProgram()
	{
		glDeleteProgram( program );
	}

	void GLProgram::bind()
	{
		glUseProgram( program );
	}

	void GLProgram::unbind()
	{
		glUseProgram( 0 );
	}


	void GLProgram::build( const char* vertsrc, const char* fragsrc, const char* geomsrc )
    {
        if( geomsrc )
            build( &vertsrc, 1, &fragsrc, 1, &geomsrc, 1 );
        else
            build( &vertsrc, 1, &fragsrc, 1, NULL, 0 );
    }

	void GLProgram::build( const char** vertsrc, int vertcount,
                           const char** fragsrc, int fragcount,
                           const char** geomsrc, int geomcount )
	{
		GLuint vs = 0, fs = 0, gs = 0;
		GLint status = GL_TRUE;
		GLint loglen;
		std::string logbuf;

		if( !vertsrc || !fragsrc )
			throw GLException( "Verter/Fragment shader source missing!\n" );

		vs = glCreateShader( GL_VERTEX_SHADER );
		if( !vs )
			throw GLException("Vertex-Shader creation failed!\n");

		glShaderSource( vs, vertcount, vertsrc, NULL );
		glCompileShader( vs );
		glGetShaderiv( vs, GL_COMPILE_STATUS, &status );
		if( status == GL_FALSE ) {
			glGetShaderiv( vs, GL_INFO_LOG_LENGTH, &loglen );
			logbuf.resize( loglen );
			glGetShaderInfoLog( vs, loglen, &loglen, &logbuf[ 0 ]  );
			glDeleteShader(vs);
			throw GLException("Vertex-Shader compilation error\n", logbuf );
		}

		fs = glCreateShader( GL_FRAGMENT_SHADER );
		if( !fs ) {
			glDeleteShader( vs );
			throw GLException("Fragment-Shader creation failed!\n");
		}

		glShaderSource( fs, fragcount, fragsrc, NULL );
		glCompileShader( fs );
		glGetShaderiv( fs, GL_COMPILE_STATUS, &status );
		if( status == GL_FALSE ) {
			glGetShaderiv( fs, GL_INFO_LOG_LENGTH, &loglen );
			logbuf.resize( loglen );
			glGetShaderInfoLog( fs, loglen, &loglen, &logbuf[ 0 ] );
			glDeleteShader(fs);
			glDeleteShader(vs);
			throw GLException("Fragment-Shader compilation error\n", logbuf );
		}

		if( geomsrc ) {
			gs = glCreateShader( GL_GEOMETRY_SHADER );
			if( !gs ) {
				glDeleteShader( vs );
				throw GLException("Geometry-Shader creation failed!\n");
			}

			glShaderSource( gs, geomcount, geomsrc, NULL );
			glCompileShader( gs );
			glGetShaderiv( gs, GL_COMPILE_STATUS, &status );
			if( status == GL_FALSE ) {
				glGetShaderiv( gs, GL_INFO_LOG_LENGTH, &loglen );
				logbuf.resize( loglen );
				glGetShaderInfoLog( gs, loglen, &loglen, &logbuf[ 0 ] );
				glDeleteShader( fs );
				glDeleteShader( vs );
				glDeleteShader( gs );
				throw GLException("Geometry-Shader compilation error\n", logbuf );
			}
		}

		glAttachShader( program, fs );
		glAttachShader( program, vs );

		if( gs )
			glAttachShader( program, gs );

		/* shaders are now referenced by programm, delete them*/
		glDeleteShader( vs );
		glDeleteShader( fs );
		if( gs )
			glDeleteShader( gs );

		glLinkProgram( program );
		glGetProgramiv( program, GL_LINK_STATUS, &status );
		if(status == GL_FALSE) {
			glGetProgramiv( program, GL_INFO_LOG_LENGTH, &loglen );
			logbuf.resize( loglen );
			glGetProgramInfoLog( program, loglen, &loglen, &logbuf[ 0 ] );
			throw GLException("Shader link error\n", logbuf );
		}
	}


	void GLProgram::bindAttribLocation( const char *name, unsigned int location )
	{
		glBindAttribLocation( program, ( GLuint ) location, name );
	}


	unsigned int GLProgram::getAttribLocation( const char* name ) const
	{
		return glGetAttribLocation( program, name );
	}

	/*void GLProgram::bindFragDataLocation( const char *name, size_t location )
	  {
	  glBindFragDataLocation( program, ( GLuint ) location, name );
	  }*/

	GLint GLProgram::uniformLocation( const char* name ) const
	{
		return glGetUniformLocation( program, name);
	}

	void GLProgram::setArg( const char* name, int i )
	{
		GLint loc = glGetUniformLocation( program, name);
		glUniform1i( loc, i );
	}

	void GLProgram::setArg( const char* name, float f )
	{
		GLint loc = glGetUniformLocation( program, name);
		glUniform1f( loc, f );
	}

	void GLProgram::setArg( const char* name, float f1, float f2 )
	{
		GLint loc = glGetUniformLocation( program, name);
		glUniform2f( loc, f1, f2 );
	}

	void GLProgram::setArg( const char* name, const Color & c )
	{
		GLint loc = glGetUniformLocation( program, name);
		glUniform4fv( loc, 1, ( GLfloat* )c.data() );
	}

	void GLProgram::setArg( const char* name, const Vector3f & c )
	{
		GLint loc = glGetUniformLocation( program, name);
		glUniform3fv( loc, 1, ( GLfloat* )c.ptr() );
	}

	void GLProgram::setArg( const char* name, const Vector4f & c )
	{
		GLint loc = glGetUniformLocation( program, name);
		glUniform4fv( loc, 1, ( GLfloat* )c.ptr() );
	}

	void GLProgram::setArg( const char* name, const Matrix3f & m )
	{
		GLint loc = glGetUniformLocation( program, name);
		glUniformMatrix3fv( loc, 1, true , ( const GLfloat* ) m.ptr() );
	}

	void GLProgram::setArg( const char* name, const Matrix4f & m )
	{
		GLint loc = glGetUniformLocation( program, name);
		glUniformMatrix4fv( loc, 1, true, ( const GLfloat* ) m.ptr() );
	}
}
