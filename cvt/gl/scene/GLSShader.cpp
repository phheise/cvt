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
#include <cvt/gl/scene/GLSShader.h>
#include <cvt/gl/scene/GLSMaterial.h>
#include <cvt/gl/scene/GLSShaderProgram.h>
#include <cvt/gl/scene/GLScene.h>

namespace cvt {

	GLSShader::GLSShader( const GLScene& scene ) : _scene( scene ), _mat( NULL )
	{
        _defaultmat = new GLSMaterial( "default" );
        generatePrograms();
	}

    GLSShader::~GLSShader()
    {
        delete _defaultmat;
        for( int i = 0; i < 16; i++ )
            delete _progs[ i ];
    }

	void GLSShader::setMaterial( const GLSMaterial* mat )
	{
        if( mat )
		    _mat = mat;
        else
            _mat = _defaultmat;
	}

	void GLSShader::bind()
	{
		_progs[ _mat->flags() ]->bind();

		_progs[ _mat->flags() ]->setMaterial( *_mat );

        for( size_t i = 0; i < _scene.lightSize(); i++ )
            _progs[ _mat->flags() ]->setLight( i, _scene.light( i ) );

        _progs[ _mat->flags() ]->setNumLight( ( GLint ) _scene.lightSize() );
	}

	void GLSShader::unbind()
	{
		_progs[ _mat->flags() ]->unbind();
	}

	void GLSShader::setTransformation( const Matrix4f& mat, bool setuniform )
	{
		_transformation = mat;
        if( setuniform )
		    _progs[ _mat->flags() ]->setProjection( _proj, _transformation );
	}

    void GLSShader::generatePrograms()
    {
        for( size_t flags = 0; flags < GLSMATERIAL_MAX; flags++ ) {
            try {
                _progs[ flags ] = new GLSShaderProgram( GLSMaterialFlags( flags ) );
            } catch( GLException& e ) {
                std::cout << e.log() << std::endl;
            }
        }
    }

}
