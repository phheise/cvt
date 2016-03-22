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

#ifndef CVT_SCENE_H
#define CVT_SCENE_H

#include <cvt/geom/scene/SceneMesh.h>
#include <cvt/geom/scene/SceneMaterial.h>
#include <cvt/geom/scene/SceneTexture.h>

namespace cvt {

	class SceneLoader;

	class Scene {
		public:
			Scene();
			~Scene();

			void					clear();

			SceneGeometry*			geometry( size_t index );
			const SceneGeometry*	geometry( size_t index ) const;
			size_t					geometrySize() const;
			void					addGeometry( SceneGeometry* geometry );

			SceneMaterial*			material( size_t index );
			const SceneMaterial*	material( size_t index ) const;
			size_t					materialSize() const;
			void					addMaterial( SceneMaterial* geometry );
			const SceneMaterial*	material( const String& name ) const;

			SceneTexture*			texture( size_t index );
			const SceneTexture*		texture( size_t index ) const;
			size_t					textureSize() const;
			void					addTexture( SceneTexture* tex );
			const SceneTexture*		texture( const String& name ) const;


			void					load( const String& path, SceneLoader* loader = NULL );

		private:
			std::vector<SceneGeometry*> _geometries;
			std::vector<SceneMaterial*> _materials;
			std::vector<SceneTexture*>	_textures;
	};

	inline Scene::Scene()
	{
	}

	inline Scene::~Scene()
	{
		clear();
	}

	inline void Scene::clear()
	{
		for( std::vector<SceneGeometry*>::iterator it = _geometries.begin(); it != _geometries.end(); ++it ) {
			delete *it;
		}
		for( std::vector<SceneMaterial*>::iterator it = _materials.begin(); it != _materials.end(); ++it ) {
			delete *it;
		}
		for( std::vector<SceneTexture*>::iterator it = _textures.begin(); it != _textures.end(); ++it ) {
			delete *it;
		}

		_geometries.clear();
		_materials.clear();
		_textures.clear();
	}

	inline SceneGeometry* Scene::geometry( size_t index )
	{
		return _geometries[ index ];
	}

	inline const SceneGeometry* Scene::geometry( size_t index ) const
	{
		return _geometries[ index ];
	}


	inline size_t Scene::geometrySize() const
	{
		return _geometries.size();
	}

	inline void Scene::addGeometry( SceneGeometry* geometry )
	{
		_geometries.push_back( geometry );
	}

	inline SceneMaterial* Scene::material( size_t index )
	{
		return _materials[ index ];
	}

	inline const SceneMaterial* Scene::material( size_t index ) const
	{
		return _materials[ index ];
	}

	inline size_t Scene::materialSize() const
	{
		return _materials.size();
	}

	inline void Scene::addMaterial( SceneMaterial* mat )
	{
		_materials.push_back( mat );
	}

	inline const SceneMaterial* Scene::material( const String& name ) const
	{
		for( size_t i = 0; i < _materials.size(); i++ ) {
			if( _materials[ i ]->name() == name ) {
				return _materials[ i ];
			}
		}
		return NULL;
	}

	inline SceneTexture* Scene::texture( size_t index )
	{
		return _textures[ index ];
	}

	inline const SceneTexture* Scene::texture( size_t index ) const
	{
		return _textures[ index ];
	}

	inline size_t Scene::textureSize() const
	{
		return _textures.size();
	}

	inline void Scene::addTexture( SceneTexture* tex )
	{
		_textures.push_back( tex );
	}


	inline const SceneTexture* Scene::texture( const String& name ) const
	{
		for( size_t i = 0; i < _textures.size(); i++ ) {
			if( _textures[ i ]->name() == name ) {
				return _textures[ i ];
			}
		}
		return NULL;
	}

	inline std::ostream& operator<<( std::ostream& out, const Scene& s )
	{
		out << "Scene:";
		out << "\n\tGeometry: " << s.geometrySize() << "\n";
		for( size_t i = 0; i < s.geometrySize(); i++ )
			out << *s.geometry( i );
		out << "\n\tMaterials: " << s.materialSize() << "\n";
		for( size_t i = 0; i < s.materialSize(); i++ )
			out << *s.material( i );
		return out;
	}
}

#endif
