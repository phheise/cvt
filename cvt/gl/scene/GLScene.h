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

#ifndef CVT_GLSCENE_H
#define CVT_GLSCENE_H

#include <cvt/geom/scene/Scene.h>

#include <cvt/gl/scene/GLSCamera.h>
#include <cvt/gl/scene/GLSLight.h>
#include <cvt/gl/scene/GLSRenderableGroup.h>
#include <cvt/gl/scene/GLSShader.h>

#include <cvt/util/Time.h>

#include <cvt/gl/progs/GLDrawImageProg.h>

namespace cvt {
	enum GLSceneDrawFeatures {
		GLS_SHOW_FPS	 = ( 1 << 0 ),
		GLS_SHADOWS		 = ( 1 << 1 ),
		GLS_SSAO		 = ( 1 << 2 ),
		GLS_DEBUG		 = ( 1 << 3 )
	};

	CVT_ENUM_TO_FLAGS( GLSceneDrawFeatures, GLSceneDrawFlags )

    class GLSMaterial;
    class GLSTexture;
    class GLMesh;

	class GLScene
	{
		public:
			GLScene();
			GLScene( const Scene& scene );
			~GLScene();


			void                    draw( size_t cam );
			GLSceneDrawFlags&       drawMode() { return _drawFlags; }
			const GLSceneDrawFlags& drawMode() const { return _drawFlags; }

            const GLSTexture*       texture( const String& path ) const;
            const GLSMaterial*      material( const String& name ) const;

			const Time&             time() const { return _time; }

			GLSCamera&              camera( size_t i ) { return _cams[ i ]; }
            GLSLight&               light( size_t i ) { return _lights[ i ];}

            const GLSCamera&        camera( size_t i ) const { return _cams[ i ]; }
            const GLSLight&         light( size_t i ) const { return _lights[ i ];}


            size_t                  cameraSize() const { return _cams.size(); }
            size_t                  lightSize() const { return _lights.size(); }

		private:
            const GLSMaterial* addSceneMaterial( const SceneMaterial* mat, const Scene& scene );
            void               addSceneTexture( const SceneTexture* mat );

			std::vector<GLSCamera>		_cams;
			std::vector<GLSLight>		_lights;
			std::vector<GLSMaterial*>	_materials;
			std::vector<GLSTexture*>	_textures;
			std::vector<GLMesh*>		_meshes;
			GLSRenderableGroup*			_renderables;
			GLSShader					_shader;
			GLDrawImageProg				_drawimgp;

			GLSceneDrawFlags			_drawFlags;
			float						_fps;
			Time						_time;
			GLTexture					_texture;
	};

	inline GLScene::GLScene() : _shader( *this )
	{
		_renderables = new GLSRenderableGroup();

		_texture.alloc( GL_DEPTH_COMPONENT, 640, 480, GL_DEPTH_COMPONENT, GL_FLOAT );
	}

 
}

#endif
