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

#include <cvt/gl/progs/GLDrawTexModelProg.h>

//#include <cvt/gl/shader/120/basiclight_120_vert.h>
//#include <cvt/gl/shader/120/basiclight_120_frag.h>
#include <cvt/gl/shader/150/basiclighttex_150_vert.h>
#include <cvt/gl/shader/150/basiclighttex_150_frag.h>

namespace cvt {
	GLDrawTexModelProg::GLDrawTexModelProg()
	{
		try {
			if( GL::isGLSLVersionSupported( 1, 50 ) ) {
				build( _basiclighttex_150_vert_source, _basiclighttex_150_frag_source );
			} else {
//				build( _basiclight_120_vert_source, _basiclight_120_frag_source );
			}
		} catch( GLException e ) {
			std::cout << e.what() << e.log() << std::endl;
		}

		bind();
		_mvploc = uniformLocation( "MVP" );
		_normmloc = uniformLocation( "NORMM" );
		_lightposloc = uniformLocation( "LIGHTPOS" );
		_texloc = uniformLocation( "Tex" );
		glUniform1i( _texloc, 0 );
		unbind();
	}

	GLDrawTexModelProg::~GLDrawTexModelProg()
	{

	}

	void GLDrawTexModelProg::setProjection( const Matrix4f& projection, const Matrix4f& modelview )
	{
		Matrix4f mvp( projection );
		mvp *= modelview;
		glUniformMatrix4fv( _mvploc, 1, true , ( const GLfloat* ) mvp.ptr() );
		Matrix3f normm( modelview );
		normm.transposeSelf();
		normm.inverseSelf();
//		std::cout << normm << std::endl;
		glUniformMatrix3fv( _normmloc, 1, true , ( const GLfloat* ) normm.ptr() );
	}

	void GLDrawTexModelProg::setLightPosition( const Vector3f& position )
	{
		glUniform3fv( _lightposloc, 1, ( const GLfloat* ) position.ptr() );
	}

}
