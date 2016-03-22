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
			_cams.push_back( GLSCamera( 80.0f, 1.333f, 0.1f, 150.0f ) );
			Matrix4f t;
			t.setIdentity();
			t *= 10.0f;
			t[ 0 ][ 3 ] = 0.0f;
			t[ 1 ][ 3 ] = 20.0f;
			t[ 2 ][ 3 ] = 180.0f;
			t[ 3 ][ 3 ] = 1.0f;
			_cams.back().setTransformation( t );
		}

        if( lightSize() == 0 ) {
            _lights.push_back( GLSLight() );
			Matrix4f t;
            t.setIdentity();
			t[ 0 ][ 3 ] = 80.0f;
			t[ 1 ][ 3 ] = 400.0f;
			t[ 2 ][ 3 ] = -20.0f;
            _lights.back().setTransformation( t );
        }

		_texture.alloc( GL_DEPTH_COMPONENT, 640, 480, GL_DEPTH_COMPONENT, GL_FLOAT );
	}

	GLScene::~GLScene()
	{
        // TODO: call delete on vector elements
		delete _renderables;
	}

	void GLScene::draw( size_t cam )
	{
		GLFBO fbo( 640, 480 );
//		GLRBO rbo( GL_RGBA, 640, 480 );
//		GLTexture tex;
//		tex.alloc( GL_RGBA, 640, 480, GL_RGBA, GL_UNSIGNED_BYTE );

		_shader.setCamera( _cams[ cam ] );
		GLSRenderVisitor rvisitor( _shader );

		fbo.bind();
//		fbo.attach( GL_COLOR_ATTACHMENT0, tex );
		fbo.attach( GL_DEPTH_ATTACHMENT, _texture );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glEnable( GL_DEPTH_TEST );
		_renderables->accept( rvisitor );
		glDisable( GL_DEPTH_TEST );
		fbo.unbind();

		glEnable( GL_DEPTH_TEST );
		glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		_renderables->accept( rvisitor );
		glDisable( GL_DEPTH_TEST );
		glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);

		Matrix4f proj;
		GL::orthoTranslation( proj, 0, ( float ) 640, 0, ( float ) 480, ( float ) 0, ( float ) 0, -100.0f, 100.0f, true );
		_drawimgp.bind();
		_drawimgp.setProjection( proj );
		_drawimgp.setAlpha( 1.0f );
//		GLTexture* _tex = _materials[ 0 ]->diffuseMap();
//		_drawimgp.drawImage( 0, 0, 320, 240, *_tex );
		_drawimgp.drawImage( 0, 0, 320, 240, _texture );
		_drawimgp.unbind();

	}

}


