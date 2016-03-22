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

#include <cvt/gl/scene/GLSShaderProgram.h>
#include <cvt/gl/scene/GLSMaterial.h>

#include <cvt/gl/scene/shader/gls_default_vert.h>
#include <cvt/gl/scene/shader/gls_default_frag.h>

namespace cvt {

#define MAX_LIGHTS 8

    GLSShaderProgram::GLSShaderProgram( const GLSMaterialFlags& flags )
    {
        String ambientmap("#define GLSMATERIAL_AMBIENTMAP 1\n");
        String diffusemap("#define GLSMATERIAL_DIFFUSEMAP 1\n");
        String specularmap("#define GLSMATERIAL_SPECULARMAP 1\n");
        String normalmap("#define GLSMATERIAL_NORMALMAP 1\n");

        String defines = "#version 150\n";

        defines.sprintfConcat("#define MAX_LIGHTS %i\n", MAX_LIGHTS );

        if( flags & GLSMATERIAL_AMBIENTMAP )
            defines += ambientmap;
        if( flags & GLSMATERIAL_DIFFUSEMAP )
            defines += diffusemap;
        if( flags & GLSMATERIAL_SPECULARMAP )
            defines += specularmap;
        if( flags & GLSMATERIAL_NORMALMAP )
            defines += normalmap;

        const char* vertglsl[] = { defines.c_str(), _gls_default_vert_source };
        const char* fragglsl[] = { defines.c_str(), _gls_default_frag_source };

        build( vertglsl, 2, fragglsl, 2 );

        _mvloc          = uniformLocation( "MV" );
        _mvploc         = uniformLocation( "MVP" );
        _normmloc       = uniformLocation( "NORMM" );

        /* Material locations */
        _matkaloc       = uniformLocation( "material.Ka" );
        _matkdloc       = uniformLocation( "material.Kd" );
        _matksloc       = uniformLocation( "material.Ks" );
        _matshinloc     = uniformLocation( "material.shininess" );
        _matkatexloc    = uniformLocation( "material.ambientmap" );
        _matkdtexloc    = uniformLocation( "material.diffusemap" );
        _matkstexloc    = uniformLocation( "material.specularmap" );
        _matnormtexloc  = uniformLocation( "material.normalmap" );

        /* Light locations*/
        _lightnumloc    = uniformLocation( "numlights" );

        for( int i = 0; i < 8; i++ ) {
            String str;
            str.sprintf( "light[%d].position", i );
            _lightloc[ i ].positionloc = uniformLocation( str.c_str() );
            str.sprintf( "light[%d].La", i );
            _lightloc[ i ].laloc = uniformLocation( str.c_str() );
            str.sprintf( "light[%d].Ld", i );
            _lightloc[ i ].ldloc = uniformLocation( str.c_str() );
            str.sprintf( "light[%d].Ls", i );
            _lightloc[ i ].lsloc = uniformLocation( str.c_str() );
            str.sprintf( "light[%d].attenuation", i );
            _lightloc[ i ].attloc = uniformLocation( str.c_str() );
        }
    }

    GLSShaderProgram::~GLSShaderProgram()
    {
    }

	void GLSShaderProgram::setProjection( const Matrix4f& projection, const Matrix4f& modelview )
	{
		Matrix4f mvp( projection );

        // MVP
		mvp *= modelview;
		glUniformMatrix4fv( _mvploc, 1, true , ( const GLfloat* ) mvp.ptr() );

        // MV
		glUniformMatrix4fv( _mvloc, 1, true , ( const GLfloat* ) modelview.ptr() );

        // NORMM
		Matrix3f normm( modelview );
		normm.transposeSelf();
		normm.inverseSelf();
		glUniformMatrix3fv( _normmloc, 1, true , ( const GLfloat* ) normm.ptr() );
	}

    void GLSShaderProgram::setMaterial( const GLSMaterial& material )
    {
        glUniform4fv( _matkaloc, 1, material.ambientColor().ptr() );
        glUniform4fv( _matkdloc, 1, material.diffuseColor().ptr() );
        glUniform4fv( _matksloc, 1, material.specularColor().ptr() );
        glUniform1f( _matshinloc, material.shininess() );

        int texunit = 0;
        if( material.flags() & GLSMATERIAL_AMBIENTMAP ) {
            glActiveTexture( GL_TEXTURE0 + texunit );
            material.ambientMap()->bind();
		    glUniform1i( _matkatexloc, texunit );
            texunit++;
        }

        if( material.flags() & GLSMATERIAL_DIFFUSEMAP ) {
            glActiveTexture( GL_TEXTURE0 + texunit );
            material.diffuseMap()->bind();
		    glUniform1i( _matkdtexloc, texunit );
            texunit++;
        }

        if( material.flags() & GLSMATERIAL_SPECULARMAP ) {
            glActiveTexture( GL_TEXTURE0 + texunit );
            material.specularMap()->bind();
		    glUniform1i( _matkstexloc, texunit );
            texunit++;
        }

        if( material.flags() & GLSMATERIAL_NORMALMAP ) {
            glActiveTexture( GL_TEXTURE0 + texunit );
            material.normalMap()->bind();
		    glUniform1i( _matkstexloc, texunit );
            texunit++;
        }

        glActiveTexture( GL_TEXTURE0 );
    }

	void GLSShaderProgram::setNumLight( GLint size )
    {
        glUniform1i( _lightnumloc, size );
    }

    void GLSShaderProgram::setLight( GLint index, const GLSLight& light )
    {
        if( index >= MAX_LIGHTS ) return;

        glUniform4fv( _lightloc[ index ].positionloc, 1, light.transformation().col( 3 ).ptr() );
        glUniform4fv( _lightloc[ index ].laloc, 1, light.ambientColor().ptr() );
        glUniform4fv( _lightloc[ index ].ldloc, 1, light.diffuseColor().ptr() );
        glUniform4fv( _lightloc[ index ].lsloc, 1, light.specularColor().ptr() );
    }

}
