/*
   The MIT License (MIT)

   Copyright (c) 2011 - 2013, Philipp Heise and Sebastian Klose
   Copyright (c) 2016, BMW Car IT GmbH, Philipp Heise (philipp.heise@bmw.de)

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
#include <cvt/gl/scene/GLScene.h>
#include <cvt/gl/scene/GLSBaseModel.h>
#include <cvt/gl/scene/GLSVisitor.h>
#include <cvt/gl/scene/GLSMaterial.h>
#include <cvt/gl/scene/GLSTexture.h>

#include <cvt/gl/GLMesh.h>
#include <cvt/gl/GLFBO.h>
#include <cvt/gl/GLRBO.h>


namespace cvt {
    const GLSTexture* GLScene::texture( const String& name ) const
    {
        for( size_t i = 0; i < _textures.size(); i++ ) {
            if( _textures[ i ]->name() == name ) {
                return _textures[ i ];
            }
        }
        return NULL;
    }

    const GLSMaterial* GLScene::material( const String& name ) const
    {
        for( size_t i = 0; i < _materials.size(); i++ ) {
            if( _materials[ i ]->name() == name ) {
                return _materials[ i ];
            }
        }
        return NULL;
    }


    const GLSMaterial* GLScene::addSceneMaterial( const SceneMaterial* mat, const Scene& scene )
    {
        if( !mat )
            return NULL;

        if( mat->flags() & SCENEMATERIAL_AMBIENTMAP && !texture( mat->ambientMap() ) ) {
            addSceneTexture( scene.texture( mat->ambientMap() ) );
        }

        if( mat->flags() & SCENEMATERIAL_DIFFUSEMAP && !texture( mat->diffuseMap() ) ) {
            addSceneTexture( scene.texture( mat->diffuseMap() ) );
        }

        if( mat->flags() & SCENEMATERIAL_SPECULARMAP && !texture( mat->specularMap() ) ) {
            addSceneTexture( scene.texture( mat->specularMap() ) );
        }

        if( mat->flags() & SCENEMATERIAL_NORMALMAP && !texture( mat->normalMap() ) ) {
            addSceneTexture( scene.texture( mat->normalMap() ) );
        }

        _materials.push_back( new GLSMaterial( *mat, *this ) );
        return _materials.back();
    }

    void GLScene::addSceneTexture( const SceneTexture* tex )
    {
        if( !tex )
            return;

        _textures.push_back( new GLSTexture( *tex ) );
    }


    GLScene::GLScene( const Scene& scene ) : _shader( *this )
    {
        _renderables = new GLSRenderableGroup();


        for( size_t i = 0; i < scene.materialSize(); i++ ) {
            addSceneMaterial( scene.material( i ), scene );
        }

        for( size_t i = 0; i < scene.geometrySize(); i++ ) {
            if( scene.geometry( i )->type() == SCENEGEOMETRY_MESH ) {
                _meshes.push_back( new GLMesh( *( ( SceneMesh* ) scene.geometry( i ) ) ) );
                _renderables->add( new GLSBaseModel( _meshes.back(), material( scene.geometry( i )->material() ) ) );
            }
        }

        if( cameraSize() == 0 ) {
            _cams.push_back( GLSCamera( 80.0f, 1.333f, 0.01f, 1000.0f ) );
            Matrix4f t;
            t.setIdentity();
            _cams.back().setTransformation( t );
        }

        if( lightSize() == 0 ) {
            _lights.push_back( GLSLight() );
            Matrix4f t;
            t.setIdentity();
            t[ 0 ][ 3 ] = 80.0f;
            t[ 1 ][ 3 ] = 1.0f;
            t[ 2 ][ 3 ] = 20.0f;
            _lights.back().setTransformation( t );
            _lights.back().setAmbientColor( Color::WHITE );
        }

    }

    GLScene::~GLScene()
    {
        // TODO: call delete on vector elements
        delete _renderables;
    }

    void GLScene::draw( size_t cam )
    {
        _shader.setCamera( _cams[ cam ] );
        GLSRenderVisitor rvisitor( _shader );

        glEnable( GL_DEPTH_TEST );
        //glAlphaFunc(GL_GREATER, 0.5);
        //glEnable( GL_ALPHA_TEST );
        glEnable( GL_BLEND );
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        glClear( ( ( _drawFlags & GLS_CLEAR_COLOR ) ? GL_COLOR_BUFFER_BIT : 0 )
                | ( ( _drawFlags & GLS_CLEAR_COLOR ) ? GL_DEPTH_BUFFER_BIT : 0 ) );
        _renderables->accept( rvisitor );

        glDisable( GL_BLEND );
        glDisable( GL_DEPTH_TEST );
        //glDisable( GL_ALPHA_TEST );
        //glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    }

}

